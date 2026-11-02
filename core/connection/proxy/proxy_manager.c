/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "proxy_manager.h"

#include "far_field/far_field_proxy_adapter.h"
#include "far_field/far_field_proxy_manager.h"
#include "dfx/far_field_proxy_dfx.h"
#include "br/br_proxy_manager.h"
#include "softbus_rc_object.h"
#include "br/proxy_observer.h"
#include "conn_log.h"
#include "securec.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_bt_common.h"
#include "softbus_conn_interface_struct.h"
#include "softbus_conn_manager_struct.h"
#include "softbus_def.h"

static ProxyConnectListener g_listener = {0};
static SoftBusMutex g_reqIdLock;
static uint32_t g_reqId = 1;

#define REQUEST_ID_MATCH_ALL 0

typedef enum {
    BR_PROXY,
    FAR_FIELD_PROXY,
    PROXY_CHANNEL_TYPE_BUTT,
} ProxyChannelType;

typedef struct {
    SOFT_BUS_RC_OBJECT_BASE;
    ProxyChannelType type;
    bool isDirectly;
    bool isFirstConnect;
    int32_t requestId;
    char realBrMac[BT_MAC_LEN];
    char srcBrMac[BT_MAC_MAX_LEN];
    char uuid[UUID_STRING_LEN];
    bool isSupportFarField;
    int32_t appIndex;
    OpenProxyChannelCallback callback;
} ProxyChannelInfo;

static uint32_t GenerateRequestId(void)
{
    int32_t ret = SoftBusMutexLock(&g_reqIdLock);
    CONN_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, PROXY_CHANNEL_MAX_STATE, CONN_PROXY,
        "lock fail, error=%{public}d", ret);
    if (g_reqId == REQUEST_ID_MATCH_ALL) {
        g_reqId++;
    }
    uint32_t reqId = g_reqId++;
    SoftBusMutexUnlock(&g_reqIdLock);
    return reqId;
}

static bool IsRealMac(const char *brMac)
{
    if (strlen(brMac) != BT_MAC_LEN - 1) {
        CONN_LOGW(CONN_PROXY, "brMac length is not match, len=%{public}zu", strlen(brMac));
        return false;
    }
    int32_t macConLonPos = 3;
    for (int32_t i = 0; i < BT_MAC_LEN - 1; i++) {
        if ((i + 1) % macConLonPos == 0) {
            if (brMac[i] != ':') {
                CONN_LOGE(CONN_PROXY, "brMac format error");
                return false;
            }
        } else if (!(brMac[i] >= '0' && brMac[i] <= '9') &&
                   !(brMac[i] >= 'a' && brMac[i] <= 'f') &&
                   !(brMac[i] >= 'A' && brMac[i] <= 'F')) {
            CONN_LOGE(CONN_PROXY, "brMac format error");
            return false;
        }
    }
    return true;
}

static void DestroyProxyChannelInfo(ProxyChannelInfo *info)
{
    SoftBusFree(info);
}

static void ProxyChannelInfoFreeHook(SoftBusRcObject *object)
{
    DestroyProxyChannelInfo((ProxyChannelInfo*)object);
}

static ProxyChannelInfo *CreateProxyChannelInfo(ProxyChannelType type, ProxyChannelParam *param, bool isRealMac)
{
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusCalloc(sizeof(ProxyChannelInfo));
    CONN_CHECK_AND_RETURN_RET_LOGE(channelInfo != NULL, NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = SoftBusRcObjectConstruct("proxy_channel", (SoftBusRcObject *)channelInfo, ProxyChannelInfoFreeHook);
    if (ret != SOFTBUS_OK) {
        SoftBusFree(channelInfo);
        return NULL;
    }
    channelInfo->type = type;
    channelInfo->requestId = param->requestId;
    channelInfo->appIndex = param->appIndex;

    ret = strcpy_s(channelInfo->srcBrMac, sizeof(channelInfo->srcBrMac), param->brMac);
    if (ret != EOK) {
        CONN_LOGE(CONN_PROXY, "reqId=%{public}u, strcpy brMac failed", param->requestId);
        SoftBusFree(channelInfo);
        return NULL;
    }
    ret = strcpy_s(channelInfo->uuid, sizeof(channelInfo->uuid), param->uuid);
    if (ret != EOK) {
        CONN_LOGE(CONN_PROXY, "reqId=%{public}u, strcpy uuid failed", param->requestId);
        SoftBusFree(channelInfo);
        return NULL;
    }
    if ((isRealMac && strcpy_s(channelInfo->realBrMac, BT_MAC_LEN, param->brMac) != EOK) ||
        (!isRealMac && GetRealMac(channelInfo->realBrMac, BT_MAC_LEN, param->brMac) != SOFTBUS_OK)) {
        CONN_LOGE(CONN_PROXY, "get real mac failed");
        SoftBusFree(channelInfo);
        return NULL;
    }

    // Check if device supports far field
    P2PDeviceInfo device = {0};
    if (strcpy_s(device.brMac, BT_MAC_LEN, channelInfo->realBrMac) != EOK ||
        strcpy_s(device.uuid, UUID_STRING_LEN, channelInfo->uuid) != EOK) {
        CONN_LOGE(CONN_PROXY, "copy device brMac or uuid failed, ret=%{public}d", ret);
        SoftBusFree(channelInfo);
        return NULL;
    }
    channelInfo->isSupportFarField = FarFieldAdapterIsDeviceSupport(&device);
    ListInit(&channelInfo->node);
    return channelInfo;
}

static bool ProxyChannelInfoMacMatcher(const SoftBusRcObject *object, const void *arg)
{
    const char *brMac = (const char *)arg;
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)object;
    return StrCmpIgnoreCase(brMac, channelInfo->realBrMac) == 0 ||
        StrCmpIgnoreCase(brMac, channelInfo->srcBrMac) == 0;
}

static void OnBrOpenSuccess(uint32_t requestId, struct ProxyChannel *channel)
{
    CONN_CHECK_AND_RETURN_LOGE(channel != NULL, CONN_PROXY, "channel is NULL");
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, channel->brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    bool isSupportFarField = channelInfo->isSupportFarField;
    channelInfo->isDirectly = false;
    OpenProxyChannelCallback callback = channelInfo->callback;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    FarFieldDfxRecordBrOpenSuccess(requestId, channelInfo->realBrMac, isSupportFarField);
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
    if (isSupportFarField) {
        ClearFarFieldProxy(channel->brMac, false);
    }
    if (callback.onOpenSuccess != NULL) {
        callback.onOpenSuccess(requestId, channel);
    }
}

static int32_t StartOpenFieldProxyChannel(uint32_t requestId, ProxyChannelInfo *channelInfo)
{
    FarFieldProxyParam param = {0};
    if (strcpy_s(param.device.brMac, BT_MAC_LEN, channelInfo->realBrMac) != EOK ||
        strcpy_s(param.device.uuid, UUID_STRING_LEN, channelInfo->uuid) != EOK ||
        strcpy_s(param.srcMac, sizeof(param.srcMac), channelInfo->srcBrMac) != EOK) {
        CONN_LOGE(CONN_PROXY, "copy param uuid or srcMac failed");
        return SOFTBUS_STRCPY_ERR;
    }
    param.requestId = requestId;
    param.device.appIndex = channelInfo->appIndex;
    int32_t ret = OpenFarFieldProxyChannel(&param);
    CONN_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, CONN_PROXY, "channelInfo failed=%{public}d", ret);
    ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    bool isDirectly = channelInfo->isDirectly;
    channelInfo->type = FAR_FIELD_PROXY;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    FarFieldDfxRecordConnectStart(requestId, channelInfo->realBrMac, isDirectly);
    return SOFTBUS_OK;
}

static void OnBrOpenFail(uint32_t requestId, int32_t reason, const char *brMac)
{
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    OpenProxyChannelCallback callback = channelInfo->callback;
    bool isFirstConnect = channelInfo->isFirstConnect;
    channelInfo->isDirectly = false;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    if (isFirstConnect) {
        SoftBusRcRemove(&GetProxyChannelManager()->proxyConnectionList, (SoftBusRcObject *)channelInfo);
    }

    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
    if (callback.onOpenFail != NULL) {
        callback.onOpenFail(requestId, reason, brMac);
    }
}

static int32_t UpdateProxyChannelInfo(ProxyChannelInfo *channelInfo, const ProxyChannelParam *param,
    const OpenProxyChannelCallback *callback, bool isSupportFarField)
{
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    channelInfo->requestId = param->requestId;
    channelInfo->isDirectly = true;
    channelInfo->isFirstConnect = param->isFirstConnect;
    channelInfo->appIndex = param->appIndex;
    channelInfo->callback = *callback;
    channelInfo->isSupportFarField = isSupportFarField;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    return SOFTBUS_OK;
}

static int32_t StartProxyChannel(ProxyChannelInfo *channelInfo, ProxyChannelParam *param,
    bool isRealMac, bool isOpenBrProxy, bool isSupportFarField)
{
    if (isOpenBrProxy) {
        OpenProxyChannelCallback innerCallback = {
            .onOpenSuccess = OnBrOpenSuccess, .onOpenFail = OnBrOpenFail,
        };
        return GetBrProxyChannelManager()->openBrProxyChannel(param, isRealMac, isSupportFarField, &innerCallback);
    }
    if (!channelInfo->isFirstConnect) {
        GetBrProxyChannelManager()->addReconnectDeviceInfo(param, isRealMac, isSupportFarField);
    }
    return StartOpenFieldProxyChannel(channelInfo->requestId, channelInfo);
}

static int32_t OpenProxyChannel(ProxyChannelParam *param, const OpenProxyChannelCallback *callback)
{
    CONN_CHECK_AND_RETURN_RET_LOGE(param != NULL, SOFTBUS_INVALID_PARAM, CONN_PROXY, "param is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(callback != NULL, SOFTBUS_INVALID_PARAM, CONN_PROXY, "callback is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(callback->onOpenSuccess != NULL, SOFTBUS_INVALID_PARAM,
        CONN_PROXY, "onOpenSuccess is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(callback->onOpenFail != NULL, SOFTBUS_INVALID_PARAM,
        CONN_PROXY, "onOpenFail is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(SoftBusGetBrState() == BR_ENABLE, SOFTBUS_CONN_BR_DISABLE_ERR,
        CONN_PROXY, "br disable");
    bool isRealMac = IsRealMac(param->brMac);
    bool isAclConnected = false;
    bool isPairedDevice = IsPairedDevice(param->brMac, isRealMac, NULL, &isAclConnected);
    CONN_CHECK_AND_RETURN_RET_LOGE(isPairedDevice, SOFTBUS_CONN_BR_UNPAIRED,
        CONN_PROXY, "is not paired device");
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, param->brMac);
    bool isSupportFarField = false;
    if (channelInfo == NULL) {
        channelInfo = CreateProxyChannelInfo(BR_PROXY, param, isRealMac);
        CONN_CHECK_AND_RETURN_RET_LOGE(channelInfo != NULL, SOFTBUS_MALLOC_ERR, CONN_PROXY, "channelInfo is NULL");
        isSupportFarField = channelInfo->isSupportFarField;
        int32_t ret = SoftBusRcSave(&GetProxyChannelManager()->proxyConnectionList, (SoftBusRcObject *)channelInfo);
        if (ret != SOFTBUS_OK) {
            channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
            return ret;
        }
    } else {
        P2PDeviceInfo device = {0};
        if (strcpy_s(device.brMac, BT_MAC_LEN, channelInfo->realBrMac) == EOK &&
            strcpy_s(device.uuid, UUID_STRING_LEN, channelInfo->uuid) == EOK) {
            isSupportFarField = FarFieldAdapterIsDeviceSupport(&device);
        }
    }
    int32_t ret = UpdateProxyChannelInfo(channelInfo, param, callback, isSupportFarField);
    if (ret != SOFTBUS_OK) {
        SoftBusRcRemove(&GetProxyChannelManager()->proxyConnectionList, (SoftBusRcObject *)channelInfo);
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return ret;
    }

    bool isOpenBrProxy = (!isSupportFarField || isAclConnected) ? true : false;
    ret = StartProxyChannel(channelInfo, param, isRealMac, isOpenBrProxy, isSupportFarField);
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_PROXY, "open fail, error=%{public}d", ret);
        SoftBusRcRemove(&GetProxyChannelManager()->proxyConnectionList, (SoftBusRcObject *)channelInfo);
    }
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
    return ret;
}

static void ClearProxyInfo(struct ProxyChannel *channel)
{
    CONN_CHECK_AND_RETURN_LOGE(channel != NULL, CONN_PROXY, "channel is NULL");
    CONN_LOGE(CONN_PROXY, "channelId=%{public}d", channel->channelId);
    ClearFarFieldProxy(channel->brMac, true);
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, channel->brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    SoftBusRcRemove(&GetProxyChannelManager()->proxyConnectionList, (SoftBusRcObject *)channelInfo);
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
}

static void OnProxyChannelDataReceived(struct ProxyChannel *channel, const uint8_t *data, uint32_t dataLen)
{
    if (g_listener.onProxyChannelDataReceived != NULL) {
        g_listener.onProxyChannelDataReceived(channel, data, dataLen);
    }
}

static void OnBrProxyDisconnected(struct ProxyChannel *channel, int32_t reason)
{
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, channel->brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    channel->requestId = channelInfo->requestId;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
    if (g_listener.onProxyChannelDisconnected != NULL) {
        g_listener.onProxyChannelDisconnected(channel, reason);
    }
    if (reason == SOFTBUS_CONN_BR_UNPAIRED) {
        ClearProxyInfo(channel);
    }
}

static void OnFarFieldProxyDisconnected(struct ProxyChannel *channel, int32_t reason)
{
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, channel->brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    ProxyChannelType type = channelInfo->type;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    if (type != FAR_FIELD_PROXY) {
        CONN_LOGI(CONN_PROXY, "not far field proxy, type=%{public}d", type);
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    FarFieldDfxRecordDisconnected(channel->requestId, channelInfo->realBrMac, reason);
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
    if (g_listener.onProxyChannelDisconnected != NULL) {
        g_listener.onProxyChannelDisconnected(channel, SOFTBUS_FAR_FIELD_DISCONNECT);
    }
}

static void OnBrProxyReconnected(const char *addr, struct ProxyChannel *channel)
{
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, addr);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    bool isDirectly = channelInfo->isDirectly;
    OpenProxyChannelCallback callback = channelInfo->callback;
    uint32_t requestId = channelInfo->requestId;
    channelInfo->isDirectly = false;
    channelInfo->type = BR_PROXY;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);

    channel->requestId = requestId;
    ClearFarFieldProxy(addr, false);
    if (isDirectly) {
        CONN_LOGI(CONN_PROXY, "br reconnect during far field opening");
        if (callback.onOpenSuccess != NULL) {
            callback.onOpenSuccess(requestId, channel);
        }
    } else {
        if (g_listener.onProxyChannelReconnected != NULL) {
            g_listener.onProxyChannelReconnected((char *)addr, channel);
        }
    }
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
}

static void OnBrProxyDisable(uint32_t requestId, const char *addr, bool *isSupportFarField)
{
    (void)requestId;
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, addr);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    bool isSupport = channelInfo->isSupportFarField;
    if (!channelInfo->isSupportFarField) {
        P2PDeviceInfo device = {0};
        if (strcpy_s(device.brMac, BT_MAC_LEN, channelInfo->realBrMac) == EOK &&
            strcpy_s(device.uuid, UUID_STRING_LEN, channelInfo->uuid) == EOK) {
            isSupport = FarFieldAdapterIsDeviceSupport(&device);
        }
    }
    *isSupportFarField = isSupport;
    if (!isSupport) {
        CONN_LOGE(CONN_PROXY, "not support far filed ability");
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    channelInfo->isSupportFarField = isSupport;
    channelInfo->isDirectly = false;
    uint32_t channelRequestId = channelInfo->requestId;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);

    (void)StartOpenFieldProxyChannel(channelRequestId, channelInfo);
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
}

static void OnFarFieldOpenFail(uint32_t requestId, int32_t reason, const char *brMac)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_PROXY, "brMac is NULL");
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo is NULL");
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    bool isDirectly = channelInfo->isDirectly;
    bool isFirstConnect = channelInfo->isFirstConnect;
    uint32_t channelRequestId = channelInfo->requestId;
    int32_t retReson = isFirstConnect ? reason : SOFTBUS_FAR_FIELD_OPEN_FAIL;
    OpenProxyChannelCallback callback = channelInfo->callback;
    channelInfo->isDirectly = false;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    FarFieldDfxRecordConnectFail(requestId, brMac, reason, isDirectly);
    CONN_LOGI(CONN_PROXY, "reason=%{public}d, isDirectly=%{public}d,"
        "isFirstConnect=%{public}d", reason, isDirectly, isFirstConnect);
    if (callback.onOpenFail != NULL && isDirectly) {
        if (isFirstConnect) {
            ClearFarFieldProxy(brMac, false);
            SoftBusRcRemove(&GetProxyChannelManager()->proxyConnectionList, (SoftBusRcObject *)channelInfo);
        }
        callback.onOpenFail(channelRequestId, retReson, brMac);
    }
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
}

static void OnFarFieldConnected(uint32_t requestId, struct ProxyChannel *channel)
{
    CONN_CHECK_AND_RETURN_LOGE(channel != NULL, CONN_PROXY, "channel is NULL");

    // Find the corresponding ProxyChannelInfo by brMac
    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, channel->brMac);
    CONN_CHECK_AND_RETURN_LOGE(channelInfo != NULL, CONN_PROXY, "channelInfo not found");

    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }
    uint32_t channelRequestId = channelInfo->requestId;
    if (channelInfo->type != FAR_FIELD_PROXY) {
        CONN_LOGW(CONN_PROXY, "Not a far field proxy channel");
        channelInfo->Unlock((SoftBusRcObject *)channelInfo);
        ClearFarFieldProxy(channel->brMac, false);
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return;
    }

    bool isDirectly = channelInfo->isDirectly;
    bool isFirstConnect = channelInfo->isFirstConnect;
    bool isSupportFarField = channelInfo->isSupportFarField;
    channelInfo->isDirectly = false;
    OpenProxyChannelCallback callback = channelInfo->callback;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    FarFieldDfxRecordConnectSuccess(requestId, channelInfo->realBrMac, isDirectly);
    if (isFirstConnect) {
        ProxyChannelParam param = {0};
        param.requestId = channelRequestId;
        if (strcpy_s(param.brMac, BT_MAC_MAX_LEN, channelInfo->srcBrMac) == EOK &&
            strcpy_s(param.uuid, UUID_STRING_LEN, channelInfo->uuid) == EOK) {
            GetBrProxyChannelManager()->addReconnectDeviceInfo(&param, IsRealMac(channelInfo->srcBrMac),
                isSupportFarField);
        }
    }
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);

    if (isDirectly) {
        if (callback.onOpenSuccess != NULL) {
            callback.onOpenSuccess(channelRequestId, channel);
        }
    } else {
        if (g_listener.onProxyChannelReconnected != NULL) {
            g_listener.onProxyChannelReconnected(channel->brMac, channel);
        }
    }
}

static int32_t RegisterProxyChannelListener(ProxyConnectListener *listener)
{
    CONN_CHECK_AND_RETURN_RET_LOGE(listener != NULL, SOFTBUS_INVALID_PARAM, CONN_PROXY, "listener is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(listener->onProxyChannelDisconnected != NULL, SOFTBUS_INVALID_PARAM,
        CONN_PROXY, "Disconnected is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(listener->onProxyChannelDataReceived != NULL, SOFTBUS_INVALID_PARAM,
        CONN_PROXY, "DataReceived is NULL");
    CONN_CHECK_AND_RETURN_RET_LOGE(listener->onProxyChannelReconnected != NULL, SOFTBUS_INVALID_PARAM,
        CONN_PROXY, "Reconnected is NULL");
    g_listener = *listener;
    return SOFTBUS_OK;
}

static void RefreshFarField(const ProxyChannelParam *param, const char *realBrMac)
{
    FarFieldProxyParam farFieldParam = {0};
    farFieldParam.requestId = param->requestId;
    farFieldParam.device.appIndex = param->appIndex;
    if (strcpy_s(farFieldParam.device.brMac, BT_MAC_LEN, realBrMac) != EOK ||
        strcpy_s(farFieldParam.device.uuid, UUID_STRING_LEN, param->uuid) != EOK ||
        strcpy_s(farFieldParam.srcMac, sizeof(farFieldParam.srcMac), param->brMac) != EOK) {
        CONN_LOGE(CONN_PROXY, "copy farFieldParam failed");
        return;
    }
    FarFieldProxyRefreshChannel(&farFieldParam);
}

static int32_t ProxyChannelRefresh(ProxyChannelParam *param, const OpenProxyChannelCallback *callback)
{
    CONN_CHECK_AND_RETURN_RET_LOGE(param != NULL, SOFTBUS_INVALID_PARAM, CONN_PROXY, "param is null");
    CONN_CHECK_AND_RETURN_RET_LOGE(callback != NULL, SOFTBUS_INVALID_PARAM, CONN_PROXY, "callback is null");

    ProxyChannelInfo *channelInfo = (ProxyChannelInfo *)SoftBusRcGetCommon(
        &GetProxyChannelManager()->proxyConnectionList, ProxyChannelInfoMacMatcher, param->brMac);
    if (channelInfo == NULL) {
        return GetProxyChannelManager()->openProxyChannel(param, callback);
    }
    int32_t ret = channelInfo->Lock((SoftBusRcObject *)channelInfo);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return ret;
    }
    bool isSupportFarField = channelInfo->isSupportFarField;
    channelInfo->Unlock((SoftBusRcObject *)channelInfo);
    P2PDeviceInfo device = {0};
    if (strcpy_s(device.brMac, BT_MAC_LEN, channelInfo->realBrMac) != EOK) {
        CONN_LOGE(CONN_PROXY, "copy realBrMac failed");
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return SOFTBUS_STRCPY_ERR;
    }
    if (!isSupportFarField) {
        if (strcpy_s(device.uuid, UUID_STRING_LEN, channelInfo->uuid) == EOK) {
            isSupportFarField = FarFieldAdapterIsDeviceSupport(&device);
        }
    }
    ret = UpdateProxyChannelInfo(channelInfo, param, callback, isSupportFarField);
    if (ret != SOFTBUS_OK) {
        channelInfo->Dereference((SoftBusRcObject **)&channelInfo);
        return ret;
    }
    channelInfo->Dereference((SoftBusRcObject **)&channelInfo);

    bool isAclConnected = false;
    bool isPairedDevice = IsPairedDevice(device.brMac, true, NULL, &isAclConnected);
    CONN_CHECK_AND_RETURN_RET_LOGE(isPairedDevice, SOFTBUS_CONN_BR_UNPAIRED, CONN_PROXY, "is not paired device");

    if (!isSupportFarField || isAclConnected) {
        OpenProxyChannelCallback innerCallback = {
            .onOpenSuccess = OnBrOpenSuccess,
            .onOpenFail = OnBrOpenFail,
        };
        return GetBrProxyChannelManager()->refreshBrProxyChannel(param, IsRealMac(param->brMac),
            isSupportFarField, &innerCallback);
    }
    RefreshFarField(param, device.brMac);
    return SOFTBUS_OK;
}

static uint32_t GenerateChannelId(void)
{
    static uint16_t nextId = 0;
    return (CONNECT_PROXY_CHANNEL << CONNECT_TYPE_SHIFT) + (++nextId);
}

static ProxyChannelManager g_proxyChannelManager = {
    .generateRequestId = GenerateRequestId,
    .openProxyChannel = OpenProxyChannel,
    .refreshProxyChannel = ProxyChannelRefresh,
    .registerProxyChannelListener = RegisterProxyChannelListener,
    .generateChannelId = GenerateChannelId,
    .clearProxyInfo = ClearProxyInfo,
};

SOFTBUS_VISIBILITY ProxyChannelManager *GetProxyChannelManager(void)
{
    return &g_proxyChannelManager;
}

SOFTBUS_VISIBILITY int32_t ProxyChannelManagerInit(void)
{
    int32_t ret = SoftBusRcCollectionConstruct("proxy_channel", &GetProxyChannelManager()->proxyConnectionList, NULL);
    CONN_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, SOFTBUS_CREATE_LIST_ERR,
        CONN_INIT, "create channels list fail");
    ret = SoftBusMutexInit(&g_reqIdLock, NULL);
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_PROXY, "init lock falied");
        SoftBusRcCollectionDestruct(&g_proxyChannelManager.proxyConnectionList);
        return ret;
    }

    static BrProxyListener listener = {
        .onProxyChannelDataReceived = OnProxyChannelDataReceived,
        .onProxyChannelDisconnected = OnBrProxyDisconnected,
        .onProxyChannelReconnected = OnBrProxyReconnected,
        .onBrProxyStateChanged = OnBrProxyDisable,
    };
    ret = GetBrProxyChannelManager()->registerBrProxyListener(&listener);
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_PROXY, "register br proxy listener failed, ret=%{public}d", ret);
        SoftBusRcCollectionDestruct(&g_proxyChannelManager.proxyConnectionList);
        return ret;
    }
    static FarFieldProxyListener farFieldProxyListener = {
        .onFarFieldProxyDataReceived = OnProxyChannelDataReceived,
        .onFarFieldProxyDisconnected = OnFarFieldProxyDisconnected,
        .onFarFieldConnected = OnFarFieldConnected,
        .onFarFieldOpenFail = OnFarFieldOpenFail,
    };
    ret = RegisterFarFieldProxyListener(&farFieldProxyListener);
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_PROXY, "register far field listener failed");
        SoftBusRcCollectionDestruct(&g_proxyChannelManager.proxyConnectionList);
        return ret;
    }
    ret = FarFieldProxyManagerInit();
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_PROXY, "far field init failed=%{public}d", ret);
        SoftBusRcCollectionDestruct(&g_proxyChannelManager.proxyConnectionList);
        return ret;
    }
    ret = FarFieldProxyDfxInit();
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_PROXY, "far field dfx init failed=%{public}d", ret);
        SoftBusRcCollectionDestruct(&g_proxyChannelManager.proxyConnectionList);
        return ret;
    }
    ret = BrProxyChannelManagerInit();
    CONN_LOGI(CONN_PROXY, "ret=%{public}d", ret);
    return ret;
}
