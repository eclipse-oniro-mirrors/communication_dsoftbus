/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "lnn_lane_link.h"

#include <securec.h>

#include "bus_center_info_key.h"
#include "bus_center_manager.h"
#include "lnn_distributed_net_ledger.h"
#include "lnn_lane_def.h"
#include "lnn_lane_score.h"
#include "lnn_lane_link_p2p.h"
#include "lnn_local_net_ledger.h"
#include "lnn_net_capability.h"
#include "lnn_network_manager.h"
#include "lnn_node_info.h"
#include "lnn_physical_subnet_manager.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_crypto.h"
#include "softbus_conn_ble_connection.h"
#include "softbus_conn_ble_manager.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_log.h"
#include "softbus_network_utils.h"
#include "softbus_protocol_def.h"
#include "softbus_utils.h"
#include "softbus_def.h"

typedef int32_t (*LaneLinkByType)(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback);

static bool LinkTypeCheck(LaneLinkType type)
{
    static const LaneLinkType supportList[] = { LANE_P2P, LANE_WLAN_2P4G, LANE_WLAN_5G, LANE_BR, LANE_BLE,
        LANE_BLE_DIRECT, LANE_P2P_REUSE, LANE_COC, LANE_COC_DIRECT };
    uint32_t size = sizeof(supportList) / sizeof(LaneLinkType);
    for (uint32_t i = 0; i < size; i++) {
        if (supportList[i] == type) {
            return true;
        }
    }
    LLOGE("link type[%d] is not supported", type);
    return false;
}

static int32_t IsLinkRequestValid(const LinkRequest *reqInfo)
{
    if (reqInfo == NULL) {
        LLOGE("reqInfo is nullptr");
        return SOFTBUS_INVALID_PARAM;
    }
    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") static int32_t LaneLinkOfBr(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    LaneLinkInfo linkInfo;
    (void)memset_s(&linkInfo, sizeof(LaneLinkInfo), 0, sizeof(LaneLinkInfo));
    int32_t ret = LnnGetRemoteStrInfo(reqInfo->peerNetworkId, STRING_KEY_BT_MAC,
        linkInfo.linkInfo.br.brMac, BT_MAC_LEN);
    if (ret != SOFTBUS_OK || strlen(linkInfo.linkInfo.br.brMac) == 0) {
        LLOGE("LnnGetRemoteStrInfo brmac is failed");
        return SOFTBUS_ERR;
    }
    linkInfo.type = LANE_BR;
    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

typedef struct P2pAddrNode {
    ListNode node;
    char networkId[NETWORK_ID_BUF_LEN];
    char addr[MAX_SOCKET_ADDR_LEN];
    uint16_t port;
}P2pAddrNode;

static SoftBusList g_P2pAddrList;

static void LaneInitP2pAddrList()
{
    ListInit(&g_P2pAddrList.list);
    g_P2pAddrList.cnt = 0;
    SoftBusMutexInit(&g_P2pAddrList.lock, NULL);
}

void LaneDeleteP2pAddress(const char *networkId)
{
    P2pAddrNode* item = NULL;
    P2pAddrNode* nextItem = NULL;

    if ((SoftBusMutexLock(&g_P2pAddrList.lock) != SOFTBUS_OK) || (networkId == NULL)) {
        return;
    }
    LIST_FOR_EACH_ENTRY_SAFE(item, nextItem, &g_P2pAddrList.list, P2pAddrNode, node) {
        if (strcmp(item->networkId, networkId) == 0) {
            ListDelete(&item->node);
            SoftBusFree(item);
        }
    }
    SoftBusMutexUnlock(&g_P2pAddrList.lock);
}

void LaneAddP2pAddress(const char *networkId, const char *ipAddr, uint16_t port)
{
    P2pAddrNode* item = NULL;
    P2pAddrNode* nextItem = NULL;
    bool find = false;

    if (SoftBusMutexLock(&g_P2pAddrList.lock) != SOFTBUS_OK) {
        return;
    }

    LIST_FOR_EACH_ENTRY_SAFE(item, nextItem, &g_P2pAddrList.list, P2pAddrNode, node) {
        if (strcmp(item->networkId, networkId) == 0) {
            find = true;
            break;
        }
    }
    if (find) {
        if (strcpy_s(item->addr, MAX_SOCKET_ADDR_LEN, ipAddr) != EOK) {
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            return;
        }
        item->port = port;
    } else {
        P2pAddrNode *p2pAddrNode = (P2pAddrNode*)SoftBusMalloc(sizeof(P2pAddrNode));
        if (p2pAddrNode == NULL) {
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            return;
        }
        if (strcpy_s(p2pAddrNode->networkId, NETWORK_ID_BUF_LEN, networkId) != EOK) {
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            SoftBusFree(p2pAddrNode);
            return;
        }
        if (strcpy_s(p2pAddrNode->addr, MAX_SOCKET_ADDR_LEN, ipAddr) != EOK) {
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            SoftBusFree(p2pAddrNode);
            return;
        }
        p2pAddrNode->port = port;
        ListAdd(&g_P2pAddrList.list, &p2pAddrNode->node);
    }

    SoftBusMutexUnlock(&g_P2pAddrList.lock);
}

void LaneAddP2pAddressByIp(const char *ipAddr, uint16_t port)
{
    P2pAddrNode* item = NULL;
    P2pAddrNode* nextItem = NULL;
    bool find = false;

    if (SoftBusMutexLock(&g_P2pAddrList.lock) != SOFTBUS_OK) {
        return;
    }

    LIST_FOR_EACH_ENTRY_SAFE(item, nextItem, &g_P2pAddrList.list, P2pAddrNode, node) {
        if (strcmp(item->addr, ipAddr) == 0) {
            find = true;
            break;
        }
    }
    if (find) {
        item->port = port;
    } else {
        P2pAddrNode *p2pAddrNode = (P2pAddrNode*)SoftBusMalloc(sizeof(P2pAddrNode));
        if (p2pAddrNode == NULL) {
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            return;
        }
        if (strcpy_s(p2pAddrNode->addr, MAX_SOCKET_ADDR_LEN, ipAddr) != EOK) {
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            SoftBusFree(p2pAddrNode);
            return;
        }
        p2pAddrNode->networkId[0] = 0;
        p2pAddrNode->port = port;
        ListAdd(&g_P2pAddrList.list, &p2pAddrNode->node);
    }

    SoftBusMutexUnlock(&g_P2pAddrList.lock);
}

void LaneUpdateP2pAddressByIp(const char *ipAddr, const char *networkId)
{
    P2pAddrNode* item = NULL;
    P2pAddrNode* nextItem = NULL;

    if (SoftBusMutexLock(&g_P2pAddrList.lock) != SOFTBUS_OK) {
        return;
    }

    LIST_FOR_EACH_ENTRY_SAFE(item, nextItem, &g_P2pAddrList.list, P2pAddrNode, node) {
        if (strcmp(item->addr, ipAddr) == 0) {
            if (strcpy_s(item->networkId, NETWORK_ID_BUF_LEN, networkId) != EOK) {
                SoftBusMutexUnlock(&g_P2pAddrList.lock);
                return;
            }
        }
    }
    SoftBusMutexUnlock(&g_P2pAddrList.lock);
}

static bool LaneGetP2PReuseMac(const char *networkId, char *ipAddr, uint32_t maxLen, uint16_t *port)
{
    P2pAddrNode* item = NULL;
    P2pAddrNode* nextItem = NULL;
    if (SoftBusMutexLock(&g_P2pAddrList.lock) != SOFTBUS_OK) {
        return false;
    }
    LIST_FOR_EACH_ENTRY_SAFE(item, nextItem, &g_P2pAddrList.list, P2pAddrNode, node) {
        if (strcmp(item->networkId, networkId) == 0) {
            if (strcpy_s(ipAddr, maxLen, item->addr) != EOK) {
                SoftBusMutexUnlock(&g_P2pAddrList.lock);
                return false;
            }
            *port = item->port;
            SoftBusMutexUnlock(&g_P2pAddrList.lock);
            return true;
        }
    }
    SoftBusMutexUnlock(&g_P2pAddrList.lock);
    return false;
}

static int32_t LaneLinkOfBleReuse(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback,
    BleProtocolType type)
{
    const char *udid = LnnConvertDLidToUdid(reqInfo->peerNetworkId, CATEGORY_NETWORK_ID);
    ConnBleConnection *connection = ConnBleGetClientConnectionByUdid(udid, type);
    if ((connection == NULL) || (connection->state != BLE_CONNECTION_STATE_EXCHANGED_BASIC_INFO)) {
        return SOFTBUS_ERR;
    }
    LaneLinkInfo linkInfo = {0};
    (void)memcpy_s(linkInfo.linkInfo.ble.bleMac, BT_MAC_LEN, connection->addr, BT_MAC_LEN);
    if (SoftBusGenerateStrHash((uint8_t*)connection->udid, strlen(connection->udid),
        (uint8_t*)linkInfo.linkInfo.ble.deviceIdHash) != SOFTBUS_OK) {
        LLOGE("generate deviceId hash err");
        ConnBleReturnConnection(&connection);
        return SOFTBUS_ERR;
    }
    linkInfo.linkInfo.ble.protoType = type;
    if (type == BLE_COC) {
        linkInfo.type = LANE_COC;
        linkInfo.linkInfo.ble.psm = connection->psm;
    } else if (type == BLE_GATT) {
        linkInfo.type = LANE_BLE;
    }
    ConnBleReturnConnection(&connection);
    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

static int32_t LaneLinkOfBle(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    if (LaneLinkOfBleReuse(reqId, reqInfo, callback, BLE_GATT) == SOFTBUS_OK) {
        LLOGI("reuse ble gatt connection");
        return SOFTBUS_OK;
    }
    LaneLinkInfo linkInfo = {0};
    if (LnnGetRemoteStrInfo(reqInfo->peerNetworkId, STRING_KEY_BLE_MAC, linkInfo.linkInfo.ble.bleMac, BT_MAC_LEN)
        != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    char peerUdid[UDID_BUF_LEN] = {0};
    if (LnnGetRemoteStrInfo(reqInfo->peerNetworkId, STRING_KEY_DEV_UDID, peerUdid, UDID_BUF_LEN) != SOFTBUS_OK) {
        LLOGE("get udid error");
        return SOFTBUS_ERR;
    }
    if (SoftBusGenerateStrHash((uint8_t*)peerUdid, strlen(peerUdid),
        (uint8_t*)linkInfo.linkInfo.ble.deviceIdHash) != SOFTBUS_OK) {
        LLOGE("generate deviceId hash err");
        return SOFTBUS_ERR;
    }
    linkInfo.linkInfo.ble.protoType = BLE_GATT;
    linkInfo.linkInfo.ble.psm = 0;
    linkInfo.type = LANE_BLE;
    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

static int32_t LaneLinkOfBleDirectCommon(const LinkRequest *reqInfo, LaneLinkInfo *linkInfo)
{
    unsigned char peerUdid[UDID_BUF_LEN];
    unsigned char peerNetwordIdHash[SHA_256_HASH_LEN];
    unsigned char localUdidHash[SHA_256_HASH_LEN];

    if (SoftBusGenerateStrHash((const unsigned char*)reqInfo->peerNetworkId, NETWORK_ID_BUF_LEN,
        peerNetwordIdHash) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    const NodeInfo* nodeInfo = LnnGetLocalNodeInfo();
    if (SoftBusGenerateStrHash((const unsigned char*)nodeInfo->deviceInfo.deviceUdid, UDID_BUF_LEN,
        localUdidHash) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    if (LnnGetRemoteStrInfo(reqInfo->peerNetworkId, STRING_KEY_DEV_UDID,
        (char *)peerUdid, UDID_BUF_LEN) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    if (SoftBusGenerateStrHash(peerUdid, strlen((char*)peerUdid),
            (unsigned char *)linkInfo->linkInfo.bleDirect.peerUdidHash) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }

    if (memcpy_s(linkInfo->linkInfo.bleDirect.nodeIdHash, NODEID_SHORT_HASH_LEN, peerNetwordIdHash,
        NODEID_SHORT_HASH_LEN) != EOK) {
        return SOFTBUS_ERR;
    }
    if (memcpy_s(linkInfo->linkInfo.bleDirect.localUdidHash, UDID_SHORT_HASH_LEN, localUdidHash, UDID_SHORT_HASH_LEN) !=
        EOK) {
        return SOFTBUS_ERR;
    }
    linkInfo->type = LANE_BLE_DIRECT;
    return SOFTBUS_OK;
}

static int32_t LaneLinkOfGattDirect(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    LaneLinkInfo linkInfo = { 0 };
    if (LaneLinkOfBleDirectCommon(reqInfo, &linkInfo) != SOFTBUS_OK) {
        LLOGE("ble direct common failed");
        return SOFTBUS_ERR;
    }
    linkInfo.linkInfo.bleDirect.protoType = BLE_GATT;
    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

static int32_t LaneLinkOfP2p(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    return LnnConnectP2p(reqInfo->peerNetworkId, reqInfo->pid, reqInfo->networkDelegate, reqId, callback);
}

static int32_t LaneLinkOfP2pReuse(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    LaneLinkInfo linkInfo;
    linkInfo.type = LANE_P2P_REUSE;
    char ipAddr[MAX_SOCKET_ADDR_LEN];
    uint16_t port;
    if (!LaneGetP2PReuseMac(reqInfo->peerNetworkId, ipAddr, MAX_SOCKET_ADDR_LEN, &port)) {
        LLOGE("p2p resue get addr failed");
        return SOFTBUS_ERR;
    }
    linkInfo.linkInfo.wlan.connInfo.protocol = LNN_PROTOCOL_IP;
    linkInfo.linkInfo.wlan.connInfo.port = port;
    if (memcpy_s(linkInfo.linkInfo.wlan.connInfo.addr, MAX_SOCKET_ADDR_LEN, ipAddr, MAX_SOCKET_ADDR_LEN) != EOK) {
        return SOFTBUS_ERR;
    }

    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

static int32_t GetWlanLinkedAttribute(int32_t *channel, bool *is5GBand, bool *isConnected)
{
    LnnWlanLinkedInfo info;
    int32_t ret = LnnGetWlanLinkedInfo(&info);
    if (ret != SOFTBUS_OK) {
        LLOGE("LnnGetWlanLinkedInfo fail, ret:%d", ret);
        return SOFTBUS_ERR;
    }
    *isConnected = info.isConnected;
    *is5GBand = (info.band != 1);

    *channel = SoftBusFrequencyToChannel(info.frequency);
    LLOGI("wlan current channel is %d", *channel);
    return SOFTBUS_OK;
}

struct SelectProtocolReq {
    LnnNetIfType localIfType;
    ProtocolType selectedProtocol;
    ProtocolType remoteSupporttedProtocol;
    uint8_t currPri;
};

VisitNextChoice FindBestProtocol(const LnnPhysicalSubnet *subnet, void *priv)
{
    if (subnet == NULL || priv == NULL || subnet->protocol == NULL) {
        return CHOICE_FINISH_VISITING;
    }
    struct SelectProtocolReq *req = (struct SelectProtocolReq *)priv;
    if (subnet->status == LNN_SUBNET_RUNNING && (subnet->protocol->supportedNetif & req->localIfType) != 0 &&
        subnet->protocol->pri > req->currPri && (subnet->protocol->id & req->remoteSupporttedProtocol) != 0) {
        req->currPri = subnet->protocol->pri;
        req->selectedProtocol = subnet->protocol->id;
    }

    return CHOICE_VISIT_NEXT;
}

static ProtocolType LnnLaneSelectProtocol(LnnNetIfType ifType, const char *netWorkId, ProtocolType acceptableProtocols)
{
    NodeInfo remoteNodeInfo = {0};
    int ret = LnnGetRemoteNodeInfoById(netWorkId, CATEGORY_NETWORK_ID, &remoteNodeInfo);
    if (ret != SOFTBUS_OK) {
        LLOGE("no such network id");
        return SOFTBUS_ERR;
    }

    const NodeInfo *localNode = LnnGetLocalNodeInfo();
    if (localNode == NULL) {
        LLOGE("get local node info failed!");
        return SOFTBUS_ERR;
    }

    struct SelectProtocolReq req = {
        .localIfType = ifType,
        .remoteSupporttedProtocol = remoteNodeInfo.supportedProtocols & acceptableProtocols,
        .selectedProtocol = 0,
        .currPri = 0,
    };

    if ((req.remoteSupporttedProtocol & LNN_PROTOCOL_NIP) != 0 &&
        (strcmp(remoteNodeInfo.nodeAddress, NODE_ADDR_LOOPBACK) == 0 ||
            strcmp(localNode->nodeAddress, NODE_ADDR_LOOPBACK) == 0)) {
        LLOGW("newip temporarily unavailable!");
        req.remoteSupporttedProtocol ^= LNN_PROTOCOL_NIP;
    }

    (void)LnnVisitPhysicalSubnet(FindBestProtocol, &req);

    LLOGI("protocol = %d", req.selectedProtocol);
    if (req.selectedProtocol == 0) {
        req.selectedProtocol = LNN_PROTOCOL_IP;
    }
 
    return req.selectedProtocol;
}

static void FillWlanLinkInfo(
    LaneLinkInfo *linkInfo, bool is5GBand, int32_t channel, uint16_t port, ProtocolType protocol)
{
    if (is5GBand) {
        linkInfo->type = LANE_WLAN_5G;
    } else {
        linkInfo->type = LANE_WLAN_2P4G;
    }
    WlanLinkInfo *wlan = &(linkInfo->linkInfo.wlan);
    wlan->channel = channel;
    wlan->bw = LANE_BW_RANDOM;
    wlan->connInfo.protocol = protocol;
    wlan->connInfo.port = port;
}

NO_SANITIZE("cfi") static int32_t LaneLinkOfWlan(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    LaneLinkInfo linkInfo;
    int32_t port = 0;
    int32_t ret = SOFTBUS_OK;
    NodeInfo node = {0};
    if (LnnGetRemoteNodeInfoById(reqInfo->peerNetworkId, CATEGORY_NETWORK_ID, &node) != SOFTBUS_OK) {
        LLOGW("can not get peer node");
        return SOFTBUS_ERR;
    }
    if (!LnnHasDiscoveryType(&node, DISCOVERY_TYPE_WIFI) && !LnnHasDiscoveryType(&node, DISCOVERY_TYPE_LSA)) {
        LLOGE("peer node is not wifi online");
        return SOFTBUS_ERR;
    }
    ProtocolType acceptableProtocols = LNN_PROTOCOL_ALL ^ LNN_PROTOCOL_NIP;
    if (reqInfo->transType == LANE_T_MSG || reqInfo->transType == LANE_T_BYTE) {
        acceptableProtocols |= LNN_PROTOCOL_NIP;
    }
    acceptableProtocols = acceptableProtocols & reqInfo->acceptableProtocols;
    ProtocolType protocol =
        LnnLaneSelectProtocol(LNN_NETIF_TYPE_WLAN | LNN_NETIF_TYPE_ETH, reqInfo->peerNetworkId, acceptableProtocols);
    if (protocol == 0) {
        SoftBusLog(SOFTBUS_LOG_LNN, SOFTBUS_LOG_ERROR, "protocal is invalid!");
        return SOFTBUS_ERR;
    }
    if (protocol == LNN_PROTOCOL_IP) {
        ret = LnnGetRemoteStrInfo(reqInfo->peerNetworkId, STRING_KEY_WLAN_IP, linkInfo.linkInfo.wlan.connInfo.addr,
            sizeof(linkInfo.linkInfo.wlan.connInfo.addr));
        if (ret != SOFTBUS_OK) {
            LLOGE("LnnGetRemote wlan ip error, ret: %d", ret);
            return SOFTBUS_ERR;
        }
        if (strnlen(linkInfo.linkInfo.wlan.connInfo.addr, sizeof(linkInfo.linkInfo.wlan.connInfo.addr)) == 0 ||
            strncmp(linkInfo.linkInfo.wlan.connInfo.addr, "127.0.0.1", strlen("127.0.0.1")) == 0) {
            LLOGE("Wlan ip not found.");
            return SOFTBUS_ERR;
        }
    } else {
        ret = LnnGetRemoteStrInfo(reqInfo->peerNetworkId, STRING_KEY_NODE_ADDR, linkInfo.linkInfo.wlan.connInfo.addr,
            sizeof(linkInfo.linkInfo.wlan.connInfo.addr));
        if (ret != SOFTBUS_OK) {
            LLOGE("LnnGetRemote wlan addr error, ret: %d", ret);
            return SOFTBUS_ERR;
        }
    }
    if (reqInfo->transType == LANE_T_MSG) {
        ret = LnnGetRemoteNumInfo(reqInfo->peerNetworkId, NUM_KEY_PROXY_PORT, &port);
        LLOGI("LnnGetRemote proxy port");
    } else {
        ret = LnnGetRemoteNumInfo(reqInfo->peerNetworkId, NUM_KEY_SESSION_PORT, &port);
        LLOGI("LnnGetRemote session port");
    }
    if (ret < 0) {
        LLOGE("LnnGetRemote is failed.");
        return SOFTBUS_ERR;
    }
    int32_t channel = -1;
    bool is5GBand = false;
    bool isConnected = false;
    if (GetWlanLinkedAttribute(&channel, &is5GBand, &isConnected) != SOFTBUS_OK) {
        LLOGE("get wlan linked info fail");
    }
    if (!isConnected) {
        LLOGE("wlan is disconnected");
    }

    FillWlanLinkInfo(&linkInfo, is5GBand, channel, (uint16_t)port, protocol);
    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

static int32_t LaneLinkOfCoc(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    return LaneLinkOfBleReuse(reqId, reqInfo, callback, BLE_COC);
}

static int32_t LaneLinkOfCocDirect(uint32_t reqId, const LinkRequest *reqInfo, const LaneLinkCb *callback)
{
    LaneLinkInfo linkInfo = { 0 };
    if (LaneLinkOfBleDirectCommon(reqInfo, &linkInfo) != SOFTBUS_OK) {
        LLOGE("ble direct common failed");
        return SOFTBUS_ERR;
    }
    linkInfo.linkInfo.bleDirect.protoType = BLE_COC;
    callback->OnLaneLinkSuccess(reqId, &linkInfo);
    return SOFTBUS_OK;
}

static LaneLinkByType g_linkTable[LANE_LINK_TYPE_BUTT] = {
    [LANE_BR] = LaneLinkOfBr,
    [LANE_BLE] = LaneLinkOfBle,
    [LANE_P2P] = LaneLinkOfP2p,
    [LANE_WLAN_2P4G] = LaneLinkOfWlan,
    [LANE_WLAN_5G] = LaneLinkOfWlan,
    [LANE_P2P_REUSE] = LaneLinkOfP2pReuse,
    [LANE_BLE_DIRECT] = LaneLinkOfGattDirect,
    [LANE_COC] = LaneLinkOfCoc,
    [LANE_COC_DIRECT] = LaneLinkOfCocDirect,
};

int32_t BuildLink(const LinkRequest *reqInfo, uint32_t reqId, const LaneLinkCb *callback)
{
    if (IsLinkRequestValid(reqInfo) != SOFTBUS_OK || !LinkTypeCheck(reqInfo->linkType)) {
        LLOGE("the reqInfo or type is invalid");
        return SOFTBUS_INVALID_PARAM;
    }
    if (callback == NULL || callback->OnLaneLinkSuccess == NULL ||
        callback->OnLaneLinkFail == NULL || callback->OnLaneLinkException == NULL) {
        LLOGE("the callback is invalid");
        return SOFTBUS_INVALID_PARAM;
    }
    LLOGI("build link, linktype:%d, peerNetworkId:%s", reqInfo->linkType, AnonymizesNetworkID(reqInfo->peerNetworkId));
    if (g_linkTable[reqInfo->linkType](reqId, reqInfo, callback) != SOFTBUS_OK) {
        LLOGE("lane link is failed");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

void DestroyLink(const char *networkId, uint32_t reqId, LaneLinkType type, int32_t pid)
{
    LLOGI("type=%d", type);
    if (networkId == NULL) {
        LLOGE("the networkId is nullptr");
        return;
    }
    if (type == LANE_P2P) {
        LLOGI("type=LANE_P2P");
        LaneDeleteP2pAddress(networkId);
        LnnDisconnectP2p(networkId, pid, reqId);
    } else {
        LLOGI("ignore this link request,link:%d", type);
    }
}

int32_t InitLaneLink(void)
{
    LaneInitP2pAddrList();
    return SOFTBUS_OK;
}

void DeinitLaneLink(void)
{
    LnnDestoryP2p();
    return;
}
