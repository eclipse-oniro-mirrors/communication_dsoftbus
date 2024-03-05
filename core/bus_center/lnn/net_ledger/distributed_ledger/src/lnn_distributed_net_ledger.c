/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "lnn_distributed_net_ledger.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <securec.h>

#include "lnn_connection_addr_utils.h"
#include "lnn_fast_offline.h"
#include "lnn_lane_info.h"
#include "lnn_map.h"
#include "lnn_node_info.h"
#include "lnn_lane_def.h"
#include "lnn_deviceinfo_to_profile.h"
#include "lnn_device_info_recovery.h"
#include "lnn_feature_capability.h"
#include "lnn_local_net_ledger.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_adapter_crypto.h"
#include "softbus_bus_center.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_adapter_crypto.h"
#include "softbus_utils.h"
#include "softbus_hidumper_buscenter.h"
#include "bus_center_manager.h"
#include "softbus_hisysevt_bus_center.h"
#include "bus_center_event.h"

#define TIME_THOUSANDS_FACTOR (1000)
#define BLE_ADV_LOST_TIME 5000
#define LONG_TO_STRING_MAX_LEN 21
#define LNN_COMMON_LEN_64 8
#define SOFTBUS_BUSCENTER_DUMP_REMOTEDEVICEINFO "remote_device_info"

#define RETURN_IF_GET_NODE_VALID(networkId, buf, info) do {                 \
        if ((networkId) == NULL || (buf) == NULL) {                        \
            return SOFTBUS_INVALID_PARAM;                               \
        }                                                               \
        (info) = LnnGetNodeInfoById((networkId), (CATEGORY_NETWORK_ID)); \
        if ((info) == NULL) {                                           \
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get node info fail."); \
            return SOFTBUS_ERR;                                         \
        }                                                               \
    } while (0)                                                        \

#define CONNECTION_FREEZE_TIMEOUT_MILLIS (10 * 1000)

// softbus version for support initConnectFlag
#define SOFTBUS_VERSION_FOR_INITCONNECTFLAG "11.1.0.001"

typedef struct {
    Map udidMap;
    Map ipMap;
    Map macMap;
} DoubleHashMap;

typedef enum {
    DL_INIT_UNKNOWN = 0,
    DL_INIT_FAIL,
    DL_INIT_SUCCESS,
} DistributedLedgerStatus;

typedef struct {
    Map connectionCode;
} ConnectionCode;

typedef struct {
    DoubleHashMap distributedInfo;
    ConnectionCode cnnCode;
    int32_t countMax;
    SoftBusMutex lock;
    DistributedLedgerStatus status;
} DistributedNetLedger;

static DistributedNetLedger g_distributedNetLedger;

static void UpdateNetworkInfo(const char *udid)
{
    NodeBasicInfo basic = { 0 };
    if (LnnGetBasicInfoByUdid(udid, &basic) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "GetBasicInfoByUdid fail.");
        return;
    }
    LnnNotifyBasicInfoChanged(&basic, TYPE_NETWORK_INFO);
}

static uint64_t GetCurrentTime(void)
{
    SoftBusSysTime now = { 0 };
    if (SoftBusGetTime(&now) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "GetCurrentTime fail.");
        return 0;
    }
    return (uint64_t)now.sec * TIME_THOUSANDS_FACTOR + (uint64_t)now.usec / TIME_THOUSANDS_FACTOR;
}

NO_SANITIZE("cfi") int32_t LnnSetAuthTypeValue(uint32_t *authTypeValue, AuthType type)
{
    if (authTypeValue == NULL || type >= AUTH_TYPE_BUTT) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "in para error!");
        return SOFTBUS_INVALID_PARAM;
    }
    *authTypeValue = (*authTypeValue) | (1 << (uint32_t)type);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnClearAuthTypeValue(uint32_t *authTypeValue, AuthType type)
{
    if (authTypeValue == NULL || type >= AUTH_TYPE_BUTT) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "in para error!");
        return SOFTBUS_INVALID_PARAM;
    }
    *authTypeValue = (*authTypeValue) & (~(1 << (uint32_t)type));
    return SOFTBUS_OK;
}

static NodeInfo *GetNodeInfoFromMap(const DoubleHashMap *map, const char *id)
{
    if (map == NULL || id == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return NULL;
    }
    NodeInfo *info = NULL;
    if ((info = (NodeInfo *)LnnMapGet(&map->udidMap, id)) != NULL) {
        return info;
    }
    if ((info = (NodeInfo *)LnnMapGet(&map->macMap, id)) != NULL) {
        return info;
    }
    if ((info = (NodeInfo *)LnnMapGet(&map->ipMap, id)) != NULL) {
        return info;
    }
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "id not exist!");
    return NULL;
}

static int32_t InitDistributedInfo(DoubleHashMap *map)
{
    if (map == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "fail:para error!");
        return SOFTBUS_INVALID_PARAM;
    }
    LnnMapInit(&map->udidMap);
    LnnMapInit(&map->ipMap);
    LnnMapInit(&map->macMap);
    return SOFTBUS_OK;
}

static void DeinitDistributedInfo(DoubleHashMap *map)
{
    if (map == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "fail: para error!");
        return;
    }
    LnnMapDelete(&map->udidMap);
    LnnMapDelete(&map->ipMap);
    LnnMapDelete(&map->macMap);
}

static int32_t InitConnectionCode(ConnectionCode *cnnCode)
{
    if (cnnCode == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "fail: para error!");
        return SOFTBUS_INVALID_PARAM;
    }
    LnnMapInit(&cnnCode->connectionCode);
    return SOFTBUS_OK;
}

static void DeinitConnectionCode(ConnectionCode *cnnCode)
{
    if (cnnCode == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "fail: para error!");
        return;
    }
    LnnMapDelete(&cnnCode->connectionCode);
    return;
}

NO_SANITIZE("cfi") void LnnDeinitDistributedLedger(void)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return;
    }
    g_distributedNetLedger.status = DL_INIT_UNKNOWN;
    DeinitDistributedInfo(&g_distributedNetLedger.distributedInfo);
    DeinitConnectionCode(&g_distributedNetLedger.cnnCode);
    if (SoftBusMutexUnlock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "unlock mutex fail!");
    }
    SoftBusMutexDestroy(&g_distributedNetLedger.lock);
}

static void NewWifiDiscovered(const NodeInfo *oldInfo, NodeInfo *newInfo)
{
    const char *macAddr = NULL;
    if (oldInfo == NULL || newInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return;
    }
    newInfo->discoveryType = newInfo->discoveryType | oldInfo->discoveryType;
    macAddr = LnnGetBtMac(newInfo);
    if (macAddr == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnGetBtMac Fail!");
        return;
    }
    if (strcmp(macAddr, DEFAULT_MAC) == 0) {
        LnnSetBtMac(newInfo, LnnGetBtMac(oldInfo));
    }
}

static void NewBrBleDiscovered(const NodeInfo *oldInfo, NodeInfo *newInfo)
{
    const char *ipAddr = NULL;
    if (oldInfo == NULL || newInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return;
    }
    newInfo->discoveryType = newInfo->discoveryType | oldInfo->discoveryType;
    ipAddr = LnnGetWiFiIp(newInfo);
    if (ipAddr == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnGetWiFiIp Fail!");
        return;
    }
    if (strcmp(ipAddr, DEFAULT_IP) == 0) {
        LnnSetWiFiIp(newInfo, LnnGetWiFiIp(oldInfo));
    }

    newInfo->connectInfo.authPort = oldInfo->connectInfo.authPort;
    newInfo->connectInfo.proxyPort = oldInfo->connectInfo.proxyPort;
    newInfo->connectInfo.sessionPort = oldInfo->connectInfo.sessionPort;
}

static void RetainOfflineCode(const NodeInfo *oldInfo, NodeInfo *newInfo)
{
    if (oldInfo == NULL || newInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return;
    }
    if (memcpy_s(newInfo->offlineCode, OFFLINE_CODE_BYTE_SIZE,
        oldInfo->offlineCode, OFFLINE_CODE_BYTE_SIZE) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "memcpy offlineCode error!");
        return;
    }
}
static int32_t ConvertNodeInfoToBasicInfo(const NodeInfo *info, NodeBasicInfo *basic)
{
    if (info == NULL || basic == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return SOFTBUS_INVALID_PARAM;
    }
    if (strncpy_s(basic->deviceName, DEVICE_NAME_BUF_LEN, info->deviceInfo.deviceName,
        strlen(info->deviceInfo.deviceName)) != EOK) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "strncpy_s name error!");
            return SOFTBUS_MEM_ERR;
    }

    if (strncpy_s(basic->networkId, NETWORK_ID_BUF_LEN, info->networkId, strlen(info->networkId)) != EOK) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "strncpy_s networkID error!");
            return SOFTBUS_MEM_ERR;
    }
    basic->deviceTypeId = info->deviceInfo.deviceTypeId;
    return SOFTBUS_OK;
}

static bool isMetaNode(NodeInfo *info)
{
    if (info == NULL) {
        return false;
    }
    return info->metaInfo.isMetaNode;
}

static int32_t GetDLOnlineNodeNumLocked(int32_t *infoNum, bool isNeedMeta)
{
    NodeInfo *info = NULL;
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    MapIterator *it = LnnMapInitIterator(&map->udidMap);

    if (it == NULL) {
        return SOFTBUS_ERR;
    }
    *infoNum = 0;
    while (LnnMapHasNext(it)) {
        it = LnnMapNext(it);
        if (it == NULL) {
            return SOFTBUS_ERR;
        }
        info = (NodeInfo *)it->node->value;
        if (!isNeedMeta) {
            if (LnnIsNodeOnline(info)) {
                (*infoNum)++;
            }
        } else {
            if (LnnIsNodeOnline(info) || isMetaNode(info)) {
                (*infoNum)++;
            }
        }
    }
    LnnMapDeinitIterator(it);
    return SOFTBUS_OK;
}

static int32_t FillDLOnlineNodeInfoLocked(NodeBasicInfo *info, int32_t infoNum, bool isNeedMeta)
{
    NodeInfo *nodeInfo = NULL;
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    MapIterator *it = LnnMapInitIterator(&map->udidMap);
    int32_t i = 0;

    if (it == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "it is null");
        return SOFTBUS_ERR;
    }
    while (LnnMapHasNext(it) && i < infoNum) {
        it = LnnMapNext(it);
        if (it == NULL) {
            LnnMapDeinitIterator(it);
            return SOFTBUS_ERR;
        }
        nodeInfo = (NodeInfo *)it->node->value;
        if (!isNeedMeta) {
            if (LnnIsNodeOnline(nodeInfo)) {
                ConvertNodeInfoToBasicInfo(nodeInfo, info + i);
                ++i;
            }
        } else {
            if (LnnIsNodeOnline(nodeInfo) || isMetaNode(nodeInfo)) {
                ConvertNodeInfoToBasicInfo(nodeInfo, info + i);
                ++i;
            }
        }
    }
    LnnMapDeinitIterator(it);
    return SOFTBUS_OK;
}

static bool IsNetworkIdChanged(NodeInfo *newInfo, NodeInfo *oldInfo)
{
    if (newInfo == NULL || oldInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return false;
    }
    if (strcmp(newInfo->networkId, oldInfo->networkId) == 0) {
        return false;
    }
    return true;
}

void PostOnlineNodesToCb(const INodeStateCb *callBack)
{
    NodeInfo *info = NULL;
    NodeBasicInfo basic;
    if (memset_s(&basic, sizeof(NodeBasicInfo), 0, sizeof(NodeBasicInfo)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "memset_s basic fail!");
    }
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    if (callBack->onNodeOnline == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "onNodeOnline IS null!");
        return;
    }
    MapIterator *it = LnnMapInitIterator(&map->udidMap);
    if (it == NULL) {
        return;
    }
    while (LnnMapHasNext(it)) {
        it = LnnMapNext(it);
        if (it == NULL) {
            return;
        }
        info = (NodeInfo *)it->node->value;
        if (LnnIsNodeOnline(info)) {
            ConvertNodeInfoToBasicInfo(info, &basic);
            callBack->onNodeOnline(&basic);
        }
    }
    LnnMapDeinitIterator(it);
}

NO_SANITIZE("cfi") NodeInfo *LnnGetNodeInfoById(const char *id, IdCategory type)
{
    NodeInfo *info = NULL;
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    if (id == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error");
        return info;
    }
    if (type == CATEGORY_UDID) {
        return GetNodeInfoFromMap(map, id);
    }
    MapIterator *it = LnnMapInitIterator(&map->udidMap);
    LNN_CHECK_AND_RETURN_RET_LOG(it != NULL, NULL, "LnnMapInitIterator is null");

    while (LnnMapHasNext(it)) {
        it = LnnMapNext(it);
        LNN_CHECK_AND_RETURN_RET_LOG(it != NULL, info, "it next is null");
        info = (NodeInfo *)it->node->value;
        if (info == NULL) {
            continue;
        }
        if (type == CATEGORY_NETWORK_ID) {
            if (strcmp(info->networkId, id) == 0) {
                LnnMapDeinitIterator(it);
                return info;
            }
        } else if (type == CATEGORY_UUID) {
            if (strcmp(info->uuid, id) == 0) {
                LnnMapDeinitIterator(it);
                return info;
            }
        } else {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "type error");
        }
    }
    LLOGE("get node info by id failed");
    LnnMapDeinitIterator(it);
    return NULL;
}

static NodeInfo *LnnGetNodeInfoByDeviceId(const char *id)
{
    NodeInfo *info = NULL;
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    NodeInfo *udidInfo = GetNodeInfoFromMap(map, id);
    if (udidInfo != NULL) {
        return udidInfo;
    }
    MapIterator *it = LnnMapInitIterator(&map->udidMap);
    if (it == NULL) {
        return info;
    }
    while (LnnMapHasNext(it)) {
        it = LnnMapNext(it);
        if (it == NULL) {
            return info;
        }
        info = (NodeInfo *)it->node->value;
        if (info == NULL) {
            continue;
        }
        if (strcmp(info->networkId, id) == 0) {
            LnnMapDeinitIterator(it);
            return info;
        }
        if (strcmp(info->uuid, id) == 0) {
            LnnMapDeinitIterator(it);
            return info;
        }
        if (strcmp(info->connectInfo.macAddr, id) == 0) {
            LnnMapDeinitIterator(it);
            return info;
        }
        if (strcmp(info->connectInfo.deviceIp, id) == 0) {
            LnnMapDeinitIterator(it);
            return info;
        }
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "type error");
    }
    LnnMapDeinitIterator(it);
    return NULL;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteNodeInfoById(const char *id, IdCategory type, NodeInfo *info)
{
    if (id == NULL || info == NULL) {
        LLOGE("param error");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(id, type);
    if (nodeInfo == NULL) {
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        LLOGI("can not find target node");
        return SOFTBUS_ERR;
    }
    if (memcpy_s(info, sizeof(NodeInfo), nodeInfo, sizeof(NodeInfo)) != EOK) {
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

/* key means networkId/udid/uuid/macAddr/ip */
int32_t LnnGetRemoteNodeInfoByKey(const char *key, NodeInfo *info)
{
    if (key == NULL || info == NULL) {
        LLOGE("param error");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoByDeviceId(key);
    if (nodeInfo == NULL) {
        LLOGI("can not find target node");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    if (memcpy_s(info, sizeof(NodeInfo), nodeInfo, sizeof(NodeInfo)) != EOK) {
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

bool LnnGetOnlineStateById(const char *id, IdCategory type)
{
    bool state = false;
    if (!IsValidString(id, ID_MAX_LEN)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "id is invalid");
        return state;
    }

    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return state;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(id, type);
    if (nodeInfo == NULL) {
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return state;
    }
    state = (nodeInfo->status == STATUS_ONLINE) ? true : false;
    if (!state) {
        state = nodeInfo->metaInfo.isMetaNode;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return state;
}

static int32_t DlGetDeviceUuid(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (strncpy_s(buf, len, info->uuid, strlen(info->uuid)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetDeviceOfflineCode(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (memcpy_s(buf, len, info->offlineCode, OFFLINE_CODE_BYTE_SIZE) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "memcpy_s offlinecode ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetDeviceUdid(const char *networkId, void *buf, uint32_t len)
{
    const char *udid = NULL;
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    udid = LnnGetDeviceUdid(info);
    if (udid == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get device udid fail");
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, udid, strlen(udid)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetNodeSoftBusVersion(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (strncpy_s(buf, len, info->softBusVersion, strlen(info->softBusVersion)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetDeviceType(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    char *deviceType = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    deviceType = LnnConvertIdToDeviceType(info->deviceInfo.deviceTypeId);
    if (deviceType == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "deviceType fail.");
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, deviceType, strlen(deviceType)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "MEM COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetDeviceTypeId(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = info->deviceInfo.deviceTypeId;
    return SOFTBUS_OK;
}

static int32_t DlGetAuthType(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((uint32_t *)buf) = info->AuthTypeValue;
    return SOFTBUS_OK;
}

static int32_t DlGetDeviceName(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *deviceName = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    deviceName = LnnGetDeviceName(&info->deviceInfo);
    if (deviceName == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get device name fail.");
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, deviceName, strlen(deviceName)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetBtMac(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *mac = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    mac = LnnGetBtMac(info);
    if (mac == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get bt mac fail.");
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, mac, strlen(mac)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetWlanIp(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *ip = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    ip = LnnGetWiFiIp(info);
    if (ip == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get wifi ip fail.");
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, ip, strlen(ip)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetMasterUdid(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *masterUdid = NULL;

    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (!LnnIsNodeOnline(info)) {
        return SOFTBUS_ERR;
    }
    masterUdid = LnnGetMasterUdid(info);
    if (masterUdid == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get master uiid fail");
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, masterUdid, strlen(masterUdid)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy master udid to buf fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetNodeBleMac(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;

    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    uint64_t currentTimeMs = GetCurrentTime();
    LNN_CHECK_AND_RETURN_RET_LOG(info->connectInfo.latestTime + BLE_ADV_LOST_TIME >= currentTimeMs, SOFTBUS_ERR,
        "ble mac out date, lastAdvTime:%llu, now:%llu", info->connectInfo.latestTime, currentTimeMs);

    if (memcpy_s(buf, len, info->connectInfo.bleMacAddr, MAC_LEN) != EOK) {
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

void LnnUpdateNodeBleMac(const char *networkId, char *bleMac, uint32_t len)
{
    if ((networkId == NULL) || (bleMac == NULL) || (len != MAC_LEN)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "invalid arg");
        return;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return;
    }
    NodeInfo *info = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get node info fail.");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return;
    }
    if (memcpy_s(info->connectInfo.bleMacAddr, MAC_LEN, bleMac, len) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "memcpy fail.");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return;
    }
    info->connectInfo.latestTime = GetCurrentTime();

    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
}

static int32_t DlGetAuthPort(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = LnnGetAuthPort(info);
    return SOFTBUS_OK;
}

static int32_t DlGetSessionPort(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = LnnGetSessionPort(info);
    return SOFTBUS_OK;
}

static int32_t DlGetProxyPort(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = LnnGetProxyPort(info);
    return SOFTBUS_OK;
}

static int32_t DlGetNetCap(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = info->netCapacity;
    return SOFTBUS_OK;
}

static int32_t DlGetFeatureCap(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != LNN_COMMON_LEN_64) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((uint64_t *)buf) = info->feature;
    return SOFTBUS_OK;
}

static int32_t DlGetNetType(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = info->discoveryType;
    return SOFTBUS_OK;
}

static int32_t DlGetMasterWeight(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;

    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    *((int32_t *)buf) = info->masterWeight;
    return SOFTBUS_OK;
}

static int32_t DlGetP2pMac(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *mac = NULL;

    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if ((!LnnIsNodeOnline(info)) && (!info->metaInfo.isMetaNode)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    mac = LnnGetP2pMac(info);
    if (mac == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get p2p mac fail");
        return SOFTBUS_ERR;
    }
    if (strcpy_s(buf, len, mac) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy p2p mac to buf fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetNodeAddr(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (!LnnIsNodeOnline(info)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }

    if (strcpy_s(buf, len, info->nodeAddress) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy node addr to buf fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetP2pGoMac(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *mac = NULL;

    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if ((!LnnIsNodeOnline(info)) && (!info->metaInfo.isMetaNode)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    mac = LnnGetP2pGoMac(info);
    if (mac == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get p2p go mac fail");
        return SOFTBUS_ERR;
    }
    if (strcpy_s(buf, len, mac) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy p2p go mac to buf fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetWifiCfg(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *wifiCfg = NULL;

    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if ((!LnnIsNodeOnline(info)) && (!info->metaInfo.isMetaNode)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    wifiCfg = LnnGetWifiCfg(info);
    if (wifiCfg == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get wifi cfg fail");
        return SOFTBUS_ERR;
    }
    if (strcpy_s(buf, len, wifiCfg) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy wifi cfg to buf fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetChanList5g(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    const char *chanList5g = NULL;

    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if ((!LnnIsNodeOnline(info)) && (!info->metaInfo.isMetaNode)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    chanList5g = LnnGetChanList5g(info);
    if (chanList5g == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get chan list 5g fail");
        return SOFTBUS_ERR;
    }
    if (strcpy_s(buf, len, chanList5g) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy chan list 5g to buf fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t DlGetP2pRole(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;

    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if ((!LnnIsNodeOnline(info)) && (!info->metaInfo.isMetaNode)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    *((int32_t *)buf) = LnnGetP2pRole(info);
    return SOFTBUS_OK;
}

static int32_t DlGetStateVersion(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;

    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (!LnnIsNodeOnline(info)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    *((int32_t *)buf) = info->stateVersion;
    return SOFTBUS_OK;
}

static int32_t DlGetStaFrequency(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;

    if (len != LNN_COMMON_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if ((!LnnIsNodeOnline(info)) && (!info->metaInfo.isMetaNode)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    *((int32_t *)buf) = LnnGetStaFrequency(info);
    return SOFTBUS_OK;
}

static int32_t DlGetNodeDataChangeFlag(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;

    if (len != DATA_CHANGE_FLAG_BUF_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (!LnnIsNodeOnline(info)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node is offline");
        return SOFTBUS_ERR;
    }
    *((int16_t *)buf) = LnnGetDataChangeFlag(info);
    return SOFTBUS_OK;
}

static int32_t DlGetNodeTlvNegoFlag(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != sizeof(bool)) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (!LnnIsNodeOnline(info)) {
        LLOGE("node is offline");
        return SOFTBUS_ERR;
    }
    *((bool *)buf) = IsFeatureSupport(info->feature, BIT_WIFI_DIRECT_TLV_NEGOTIATION);
    return SOFTBUS_OK;
}

static int32_t DlGetAccountHash(const char *networkId, void *buf, uint32_t len)
{
    NodeInfo *info = NULL;
    if (len != SHA_256_HASH_LEN) {
        return SOFTBUS_INVALID_PARAM;
    }
    RETURN_IF_GET_NODE_VALID(networkId, buf, info);
    if (!LnnIsNodeOnline(info)) {
        LLOGE("node is offline");
        return SOFTBUS_ERR;
    }
    if (memcpy_s(buf, len, info->accountHash, SHA_256_HASH_LEN) != EOK) {
        LLOGE("memcpy account hash fail");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static DistributedLedgerKey g_dlKeyTable[] = {
    {STRING_KEY_HICE_VERSION, DlGetNodeSoftBusVersion},
    {STRING_KEY_DEV_UDID, DlGetDeviceUdid},
    {STRING_KEY_UUID, DlGetDeviceUuid},
    {STRING_KEY_DEV_TYPE, DlGetDeviceType},
    {STRING_KEY_DEV_NAME, DlGetDeviceName},
    {STRING_KEY_BT_MAC, DlGetBtMac},
    {STRING_KEY_WLAN_IP, DlGetWlanIp},
    {STRING_KEY_MASTER_NODE_UDID, DlGetMasterUdid},
    {STRING_KEY_P2P_MAC, DlGetP2pMac},
    {STRING_KEY_WIFI_CFG, DlGetWifiCfg},
    {STRING_KEY_CHAN_LIST_5G, DlGetChanList5g},
    {STRING_KEY_P2P_GO_MAC, DlGetP2pGoMac},
    {STRING_KEY_NODE_ADDR, DlGetNodeAddr},
    {STRING_KEY_OFFLINE_CODE, DlGetDeviceOfflineCode},
    {STRING_KEY_BLE_MAC, DlGetNodeBleMac},
    {NUM_KEY_META_NODE, DlGetAuthType},
    {NUM_KEY_SESSION_PORT, DlGetSessionPort},
    {NUM_KEY_AUTH_PORT, DlGetAuthPort},
    {NUM_KEY_PROXY_PORT, DlGetProxyPort},
    {NUM_KEY_NET_CAP, DlGetNetCap},
    {NUM_KEY_FEATURE_CAPA, DlGetFeatureCap},
    {NUM_KEY_DISCOVERY_TYPE, DlGetNetType},
    {NUM_KEY_MASTER_NODE_WEIGHT, DlGetMasterWeight},
    {NUM_KEY_STA_FREQUENCY, DlGetStaFrequency},
    {NUM_KEY_P2P_ROLE, DlGetP2pRole},
    {NUM_KEY_STATE_VERSION, DlGetStateVersion},
    {NUM_KEY_DATA_CHANGE_FLAG, DlGetNodeDataChangeFlag},
    {NUM_KEY_DEV_TYPE_ID, DlGetDeviceTypeId},
    {BOOL_KEY_TLV_NEGOTIATION, DlGetNodeTlvNegoFlag},
    {BYTE_KEY_ACCOUNT_HASH, DlGetAccountHash},
};

static char *CreateCnnCodeKey(const char *uuid, DiscoveryType type)
{
    if (uuid == NULL || strlen(uuid) >= UUID_BUF_LEN) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return NULL;
    }
    char *key = (char *)SoftBusCalloc(INT_TO_STR_SIZE + UUID_BUF_LEN);
    if (key == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "SoftBusCalloc fail!");
        return NULL;
    }
    if (sprintf_s(key, INT_TO_STR_SIZE + UUID_BUF_LEN, "%d%s", type, uuid) == -1) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "type convert char error!");
        goto EXIT_FAIL;
    }
    return key;
EXIT_FAIL:
    SoftBusFree(key);
    return NULL;
}

static void DestroyCnnCodeKey(char *key)
{
    if (key == NULL) {
        return;
    }
    SoftBusFree(key);
}


static int32_t AddCnnCode(Map *cnnCode, const char *uuid, DiscoveryType type, int64_t authSeqNum)
{
    short seq = (short)authSeqNum;
    char *key = CreateCnnCodeKey(uuid, type);
    if (key == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "CreateCnnCodeKey error!");
        return SOFTBUS_ERR;
    }
    if (LnnMapSet(cnnCode, key, (void *)&seq, sizeof(short)) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnMapSet error!");
        DestroyCnnCodeKey(key);
        return SOFTBUS_ERR;
    }
    DestroyCnnCodeKey(key);
    return SOFTBUS_OK;
}

static void RemoveCnnCode(Map *cnnCode, const char *uuid, DiscoveryType type)
{
    char *key = CreateCnnCodeKey(uuid, type);
    if (key == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "CreateCnnCodeKey error!");
        return;
    }
    if (LnnMapErase(cnnCode, key) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnMapErase error!");
    }
    DestroyCnnCodeKey(key);
    return;
}

NO_SANITIZE("cfi") short LnnGetCnnCode(const char *uuid, DiscoveryType type)
{
    char *key = CreateCnnCodeKey(uuid, type);
    if (key == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "CreateCnnCodeKey error!");
        return INVALID_CONNECTION_CODE_VALUE;
    }
    short *ptr = (short *)LnnMapGet(&g_distributedNetLedger.cnnCode.connectionCode, key);
    if (ptr == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, " KEY not exist.");
        DestroyCnnCodeKey(key);
        return INVALID_CONNECTION_CODE_VALUE;
    }
    DestroyCnnCodeKey(key);
    return (*ptr);
}

static void MergeLnnInfo(const NodeInfo *oldInfo, NodeInfo *info)
{
    int32_t i;

    for (i = 0; i < CONNECTION_ADDR_MAX; ++i) {
        info->relation[i] += oldInfo->relation[i];
        info->relation[i] &= LNN_RELATION_MASK;
        if (oldInfo->authChannelId[i] != 0) {
            info->authChannelId[i] = oldInfo->authChannelId[i];
        }
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO,
            "Update authChannelId: %d, addrType=%d.", info->authChannelId[i], i);
    }
}

static void UpdateAuthSeq(const NodeInfo *oldInfo, NodeInfo *info)
{
    DiscoveryType type;
    for (type = DISCOVERY_TYPE_WIFI; type < DISCOVERY_TYPE_P2P; type++) {
        if (LnnHasDiscoveryType(info, type)) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO,
                "UpdateAuthSeq: authSeq=%" PRId64 ", type=%d.", info->authSeq[type], type);
            continue;
        }
        info->authSeq[type] = oldInfo->authSeq[type];
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO,
            "UpdateAuthSeq: authSeq=%" PRId64 ", type=%d.", info->authSeq[type], type);
    }
}

NO_SANITIZE("cfi") int32_t LnnUpdateNodeInfo(NodeInfo *newInfo)
{
    const char *udid = NULL;
    DoubleHashMap *map = NULL;
    NodeInfo *oldInfo = NULL;

    udid = LnnGetDeviceUdid(newInfo);
    map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    oldInfo = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (oldInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "no online node newInfo!");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    if (LnnHasDiscoveryType(newInfo, DISCOVERY_TYPE_WIFI) ||
        LnnHasDiscoveryType(newInfo, DISCOVERY_TYPE_LSA)) {
        oldInfo->discoveryType = newInfo->discoveryType | oldInfo->discoveryType;
        oldInfo->connectInfo.authPort = newInfo->connectInfo.authPort;
        oldInfo->connectInfo.proxyPort = newInfo->connectInfo.proxyPort;
        oldInfo->connectInfo.sessionPort = newInfo->connectInfo.sessionPort;
    }
    if (strcpy_s(oldInfo->deviceInfo.deviceName, DEVICE_NAME_BUF_LEN, newInfo->deviceInfo.deviceName) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "strcpy_s fail");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnAddMetaInfo(NodeInfo *info)
{
    const char *udid = NULL;
    DoubleHashMap *map = NULL;
    NodeInfo *oldInfo = NULL;
    udid = LnnGetDeviceUdid(info);
    map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnAddMetaInfo lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    oldInfo = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (oldInfo != NULL && strcmp(oldInfo->networkId, info->networkId) == 0) {
        MetaInfo temp = info->metaInfo;
        if (memcpy_s(info, sizeof(NodeInfo), oldInfo, sizeof(NodeInfo)) != EOK) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnAddMetaInfo copy fail!");
            SoftBusMutexUnlock(&g_distributedNetLedger.lock);
            return SOFTBUS_MEM_ERR;
        }
        info->metaInfo.isMetaNode = true;
        info->metaInfo.metaDiscType = info->metaInfo.metaDiscType | temp.metaDiscType;
    }
    LnnSetAuthTypeValue(&info->AuthTypeValue, ONLINE_METANODE);
    int32_t ret = LnnMapSet(&map->udidMap, udid, info, sizeof(NodeInfo));
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lnn map set failed, ret=%d", ret);
    }
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnAddMetaInfo success");
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnDeleteMetaInfo(const char *udid, ConnectionAddrType type)
{
    NodeInfo *info = NULL;
    DiscoveryType discType = LnnConvAddrTypeToDiscType(type);
    if (discType == DISCOVERY_TYPE_COUNT) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "DeleteMetaInfo type error fail!");
        return SOFTBUS_ERR;
    }
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "DeleteAddMetaInfo lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    info = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "DeleteAddMetaInfo para error!");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    info->metaInfo.metaDiscType = (uint32_t)info->metaInfo.metaDiscType & ~(1 << (uint32_t)discType);
    if (info->metaInfo.metaDiscType == 0) {
        info->metaInfo.isMetaNode = false;
    }
    LnnClearAuthTypeValue(&info->AuthTypeValue, ONLINE_METANODE);
    int32_t ret = LnnMapSet(&map->udidMap, udid, info, sizeof(NodeInfo));
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lnn map set failed, ret=%d", ret);
    }
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "LnnDeleteMetaInfo success");
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

static void UpdateNewNodeAccountHash(NodeInfo *info)
{
    char accountString[LONG_TO_STRING_MAX_LEN] = {0};
    if (sprintf_s(accountString, LONG_TO_STRING_MAX_LEN, "%" PRId64, info->accountId) == -1) {
        LLOGE("long to string fail");
        return;
    }
    LLOGD("account string:%s", accountString);
    int ret = SoftBusGenerateStrHash((uint8_t *)accountString,
        strlen(accountString), (unsigned char *)info->accountHash);
    if (ret != SOFTBUS_OK) {
        LLOGE("account hash fail,ret:%d", ret);
        return;
    }
}

static void OnlinePreventBrConnection(const NodeInfo *info)
{
    const NodeInfo *localNodeInfo = LnnGetLocalNodeInfo();
    if (localNodeInfo == NULL) {
        LLOGE("get local node info fail");
        return;
    }
    ConnectOption option = {0};
    option.type = CONNECT_BR;
    if (strcpy_s(option.brOption.brMac, BT_MAC_LEN, info->connectInfo.macAddr) != EOK) {
        LLOGE("copy br mac fail");
        return;
    }

    bool preventFlag = false;
    do {
        LLOGI("check the ble start timestamp, local:%"PRId64", peer:%"PRId64"",
            localNodeInfo->bleStartTimestamp, info->bleStartTimestamp);
        if (localNodeInfo->bleStartTimestamp < info->bleStartTimestamp) {
            LLOGI("peer later, prevent br connection");
            preventFlag = true;
            break;
        }
        if (localNodeInfo->bleStartTimestamp > info->bleStartTimestamp) {
            LLOGI("local later, do not prevent br connection");
            break;
        }
        if (strcmp(info->softBusVersion, SOFTBUS_VERSION_FOR_INITCONNECTFLAG) < 0) {
            LLOGI("peer is old version, peerVersion:%s", info->softBusVersion);
            preventFlag = true;
            break;
        }
        if (strcmp(info->networkId, localNodeInfo->networkId) <= 0) {
            LLOGI("peer network id is smaller");
            preventFlag = true;
            break;
        }
    } while (false);
    if (preventFlag) {
        LLOGI("prevent br connection for a while");
        ConnPreventConnection(&option, CONNECTION_FREEZE_TIMEOUT_MILLIS);
    }
}

static void NotifyMigrateUpgrade(NodeInfo *info)
{
    NodeBasicInfo basic;
    (void)memset_s(&basic, sizeof(NodeBasicInfo), 0, sizeof(NodeBasicInfo));
    if (LnnGetBasicInfoByUdid(info->deviceInfo.deviceUdid, &basic) == SOFTBUS_OK) {
        LnnNotifyMigrate(true, &basic);
    } else {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "NotifyMigrateUpgrade,GetBasicInfoByUdid fail!");
    }
}

static void FilterWifiInfo(NodeInfo *info)
{
    (void)LnnClearDiscoveryType(info, DISCOVERY_TYPE_WIFI);
    info->authChannelId[CONNECTION_ADDR_WLAN] = 0;
}

static void FilterBrInfo(NodeInfo *info)
{
    (void)LnnClearDiscoveryType(info, DISCOVERY_TYPE_BR);
    info->authChannelId[CONNECTION_ADDR_BR] = 0;
}

static void BleDirectlyOnlineProc(NodeInfo *info)
{
    if (!LnnHasDiscoveryType(info, DISCOVERY_TYPE_BLE)) {
        return;
    }
    if (LnnHasDiscoveryType(info, DISCOVERY_TYPE_WIFI)) {
        FilterWifiInfo(info);
    }
    if (LnnHasDiscoveryType(info, DISCOVERY_TYPE_BR)) {
        FilterBrInfo(info);
    }
    if (LnnSaveRemoteDeviceInfo(info) != SOFTBUS_OK) {
        LLOGE("save remote devInfo fail");
        return;
    }
}

static void NodeOnlineProc(NodeInfo *info)
{
    NodeInfo nodeInfo;
    if (memcpy_s(&nodeInfo, sizeof(nodeInfo), info, sizeof(NodeInfo)) != EOK) {
        return;
    }
    BleDirectlyOnlineProc(&nodeInfo);
}

NO_SANITIZE("cfi") ReportCategory LnnAddOnlineNode(NodeInfo *info)
{
    // judge map
    info->onlinetTimestamp = LnnUpTimeMs();
    if (info == NULL) {
        return REPORT_NONE;
    }
    const char *udid = NULL;
    DoubleHashMap *map = NULL;
    NodeInfo *oldInfo = NULL;
    bool isOffline = true;
    bool oldWifiFlag = false;
    bool oldBrFlag = false;
    bool oldBleFlag = false;
    bool isChanged = false;
    bool isMigrateEvent = false;
    bool isNetworkChanged = false;
    bool newWifiFlag = LnnHasDiscoveryType(info, DISCOVERY_TYPE_WIFI);
    bool newBleBrFlag = LnnHasDiscoveryType(info, DISCOVERY_TYPE_BLE)
        || LnnHasDiscoveryType(info, DISCOVERY_TYPE_BR);
    if (LnnHasDiscoveryType(info, DISCOVERY_TYPE_BR)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "DiscoveryType = BR.");
        AddCnnCode(&g_distributedNetLedger.cnnCode.connectionCode, info->uuid, DISCOVERY_TYPE_BR,
            info->authSeqNum);
    }

    udid = LnnGetDeviceUdid(info);
    map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return REPORT_NONE;
    }
    oldInfo = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (oldInfo != NULL) {
        info->metaInfo = oldInfo->metaInfo;
        oldInfo->groupType = info->groupType;
    }
    if (oldInfo != NULL && LnnIsNodeOnline(oldInfo)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "addOnlineNode find online node");
        UpdateAuthSeq(oldInfo, info);
        isOffline = false;
        isChanged = IsNetworkIdChanged(info, oldInfo);
        oldWifiFlag = LnnHasDiscoveryType(oldInfo, DISCOVERY_TYPE_WIFI);
        oldBleFlag = LnnHasDiscoveryType(oldInfo, DISCOVERY_TYPE_BLE);
        oldBrFlag = LnnHasDiscoveryType(oldInfo, DISCOVERY_TYPE_BR);
        if ((oldBleFlag || oldBrFlag) && newWifiFlag) {
            NewWifiDiscovered(oldInfo, info);
            isNetworkChanged = true;
        } else if (oldWifiFlag && newBleBrFlag) {
            RetainOfflineCode(oldInfo, info);
            NewBrBleDiscovered(oldInfo, info);
            isNetworkChanged = true;
        } else {
            RetainOfflineCode(oldInfo, info);
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "flag error");
        }
        if ((oldBleFlag || oldBrFlag) && !oldWifiFlag && newWifiFlag) {
            isMigrateEvent = true;
        }
        // update lnn discovery type
        info->discoveryType |= oldInfo->discoveryType;
        info->heartbeatTimeStamp = oldInfo->heartbeatTimeStamp;
        MergeLnnInfo(oldInfo, info);
        UpdateProfile(info);
    }
    LnnSetNodeConnStatus(info, STATUS_ONLINE);
    LnnSetAuthTypeValue(&info->AuthTypeValue, ONLINE_HICHAIN);
    UpdateNewNodeAccountHash(info);
    int32_t ret = LnnMapSet(&map->udidMap, udid, info, sizeof(NodeInfo));
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lnn map set failed, ret=%d", ret);
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    NodeOnlineProc(info);
    if (isNetworkChanged) {
        UpdateNetworkInfo(info->deviceInfo.deviceUdid);
    }
    if (isOffline) {
        if (!oldWifiFlag && !newWifiFlag && newBleBrFlag) {
            OnlinePreventBrConnection(info);
        }
        InsertToProfile(info);
        return REPORT_ONLINE;
    }
    if (isMigrateEvent) {
        NotifyMigrateUpgrade(info);
    }
    if (isChanged) {
        return REPORT_CHANGE;
    }
    return REPORT_NONE;
}

NO_SANITIZE("cfi") int32_t LnnUpdateAccountInfo(const NodeInfo *info)
{
    if (info == NULL) {
        LLOGE("info is null");
    }
    const char *udid = NULL;
    DoubleHashMap *map = NULL;
    NodeInfo *oldInfo = NULL;
    udid = LnnGetDeviceUdid(info);
    map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail!");
        return REPORT_NONE;
    }
    oldInfo = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (oldInfo != NULL) {
        oldInfo->accountId = info->accountId;
        UpdateNewNodeAccountHash(oldInfo);
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnUpdateGroupType(const NodeInfo *info)
{
    if (info == NULL) {
        LLOGE("info is null");
        return SOFTBUS_ERR;
    }
    const char *udid = NULL;
    DoubleHashMap *map = NULL;
    NodeInfo *oldInfo = NULL;
    udid = LnnGetDeviceUdid(info);
    int32_t groupType = AuthGetGroupType(udid, info->uuid);
    LLOGI("groupType = %d", groupType);
    int32_t ret = SOFTBUS_ERR;
    map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail!");
        return REPORT_NONE;
    }
    oldInfo = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (oldInfo != NULL) {
        oldInfo->groupType = groupType;
        ret = SOFTBUS_OK;
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return ret;
}

static void NotifyMigrateDegrade(const char *udid)
{
    NodeBasicInfo basic;
    (void)memset_s(&basic, sizeof(NodeBasicInfo), 0, sizeof(NodeBasicInfo));
    if (LnnGetBasicInfoByUdid(udid, &basic) == SOFTBUS_OK) {
        LnnNotifyMigrate(false, &basic);
    } else {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "NotifyMigrateDegrade,GetBasicInfoByUdid fail!");
    }
}

NO_SANITIZE("cfi") ReportCategory LnnSetNodeOffline(const char *udid, ConnectionAddrType type, int32_t authId)
{
    NodeInfo *info = NULL;

    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return REPORT_NONE;
    }
    info = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "PARA ERROR!");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return REPORT_NONE;
    }
    if (type != CONNECTION_ADDR_MAX && info->relation[type] > 0) {
        info->relation[type]--;
    }
    if (LnnHasDiscoveryType(info, DISCOVERY_TYPE_BR) && LnnConvAddrTypeToDiscType(type) == DISCOVERY_TYPE_BR) {
        RemoveCnnCode(&g_distributedNetLedger.cnnCode.connectionCode, info->uuid, DISCOVERY_TYPE_BR);
    }
    if (LnnHasDiscoveryType(info, DISCOVERY_TYPE_WIFI) && LnnConvAddrTypeToDiscType(type) == DISCOVERY_TYPE_WIFI &&
        info->authChannelId[type] != authId) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "authChannelId != authId, not need to report offline.");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return REPORT_NONE;
    }
    info->authChannelId[type] = 0;
    if (LnnConvAddrTypeToDiscType(type) == DISCOVERY_TYPE_WIFI) {
        LnnSetWiFiIp(info, LOCAL_IP);
    }
    LnnClearDiscoveryType(info, LnnConvAddrTypeToDiscType(type));
    if (info->discoveryType != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "discoveryType=%u after clear, not need to report offline.",
            info->discoveryType);
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        UpdateNetworkInfo(udid);
        if (type == CONNECTION_ADDR_WLAN) {
            NotifyMigrateDegrade(udid);
        }
        return REPORT_NONE;
    }
    if (!LnnIsNodeOnline(info)) {
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "the state is already offline, no need to report offline.");
        return REPORT_NONE;
    }
    LnnSetNodeConnStatus(info, STATUS_OFFLINE);
    LnnClearAuthTypeValue(&info->AuthTypeValue, ONLINE_HICHAIN);
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "need to report offline.");
    return REPORT_OFFLINE;
}

NO_SANITIZE("cfi") int32_t LnnGetBasicInfoByUdid(const char *udid, NodeBasicInfo *basicInfo)
{
    if (udid == NULL || basicInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "PARA ERROR!");
        return SOFTBUS_INVALID_PARAM;
    }
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *info = (NodeInfo *)LnnMapGet(&map->udidMap, udid);
    int32_t ret = ConvertNodeInfoToBasicInfo(info, basicInfo);
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return ret;
}

NO_SANITIZE("cfi") void LnnRemoveNode(const char *udid)
{
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    if (udid == NULL) {
        return;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return;
    }
    LnnMapErase(&map->udidMap, udid);
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
}

NO_SANITIZE("cfi") const char *LnnConvertDLidToUdid(const char *id, IdCategory type)
{
    NodeInfo *info = NULL;
    if (id == NULL) {
        return NULL;
    }
    info = LnnGetNodeInfoById(id, type);
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "uuid not find node info.");
        return NULL;
    }
    return LnnGetDeviceUdid(info);
}

NO_SANITIZE("cfi") int32_t LnnConvertDlId(const char *srcId, IdCategory srcIdType, IdCategory dstIdType,
    char *dstIdBuf, uint32_t dstIdBufLen)
{
    NodeInfo *info = NULL;
    const char *id = NULL;
    int32_t rc = SOFTBUS_OK;

    if (srcId == NULL || dstIdBuf == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail");
        return SOFTBUS_LOCK_ERR;
    }
    info = LnnGetNodeInfoById(srcId, srcIdType);
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "no node info for: %d", srcIdType);
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_NOT_FIND;
    }
    switch (dstIdType) {
        case CATEGORY_UDID:
            id = info->deviceInfo.deviceUdid;
            break;
        case CATEGORY_UUID:
            id = info->uuid;
            break;
        case CATEGORY_NETWORK_ID:
            id = info->networkId;
            break;
        default:
            SoftBusMutexUnlock(&g_distributedNetLedger.lock);
            return SOFTBUS_INVALID_PARAM;
    }
    if (strcpy_s(dstIdBuf, dstIdBufLen, id) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy id fail");
        rc = SOFTBUS_MEM_ERR;
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return rc;
}

NO_SANITIZE("cfi") int32_t LnnGetLnnRelation(const char *id, IdCategory type, uint8_t *relation, uint32_t len)
{
    NodeInfo *info = NULL;

    if (id == NULL || relation == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail");
        return SOFTBUS_LOCK_ERR;
    }
    info = LnnGetNodeInfoById(id, type);
    if (info == NULL || !LnnIsNodeOnline(info)) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "node not online");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_NOT_FIND;
    }
    if (memcpy_s(relation, len, info->relation, CONNECTION_ADDR_MAX) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "copy relation fail");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") bool LnnSetDLDeviceInfoName(const char *udid, const char *name)
{
    DoubleHashMap *map = &g_distributedNetLedger.distributedInfo;
    NodeInfo *info = NULL;
    if (udid == NULL || name == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "para error!");
        return false;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return false;
    }
    info = GetNodeInfoFromMap(map, udid);
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "udid not exist !");
        goto EXIT;
    }
    if (strcmp(LnnGetDeviceName(&info->deviceInfo), name) == 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "devicename not change!");
        SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return true;
    }
    if (LnnSetDeviceName(&info->deviceInfo, name) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "set device name error!");
        goto EXIT;
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return true;
EXIT:
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return false;
}

NO_SANITIZE("cfi") bool LnnSetDLP2pInfo(const char *networkId, const P2pInfo *info)
{
    NodeInfo *node = NULL;
    if (networkId == NULL || info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "%s:invalid param.", __func__);
        return false;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail.");
        return false;
    }
    node = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (node == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "udid not found.");
        goto EXIT;
    }
    if (LnnSetP2pRole(node, info->p2pRole) != SOFTBUS_OK ||
        LnnSetP2pMac(node, info->p2pMac) != SOFTBUS_OK ||
        LnnSetP2pGoMac(node, info->goMac) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "set p2p info fail.");
        goto EXIT;
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return true;
EXIT:
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return false;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteStrInfo(const char *networkId, InfoKey key, char *info, uint32_t len)
{
    uint32_t i;
    int32_t ret;
    if (!IsValidString(networkId, ID_MAX_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "info is null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (key >= STRING_KEY_END) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    for (i = 0; i < sizeof(g_dlKeyTable) / sizeof(DistributedLedgerKey); i++) {
        if (key == g_dlKeyTable[i].key) {
            if (g_dlKeyTable[i].getInfo != NULL) {
                ret = g_dlKeyTable[i].getInfo(networkId, (void *)info, len);
                SoftBusMutexUnlock(&g_distributedNetLedger.lock);
                return ret;
            }
        }
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY NOT exist.");
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteNumInfo(const char *networkId, InfoKey key, int32_t *info)
{
    uint32_t i;
    int32_t ret;
    if (!IsValidString(networkId, ID_MAX_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "info is null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (key < NUM_KEY_BEGIN || key >= NUM_KEY_END) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    for (i = 0; i < sizeof(g_dlKeyTable) / sizeof(DistributedLedgerKey); i++) {
        if (key == g_dlKeyTable[i].key) {
            if (g_dlKeyTable[i].getInfo != NULL) {
                ret = g_dlKeyTable[i].getInfo(networkId, (void *)info, LNN_COMMON_LEN);
                SoftBusMutexUnlock(&g_distributedNetLedger.lock);
                return ret;
            }
        }
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY NOT exist.");
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteNumU64Info(const char *networkId, InfoKey key, uint64_t *info)
{
    uint32_t i;
    int32_t ret;
    if (!IsValidString(networkId, ID_MAX_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "info is null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (key < NUM_KEY_BEGIN || key >= NUM_KEY_END) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    for (i = 0; i < sizeof(g_dlKeyTable) / sizeof(DistributedLedgerKey); i++) {
        if (key == g_dlKeyTable[i].key) {
            if (g_dlKeyTable[i].getInfo != NULL) {
                ret = g_dlKeyTable[i].getInfo(networkId, (void *)info, LNN_COMMON_LEN_64);
                SoftBusMutexUnlock(&g_distributedNetLedger.lock);
                return ret;
            }
        }
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY NOT exist.");
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteNum16Info(const char *networkId, InfoKey key, int16_t *info)
{
    uint32_t i;
    int32_t ret;
    if (!IsValidString(networkId, ID_MAX_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "info is null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (key < NUM_KEY_BEGIN || key >= NUM_KEY_END) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    for (i = 0; i < sizeof(g_dlKeyTable) / sizeof(DistributedLedgerKey); i++) {
        if (key == g_dlKeyTable[i].key) {
            if (g_dlKeyTable[i].getInfo != NULL) {
                ret = g_dlKeyTable[i].getInfo(networkId, (void *)info, sizeof(int16_t));
                SoftBusMutexUnlock(&g_distributedNetLedger.lock);
                return ret;
            }
        }
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "KEY NOT exist.");
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteBoolInfo(const char *networkId, InfoKey key, bool *info)
{
    uint32_t i;
    int32_t ret;
    if (!IsValidString(networkId, ID_MAX_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info == NULL) {
        LLOGE("info is null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (key < BOOL_KEY_BEGIN || key >= BOOL_KEY_END) {
        LLOGE("KEY error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    for (i = 0; i < sizeof(g_dlKeyTable) / sizeof(DistributedLedgerKey); i++) {
        if (key == g_dlKeyTable[i].key) {
            if (g_dlKeyTable[i].getInfo != NULL) {
                ret = g_dlKeyTable[i].getInfo(networkId, (void *)info, sizeof(bool));
                SoftBusMutexUnlock(&g_distributedNetLedger.lock);
                return ret;
            }
        }
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    LLOGE("KEY NOT exist.");
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetRemoteByteInfo(const char *networkId, InfoKey key, uint8_t *info, uint32_t len)
{
    uint32_t i;
    int32_t ret;
    if (!IsValidString(networkId, ID_MAX_LEN)) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info == NULL) {
        LLOGE("para error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (key < BYTE_KEY_BEGIN || key >= BYTE_KEY_END) {
        LLOGE("KEY error.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    for (i = 0; i < sizeof(g_dlKeyTable) / sizeof(DistributedLedgerKey); i++) {
        if (key == g_dlKeyTable[i].key) {
            if (g_dlKeyTable[i].getInfo != NULL) {
                ret = g_dlKeyTable[i].getInfo(networkId, info, len);
                SoftBusMutexUnlock(&g_distributedNetLedger.lock);
                return ret;
            }
        }
    }
    SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    LLOGE("KEY NOT exist.");
    return SOFTBUS_ERR;
}

static int32_t GetAllOnlineAndMetaNodeInfo(NodeBasicInfo **info, int32_t *infoNum, bool isNeedMeta)
{
    if (info == NULL || infoNum == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "key params are null");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    int32_t ret = SOFTBUS_ERR;
    do {
        *info = NULL;
        if (GetDLOnlineNodeNumLocked(infoNum, isNeedMeta) != SOFTBUS_OK) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get online node num failed");
            break;
        }
        if (*infoNum == 0) {
            ret = SOFTBUS_OK;
            break;
        }
        *info = SoftBusMalloc((*infoNum) * sizeof(NodeBasicInfo));
        if (*info == NULL) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "malloc node info buffer failed");
            break;
        }
        if (FillDLOnlineNodeInfoLocked(*info, *infoNum, isNeedMeta) != SOFTBUS_OK) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "fill online node num failed");
            break;
        }
        ret = SOFTBUS_OK;
    } while (false);
    if (ret != SOFTBUS_OK) {
        if (*info != NULL) {
            SoftBusFree(*info);
            *info = NULL;
        }
        *infoNum = 0;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return ret;
}

NO_SANITIZE("cfi") bool LnnIsLSANode(const NodeBasicInfo *info)
{
    NodeInfo *nodeInfo = LnnGetNodeInfoById(info->networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo != NULL && LnnHasDiscoveryType(nodeInfo, DISCOVERY_TYPE_LSA)) {
        return true;
    }
    return false;
}

int32_t LnnGetAllOnlineNodeNum(int32_t *nodeNum)
{
    if (nodeNum == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    /* node num include meta node */
    if (GetDLOnlineNodeNumLocked(nodeNum, true) != SOFTBUS_OK) {
        LLOGE("get online node num failed");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnGetAllOnlineNodeInfo(NodeBasicInfo **info, int32_t *infoNum)
{
    return GetAllOnlineAndMetaNodeInfo(info, infoNum, false);
}

NO_SANITIZE("cfi") int32_t LnnGetAllOnlineAndMetaNodeInfo(NodeBasicInfo **info, int32_t *infoNum)
{
    return GetAllOnlineAndMetaNodeInfo(info, infoNum, true);
}

NO_SANITIZE("cfi") int32_t LnnGetNetworkIdByBtMac(const char *btMac, char *buf, uint32_t len)
{
    if (btMac == NULL || btMac[0] == '\0' || buf == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "btMac is empty");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    MapIterator *it = LnnMapInitIterator(&g_distributedNetLedger.distributedInfo.udidMap);
    if (it == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "it is null");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    while (LnnMapHasNext(it)) {
        it = LnnMapNext(it);
        if (it == NULL) {
            (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
            return SOFTBUS_ERR;
        }
        NodeInfo *nodeInfo = (NodeInfo *)it->node->value;
        if ((LnnIsNodeOnline(nodeInfo) || nodeInfo->metaInfo.isMetaNode) &&
            StrCmpIgnoreCase(nodeInfo->connectInfo.macAddr, btMac) == 0) {
            if (strcpy_s(buf, len, nodeInfo->networkId) != EOK) {
                SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "strcpy_s networkId fail!");
            }
            LnnMapDeinitIterator(it);
            (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
            return SOFTBUS_OK;
        }
    }
    LnnMapDeinitIterator(it);
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetNetworkIdByUdidHash(const char *udidHash, char *buf, uint32_t len)
{
    if (udidHash == NULL || udidHash[0] == '\0' || buf == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "udidHash is empty");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    MapIterator *it = LnnMapInitIterator(&g_distributedNetLedger.distributedInfo.udidMap);
    if (it == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "it is null");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    char nodeUdidHash[SHA_256_HASH_LEN] = {0};
    while (LnnMapHasNext(it)) {
        it = LnnMapNext(it);
        if (it == NULL) {
            (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
            return SOFTBUS_ERR;
        }
        NodeInfo *nodeInfo = (NodeInfo *)it->node->value;
        if (LnnIsNodeOnline(nodeInfo) || nodeInfo->metaInfo.isMetaNode) {
            if (SoftBusGenerateStrHash((uint8_t*)nodeInfo->deviceInfo.deviceUdid,
                strlen(nodeInfo->deviceInfo.deviceUdid), (uint8_t*)nodeUdidHash) != SOFTBUS_OK) {
                continue;
            }
            if (memcmp(nodeUdidHash, udidHash, SHA_256_HASH_LEN) != 0) {
                continue;
            }
            if (strcpy_s(buf, len, nodeInfo->networkId) != EOK) {
                SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "strcpy_s networkId fail!");
            }
            LnnMapDeinitIterator(it);
            (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
            return SOFTBUS_OK;
        }
    }
    LnnMapDeinitIterator(it);
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_ERR;
}

NO_SANITIZE("cfi") int32_t LnnGetNetworkIdByUuid(const char *uuid, char *buf, uint32_t len)
{
    if (!IsValidString(uuid, ID_MAX_LEN)) {
        SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_ERROR, "uuid is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(uuid, CATEGORY_UUID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, nodeInfo->networkId, strlen(nodeInfo->networkId)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnGetNetworkIdByUdid(const char *udid, char *buf, uint32_t len)
{
    if (!IsValidString(udid, ID_MAX_LEN)) {
        SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_ERROR, "udid is invalid");
        return SOFTBUS_INVALID_PARAM;
    }

    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(udid, CATEGORY_UDID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    if (strncpy_s(buf, len, nodeInfo->networkId, strlen(nodeInfo->networkId)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "STR COPY ERROR!");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnGetAllAuthSeq(const char *udid, int64_t *authSeq, uint32_t num)
{
    if (!IsValidString(udid, ID_MAX_LEN) || authSeq == NULL || num != DISCOVERY_TYPE_COUNT) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "[offline]udid is invalid");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "[offline]lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(udid, CATEGORY_UDID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "[offline] get node info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    if (memcpy_s(authSeq, sizeof(int64_t) * num, nodeInfo->authSeq, sizeof(nodeInfo->authSeq)) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "[offline]memcpy_s authSeq fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    DiscoveryType type;
    for (type = DISCOVERY_TYPE_WIFI; type < DISCOVERY_TYPE_P2P; type++) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO,
            "[offline]LnnGetAllAuthSeq: authSeq=%" PRId64 ", type=%d.", authSeq[type], type);
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnGetDLOnlineTimestamp(const char *networkId, uint64_t *timestamp)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    *timestamp = nodeInfo->onlinetTimestamp;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnGetDLHeartbeatTimestamp(const char *networkId, uint64_t *timestamp)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    *timestamp = nodeInfo->heartbeatTimeStamp;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnSetDLHeartbeatTimestamp(const char *networkId, uint64_t timestamp)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    nodeInfo->heartbeatTimeStamp = timestamp;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnSetDLConnCapability(const char *networkId, uint32_t connCapability)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    nodeInfo->netCapacity = connCapability;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnSetDLBatteryInfo(const char *networkId, const BatteryInfo *info)
{
    if (networkId == NULL || info == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo == NULL) {
        LLOGE("get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    nodeInfo->batteryInfo.batteryLevel = info->batteryLevel;
    nodeInfo->batteryInfo.isCharging = info->isCharging;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnSetDLBssTransInfo(const char *networkId, const BssTransInfo *info)
{
    if (networkId == NULL || info == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        LLOGE("lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(networkId, CATEGORY_NETWORK_ID);
    if (nodeInfo == NULL) {
        LLOGE("get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    if (memcpy_s(&(nodeInfo->bssTransInfo), sizeof(BssTransInfo), info,
        sizeof(BssTransInfo)) != SOFTBUS_OK) {
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_MEM_ERR;
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnSetDLNodeAddr(const char *id, IdCategory type, const char *addr)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_LOCK_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(id, type);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    int ret = strcpy_s(nodeInfo->nodeAddress, sizeof(nodeInfo->nodeAddress), addr);
    if (ret != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "set node addr failed!ret=%d", ret);
    }
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return ret == EOK ? SOFTBUS_OK : SOFTBUS_ERR;
}

int32_t LnnSetDLProxyPort(const char *id, IdCategory type, int32_t proxyPort)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(id, type);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    nodeInfo->connectInfo.proxyPort = proxyPort;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

int32_t LnnSetDLSessionPort(const char *id, IdCategory type, int32_t sessionPort)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(id, type);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    nodeInfo->connectInfo.sessionPort = sessionPort;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

int32_t LnnSetDLAuthPort(const char *id, IdCategory type, int32_t authPort)
{
    if (SoftBusMutexLock(&g_distributedNetLedger.lock) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "lock mutex fail!");
        return SOFTBUS_ERR;
    }
    NodeInfo *nodeInfo = LnnGetNodeInfoById(id, type);
    if (nodeInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get info fail");
        (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
        return SOFTBUS_ERR;
    }
    nodeInfo->connectInfo.authPort = authPort;
    (void)SoftBusMutexUnlock(&g_distributedNetLedger.lock);
    return SOFTBUS_OK;
}

int32_t SoftBusDumpBusCenterRemoteDeviceInfo(int32_t fd)
{
    SOFTBUS_DPRINTF(fd, "-----RemoteDeviceInfo-----\n");
    NodeBasicInfo *remoteNodeInfo = NULL;
    int32_t infoNum = 0;
    if (LnnGetAllOnlineNodeInfo(&remoteNodeInfo, &infoNum) != 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "LnnGetAllOnlineNodeInfo failed!");
        return SOFTBUS_ERR;
    }
    SOFTBUS_DPRINTF(fd, "remote device num = %d\n", infoNum);
    for (int32_t i = 0; i < infoNum; i++) {
        SOFTBUS_DPRINTF(fd, "\n[NO.%d]\n", i + 1);
        SoftBusDumpBusCenterPrintInfo(fd, remoteNodeInfo + i);
    }
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") int32_t LnnInitDistributedLedger(void)
{
    if (g_distributedNetLedger.status == DL_INIT_SUCCESS) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "Distributed Ledger already init");
        return SOFTBUS_OK;
    }

    if (InitDistributedInfo(&g_distributedNetLedger.distributedInfo) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "InitDistributedInfo ERROR!");
        g_distributedNetLedger.status = DL_INIT_FAIL;
        return SOFTBUS_ERR;
    }

    if (InitConnectionCode(&g_distributedNetLedger.cnnCode) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "InitConnectionCode ERROR!");
        g_distributedNetLedger.status = DL_INIT_FAIL;
        return SOFTBUS_ERR;
    }
    if (SoftBusMutexInit(&g_distributedNetLedger.lock, NULL) != SOFTBUS_OK) {
        g_distributedNetLedger.status = DL_INIT_FAIL;
        return SOFTBUS_ERR;
    }
    if (SoftBusRegBusCenterVarDump(SOFTBUS_BUSCENTER_DUMP_REMOTEDEVICEINFO,
        &SoftBusDumpBusCenterRemoteDeviceInfo) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_CONN, SOFTBUS_LOG_ERROR, "SoftBusRegBusCenterVarDump regist fail");
        return SOFTBUS_ERR;
    }
    g_distributedNetLedger.status = DL_INIT_SUCCESS;
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") const NodeInfo *LnnGetOnlineNodeByUdidHash(const char *recvUdidHash)
{
    int32_t i;
    int32_t infoNum = 0;
    NodeBasicInfo *info = NULL;
    unsigned char shortUdidHash[SHORT_UDID_HASH_LEN + 1] = {0};

    if (LnnGetAllOnlineNodeInfo(&info, &infoNum) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "get all online node info fail");
        return NULL;
    }
    if (info == NULL || infoNum == 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "none online node");
        if (info != NULL) {
            SoftBusFree(info);
        }
        return NULL;
    }
    for (i = 0; i < infoNum; ++i) {
        const NodeInfo *nodeInfo = LnnGetNodeInfoById(info[i].networkId, CATEGORY_NETWORK_ID);
        if (nodeInfo == NULL) {
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "nodeInfo is null.");
            continue;
        }
        if (GenerateStrHashAndConvertToHexString((const unsigned char *)nodeInfo->deviceInfo.deviceUdid,
            SHORT_UDID_HASH_LEN, shortUdidHash, SHORT_UDID_HASH_LEN + 1) != SOFTBUS_OK) {
            continue;
        }
        if (memcmp(shortUdidHash, recvUdidHash, SHORT_UDID_HASH_LEN) == 0) {
            char *anoyUdid = NULL;
            SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "node udid:%s, shortUdidHash:%s is online",
                ToSecureStrDeviceID(nodeInfo->deviceInfo.deviceUdid, &anoyUdid),
                AnonymizesUDID((const char *)shortUdidHash));
            SoftBusFree(anoyUdid);
            SoftBusFree(info);
            return nodeInfo;
        }
    }
    SoftBusFree(info);
    return NULL;
}

static void RefreshDeviceInfoByDevId(DeviceInfo *device, const InnerDeviceInfoAddtions *addtions)
{
    if (addtions->medium != COAP) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "parameter error");
        return;
    }
    unsigned char shortUdidHash[SHORT_UDID_HASH_LEN + 1] = {0};
    if (GenerateStrHashAndConvertToHexString((const unsigned char *)device->devId,
        SHORT_UDID_HASH_LEN, shortUdidHash, SHORT_UDID_HASH_LEN + 1) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "refresh device info get short udid hash fail");
        return;
    }
    if (memset_s(device->devId, DISC_MAX_DEVICE_ID_LEN, 0, DISC_MAX_DEVICE_ID_LEN) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "refresh device info memset_s fail");
        return;
    }
    if (memcpy_s(device->devId, DISC_MAX_DEVICE_ID_LEN, shortUdidHash, SHORT_UDID_HASH_LEN) != EOK) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "memcpy_s device short udid hash fail");
    }
}

static void RefreshDeviceOnlineStateInfo(DeviceInfo *device, const InnerDeviceInfoAddtions *addtions)
{
    if (addtions->medium == COAP) {
        device->isOnline = LnnGetOnlineStateById(device->devId, CATEGORY_UDID);
    }
    if (addtions->medium == BLE) {
        device->isOnline = ((LnnGetOnlineNodeByUdidHash(device->devId)) != NULL) ? true : false;
    }
}

NO_SANITIZE("cfi") void LnnRefreshDeviceOnlineStateAndDevIdInfo(const char *pkgName, DeviceInfo *device,
    const InnerDeviceInfoAddtions *addtions)
{
    (void)pkgName;
    RefreshDeviceOnlineStateInfo(device, addtions);
    RefreshDeviceInfoByDevId(device, addtions);
    SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_INFO, "device found by medium=%d, udidhash=%s, online status=%d",
        addtions->medium, AnonymizesUDID(device->devId), device->isOnline);
}
