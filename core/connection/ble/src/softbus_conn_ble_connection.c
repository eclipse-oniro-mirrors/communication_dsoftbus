/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_conn_ble_connection.h"

#include <securec.h>

#include "bus_center_manager.h"
#include "softbus_adapter_mem.h"
#include "softbus_conn_ble_client.h"
#include "softbus_conn_ble_manager.h"
#include "softbus_conn_ble_server.h"
#include "softbus_conn_ble_trans.h"
#include "softbus_conn_common.h"
#include "softbus_datahead_transform.h"
#include "softbus_json_utils.h"
#include "softbus_log.h"
#include "ble_protocol_interface_factory.h"

// basic info json key definition
#define BASIC_INFO_KEY_DEVID   "devid"
#define BASIC_INFO_KEY_ROLE    "type"
#define BASIC_INFO_KEY_DEVTYPE "devtype"
#define BASIC_INFO_KEY_FEATURE "FEATURE_SUPPORT"

enum ConnectionLoopMsgType {
    MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT = 100,
    MSG_CONNECTION_EXCHANGE_BASIC_INFO_TIMEOUT,
    MSG_CONNECTION_WAIT_NEGOTIATION_CLOSING_TIMEOUT,
    MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT,
};

enum BleServerState {
    BLE_SERVER_STATE_STARTING,
    BLE_SERVER_STATE_STARTED,
    BLE_SERVER_STATE_STOPPING,
    BLE_SERVER_STATE_STOPPED,
};

// As bluetooth may be change quickly and ble server start and stop is async,
// we should coordinate and keep this finally state consistent
typedef struct {
    SoftBusMutex lock;
    enum BleServerState expect;
    enum BleServerState actual;
    int32_t status[BLE_PROTOCOL_MAX];
} BleServerCoordination;

static void BleConnectionMsgHandler(SoftBusMessage *msg);
static int BleCompareConnectionLooperEventFunc(const SoftBusMessage *msg, void *args);

static ConnBleConnectionEventListener g_connectionListener;
static SoftBusHandlerWrapper g_bleConnectionAsyncHandler = {
    .handler = {
        .name = (char *)"BleConnectionAsyncHandler",
        .HandleMessage = BleConnectionMsgHandler,
        // assign when initiation
        .looper = NULL,
    },
    .eventCompareFunc = BleCompareConnectionLooperEventFunc,
};
static BleServerCoordination g_serverCoordination = {
    .actual = BLE_SERVER_STATE_STOPPED,
    .expect = BLE_SERVER_STATE_STOPPED,
};
// compatible with old devices, old device not support remote disconnect
static const ConnBleFeatureBitSet g_featureBitSet = (1 << BLE_FEATURE_SUPPORT_REMOTE_DISCONNECT);

ConnBleConnection *ConnBleCreateConnection(
    const char *addr, BleProtocolType protocol, ConnSideType side, int32_t underlayerHandle, bool fastestConnectEnable)
{
    CONN_CHECK_AND_RETURN_RET_LOG(addr != NULL, NULL, "invalid parameter: ble addr is NULL");

    ConnBleConnection *connection = SoftBusCalloc(sizeof(ConnBleConnection));
    CONN_CHECK_AND_RETURN_RET_LOG(connection != NULL, NULL, "calloc ble connection failed");
    ListInit(&connection->node);
    // the final connectionId value is allocate on saving global
    connection->connectionId = 0;
    connection->protocol = protocol;
    connection->side = side;
    connection->fastestConnectEnable = fastestConnectEnable;
    if (strcpy_s(connection->addr, BT_MAC_LEN, addr) != EOK) {
        CLOGE("copy address failed");
        SoftBusFree(connection);
        return NULL;
    }
    connection->sequence = 0;

    connection->buffer.seq = 0;
    connection->buffer.total = 0;
    ListInit(&connection->buffer.packets);

    if (SoftBusMutexInit(&connection->lock, NULL) != SOFTBUS_OK) {
        CLOGE("init lock failed");
        SoftBusFree(connection);
        return NULL;
    }
    connection->state =
        (side == CONN_SIDE_CLIENT ? BLE_CONNECTION_STATE_CONNECTING : BLE_CONNECTION_STATE_EXCHANGING_BASIC_INFO);
    connection->underlayerHandle = underlayerHandle;
    // udid field will be assigned after basic info exchanged
    // the final MTU value will be signed on mtu exchange stage
    connection->mtu = 0;
    // ble connection need exchange connection reference even if establish first time, so the init value is 0
    connection->connectionRc = 0;
    connection->objectRc = 1;

    connection->retrySearchServiceCnt = 0;
    return connection;
}

void ConnBleFreeConnection(ConnBleConnection *connection)
{
    SoftBusMutexDestroy(&connection->lock);
    ConnBlePacket *it = NULL;
    ConnBlePacket *next = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(it, next, &connection->buffer.packets, ConnBlePacket, node) {
        ListDelete(&it->node);
        SoftBusFree(it->data);
        SoftBusFree(it);
    }
    SoftBusFree(connection);
}

int32_t ConnBleStartServer(void)
{
    CONN_CHECK_AND_RETURN_RET_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        "ATTENTION UNEXPECTED EXCEPTION: ble start server failed, try to lock failed");
    g_serverCoordination.expect = BLE_SERVER_STATE_STARTED;
    enum BleServerState actual = g_serverCoordination.actual;
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
    if (actual == BLE_SERVER_STATE_STARTING || actual == BLE_SERVER_STATE_STARTED) {
        return SOFTBUS_OK;
    }
    const BleUnifyInterface *interface;
    for (int i = BLE_GATT; i < BLE_PROTOCOL_MAX; i++) {
        interface = ConnBleGetUnifyInterface(i);
        if (interface == NULL) {
            continue;
        }
        g_serverCoordination.status[i] = interface->bleServerStartService();
    }
    for (int i = BLE_GATT; i < BLE_PROTOCOL_MAX; i++) {
        if (g_serverCoordination.status[i] != SOFTBUS_OK) {
            ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT, 0, 0, NULL,
                RETRY_SERVER_STATE_CONSISTENT_MILLIS);
            return SOFTBUS_OK;
        }
    }
    CONN_CHECK_AND_RETURN_RET_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        "ATTENTION UNEXPECTED EXCEPTION: ble start server failed, try to lock failed");
    g_serverCoordination.actual = BLE_SERVER_STATE_STARTING;
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
    return SOFTBUS_OK;
}

int32_t ConnBleStopServer(void)
{
    CONN_CHECK_AND_RETURN_RET_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        "ATTENTION UNEXPECTED EXCEPTION: ble stop server failed, try to lock failed");
    g_serverCoordination.expect = BLE_SERVER_STATE_STOPPED;
    enum BleServerState actual = g_serverCoordination.actual;
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
    if (actual == BLE_SERVER_STATE_STOPPING || actual == BLE_SERVER_STATE_STOPPED) {
        return SOFTBUS_OK;
    }
    const BleUnifyInterface *interface;
    for (int i = BLE_GATT; i < BLE_PROTOCOL_MAX; i++) {
        interface = ConnBleGetUnifyInterface(i);
        if (interface == NULL) {
            continue;
        }
        g_serverCoordination.status[i] = interface->bleServerStopService();
    }
    for (int i = BLE_GATT; i < BLE_PROTOCOL_MAX; i++) {
        if (g_serverCoordination.status[i] != SOFTBUS_OK) {
            ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT, 0, 0, NULL,
                RETRY_SERVER_STATE_CONSISTENT_MILLIS);
            return SOFTBUS_OK;
        }
    }
    CONN_CHECK_AND_RETURN_RET_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        "ATTENTION UNEXPECTED EXCEPTION: ble close server failed, try to lock failed");
    g_serverCoordination.actual = BLE_SERVER_STATE_STOPPING;
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
    return SOFTBUS_OK;
}

int32_t ConnBleConnect(ConnBleConnection *connection)
{
    CONN_CHECK_AND_RETURN_RET_LOG(connection != NULL, SOFTBUS_INVALID_PARAM,
        "ble connection connect failed, invalid param, connection is null");
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    CONN_CHECK_AND_RETURN_RET_LOG(interface != NULL, SOFTBUS_ERR,
        "ble connection connect failed, protocol not support");
    return interface->bleClientConnect(connection);
}

static bool ShouldGrace(enum ConnBleDisconnectReason reason)
{
    switch (reason) {
        case BLE_DISCONNECT_REASON_CONNECT_TIMEOUT:
        case BLE_DISCONNECT_REASON_INTERNAL_ERROR:
        case BLE_DISCONNECT_REASON_POST_BYTES_FAILED:
        case BLE_DISCONNECT_REASON_RESET:
            return false;
        default:
            return true;
    }
}

static bool ShoudRefreshGatt(enum ConnBleDisconnectReason reason)
{
    switch (reason) {
        case BLE_DISCONNECT_REASON_CONNECT_TIMEOUT:
            return true;
        default:
            return false;
    }
}

int32_t ConnBleDisconnectNow(ConnBleConnection *connection, enum ConnBleDisconnectReason reason)
{
    CONN_CHECK_AND_RETURN_RET_LOG(connection != NULL, SOFTBUS_INVALID_PARAM,
        "ble connection disconnect failed, invalid param, connection is null");
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    CONN_CHECK_AND_RETURN_RET_LOG(interface != NULL, SOFTBUS_ERR,
        "ble connection disconnect failed, protocol not support");
    CLOGW("receive ble disconnect now, connection id=%u, side=%d, reason=%d", connection->connectionId,
        connection->side, reason);
    ConnRemoveMsgFromLooper(
        &g_bleConnectionAsyncHandler, MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT, connection->connectionId, 0, NULL);
    if (connection->side == CONN_SIDE_CLIENT) {
        bool grace = ShouldGrace(reason);
        bool refreshGatt = ShoudRefreshGatt(reason);
        return interface->bleClientDisconnect(connection, grace, refreshGatt);
    }
    return interface->bleServerDisconnect(connection);
}

int32_t ConnBleUpdateConnectionRc(ConnBleConnection *connection, int32_t delta)
{
    int32_t status = SoftBusMutexLock(&connection->lock);
    if (status != SOFTBUS_OK) {
        return SOFTBUS_LOCK_ERR;
    }
    int32_t underlayerHandle = connection->underlayerHandle;
    ConnBleFeatureBitSet featureBitSet = connection->featureBitSet;
    connection->connectionRc += delta;
    int32_t localRc = connection->connectionRc;
    if (localRc <= 0) {
        connection->state = BLE_CONNECTION_STATE_NEGOTIATION_CLOSING;
    }
    (void)SoftBusMutexUnlock(&connection->lock);
    CLOGI("ble notify refrence, connection id=%u, side=%d, delta=%d, after update reference, localRc=%d,"
          "underlayer handle=%d",
        connection->connectionId, connection->side, delta, localRc, underlayerHandle);

    if (localRc <= 0) {
        if ((featureBitSet & (1 << BLE_FEATURE_SUPPORT_REMOTE_DISCONNECT)) == 0) {
            CLOGW("ble connection reference count <= 0 and peer not support negotiation disconnect by notify msg, "
                  "disconnect directly, connection id=%u, underlayer handle=%d, support feature bitset=%u",
                connection->connectionId, underlayerHandle, featureBitSet);
            ConnBleDisconnectNow(connection, BLE_DISCONNECT_REASON_NO_REFERENCE);
            return SOFTBUS_OK;
        }
        ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_WAIT_NEGOTIATION_CLOSING_TIMEOUT,
            connection->connectionId, 0, NULL, WAIT_NEGOTIATION_CLOSING_TIMEOUT_MILLIS);
    }

    int32_t flag = delta >= 0 ? CONN_HIGH : CONN_LOW;
    BleCtlMessageSerializationContext ctx = {
        .connectionId = connection->connectionId,
        .flag = flag,
        .method = CTRL_MSG_METHOD_NOTIFY_REQUEST,
        .referenceRequest = {
            .delta = delta,
            .referenceNumber = localRc,
        },
    };
    uint8_t *data = NULL;
    uint32_t dataLen = 0;
    int64_t seq = ConnBlePackCtlMessage(ctx, &data, &dataLen);
    if (seq < 0) {
        CLOGE("ATTENTION, ble pack notify request message failed, connection id=%u, underlayer handle=%d, error=%d",
            connection->connectionId, underlayerHandle, (int32_t)seq);
        return (int32_t)seq;
    }
    status = ConnBlePostBytes(connection->connectionId, data, dataLen, 0, flag, MODULE_CONNECTION, seq);
    return status;
}

int32_t ConnBleOnReferenceRequest(ConnBleConnection *connection, const cJSON *json)
{
    int32_t delta = 0;
    int32_t peerRc = 0;
    if (!GetJsonObjectSignedNumberItem(json, CTRL_MSG_KEY_DELTA, &delta) ||
        !GetJsonObjectSignedNumberItem(json, CTRL_MSG_KEY_REF_NUM, &peerRc)) {
        CLOGE("ble connection %u reference request message failed: parse delta or reference number fields failed, "
              "delta=%d, peer reference count=%d",
            connection->connectionId, delta, peerRc);
        return SOFTBUS_PARSE_JSON_ERR;
    }

    int32_t status = SoftBusMutexLock(&connection->lock);
    if (status != SOFTBUS_OK) {
        CLOGE("ATTENTION UNEXPECTED ERROR! ble connection %u reference request message failed: try to lock failed, "
              "error=%d",
            connection->connectionId, status);
        return SOFTBUS_LOCK_ERR;
    }
    connection->connectionRc += delta;
    int32_t localRc = connection->connectionRc;

    CLOGI("ble received reference request, connection id=%u, delta=%d, peerRef=%d, localRc=%d",
        connection->connectionId, delta, peerRc, localRc);
    if (peerRc > 0) {
        if (connection->state == BLE_CONNECTION_STATE_NEGOTIATION_CLOSING) {
            ConnRemoveMsgFromLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_WAIT_NEGOTIATION_CLOSING_TIMEOUT,
                connection->connectionId, 0, NULL);
            connection->state = BLE_CONNECTION_STATE_EXCHANGED_BASIC_INFO;
            g_connectionListener.onConnectionResume(connection->connectionId);
        }
        (void)SoftBusMutexUnlock(&connection->lock);
        return SOFTBUS_OK;
    }
    if (localRc <= 0) {
        connection->state = BLE_CONNECTION_STATE_CLOSING;
        (void)SoftBusMutexUnlock(&connection->lock);
        ConnBleDisconnectNow(connection, BLE_DISCONNECT_REASON_NEGOTIATION_NO_REFERENCE);
        return SOFTBUS_OK;
    }
    (void)SoftBusMutexUnlock(&connection->lock);

    int32_t flag = CONN_HIGH;
    BleCtlMessageSerializationContext ctx = {
        .connectionId = connection->connectionId,
        .flag = flag,
        .method = CTRL_MSG_METHOD_NOTIFY_REQUEST,
        .referenceRequest = {
            .referenceNumber = localRc,
            .delta = 0,
        },
    };
    uint8_t *data = NULL;
    uint32_t dataLen = 0;
    int64_t seq = ConnBlePackCtlMessage(ctx, &data, &dataLen);
    if (seq < 0) {
        CLOGI("connection %u reference request message: pack reply message faild, error=%d", connection->connectionId,
            (int32_t)seq);
        return (int32_t)seq;
    }
    status = ConnBlePostBytes(connection->connectionId, data, dataLen, 0, flag, MODULE_CONNECTION, seq);
    return status;
}

int32_t ConnBleUpdateConnectionPriority(ConnBleConnection *connection, ConnectBlePriority priority)
{
    CONN_CHECK_AND_RETURN_RET_LOG(connection != NULL, SOFTBUS_INVALID_PARAM,
        "ble connection update connection priority failed, invalid param, connection is null");
    if (connection->side == CONN_SIDE_SERVER) {
        return SOFTBUS_FUNC_NOT_SUPPORT;
    }
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    CONN_CHECK_AND_RETURN_RET_LOG(interface != NULL, SOFTBUS_ERR,
        "ble connection update connection priority failed, protocol not support");
    return interface->bleClientUpdatePriority(connection, priority);
}

int32_t ConnBleSend(ConnBleConnection *connection, const uint8_t *data, uint32_t dataLen, int32_t module)
{
    CONN_CHECK_AND_RETURN_RET_LOG(connection != NULL, SOFTBUS_INVALID_PARAM,
        "ble connection send data failed, invalid param, connection is null");
    CONN_CHECK_AND_RETURN_RET_LOG(
        data != NULL, SOFTBUS_INVALID_PARAM, "ble connection send data failed, invalid param, data is null");
    CONN_CHECK_AND_RETURN_RET_LOG(
        dataLen != 0, SOFTBUS_INVALID_PARAM, "ble connection send data failed, invalid param, data len is 0");
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    CONN_CHECK_AND_RETURN_RET_LOG(interface != NULL, SOFTBUS_ERR,
        "ble connection send data failed, protocol not support");
    return connection->side == CONN_SIDE_SERVER ?
        interface->bleServerSend(connection, data, dataLen, module) :
        interface->bleClientSend(connection, data, dataLen, module);
}

void ConnBleRefreshIdleTimeout(ConnBleConnection *connection)
{
    ConnRemoveMsgFromLooper(
        &g_bleConnectionAsyncHandler, MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT, connection->connectionId, 0, NULL);
    ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT, connection->connectionId,
        0, NULL, CONNECTION_IDLE_DISCONNECT_TIMEOUT_MILLIS);
}

void ConnBleInnerComplementDeviceId(ConnBleConnection *connection)
{
    if (connection->protocol == BLE_GATT || strlen(connection->udid) != 0) {
        return;
    }
    if (strlen(connection->networkId) == 0) {
        CLOGE("complementation ble connection device id failed: network id not exchange yet, connection id=%u, "
              "protocol=%d",
            connection->connectionId, connection->protocol);
        return;
    }
    int32_t status = LnnGetRemoteStrInfo(connection->networkId, STRING_KEY_DEV_UDID, connection->udid, UDID_BUF_LEN);
    CLOGI("complementation ble connection device id, connection id=%u, protocol=%d, status=%d",
        connection->connectionId, connection->protocol, status);
}

static void ConnBlePackCtrlMsgHeader(ConnPktHead *header, uint32_t dataLen)
{
    static int64_t ctlMsgSeqGenerator = 0;
    int64_t seq = ctlMsgSeqGenerator++;

    header->magic = MAGIC_NUMBER;
    header->module = MODULE_CONNECTION;
    header->seq = seq;
    header->flag = CONN_HIGH;
    header->len = dataLen;
    PackConnPktHead(header);
}

// CoC connection exchange 'networdId' as old udid field may disclosure of user privacy and be traced;
// GATT connection keep exchange 'udid' as keeping compatibility
static int32_t SendBasicInfo(ConnBleConnection *connection)
{
    int32_t status = SOFTBUS_OK;
    char devId[DEVID_BUFF_LEN] = { 0 };
    switch (connection->protocol) {
        case BLE_GATT:
            status = LnnGetLocalStrInfo(STRING_KEY_DEV_UDID, devId, DEVID_BUFF_LEN);
            break;
        case BLE_COC:
            status = LnnGetLocalStrInfo(STRING_KEY_NETWORKID, devId, DEVID_BUFF_LEN);
            break;
        default:
            status = SOFTBUS_ERR;
            break;
    }
    if (status != SOFTBUS_OK) {
        CLOGE("ble send basic info failed: get devid from net ledger failed, connection id=%u, protocol=%d, error=%d",
            connection->connectionId, connection->protocol, status);
        return status;
    }

    int32_t deviceType = 0;
    status = LnnGetLocalNumInfo(NUM_KEY_DEV_TYPE_ID, &deviceType);
    if (status != SOFTBUS_OK) {
        CLOGE("ble send basic info failed: get device type from net ledger failed, connection id=%u, error=%d",
            connection->connectionId, status);
        return status;
    }

    cJSON *json = cJSON_CreateObject();
    if (json == NULL) {
        CLOGE("ble send basic info failed: create json object failed, connection id=%u", connection->connectionId);
        return SOFTBUS_CREATE_JSON_ERR;
    }
    char *payload = NULL;
    do {
        if (!AddStringToJsonObject(json, BASIC_INFO_KEY_DEVID, devId) ||
            !AddNumberToJsonObject(json, BASIC_INFO_KEY_ROLE, connection->side) ||
            !AddNumberToJsonObject(json, BASIC_INFO_KEY_DEVTYPE, deviceType) ||
            !AddNumberToJsonObject(json, BASIC_INFO_KEY_FEATURE, g_featureBitSet)) {
            CLOGE("ble send basic info failed: add json info failed, connection id=%u", connection->connectionId);
            status = SOFTBUS_CREATE_JSON_ERR;
            break;
        }
        payload = cJSON_PrintUnformatted(json);
        uint32_t payloadLen = strlen(payload);
        uint32_t dataLen = NET_CTRL_MSG_TYPE_HEADER_SIZE + payloadLen + 1;
        uint32_t bufLen = dataLen + (connection->protocol == BLE_COC ? sizeof(ConnPktHead) : 0);
        uint8_t *buf = (uint8_t *)SoftBusCalloc(bufLen);
        if (buf == NULL) {
            CLOGE("ATTENTION UNEXPECTED ERROR! ble send basic info failed: malloc buf failed, connection id=%u, buf "
                  "len=%u",
                connection->connectionId, bufLen);
            status = SOFTBUS_MALLOC_ERR;
            break;
        }

        int32_t offset = 0;
        if (connection->protocol == BLE_COC) {
            ConnPktHead *header = (ConnPktHead *)buf;
            ConnBlePackCtrlMsgHeader(header, dataLen);
            offset += sizeof(ConnPktHead);
        }
        int32_t *netCtrlMsgHeader = (int32_t *)(buf + offset);
        netCtrlMsgHeader[0] = NET_CTRL_MSG_TYPE_BASIC_INFO;
        offset += NET_CTRL_MSG_TYPE_HEADER_SIZE;
        if (memcpy_s(buf + offset, bufLen - offset, payload, payloadLen) != EOK) {
            CLOGE("ble send basic info failed: memcpy_s buf failed, connection id=%u, buf len=%u, paylaod len=%u",
                connection->connectionId, bufLen, payloadLen);
            status = SOFTBUS_MEM_ERR;
            SoftBusFree(buf);
            break;
        }
        status = ConnBlePostBytes(connection->connectionId, buf, bufLen, 0, CONN_HIGH, MODULE_BLE_NET, 0);
        CLOGI("ble send basic info, connection id=%u, side=%s, status=%d", connection->connectionId,
            connection->side == CONN_SIDE_CLIENT ? "client" : "server", status);
        if (status != SOFTBUS_OK) {
            break;
        }
    } while (false);
    cJSON_Delete(json);
    if (payload != NULL) {
        cJSON_free(payload);
    }
    return status;
}

static int32_t ParseBasicInfo(ConnBleConnection *connection, const uint8_t *data, uint32_t dataLen)
{
    if (dataLen <= NET_CTRL_MSG_TYPE_HEADER_SIZE) {
        CLOGI("ble parse basic info failed: date len exceed, connection id=%u, data len=%d", connection->connectionId,
            dataLen);
        return SOFTBUS_ERR;
    }
    int offset = 0;
    if (connection->protocol == BLE_COC) {
        offset += sizeof(ConnPktHead);
    }
    int32_t *netCtrlMsgHeader = (int32_t *)(data + offset);
    if (netCtrlMsgHeader[0] != NET_CTRL_MSG_TYPE_BASIC_INFO) {
        CLOGI("ble parse basic info failed: not basic info type, connection id=%u, type=%d", connection->connectionId,
            netCtrlMsgHeader[0]);
        return SOFTBUS_ERR;
    }
    offset += NET_CTRL_MSG_TYPE_HEADER_SIZE;
    cJSON *json = cJSON_ParseWithLength((char *)(data + offset), dataLen - offset);
    if (json == NULL) {
        CLOGI("ble parse basic info failed: parse json failed, connection id=%u", connection->connectionId);
        return SOFTBUS_PARSE_JSON_ERR;
    }
    // mandatory fields
    char devId[DEVID_BUFF_LEN] = { 0 };
    int32_t type = 0;
    if (!GetJsonObjectStringItem(json, BASIC_INFO_KEY_DEVID, devId, DEVID_BUFF_LEN) ||
        !GetJsonObjectNumberItem(json, BASIC_INFO_KEY_ROLE, &type)) {
        cJSON_Delete(json);
        CLOGE("ble parse basic info failed: basic info field not exist, connection id=%u", connection->connectionId);
        return SOFTBUS_ERR;
    }
    // optional field
    int32_t deviceType = 0;
    if (!GetJsonObjectNumberItem(json, BASIC_INFO_KEY_DEVTYPE, &deviceType)) {
        CLOGE("ble parse basic info warning, 'devType' is not exist, connection id=%u", connection->connectionId);
        // fall through
    }
    int32_t feature = 0;
    if (!GetJsonObjectNumberItem(json, BASIC_INFO_KEY_FEATURE, &feature)) {
        CLOGE(
            "ble parse basic info warning, 'FEATURE_SUPPORT' is not exist, connection id=%u", connection->connectionId);
        // fall through
    }
    cJSON_Delete(json);

    int32_t status = SoftBusMutexLock(&connection->lock);
    if (status != SOFTBUS_OK) {
        CLOGE("ATTENTION UNEXPECTED ERROR! ble parse basic info failed: try to lock connection failed, connection "
              "id=%u, error=%d",
            connection->connectionId, status);
        return SOFTBUS_LOCK_ERR;
    }
    if (connection->protocol == BLE_GATT) {
        if (memcpy_s(connection->udid, UDID_BUF_LEN, devId, DEVID_BUFF_LEN) != EOK) {
            (void)SoftBusMutexUnlock(&connection->lock);
            CLOGE("ble parse basic info failed: memcpy_s udid failed, connection id=%u", connection->connectionId);
            return SOFTBUS_MEM_ERR;
        }
    } else {
        if (memcpy_s(connection->networkId, NETWORK_ID_BUF_LEN, devId, DEVID_BUFF_LEN) != EOK) {
            (void)SoftBusMutexUnlock(&connection->lock);
            CLOGE(
                "ble parse basic info failed: memcpy_s network id failed, connection id=%u", connection->connectionId);
            return SOFTBUS_MEM_ERR;
        }
        ConnBleInnerComplementDeviceId(connection);
    }
    connection->featureBitSet = (ConnBleFeatureBitSet)feature;
    connection->state = BLE_CONNECTION_STATE_EXCHANGED_BASIC_INFO;
    (void)SoftBusMutexUnlock(&connection->lock);

    // revert current side role is peer side role
    int32_t expectedPeerType = connection->side == CONN_SIDE_CLIENT ? 2 : 1;
    if (expectedPeerType != type) {
        CLOGW("parse basic info, the role of connection is mismatch, expected peer side role=%d from current "
              "connection info, actual peer side role=%d from basic info",
            expectedPeerType, type);
    }
    CLOGE("ble parse basic info, connection id=%u, side=%s, device type=%d, support feature=%u",
        connection->connectionId, connection->side == CONN_SIDE_CLIENT ? "client" : "server", deviceType, feature);
    return SOFTBUS_OK;
}

void BleOnClientConnected(uint32_t connectionId)
{
    ConnBleConnection *connection = ConnBleGetConnectionById(connectionId);
    CONN_CHECK_AND_RETURN_LOG(
        connection != NULL, "ble on client connected failed: connection not exist, connection id=%u", connectionId);
    int32_t status = SOFTBUS_OK;
    do {
        status = SoftBusMutexLock(&connection->lock);
        if (status != SOFTBUS_OK) {
            CLOGE("ATTENTION UNEXPECTED ERROR! ble on client connected failed: try to lock failed, connection id=%u, "
                  "error=%d",
                connectionId, status);
            break;
        }
        connection->state = BLE_CONNECTION_STATE_EXCHANGING_BASIC_INFO;
        (void)SoftBusMutexUnlock(&connection->lock);
        status = ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_EXCHANGE_BASIC_INFO_TIMEOUT,
            connectionId, 0, NULL, BASIC_INFO_EXCHANGE_TIMEOUT);
        if (status != SOFTBUS_OK) {
            CLOGE("ble on client connected failed: post basic info exchange timeout event failed,connection id=%u, "
                  "error=%d",
                connectionId, status);
            break;
        }
        status = SendBasicInfo(connection);
        if (status != SOFTBUS_OK) {
            CLOGE("ble on client connected failed: send basic info message failed, connection id=%u, error=%d",
                connectionId, status);
            break;
        }
    } while (false);
    if (status != SOFTBUS_OK) {
        g_connectionListener.onConnectFailed(connection->connectionId, status);
    }
    ConnBleReturnConnection(&connection);
}

void BleOnClientFailed(uint32_t connectionId, int32_t error)
{
    g_connectionListener.onConnectFailed(connectionId, error);
}

// Memory Management Conventions: MUST free data if dispatch process is intercepted,
// otherwise it is responsibity of uplayer to free data
void BleOnDataReceived(uint32_t connectionId, bool isConnCharacteristic, uint8_t *data, uint32_t dataLen)
{
    ConnBleConnection *connection = ConnBleGetConnectionById(connectionId);
    if (connection == NULL) {
        CLOGE("ble connection data received failed, connection not exist, connection id=%u", connectionId);
        SoftBusFree(data);
        return;
    }
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    if (interface == NULL) {
        CLOGE("ble connection data received failed, protocol not support, connection id=%u", connectionId);
        SoftBusFree(data);
        return;
    }
    int32_t status = SOFTBUS_OK;
    do {
        if (isConnCharacteristic) {
            ConnBleRefreshIdleTimeout(connection);
            g_connectionListener.onDataReceived(connectionId, isConnCharacteristic, data, dataLen);
            break;
        }

        status = SoftBusMutexLock(&connection->lock);
        if (status != SOFTBUS_OK) {
            SoftBusFree(data);
            // NOT notify client 'onConnectFailed' or server disconnect as it can not get state safely here,
            // connection will fail after basic info change timeout, all resouces will cleanup in timeout handle method
            CLOGE("ATTENTION UNEXPECTED ERROR! ble connection data received failed, try to lock failed, connection "
                  "id=%u, error=%d",
                connectionId, status);
            break;
        }
        enum ConnBleConnectionState state = connection->state;
        int32_t underlayerHandle = connection->underlayerHandle;
        (void)SoftBusMutexUnlock(&connection->lock);
        if (state != BLE_CONNECTION_STATE_EXCHANGING_BASIC_INFO) {
            ConnBleRefreshIdleTimeout(connection);
            isConnCharacteristic = (connection->protocol == BLE_COC) ? true : isConnCharacteristic;
            g_connectionListener.onDataReceived(connectionId, isConnCharacteristic, data, dataLen);
            break;
        }

        status = ParseBasicInfo(connection, data, dataLen);
        SoftBusFree(data);
        if (status != SOFTBUS_OK) {
            CLOGE("ble connection data received failed, parse basic info failed, connection id=%u, side=%d, "
                  "underlayer handle=%d, error=%d",
                connectionId, connection->side, underlayerHandle, status);
            if (connection->side == CONN_SIDE_CLIENT) {
                g_connectionListener.onConnectFailed(connection->connectionId, status);
            } else {
                interface->bleServerDisconnect(connection);
            }
            break;
        }
        ConnRemoveMsgFromLooper(
            &g_bleConnectionAsyncHandler, MSG_CONNECTION_EXCHANGE_BASIC_INFO_TIMEOUT, connectionId, 0, NULL);
        if (connection->side == CONN_SIDE_SERVER) {
            status = SendBasicInfo(connection);
            if (status != SOFTBUS_OK) {
                CLOGE("ble connection data received failed, send server side basic info failed, connection id=%u, "
                      "underlayer handle=%d, error=%d",
                    connectionId, underlayerHandle, status);
                interface->bleServerDisconnect(connection);
                break;
            }
            status = interface->bleServerConnect(connection);
            CLOGI("ble connection data received, server side finish exchange basic info, connection id=%u, "
                  "underlayer handle=%d, server connect status=%d",
                connectionId, underlayerHandle, status);
            ConnBleRefreshIdleTimeout(connection);
            g_connectionListener.onServerAccepted(connection->connectionId);
        } else {
            CLOGI("ble connection data received, client side finish exchange basic info, connection id=%u, "
                  "underlayer handle=%d",
                connectionId, underlayerHandle);
            ConnBleRefreshIdleTimeout(connection);
            g_connectionListener.onConnected(connection->connectionId);
        }
    } while (false);
    ConnBleReturnConnection(&connection);
}

void BleOnConnectionClosed(uint32_t connectionId, int32_t status)
{
    ConnRemoveMsgFromLooper(
        &g_bleConnectionAsyncHandler, MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT, connectionId, 0, NULL);
    g_connectionListener.onConnectionClosed(connectionId, status);
}

void BleOnServerStarted(BleProtocolType protocol, int32_t status)
{
    CLOGI("receive ble server started event, status=%d", status);

    CONN_CHECK_AND_RETURN_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK,
        "ATTENTION UNEXPECTED EXCEPTION: on server start event handle failed, try to lock failed");
    g_serverCoordination.status[protocol] = status;
    g_serverCoordination.actual =
        (g_serverCoordination.status[BLE_GATT] == SOFTBUS_OK && g_serverCoordination.status[BLE_COC] == SOFTBUS_OK ?
                BLE_SERVER_STATE_STARTED :
                BLE_SERVER_STATE_STOPPED);
    if (g_serverCoordination.expect != g_serverCoordination.actual) {
        ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT, 0, 0, NULL,
            RETRY_SERVER_STATE_CONSISTENT_MILLIS);
    }
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
}

void BleOnServerClosed(BleProtocolType protocol, int32_t status)
{
    CLOGI("receive ble server closed event, status=%d", status);

    CONN_CHECK_AND_RETURN_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK,
        "ATTENTION UNEXPECTED EXCEPTION: on server close event handle failed, try to lock failed");
    g_serverCoordination.status[protocol] = status;
    g_serverCoordination.actual =
        (g_serverCoordination.status[BLE_GATT] == SOFTBUS_OK && g_serverCoordination.status[BLE_COC] == SOFTBUS_OK ?
                BLE_SERVER_STATE_STOPPED :
                BLE_SERVER_STATE_STARTED);
    if (g_serverCoordination.expect != g_serverCoordination.actual) {
        ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT, 0, 0, NULL,
            RETRY_SERVER_STATE_CONSISTENT_MILLIS);
    }
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
}

void BleOnServerAccepted(uint32_t connectionId)
{
    ConnBleConnection *connection = ConnBleGetConnectionById(connectionId);
    CONN_CHECK_AND_RETURN_LOG(
        connection != NULL, "ble server accepted failed, connection not exist, connection id=%u", connectionId);
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    CONN_CHECK_AND_RETURN_LOG(
        interface != NULL, "ble server accepted failed, interface not support, connection id=%u", connectionId);
    int32_t status = SOFTBUS_OK;
    do {
        status = SoftBusMutexLock(&connection->lock);
        if (status != SOFTBUS_OK) {
            CLOGE("ATTENTION UNEXPECTED ERROR! ble server accepted failed, try to lock failed, connection id=%u, "
                  "error=%d",
                connectionId, status);
            break;
        }
        connection->state = BLE_CONNECTION_STATE_EXCHANGING_BASIC_INFO;
        (void)SoftBusMutexUnlock(&connection->lock);
        status = ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_EXCHANGE_BASIC_INFO_TIMEOUT,
            connectionId, 0, NULL, BASIC_INFO_EXCHANGE_TIMEOUT);
        if (status != SOFTBUS_OK) {
            CLOGE(
                "ble server accepted failed, post basic info exchange timeout event failed,connection id=%u, error=%d",
                connectionId, status);
            break;
        }
    } while (false);
    if (status != SOFTBUS_OK) {
        interface->bleServerDisconnect(connection);
    }
    ConnBleReturnConnection(&connection);
}

static int32_t DoRetryAction(enum BleServerState expect)
{
    int32_t statusGatt = SOFTBUS_OK;
    int32_t statusCoc = SOFTBUS_OK;
    if (g_serverCoordination.status[BLE_GATT] != SOFTBUS_OK) {
        const BleUnifyInterface *interface = ConnBleGetUnifyInterface(BLE_GATT);
        if (interface != NULL) {
            statusGatt = (expect == BLE_SERVER_STATE_STARTED) ? interface->bleServerStartService() :
                                 interface->bleServerStopService();
        }
    }

    if (g_serverCoordination.status[BLE_COC] != SOFTBUS_OK) {
        const BleUnifyInterface *interface = ConnBleGetUnifyInterface(BLE_COC);
        if (interface != NULL) {
            statusCoc = (expect == BLE_SERVER_STATE_STARTED) ? interface->bleServerStartService() :
                                interface->bleServerStopService();
        }
    }

    return (statusGatt != SOFTBUS_OK || statusCoc != SOFTBUS_OK) ? SOFTBUS_ERR : SOFTBUS_OK;
}

static void RetryServerStatConsistentHandler(void)
{
    CONN_CHECK_AND_RETURN_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK,
        "ATTENTION UNEXPECTED EXCEPTION: retry server state consistent msg handle, try to lock failed");
    enum BleServerState expect = g_serverCoordination.expect;
    enum BleServerState actual = g_serverCoordination.actual;
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
    if (expect == actual) {
        // consistent reached
        return;
    }
    if (actual == BLE_SERVER_STATE_STARTING || actual == BLE_SERVER_STATE_STOPPING) {
        // action on the fly, try later
        ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT, 0, 0, NULL,
            RETRY_SERVER_STATE_CONSISTENT_MILLIS);
        return;
    }
    int32_t status = DoRetryAction(expect);
    if (status != SOFTBUS_OK) {
        ConnPostMsgToLooper(&g_bleConnectionAsyncHandler, MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT, 0, 0, NULL,
            RETRY_SERVER_STATE_CONSISTENT_MILLIS);
        return;
    }
    CONN_CHECK_AND_RETURN_LOG(SoftBusMutexLock(&g_serverCoordination.lock) == SOFTBUS_OK,
        "ATTENTION UNEXPECTED EXCEPTION: retry server state consistent msg handle, try to lock failed");
    g_serverCoordination.actual =
        (expect == BLE_SERVER_STATE_STARTED ? BLE_SERVER_STATE_STARTING : BLE_SERVER_STATE_STOPPING);
    (void)SoftBusMutexUnlock(&g_serverCoordination.lock);
}

static void BasicInfoExchangeTimeoutHandler(uint32_t connectionId)
{
    ConnBleConnection *connection = ConnBleGetConnectionById(connectionId);
    CONN_CHECK_AND_RETURN_LOG(connection != NULL,
        "ble basic info exchange timeout handle failed, connection not exist, connection id=%u", connectionId);
    const BleUnifyInterface *interface = ConnBleGetUnifyInterface(connection->protocol);
    CONN_CHECK_AND_RETURN_LOG(interface != NULL,
        "ble basic info exchange timeout handle failed, protocol not support, connection id=%u", connectionId);
    CLOGW("ble basic info exchange timeout, connection id=%u, side=%s", connectionId,
        connection->side == CONN_SIDE_CLIENT ? "client" : "server");
    if (connection->side == CONN_SIDE_CLIENT) {
        g_connectionListener.onConnectFailed(connectionId, SOFTBUS_CONN_BLE_EXCHANGE_BASIC_INFO_TIMEOUT_ERR);
    } else {
        interface->bleServerDisconnect(connection);
    }
    ConnBleReturnConnection(&connection);
}

static void WaitNegotiationClosingTimeoutHandler(uint32_t connectionId)
{
    ConnBleConnection *connection = ConnBleGetConnectionById(connectionId);
    CONN_CHECK_AND_RETURN_LOG(connection != NULL,
        "ble wait negotiation closing timeout handler failed: connection not exist, connection id=%u", connectionId);
    int32_t status = SoftBusMutexLock(&connection->lock);
    if (status != SOFTBUS_OK) {
        CLOGE(
            "ble wait negotiation closing timeout handler failed: try to lock failed, connection id=%u", connectionId);
        ConnBleReturnConnection(&connection);
        return;
    }
    enum ConnBleConnectionState state = connection->state;
    (void)SoftBusMutexUnlock(&connection->lock);
    CLOGW("ble wait negotiation closing timeout handler, connection id=%u, state=%d", connectionId, state);
    if (state == BLE_CONNECTION_STATE_NEGOTIATION_CLOSING) {
        ConnBleDisconnectNow(connection, BLE_DISCONNECT_REASON_NEGOTIATION_WAIT_TIMEOUT);
    }
    ConnBleReturnConnection(&connection);
}

static void ConnectionIdleDisconnectTimeoutHandler(uint32_t connectionId)
{
    ConnBleConnection *connection = ConnBleGetConnectionById(connectionId);
    CONN_CHECK_AND_RETURN_LOG(connection != NULL,
        "connection idle disconnect timeout handler failed: connection not exist, connection id=%u", connectionId);
    CLOGW("connection idle disconnect timeout handler, connection idle exceed more %u ms, forgot call disconnect? "
          "disconnect now, connection id=%u",
        CONNECTION_IDLE_DISCONNECT_TIMEOUT_MILLIS, connectionId);
    ConnBleDisconnectNow(connection, BLE_DISCONNECT_REASON_IDLE_WAIT_TIMEOUT);
    ConnBleReturnConnection(&connection);
}

static void BleConnectionMsgHandler(SoftBusMessage *msg)
{
    CLOGI("ble connection looper receive msg %d", msg->what);
    switch (msg->what) {
        case MSG_CONNECTION_RETRY_SERVER_STATE_CONSISTENT:
            RetryServerStatConsistentHandler();
            break;
        case MSG_CONNECTION_EXCHANGE_BASIC_INFO_TIMEOUT:
            BasicInfoExchangeTimeoutHandler((uint32_t)msg->arg1);
            break;
        case MSG_CONNECTION_WAIT_NEGOTIATION_CLOSING_TIMEOUT:
            WaitNegotiationClosingTimeoutHandler((uint32_t)msg->arg1);
            break;
        case MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT:
            ConnectionIdleDisconnectTimeoutHandler((uint32_t)msg->arg1);
            break;
        default:
            CLOGE("ATTENTION, ble connection looper receive unexpected msg, what=%d, just ignore, FIX it quickly.",
                msg->what);
            break;
    }
}

static int BleCompareConnectionLooperEventFunc(const SoftBusMessage *msg, void *args)
{
    SoftBusMessage *ctx = (SoftBusMessage *)args;
    if (msg->what != ctx->what) {
        return COMPARE_FAILED;
    }
    switch (ctx->what) {
        case MSG_CONNECTION_EXCHANGE_BASIC_INFO_TIMEOUT:
        case MSG_CONNECTION_WAIT_NEGOTIATION_CLOSING_TIMEOUT:
        case MSG_CONNECTION_IDLE_DISCONNECT_TIMEOUT: {
            if (msg->arg1 == ctx->arg1) {
                return COMPARE_SUCCESS;
            }
            return COMPARE_FAILED;
        }
        default:
            break;
    }
    if (ctx->arg1 != 0 || ctx->arg2 != 0 || ctx->obj != NULL) {
        CLOGE("ble compare connection looper event failed: there is compare context value not use, forgot implement? "
              "compare failed to avoid fault silence, what=%d, arg1=%" PRIu64 ", arg2=%" PRIu64 ", obj is null? %d",
            ctx->what, ctx->arg1, ctx->arg2, ctx->obj == NULL);
        return COMPARE_FAILED;
    }
    return COMPARE_SUCCESS;
}

int32_t ConnBleInitConnectionMudule(SoftBusLooper *looper, ConnBleConnectionEventListener *listener)
{
    CONN_CHECK_AND_RETURN_RET_LOG(
        looper != NULL, SOFTBUS_INVALID_PARAM, "init ble connection failed: invalid param, looper is null");
    CONN_CHECK_AND_RETURN_RET_LOG(
        listener != NULL, SOFTBUS_INVALID_PARAM, "init ble connection failed: invalid param, listener is null");
    CONN_CHECK_AND_RETURN_RET_LOG(listener->onServerAccepted != NULL, SOFTBUS_INVALID_PARAM,
        "init ble connection failed: invalid param, listener onServerAccepted is null");
    CONN_CHECK_AND_RETURN_RET_LOG(listener->onConnected != NULL, SOFTBUS_INVALID_PARAM,
        "init ble connection failed: invalid param, listener onConnected is null");
    CONN_CHECK_AND_RETURN_RET_LOG(listener->onConnectFailed != NULL, SOFTBUS_INVALID_PARAM,
        "init ble connection failed: invalid param, listener onConnectFailed is null");
    CONN_CHECK_AND_RETURN_RET_LOG(listener->onDataReceived != NULL, SOFTBUS_INVALID_PARAM,
        "init ble connection failed: invalid param, listener onDataReceived is null");
    CONN_CHECK_AND_RETURN_RET_LOG(listener->onConnectionClosed != NULL, SOFTBUS_INVALID_PARAM,
        "init ble connection failed: invalid param, listener onConnectionClosed is null");
    CONN_CHECK_AND_RETURN_RET_LOG(listener->onConnectionResume != NULL, SOFTBUS_INVALID_PARAM,
        "init ble connection failed: invalid param, listener onConnectionResume is null");
    ConnBleClientEventListener clientEventListener = {
        .onClientConnected = BleOnClientConnected,
        .onClientFailed = BleOnClientFailed,
        .onClientDataReceived = BleOnDataReceived,
        .onClientConnectionClosed = BleOnConnectionClosed,
    };
    ConnBleServerEventListener serverEventListener = {
        .onServerStarted = BleOnServerStarted,
        .onServerClosed = BleOnServerClosed,
        .onServerAccepted = BleOnServerAccepted,
        .onServerDataReceived = BleOnDataReceived,
        .onServerConnectionClosed = BleOnConnectionClosed,
    };
    int32_t status = SOFTBUS_ERR;
    const BleUnifyInterface *interface;
    for (int i = BLE_GATT; i < BLE_PROTOCOL_MAX; i++) {
        interface = ConnBleGetUnifyInterface(i);
        if (interface == NULL) {
            continue;
        }
        status = interface->bleClientInitModule(looper, &clientEventListener);
        CONN_CHECK_AND_RETURN_RET_LOG(status == SOFTBUS_OK, status,
            "init ble connection failed: init ble %d client failed, error=%d", i, status);
        status = interface->bleServerInitModule(looper, &serverEventListener);
        CONN_CHECK_AND_RETURN_RET_LOG(status == SOFTBUS_OK, status,
            "init ble connection failed: init ble %d server failed, error=%d", i, status);
    }
    status = SoftBusMutexInit(&g_serverCoordination.lock, NULL);
    CONN_CHECK_AND_RETURN_RET_LOG(status == SOFTBUS_OK, status,
        "init ble connection failed: init server coordination lock failed, error=%d", status);
    g_bleConnectionAsyncHandler.handler.looper = looper;
    g_connectionListener = *listener;
    return SOFTBUS_OK;
}
