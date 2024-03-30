/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lnn_lane_listener.h"

#include <securec.h>

#include "anonymizer.h"
#include "lnn_distributed_net_ledger.h"
#include "lnn_lane_common.h"
#include "lnn_log.h"
#include "lnn_node_info.h"
#include "lnn_trans_lane.h"
#include "softbus_adapter_mem.h"
#include "softbus_errcode.h"
#include "wifi_direct_manager.h"

#define HML_IP_PREFIX_LEN 7
#define HML_IP_PREFIX "172.30."
const static LaneType SUPPORT_TYPE_LIST[] = {LANE_TYPE_HDLC, LANE_TYPE_TRANS, LANE_TYPE_CTRL};

typedef struct {
    ListNode node;
    LaneType laneType;
    LaneLinkInfo laneLinkInfo;
    uint32_t ref;
} LaneBusinessInfo;

typedef struct {
    ListNode node;
    LaneStatusListener listener;
    LaneType type;
} LaneListenerInfo;

static SoftBusMutex g_laneStateListenerMutex;
static ListNode g_laneListenerList;
static ListNode g_laneBusinessInfoList;

static int32_t LaneListenerLock(void)
{
    return SoftBusMutexLock(&g_laneStateListenerMutex);
}

static void LaneListenerUnlock(void)
{
    (void)SoftBusMutexUnlock(&g_laneStateListenerMutex);
}

static bool LaneTypeCheck(LaneType type)
{
    uint32_t size = sizeof(SUPPORT_TYPE_LIST) / sizeof(LaneType);
    for (uint32_t i = 0; i < size; i++) {
        if (SUPPORT_TYPE_LIST[i] == type) {
            return true;
        }
    }
    LNN_LOGE(LNN_LANE, "laneType=%{public}d not supported", type);
    return false;
}

static bool CompareLaneBusinessLinkInfo(const LaneLinkInfo *laneLinkInfoSrc, const LaneLinkInfo *laneLinkInfoDst)
{
    if (laneLinkInfoSrc == NULL || laneLinkInfoDst == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return false;
    }

    LaneResource laneResourceSrc;
    (void)memset_s(&laneResourceSrc, sizeof(LaneResource), 0, sizeof(LaneResource));
    LaneResource laneResourceDst;
    (void)memset_s(&laneResourceDst, sizeof(LaneResource), 0, sizeof(LaneResource));
    if (ConvertToLaneResource(laneLinkInfoSrc, &laneResourceSrc) != SOFTBUS_OK ||
        ConvertToLaneResource(laneLinkInfoDst, &laneResourceDst) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "convert to lane resource fail");
        return false;
    }
    return CompLaneResource(&laneResourceSrc, &laneResourceDst);
}

static LaneBusinessInfo *GetLaneBusinessInfoWithoutLock(const LaneBusinessInfo *laneBusinessInfo)
{
    if (laneBusinessInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return NULL;
    }
    LaneBusinessInfo *item = NULL;
    LaneBusinessInfo *next = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(item, next, &g_laneBusinessInfoList, LaneBusinessInfo, node) {
        if (laneBusinessInfo->laneType == item->laneType &&
            laneBusinessInfo->laneLinkInfo.type == item->laneLinkInfo.type &&
            CompareLaneBusinessLinkInfo(&laneBusinessInfo->laneLinkInfo, &item->laneLinkInfo)) {
            return item;
        }
    }
    return NULL;
}

static int32_t CreateLaneBusinessInfoItem(LaneType laneType, const LaneLinkInfo *laneLinkInfo,
    LaneBusinessInfo *laneBusinessInfo)
{
    if (laneLinkInfo == NULL || laneBusinessInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (memcpy_s(&(laneBusinessInfo->laneLinkInfo), sizeof(LaneLinkInfo), laneLinkInfo, sizeof(LaneLinkInfo)) != EOK) {
        LNN_LOGE(LNN_LANE, "memcpy laneLinkInfo fail");
        return SOFTBUS_MEM_ERR;
    }
    laneBusinessInfo->laneType = laneType;
    laneBusinessInfo->ref = 1;
    return SOFTBUS_OK;
}

int32_t AddLaneBusinessInfoItem(LaneType laneType, const LaneLinkInfo *laneLinkInfo)
{
    if (!LaneTypeCheck(laneType) || laneLinkInfo == NULL) {
        LNN_LOGE(LNN_LANE, "[CreateLaneType]invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    LaneBusinessInfo laneBusinessInfo;
    (void)memset_s(&laneBusinessInfo, sizeof(LaneBusinessInfo), 0, sizeof(LaneBusinessInfo));
    if (CreateLaneBusinessInfoItem(laneType, laneLinkInfo, &laneBusinessInfo) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "create laneBusinessInfo fail");
        return SOFTBUS_ERR;
    }
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    LaneBusinessInfo *item = GetLaneBusinessInfoWithoutLock(&laneBusinessInfo);
    if (item != NULL) {
        item->ref++;
        LaneListenerUnlock();
        return SOFTBUS_OK;
    }
    LaneBusinessInfo *laneBusinessInfoItem = (LaneBusinessInfo *)SoftBusCalloc(sizeof(LaneBusinessInfo));
    if (laneBusinessInfoItem == NULL) {
        LNN_LOGE(LNN_LANE, "calloc laneBusinessInfoItem fail");
        LaneListenerUnlock();
        return SOFTBUS_MALLOC_ERR;
    }
    if (memcpy_s(laneBusinessInfoItem, sizeof(LaneBusinessInfo), &laneBusinessInfo,
        sizeof(LaneBusinessInfo)) != EOK) {
        SoftBusFree(laneBusinessInfoItem);
        LNN_LOGE(LNN_LANE, "memcpy laneBusinessInfo fail");
        LaneListenerUnlock();
        return SOFTBUS_MEM_ERR;
    }
    ListAdd(&g_laneBusinessInfoList, &laneBusinessInfoItem->node);
    LaneListenerUnlock();
    return SOFTBUS_OK;
}

int32_t DelLaneBusinessInfoItem(LaneType laneType, const LaneLinkInfo *laneLinkInfo)
{
    if (!LaneTypeCheck(laneType) || laneLinkInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    LaneBusinessInfo laneBusinessInfo;
    (void)memset_s(&laneBusinessInfo, sizeof(LaneBusinessInfo), 0, sizeof(LaneBusinessInfo));
    if (CreateLaneBusinessInfoItem(laneType, laneLinkInfo, &laneBusinessInfo) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "create laneBusinessInfo fail");
        return SOFTBUS_ERR;
    }
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    LaneBusinessInfo *item = GetLaneBusinessInfoWithoutLock(&laneBusinessInfo);
    if (item != NULL) {
        LNN_LOGI(LNN_LANE, "laneType=%{public}d, linkType=%{public}d, ref=%{public}u",
            item->laneLinkInfo.type, item->laneType, item->ref);
        if ((--item->ref) == 0) {
            ListDelete(&item->node);
            SoftBusFree(item);
        }
    }
    LaneListenerUnlock();
    return SOFTBUS_OK;
}

static int32_t FindLaneBusinessInfoByLinkInfo(const LaneLinkInfo *laneLinkInfo,
    uint32_t *resNum, LaneBusinessInfo *laneBusinessInfo, uint32_t laneBusinessNum)
{
    if (laneLinkInfo == NULL || laneBusinessInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    *resNum = 0;
    LaneBusinessInfo *item = NULL;
    LaneBusinessInfo *next = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(item, next, &g_laneBusinessInfoList, LaneBusinessInfo, node) {
        if (item->laneLinkInfo.type == laneLinkInfo->type &&
            CompareLaneBusinessLinkInfo(laneLinkInfo, &item->laneLinkInfo)) {
            if (memcpy_s(&laneBusinessInfo[(*resNum)++], sizeof(LaneBusinessInfo),
                item, sizeof(LaneBusinessInfo)) != EOK) {
                LNN_LOGE(LNN_LANE, "memcpy lane bussiness info fail");
                LaneListenerUnlock();
                return SOFTBUS_MEM_ERR;
            }
        }
        if (*resNum >= laneBusinessNum) {
            LNN_LOGE(LNN_LANE, "find laneBusinessinfo num more than expected");
            break;
        }
    }
    LaneListenerUnlock();
    return SOFTBUS_OK;
}

static LaneListenerInfo *LaneListenerIsExist(LaneType type)
{
    LaneListenerInfo *item = NULL;
    LaneListenerInfo *next = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(item, next, &g_laneListenerList, LaneListenerInfo, node) {
        if (type == item->type) {
            return item;
        }
    }
    return NULL;
}

static int32_t FindLaneListenerInfoByLaneType(LaneType type, LaneListenerInfo *laneListenerInfo)
{
    if (!LaneTypeCheck(type) || laneListenerInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    LaneListenerInfo* item = LaneListenerIsExist(type);
    if (item == NULL) {
        LaneListenerUnlock();
        LNN_LOGE(LNN_LANE, "lane listener is not exist");
        return SOFTBUS_ERR;
    }
    laneListenerInfo->type = item->type;
    laneListenerInfo->listener = item->listener;
    LaneListenerUnlock();
    return SOFTBUS_OK;
}

int32_t LaneLinkupNotify(const char *peerUdid, const LaneLinkInfo *laneLinkInfo)
{
    if (peerUdid == NULL || laneLinkInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    LaneProfile profile;
    (void)memset_s(&profile, sizeof(LaneProfile), 0, sizeof(LaneProfile));
    LaneConnInfo laneConnInfo;
    (void)memset_s(&laneConnInfo, sizeof(LaneConnInfo), 0, sizeof(LaneConnInfo));
    if (LaneInfoProcess(laneLinkInfo, &laneConnInfo, &profile) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "laneInfo proc fail");
        return SOFTBUS_ERR;
    }
    LaneListenerInfo listenerList[LANE_TYPE_BUTT];
    (void)memset_s(listenerList, sizeof(listenerList), 0, sizeof(listenerList));
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    LaneListenerInfo *item = NULL;
    LaneListenerInfo *next = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(item, next, &g_laneListenerList, LaneListenerInfo, node) {
        listenerList[item->type].listener = item->listener;
    }
    LaneListenerUnlock();
    for (uint32_t i = 0; i < LANE_TYPE_BUTT; i++) {
        if (listenerList[i].listener.onLaneLinkup != NULL) {
            LNN_LOGI(LNN_LANE, "notify lane linkup, laneType=%{public}u", i);
            listenerList[i].listener.onLaneLinkup(INVALID_LANE_ID, peerUdid, &laneConnInfo);
        }
    }
    return SOFTBUS_OK;
}

int32_t LaneLinkdownNotify(const char *peerUdid, const LaneLinkInfo *laneLinkInfo)
{
    if (peerUdid == NULL || laneLinkInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    uint32_t resNum;
    LaneBusinessInfo laneBusinessInfo[LANE_TYPE_BUTT];
    if (FindLaneBusinessInfoByLinkInfo(laneLinkInfo, &resNum, laneBusinessInfo, LANE_TYPE_BUTT) != SOFTBUS_OK) {
        LNN_LOGE(LNN_STATE, "not found laneBusinessInfo, no need to notify");
        return SOFTBUS_OK;
    }
    LaneListenerInfo laneListener;
    LaneProfile profile;
    (void)memset_s(&profile, sizeof(LaneProfile), 0, sizeof(LaneProfile));
    LaneConnInfo laneConnInfo;
    (void)memset_s(&laneConnInfo, sizeof(LaneConnInfo), 0, sizeof(LaneConnInfo));
    for (uint32_t i = 0; i < resNum; i++) {
        if (FindLaneListenerInfoByLaneType(laneBusinessInfo[i].laneType, &laneListener) != SOFTBUS_OK) {
            LNN_LOGE(LNN_STATE, "find lane listener fail, laneType=%{public}d", laneBusinessInfo[i].laneType);
            continue;
        }
        if (laneListener.listener.onLaneLinkdown == NULL) {
            LNN_LOGE(LNN_STATE, "invalid lane status listen");
            continue;
        }
        if (LaneInfoProcess(&laneBusinessInfo[i].laneLinkInfo, &laneConnInfo, &profile) != SOFTBUS_OK) {
            LNN_LOGE(LNN_LANE, "laneInfo proc fail");
            continue;
        }
        LNN_LOGI(LNN_LANE, "notify lane linkdown, laneType=%{public}d", laneListener.type);
        laneListener.listener.onLaneLinkdown(INVALID_LANE_ID, peerUdid, &laneConnInfo);
    }
    return SOFTBUS_OK;
}

static int32_t GetStateNotifyInfo(const char *peerIp, const char *peerUuid, char *peerUdid,
    LaneLinkInfo *laneLinkInfo)
{
    if (peerIp == NULL || peerUuid == NULL || peerUdid == NULL || laneLinkInfo == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    laneLinkInfo->type = (strncmp(peerIp, HML_IP_PREFIX, HML_IP_PREFIX_LEN) == 0) ? LANE_HML : LANE_P2P;
    if (strncpy_s(laneLinkInfo->linkInfo.p2p.connInfo.peerIp, IP_LEN, peerIp, IP_LEN) != EOK) {
        LNN_LOGE(LNN_STATE, "strncpy peerIp fail");
        return SOFTBUS_ERR;
    }

    NodeInfo nodeInfo;
    (void)memset_s(&nodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    if (LnnGetRemoteNodeInfoById(peerUuid, CATEGORY_UUID, &nodeInfo) != SOFTBUS_OK) {
        char *anonyUuid = NULL;
        Anonymize(peerUuid, &anonyUuid);
        LNN_LOGE(LNN_STATE, "get remote nodeinfo failed, peerUuid=%{public}s", anonyUuid);
        AnonymizeFree(anonyUuid);
        return SOFTBUS_ERR;
    }
    if (strncpy_s(peerUdid, UDID_BUF_LEN, nodeInfo.masterUdid, UDID_BUF_LEN) != EOK) {
        LNN_LOGE(LNN_STATE, "copy peerudid fail");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static void LnnOnWifiDirectDeviceOnline(const char *peerMac, const char *peerIp, const char *peerUuid)
{
    LNN_LOGD(LNN_LANE, "lnn wifiDerectDevice online");
    if (peerMac == NULL || peerUuid == NULL || peerIp == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return;
    }
    char peerUdid[UDID_BUF_LEN] = {0};
    LaneLinkInfo laneLinkInfo;
    (void)memset_s(&laneLinkInfo, sizeof(LaneLinkInfo), 0, sizeof(LaneLinkInfo));
    if (GetStateNotifyInfo(peerIp, peerUuid, peerUdid, &laneLinkInfo) != SOFTBUS_OK) {
        LNN_LOGE(LNN_STATE, "get lane state notify info fail");
        return;
    }
    if (PostLaneStateChangeMessage(LANE_STATE_LINKUP, peerUdid, &laneLinkInfo) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "post laneState linkup msg fail");
    }
}

static void LnnOnWifiDirectDeviceOffline(const char *peerMac, const char *peerIp, const char *peerUuid)
{
    LNN_LOGD(LNN_LANE, "lnn wifiDerectDevice offline");
    if (peerMac == NULL || peerUuid == NULL || peerIp == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return;
    }
    char peerUdid[UDID_BUF_LEN] = {0};
    LaneLinkInfo laneLinkInfo;
    (void)memset_s(&laneLinkInfo, sizeof(LaneLinkInfo), 0, sizeof(LaneLinkInfo));
    if (GetStateNotifyInfo(peerIp, peerUuid, peerUdid, &laneLinkInfo) != SOFTBUS_OK) {
        LNN_LOGE(LNN_STATE, "get lane state notify info fail");
        return;
    }
    if (PostLaneStateChangeMessage(LANE_STATE_LINKDOWN, peerUdid, &laneLinkInfo) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "post laneState linkdown msg fail");
    }
}

static void LnnOnWifiDirectRoleChange(enum WifiDirectRole myRole)
{
    LNN_LOGD(LNN_LANE, "lnn wifiDerect roleChange");
    (void)myRole;
}

int32_t RegisterLaneListener(LaneType type, const LaneStatusListener *listener)
{
    LNN_LOGI(LNN_LANE, "register lane listener, laneType=%{public}d", type);
    if (!LaneTypeCheck(type) || listener == NULL) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    LaneListenerInfo *item = LaneListenerIsExist(type);
    if (item != NULL) {
        LaneListenerUnlock();
        return SOFTBUS_OK;
    }
    LaneListenerInfo *laneListenerItem = (LaneListenerInfo *)SoftBusCalloc(sizeof(LaneListenerInfo));
    if (laneListenerItem == NULL) {
        LNN_LOGE(LNN_LANE, "calloc lane listener fail");
        LaneListenerUnlock();
        return SOFTBUS_MALLOC_ERR;
    }
    laneListenerItem->type = type;
    laneListenerItem->listener = *listener;
    ListAdd(&g_laneListenerList, &laneListenerItem->node);
    LaneListenerUnlock();
    return SOFTBUS_OK;
}

int32_t UnRegisterLaneListener(LaneType type)
{
    LNN_LOGI(LNN_LANE, "unregister lane listener, laneType=%{public}d", type);
    if (!LaneTypeCheck(type)) {
        LNN_LOGE(LNN_LANE, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    if (LaneListenerLock() != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "lane listener lock fail");
        return SOFTBUS_LOCK_ERR;
    }
    LaneListenerInfo *item = LaneListenerIsExist(type);
    if (item != NULL) {
        ListDelete(&item->node);
        SoftBusFree(item);
    }
    LaneListenerUnlock();
    return SOFTBUS_OK;
}

static void RegisterWifiDirectListener(void)
{
    struct WifiDirectStatusListener listener = {
        .onDeviceOffLine = LnnOnWifiDirectDeviceOffline,
        .onLocalRoleChange = LnnOnWifiDirectRoleChange,
        .onDeviceOnLine = LnnOnWifiDirectDeviceOnline,
    };
    struct WifiDirectManager *mgr = GetWifiDirectManager();
    if (mgr == NULL) {
        LNN_LOGE(LNN_LANE, "get wifiDirect manager null");
        return;
    }
    if (mgr->registerStatusListener != NULL) {
        LNN_LOGD(LNN_LANE, "regist listener to wifiDirect");
        mgr->registerStatusListener(LNN_LANE_MODULE, &listener);
    }
}

int32_t InitLaneListener(void)
{
    if (SoftBusMutexInit(&g_laneStateListenerMutex, NULL) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "g_laneStateListenerMutex init fail");
        return SOFTBUS_ERR;
    }

    ListInit(&g_laneListenerList);
    ListInit(&g_laneBusinessInfoList);

    RegisterWifiDirectListener();
    return SOFTBUS_OK;
}