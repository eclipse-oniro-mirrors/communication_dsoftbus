/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "softbus_proxychannel_listener.h"

#include <securec.h>

#include "bus_center_manager.h"
#include "lnn_distributed_net_ledger.h"
#include "lnn_lane_interface.h"
#include "softbus_conn_interface.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_hisysevt_transreporter.h"
#include "softbus_proxychannel_callback.h"
#include "softbus_proxychannel_manager.h"
#include "softbus_proxychannel_network.h"
#include "softbus_proxychannel_session.h"
#include "softbus_proxychannel_control.h"
#include "softbus_utils.h"
#include "softbus_adapter_mem.h"
#include "trans_channel_common.h"
#include "trans_lane_manager.h"
#include "trans_lane_pending_ctl.h"
#include "trans_log.h"
#include "trans_event.h"

static int32_t NotifyNormalChannelClosed(const char *pkgName, int32_t pid, int32_t channelId)
{
    int32_t ret = TransProxyOnChannelClosed(pkgName, pid, channelId);
    TRANS_LOGI(TRANS_CTRL, "proxy channel close, channelId=%{public}d, ret=%{public}d", channelId, ret);
    return ret;
}

static int32_t NotifyNormalChannelOpenFailed(const char *pkgName, int32_t pid, int32_t channelId, int32_t errCode)
{
    int32_t ret = TransProxyOnChannelOpenFailed(pkgName, pid, channelId, errCode);
    TRANS_LOGW(TRANS_CTRL, "proxy channel open fail, channelId=%{public}d, ret=%{public}d", channelId, ret);
    return ret;
}

static void GetProxyChannelInfo(int32_t channelId, const AppInfo *appInfo, bool isServer, ChannelInfo *info)
{
    info->channelId = channelId;
    info->channelType = CHANNEL_TYPE_PROXY;
    info->isServer = isServer;
    info->isEnabled = true;
    info->isEncrypt = appInfo->appType != APP_TYPE_AUTH;
    info->groupId = (char *)appInfo->groupId;
    info->peerSessionName = (char *)appInfo->peerData.sessionName;
    info->peerPid = appInfo->peerData.pid;
    info->peerUid = appInfo->peerData.uid;
    info->sessionKey = (char *)appInfo->sessionKey;
    info->keyLen = SESSION_KEY_LENGTH;
    info->fileEncrypt = appInfo->encrypt;
    info->algorithm = appInfo->algorithm;
    info->crc = appInfo->crc;
    info->routeType = appInfo->routeType;
    info->businessType = (int32_t)(appInfo->appType == APP_TYPE_AUTH ? BUSINESS_TYPE_NOT_CARE : appInfo->businessType);
    info->autoCloseTime = appInfo->autoCloseTime;
    info->myHandleId = appInfo->myHandleId;
    info->peerHandleId = appInfo->peerHandleId;
    info->linkType = appInfo->linkType;
    info->dataConfig = appInfo->myData.dataConfig;
    if (appInfo->appType == APP_TYPE_AUTH) {
        info->reqId = (char *)appInfo->reqId;
    }
    info->timeStart = appInfo->timeStart;
    info->linkType = appInfo->linkType;
    info->connectType = appInfo->connectType;
    info->osType = appInfo->osType;
    TransGetLaneIdByChannelId(channelId, &info->laneId);
}

static int32_t NotifyNormalChannelOpened(int32_t channelId, const AppInfo *appInfo, bool isServer)
{
    ChannelInfo info = {0};
    int32_t ret = SOFTBUS_ERR;
    char buf[NETWORK_ID_BUF_LEN] = {0};

    GetProxyChannelInfo(channelId, appInfo, isServer, &info);
    if (appInfo->appType != APP_TYPE_AUTH) {
        ret = LnnGetNetworkIdByUuid(appInfo->peerData.deviceId, buf, NETWORK_ID_BUF_LEN);
        if (ret != SOFTBUS_OK) {
            char *anonyUuid = NULL;
            Anonymize(appInfo->peerData.deviceId, &anonyUuid);
            TRANS_LOGE(TRANS_CTRL, "get info networkId fail, uuid=%{public}s", anonyUuid);
            AnonymizeFree(anonyUuid);
            return ret;
        }
        info.peerDeviceId = buf;
        if (LnnGetOsTypeByNetworkId(buf, &(info.osType)) != SOFTBUS_OK) {
            TRANS_LOGE(TRANS_CTRL, "get remote osType fail, channelId=%{public}d", channelId);
        }
    } else {
        info.peerDeviceId = (char *)appInfo->peerData.deviceId;
    }

    ret = TransProxyOnChannelOpened(appInfo->myData.pkgName, appInfo->myData.pid, appInfo->myData.sessionName, &info);
    TRANS_LOGI(TRANS_CTRL, "proxy channel open, channelId=%{public}d, ret=%{public}d", channelId, ret);
    return ret;
}

static void FillExtraByProxyChannelErrorEnd(TransEventExtra *extra, const AppInfo *appInfo, char *localUdid,
    uint32_t len)
{
    if (extra == NULL || appInfo == NULL || localUdid == NULL) {
        TRANS_LOGE(TRANS_CTRL, "invalid param.");
        return;
    }
    extra->socketName = appInfo->myData.sessionName;
    extra->callerPkg = appInfo->myData.pkgName;
    extra->linkType = appInfo->connectType;
    if (appInfo->appType == APP_TYPE_AUTH && strlen(appInfo->peerVersion) == 0) {
        TransGetRemoteDeviceVersion(extra->peerUdid, CATEGORY_UDID, (char *)(appInfo->peerVersion),
            sizeof(appInfo->peerVersion));
    }
    extra->peerDevVer = appInfo->peerVersion;
    (void)LnnGetLocalStrInfo(STRING_KEY_DEV_UDID, localUdid, len);
    extra->localUdid = localUdid;
}

int32_t OnProxyChannelOpened(int32_t channelId, const AppInfo *appInfo, unsigned char isServer)
{
    int32_t ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
    if (appInfo == NULL) {
        TRANS_LOGE(TRANS_CTRL, "proxy channel opened app info invalid.");
        return SOFTBUS_INVALID_PARAM;
    }
    TRANS_LOGI(TRANS_CTRL, "proxy channel opened: channelId=%{public}d, appType=%{public}d, isServer=%{public}d",
        channelId, appInfo->appType, isServer);

    switch (appInfo->appType) {
        case APP_TYPE_NORMAL:
        case APP_TYPE_AUTH:
            ret = NotifyNormalChannelOpened(channelId, appInfo, isServer);
            break;
        case APP_TYPE_INNER:
            ret = NotifyNetworkingChannelOpened(appInfo->myData.sessionName, channelId, appInfo, isServer);
            break;
        default:
            ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
            break;
    }
    TransEventExtra extra = {
        .socketName = NULL,
        .peerNetworkId = NULL,
        .calleePkg = NULL,
        .callerPkg = NULL,
        .channelId = channelId,
        .costTime = GetSoftbusRecordTimeMillis() - appInfo->connectedStart,
        .errcode = ret,
        .result = (ret == SOFTBUS_OK) ? EVENT_STAGE_RESULT_OK : EVENT_STAGE_RESULT_FAILED
    };
    extra.osType = (appInfo->osType < 0) ? UNKNOW_OS_TYPE : appInfo->osType;
    extra.peerUdid = appInfo->peerUdid;
    extra.deviceState = TransGetDeviceState(appInfo->peerNetWorkId);
    if (!isServer) {
        extra.peerUdid = appInfo->appType == APP_TYPE_AUTH ? appInfo->peerData.deviceId : extra.peerUdid;
        if (extra.result == EVENT_STAGE_RESULT_FAILED) {
            char localUdid[UDID_BUF_LEN] = { 0 };
            FillExtraByProxyChannelErrorEnd(&extra, appInfo, localUdid, sizeof(localUdid));
            TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL, EVENT_STAGE_OPEN_CHANNEL_END, extra);
        } else {
            TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL, EVENT_STAGE_HANDSHAKE_REPLY, extra);
        }
    } else if (ret != SOFTBUS_OK) {
        TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL_SERVER, EVENT_STAGE_OPEN_CHANNEL_END, extra);
    }
    TRANS_LOGI(TRANS_CTRL, "on open, channelId=%{public}d, ret=%{public}d", channelId, ret);
    return ret;
}

static int32_t TransProxyGetChannelIsServer(int32_t channelId, int8_t *isServer)
{
    ProxyChannelInfo *chan = (ProxyChannelInfo *)SoftBusCalloc(sizeof(ProxyChannelInfo));
    if (chan == NULL) {
        TRANS_LOGE(TRANS_MSG, "malloc in trans proxy send message. channelId=%{public}d", channelId);
        return SOFTBUS_MALLOC_ERR;
    }
    if (TransProxyGetChanByChanId(channelId, chan) != SOFTBUS_OK) {
        SoftBusFree(chan);
        return SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND;
    }
    *isServer = chan->isServer;
    SoftBusFree(chan);
    return SOFTBUS_OK;
}

static int32_t TransProxyNotifyOpenFailedByType(
    const AppInfo *appInfo, AppType appType, int32_t channelId, int32_t errCode)
{
    switch (appType) {
        case APP_TYPE_NORMAL:
        case APP_TYPE_AUTH:
            return NotifyNormalChannelOpenFailed(appInfo->myData.pkgName, appInfo->myData.pid, channelId, errCode);
        case APP_TYPE_INNER:
            NotifyNetworkingChannelOpenFailed(appInfo->myData.sessionName, channelId, appInfo->peerData.deviceId);
            return SOFTBUS_TRANS_NOTIFY_NETWORK_OPEN_ERR;
        default:
            TRANS_LOGE(TRANS_CTRL, "SOFTBUS_INVALID_APPTYPE appType=%{public}d", appType);
            return SOFTBUS_INVALID_APPTYPE;
    }
}

int32_t OnProxyChannelBind(int32_t channelId, const AppInfo *appInfo)
{
    int32_t ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
    if (appInfo == NULL) {
        TRANS_LOGE(TRANS_CTRL, "proxy channel bind app info invalid channelId=%{public}d", channelId);
        return SOFTBUS_INVALID_PARAM;
    }

    switch (appInfo->appType) {
        case APP_TYPE_NORMAL:
        case APP_TYPE_AUTH:
            ret = TransProxyOnChannelBind(appInfo->myData.pkgName, appInfo->myData.pid, channelId);
            break;
        case APP_TYPE_INNER:
            ret = SOFTBUS_OK;
            break;
        default:
            ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
            break;
    }
    TRANS_LOGI(
        TRANS_CTRL, "channelId=%{public}d, ret=%{public}d, appType=%{public}d", channelId, ret, appInfo->appType);
    return ret;
}

static void TransNotifyAlarm(const AppInfo *appInfo, int32_t errCode)
{
    TransAlarmExtra extraAlarm = {
        .conflictName = NULL,
        .conflictedName = NULL,
        .occupyedName = NULL,
        .permissionName = NULL,
        .linkType = appInfo->linkType,
        .errcode = errCode,
        .sessionName = appInfo->myData.sessionName,
    };
    TRANS_ALARM(OPEN_SESSION_FAIL_ALARM, CONTROL_ALARM_TYPE, extraAlarm);
    return;
}

int32_t OnProxyChannelOpenFailed(int32_t channelId, const AppInfo *appInfo, int32_t errCode)
{
    if (appInfo == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    int64_t timeStart = appInfo->timeStart;
    int64_t timediff = GetSoftbusRecordTimeMillis() - timeStart;
    int8_t isServer;

    if ((TransProxyGetChannelIsServer(channelId, &isServer) == SOFTBUS_OK && !isServer) || appInfo->isClient) {
        char localUdid[UDID_BUF_LEN] = { 0 };
        (void)LnnGetLocalStrInfo(STRING_KEY_DEV_UDID, localUdid, sizeof(localUdid));
        TransEventExtra extra = {
            .calleePkg = NULL,
            .peerNetworkId = (appInfo->appType == APP_TYPE_AUTH) ? appInfo->peerData.deviceId : appInfo->peerNetWorkId,
            .linkType = appInfo->connectType,
            .channelId = channelId,
            .costTime = timediff,
            .errcode = errCode,
            .callerPkg = appInfo->myData.pkgName,
            .socketName = appInfo->myData.sessionName,
            .localUdid = localUdid,
            .peerUdid = appInfo->appType == APP_TYPE_AUTH ? appInfo->peerData.deviceId : appInfo->peerUdid,
            .osType = (appInfo->osType < 0) ? UNKNOW_OS_TYPE : appInfo->osType,
            .result = EVENT_STAGE_RESULT_FAILED
        };
        extra.deviceState = TransGetDeviceState(appInfo->peerNetWorkId);
        if (appInfo->appType == APP_TYPE_AUTH && strlen(appInfo->peerVersion) == 0) {
            TransGetRemoteDeviceVersion(extra.peerUdid, CATEGORY_UDID, (char *)(appInfo->peerVersion),
                sizeof(appInfo->peerVersion));
        }
        extra.peerDevVer = appInfo->peerVersion;
        TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL, EVENT_STAGE_OPEN_CHANNEL_END, extra);
        TransNotifyAlarm(appInfo, errCode);
    }
    SoftbusRecordOpenSessionKpi(appInfo->myData.pkgName, appInfo->linkType, SOFTBUS_EVT_OPEN_SESSION_FAIL, timediff);
    char *tmpName = NULL;
    Anonymize(appInfo->myData.sessionName, &tmpName);
    TRANS_LOGI(TRANS_CTRL,
        "proxy channel openfailed:sessionName=%{public}s, channelId=%{public}d, appType=%{public}d, errCode=%{public}d",
        tmpName, channelId, appInfo->appType, errCode);
    AnonymizeFree(tmpName);
    return TransProxyNotifyOpenFailedByType(appInfo, appInfo->appType, channelId, errCode);
}

int32_t OnProxyChannelClosed(int32_t channelId, const AppInfo *appInfo)
{
    if (appInfo == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    TRANS_LOGI(TRANS_CTRL,
        "proxy channel closed: channelId=%{public}d, appType=%{public}d", channelId, appInfo->appType);

    int32_t ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
    switch (appInfo->appType) {
        case APP_TYPE_NORMAL:
        case APP_TYPE_AUTH:
            ret = NotifyNormalChannelClosed(appInfo->myData.pkgName, appInfo->myData.pid, channelId);
            break;
        case APP_TYPE_INNER:
            NotifyNetworkingChannelClosed(appInfo->myData.sessionName, channelId);
            break;
        default:
            ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
            break;
    }
    return ret;
}

int32_t OnProxyChannelMsgReceived(int32_t channelId, const AppInfo *appInfo, const char *data,
    uint32_t len)
{
    int32_t ret = SOFTBUS_OK;
    if (appInfo == NULL || data == NULL || len == 0) {
        return SOFTBUS_INVALID_PARAM;
    }

    switch (appInfo->appType) {
        case APP_TYPE_NORMAL:
        case APP_TYPE_AUTH:
            TransOnNormalMsgReceived(appInfo->myData.pkgName, appInfo->myData.pid, channelId, data, len);
            break;
        case APP_TYPE_INNER:
            NotifyNetworkingMsgReceived(appInfo->myData.sessionName, channelId, data, len);
            break;
        default:
            ret = SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
            break;
    }
    return ret;
}

static int32_t TransProxyGetAppInfo(const char *sessionName, const char *peerNetworkId, AppInfo *appInfo)
{
    int ret = SOFTBUS_TRANS_GET_APP_INFO_FAILED;
    int32_t osType = 0;
    GetOsTypeByNetworkId(peerNetworkId, &osType);
    appInfo->osType = osType;
    appInfo->appType = APP_TYPE_INNER;
    appInfo->myData.apiVersion = API_V2;
    appInfo->autoCloseTime = 0;
    ret = LnnGetLocalStrInfo(STRING_KEY_UUID, appInfo->myData.deviceId, sizeof(appInfo->myData.deviceId));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "get local uuid fail. ret=%{public}d", ret);
        return ret;
    }
    if (strcpy_s(appInfo->myData.sessionName, sizeof(appInfo->myData.sessionName), sessionName) != EOK) {
        TRANS_LOGE(TRANS_CTRL, "strcpy_s my sessionName failed");
        return SOFTBUS_STRCPY_ERR;
    }
    appInfo->peerData.apiVersion = API_V2;
    if (strcpy_s(appInfo->peerData.sessionName, sizeof(appInfo->peerData.sessionName), sessionName) != EOK) {
        TRANS_LOGE(TRANS_CTRL, "strcpy_s peer sessionName failed");
        return SOFTBUS_STRCPY_ERR;
    }
    if (strcpy_s(appInfo->peerNetWorkId, sizeof(appInfo->peerNetWorkId), peerNetworkId) != EOK) {
        TRANS_LOGE(TRANS_CTRL, "strcpy_s peerNetworkId failed");
        return SOFTBUS_STRCPY_ERR;
    }

    ret = LnnGetRemoteStrInfo(peerNetworkId, STRING_KEY_UUID,
        appInfo->peerData.deviceId, sizeof(appInfo->peerData.deviceId));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "get remote node uuid err. ret=%{public}d", ret);
        return SOFTBUS_GET_REMOTE_UUID_ERR;
    }

    GetRemoteUdidWithNetworkId(peerNetworkId, appInfo->peerUdid, sizeof(appInfo->peerUdid));
    TransGetRemoteDeviceVersion(peerNetworkId, CATEGORY_NETWORK_ID, appInfo->peerVersion, sizeof(appInfo->peerVersion));

    return SOFTBUS_OK;
}

static int32_t TransGetConnectOption(
    const char *peerNetworkId, ConnectOption *connOpt, const LanePreferredLinkList *preferred)
{
    uint32_t laneHandle = INVALID_LANE_REQ_ID;
    LaneConnInfo connInfo;
    LaneRequestOption option;
    (void)memset_s(&option, sizeof(LaneRequestOption), 0, sizeof(LaneRequestOption));
#define DEFAULT_PID 0
    option.type = LANE_TYPE_TRANS;
    option.requestInfo.trans.pid = DEFAULT_PID;
    option.requestInfo.trans.transType = LANE_T_MSG;
    option.requestInfo.trans.expectedBw = 0;
    option.requestInfo.trans.acceptableProtocols = LNN_PROTOCOL_ALL ^ LNN_PROTOCOL_NIP;
    if (memcpy_s(option.requestInfo.trans.networkId, NETWORK_ID_BUF_LEN,
        peerNetworkId, NETWORK_ID_BUF_LEN) != EOK) {
        TRANS_LOGE(TRANS_CTRL, "memcpy networkId failed.");
        return SOFTBUS_MEM_ERR;
    }
    if (preferred != NULL) {
        for (uint32_t i = 0; i < preferred->linkTypeNum; i++) {
            option.requestInfo.trans.expectedLink.linkType[i] = preferred->linkType[i];
        }
        option.requestInfo.trans.expectedLink.linkTypeNum = preferred->linkTypeNum;
    }
    if (TransGetLaneInfoByOption(&option, &connInfo, &laneHandle) != SOFTBUS_OK) {
        goto EXIT_ERR;
    }
    TRANS_LOGI(TRANS_CTRL, "net channel lane info. laneHandle=%{public}u, type=%{public}d", laneHandle, connInfo.type);
    if (TransGetConnectOptByConnInfo(&connInfo, connOpt) != SOFTBUS_OK) {
        goto EXIT_ERR;
    }
    LnnFreeLane(laneHandle);
    return SOFTBUS_OK;
EXIT_ERR:
    if (laneHandle != 0) {
        LnnFreeLane(laneHandle);
    }
    return SOFTBUS_TRANS_GET_LANE_INFO_ERR;
}


int32_t TransOpenNetWorkingChannel(
    const char *sessionName, const char *peerNetworkId, const LanePreferredLinkList *preferred)
{
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = INVALID_CHANNEL_ID;

    if (!IsValidString(sessionName, SESSION_NAME_SIZE_MAX) ||
        !IsValidString(peerNetworkId, DEVICE_ID_SIZE_MAX)) {
        return channelId;
    }
    if (TransGetConnectOption(peerNetworkId, &connOpt, preferred) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "networking get connect option fail");
        return channelId;
    }
    (void)memset_s(&appInfo, sizeof(AppInfo), 0, sizeof(AppInfo));
    if (TransProxyGetAppInfo(sessionName, peerNetworkId, &appInfo) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "networking get app info fail");
        return channelId;
    }

    if (TransProxyOpenProxyChannel(&appInfo, &connOpt, &channelId) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "networking open channel fail");
        channelId = INVALID_CHANNEL_ID;
    }
    return channelId;
}

int32_t TransSendNetworkingMessage(int32_t channelId, const char *data, uint32_t dataLen,
    int32_t priority)
{
    ProxyChannelInfo *info = (ProxyChannelInfo *)SoftBusCalloc(sizeof(ProxyChannelInfo));
    if (info == NULL) {
        TRANS_LOGE(TRANS_MSG, "malloc in trans proxy send message. channelId=%{public}d", channelId);
        return SOFTBUS_MALLOC_ERR;
    }

    int32_t ret = TransProxyGetSendMsgChanInfo(channelId, info);
    (void)memset_s(info->appInfo.sessionKey, sizeof(info->appInfo.sessionKey), 0, sizeof(info->appInfo.sessionKey));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_MSG, "get proxy channelId failed. channelId=%{public}d", channelId);
        SoftBusFree(info);
        return SOFTBUS_TRANS_PROXY_INVALID_CHANNEL_ID;
    }

    if (info->status != PROXY_CHANNEL_STATUS_COMPLETED && info->status != PROXY_CHANNEL_STATUS_KEEPLIVEING) {
        TRANS_LOGE(TRANS_MSG, "proxy channel status is err. status=%{public}d", info->status);
        SoftBusFree(info);
        return SOFTBUS_TRANS_PROXY_CHANNLE_STATUS_INVALID;
    }

    if (info->appInfo.appType != APP_TYPE_INNER) {
        TRANS_LOGE(TRANS_MSG, "wrong appType=%{public}d", info->appInfo.appType);
        SoftBusFree(info);
        return SOFTBUS_TRANS_PROXY_ERROR_APP_TYPE;
    }

    ret = TransProxySendInnerMessage(info, (char *)data, dataLen, priority);
    SoftBusFree(info);
    return ret;
}

int32_t TransCloseNetWorkingChannel(int32_t channelId)
{
    return TransProxyCloseProxyChannel(channelId);
}
