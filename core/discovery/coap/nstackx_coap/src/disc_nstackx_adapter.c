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

#include "disc_nstackx_adapter.h"

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "nstackx.h"
#include "anonymizer.h"
#include "bus_center_manager.h"
#include "disc_coap_capability.h"
#include "disc_coap_parser.h"
#include "disc_log.h"
#include "lnn_ohos_account.h"
#include "locale_config_wrapper.h"
#include "securec.h"
#include "softbus_adapter_crypto.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "softbus_feature_config.h"
#include "legacy/softbus_hidumper_disc.h"
#include "legacy/softbus_hisysevt_discreporter.h"
#include "softbus_json_utils.h"
#include "softbus_utils.h"

#define WLAN_IFACE_NAME_PREFIX "wlan"
#define NCM_LINK_NAME_PREFIX   "ncm0"
#define NCM_HOST_NAME_PREFIX   "wwan0"
#define DISC_FREQ_COUNT_MASK   0xFFFF
#define DISC_FREQ_DURATION_BIT 16
#define DISC_USECOND           1000
#define DEFAULT_MAX_DEVICE_NUM 20

#define NSTACKX_LOCAL_DEV_INFO "NstackxLocalDevInfo"
#define HYPHEN_ZH        "的"
#define HYPHEN_EXCEPT_ZH "-"
#define EMPTY_STRING     ""

static NSTACKX_LocalDeviceInfoV2 *g_localDeviceInfo = NULL;
static DiscInnerCallback *g_discCoapInnerCb = NULL;
static SoftBusMutex g_localDeviceInfoLock = {0};
static SoftBusMutex g_discCoapInnerCbLock = {0};
static int32_t NstackxLocalDevInfoDump(int fd);
static int32_t g_LinkStatus[MAX_IF + 1] = { LINK_STATUS_DOWN, LINK_STATUS_DOWN };
static int32_t g_currentLinkUpNums = 0;
static char g_serviceData[NSTACKX_MAX_SERVICE_DATA_LEN] = {0};

#if defined(DSOFTBUS_FEATURE_DISC_LNN_COAP) || defined(DSOFTBUS_FEATURE_DISC_SHARE_COAP)
static int32_t FillRspSettings(NSTACKX_ResponseSettings *settings, const DeviceInfo *deviceInfo, uint8_t bType)
{
    settings->businessData = NULL;
    settings->length = 0;
    settings->businessType = bType;
 
    bool hasMatchIp = false;
    char localIp[IP_STR_MAX_LEN] = {0};
    char localNetifName[NSTACKX_MAX_INTERFACE_NAME_LEN] = {0};
    for (int32_t index = 0; index <= MAX_IF; index++) {
        if (g_LinkStatus[index] == LINK_STATUS_DOWN) {
            continue;
        }
        int32_t ret = 0;
        ret = LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_IP, localIp, IP_STR_MAX_LEN, index);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "get localIp failed");
        ret = LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_NET_IF_NAME, localNetifName,
            NSTACKX_MAX_INTERFACE_NAME_LEN, index);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "get localNetifName failed");
        if (strncmp(localIp, deviceInfo->addr[0].info.ip.ip, IP_STR_MAX_LEN) == 0) {
            hasMatchIp = true;
            break;
        }
    }
    DISC_CHECK_AND_RETURN_RET_LOGE(hasMatchIp, SOFTBUS_INVALID_PARAM, DISC_COAP, "no match ip, do not rsp");
    if (strcpy_s(settings->localNetworkName, sizeof(settings->localNetworkName), localNetifName) != EOK) {
        DISC_LOGE(DISC_COAP, "copy disc response settings network name failed");
        goto EXIT;
    }
    if (strcpy_s(settings->remoteIp, sizeof(settings->remoteIp), deviceInfo->addr[0].info.ip.ip) != EOK) {
        DISC_LOGE(DISC_COAP, "copy disc response settings remote IP failed");
        goto EXIT;
    }
    return SOFTBUS_OK;
EXIT:
    return SOFTBUS_STRCPY_ERR;
}
#endif /* DSOFTBUS_FEATURE_DISC_LNN_COAP || DSOFTBUS_FEATURE_DISC_SHARE_COAP */

int32_t DiscCoapSendRsp(const DeviceInfo *deviceInfo, uint8_t bType)
{
#if defined(DSOFTBUS_FEATURE_DISC_LNN_COAP) || defined(DSOFTBUS_FEATURE_DISC_SHARE_COAP)
    DISC_CHECK_AND_RETURN_RET_LOGE(deviceInfo, SOFTBUS_INVALID_PARAM, DISC_COAP, "DiscRsp devInfo is null");
    NSTACKX_ResponseSettings *settings = (NSTACKX_ResponseSettings *)SoftBusCalloc(sizeof(NSTACKX_ResponseSettings));
    DISC_CHECK_AND_RETURN_RET_LOGE(settings, SOFTBUS_MALLOC_ERR, DISC_COAP, "malloc disc response settings failed");

    int32_t ret = FillRspSettings(settings, deviceInfo, bType);
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "fill nstackx response settings failed");
        SoftBusFree(settings);
        return ret;
    }

    DISC_LOGI(DISC_COAP, "send rsp with bType=%{public}u", bType);
    ret = NSTACKX_SendDiscoveryRsp(settings);
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "disc send response failed, ret=%{public}d", ret);
    }
    SoftBusFree(settings);
    return ret;
#else
    return SOFTBUS_OK;
#endif /* DSOFTBUS_FEATURE_DISC_LNN_COAP || DSOFTBUS_FEATURE_DISC_SHARE_COAP */
}

static int32_t ParseReservedInfo(const NSTACKX_DeviceInfo *nstackxDevice, DeviceInfo *device, char *nickName)
{
    cJSON *reserveInfo = cJSON_Parse(nstackxDevice->reservedInfo);
    DISC_CHECK_AND_RETURN_RET_LOGE(reserveInfo != NULL, SOFTBUS_PARSE_JSON_ERR, DISC_COAP,
        "parse reserve data failed.");

    DiscCoapParseWifiIpAddr(reserveInfo, device);
    DiscCoapParseHwAccountHash(reserveInfo, device);
    DiscCoapParseNickname(reserveInfo, nickName, DISC_MAX_NICKNAME_LEN);
    if (DiscCoapParseServiceData(reserveInfo, device) != SOFTBUS_OK) {
        DISC_LOGD(DISC_COAP, "parse service data failed");
    }
    cJSON_Delete(reserveInfo);
    return SOFTBUS_OK;
}

static int32_t SpliceCoapDisplayName(char *devName, char *nickName, DeviceInfo *device)
{
    char *hyphen = NULL;
    bool isSameAccount = false;
    bool isZH = IsZHLanguage();
    char accountIdStr[MAX_ACCOUNT_HASH_LEN] = { 0 };
    char accountHash[MAX_ACCOUNT_HASH_LEN] = { 0 };
    int32_t ret = SOFTBUS_OK;

    if (!LnnIsDefaultOhosAccount()) {
        int64_t accountId = 0;
        ret = LnnGetLocalNum64Info(NUM_KEY_ACCOUNT_LONG, &accountId);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "get local account failed");

        ret = sprintf_s(accountIdStr, MAX_ACCOUNT_HASH_LEN, "%ju", (uint64_t)accountId);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret >= 0, SOFTBUS_STRCPY_ERR, DISC_COAP,
            "set accountIdStr error, ret=%{public}d", ret);
        ret = SoftBusGenerateStrHash((const unsigned char *)accountIdStr, strlen(accountIdStr),
            (unsigned char *)accountHash);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP,
            "generate account hash failed, ret=%{public}d", ret);

        if (memcmp(device->accountHash, accountHash, MAX_ACCOUNT_HASH_LEN) == 0) {
            isSameAccount = true;
        }
    }
    if (!isSameAccount && strlen(nickName) > 0) {
        hyphen = isZH ? (char *)HYPHEN_ZH : (char *)HYPHEN_EXCEPT_ZH;
    } else {
        hyphen = (char *)EMPTY_STRING;
    }

    ret = sprintf_s(device->devName, DISC_MAX_DEVICE_NAME_LEN, "%s%s%s",
        isSameAccount ? EMPTY_STRING : nickName, hyphen, devName);
    DISC_CHECK_AND_RETURN_RET_LOGE(ret >= 0, SOFTBUS_STRCPY_ERR, DISC_COAP,
        "splice displayname failed, ret=%{public}d", ret);

    return SOFTBUS_OK;
}

static int32_t ParseDiscDevInfo(const NSTACKX_DeviceInfo *nstackxDevInfo, DeviceInfo *discDevInfo)
{
    char devName[DISC_MAX_DEVICE_NAME_LEN] = { 0 };
    char nickName[DISC_MAX_NICKNAME_LEN] = { 0 };
    if (strcpy_s(devName, DISC_MAX_DEVICE_NAME_LEN, nstackxDevInfo->deviceName) != EOK ||
        memcpy_s(discDevInfo->capabilityBitmap, sizeof(discDevInfo->capabilityBitmap),
                 nstackxDevInfo->capabilityBitmap, sizeof(nstackxDevInfo->capabilityBitmap)) != EOK) {
        DISC_LOGE(DISC_COAP, "strcpy_s devName or memcpy_s capabilityBitmap failed.");
        return SOFTBUS_MEM_ERR;
    }

    discDevInfo->devType = (DeviceType)nstackxDevInfo->deviceType;
    discDevInfo->capabilityBitmapNum = nstackxDevInfo->capabilityBitmapNum;

    if (strncmp(nstackxDevInfo->networkName, WLAN_IFACE_NAME_PREFIX, strlen(WLAN_IFACE_NAME_PREFIX)) == 0) {
        discDevInfo->addr[0].type = CONNECTION_ADDR_WLAN;
    } else if (strncmp(nstackxDevInfo->networkName, NCM_LINK_NAME_PREFIX, strlen(NCM_LINK_NAME_PREFIX)) == 0 ||
        strncmp(nstackxDevInfo->networkName, NCM_HOST_NAME_PREFIX, strlen(NCM_HOST_NAME_PREFIX)) == 0) {
        discDevInfo->addr[0].type = CONNECTION_ADDR_NCM;
    } else {
        discDevInfo->addr[0].type = CONNECTION_ADDR_ETH;
    }

    int32_t ret = DiscCoapParseDeviceUdid(nstackxDevInfo->deviceId, discDevInfo);
    DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP,
        "parse device udid failed, ret=%{public}d", ret);

    ret = ParseReservedInfo(nstackxDevInfo, discDevInfo, nickName);
    DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP,
        "parse reserve information failed, ret=%{public}d", ret);

    // coap not support range now, just assign -1 as unknown
    discDevInfo->range = -1;
    ret = SpliceCoapDisplayName(devName, nickName, discDevInfo);
    DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP,
        "parse display name failed, ret=%{public}d", ret);

    return SOFTBUS_OK;
}

static void OnDeviceFound(const NSTACKX_DeviceInfo *deviceList, uint32_t deviceCount)
{
    DISC_CHECK_AND_RETURN_LOGE(deviceList != NULL && deviceCount != 0, DISC_COAP, "invalid param.");
    DISC_LOGD(DISC_COAP, "Disc device found, count=%{public}u", deviceCount);
    DeviceInfo *discDeviceInfo = (DeviceInfo *)SoftBusCalloc(sizeof(DeviceInfo));
    DISC_CHECK_AND_RETURN_LOGE(discDeviceInfo != NULL, DISC_COAP, "malloc device info failed.");

    int32_t ret;
    for (uint32_t i = 0; i < deviceCount; i++) {
        const NSTACKX_DeviceInfo *nstackxDeviceInfo = deviceList + i;
        if (nstackxDeviceInfo == NULL) {
            DISC_LOGE(DISC_COAP, "device count from nstackx is invalid");
            SoftBusFree(discDeviceInfo);
            return;
        }

        if ((nstackxDeviceInfo->update & 0x1) == 0) {
            char *anonymizedName = NULL;
            Anonymize(nstackxDeviceInfo->deviceName, &anonymizedName);
            DISC_LOGI(DISC_COAP, "duplicate device do not need report. deviceName=%{public}s",
                AnonymizeWrapper(anonymizedName));
            AnonymizeFree(anonymizedName);
            continue;
        }
        (void)memset_s(discDeviceInfo, sizeof(DeviceInfo), 0, sizeof(DeviceInfo));
        ret = ParseDiscDevInfo(nstackxDeviceInfo, discDeviceInfo);
        if (ret != SOFTBUS_OK) {
            DISC_LOGW(DISC_COAP, "parse discovery device info failed.");
            continue;
        }
        ret = DiscCoapProcessDeviceInfo(nstackxDeviceInfo, discDeviceInfo, g_discCoapInnerCb,
            &g_discCoapInnerCbLock);
        if (ret != SOFTBUS_OK) {
            DISC_LOGD(DISC_COAP, "DiscRecv: process device info failed, ret=%{public}d", ret);
        }
    }

    SoftBusFree(discDeviceInfo);
}

static void OnNotificationReceived(const NSTACKX_NotificationConfig *notification)
{
    DiscCoapReportNotification(notification);
}

static NSTACKX_Parameter g_nstackxCallBack = {
    .onDeviceListChanged = OnDeviceFound,
    .onDeviceFound = NULL,
    .onMsgReceived = NULL,
    .onDFinderMsgReceived = NULL,
    .onNotificationReceived = OnNotificationReceived,
};

int32_t DiscCoapRegisterCb(const DiscInnerCallback *discCoapCb)
{
    DISC_CHECK_AND_RETURN_RET_LOGE(discCoapCb != NULL, SOFTBUS_INVALID_PARAM, DISC_COAP, "invalid param");
    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_discCoapInnerCbLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
    if (g_discCoapInnerCb == NULL) {
        DISC_LOGE(DISC_COAP, "coap inner callback not init.");
        (void)SoftBusMutexUnlock(&g_discCoapInnerCbLock);
        return SOFTBUS_DISCOVER_COAP_NOT_INIT;
    }
    if (memcpy_s(g_discCoapInnerCb, sizeof(DiscInnerCallback), discCoapCb, sizeof(DiscInnerCallback)) != EOK) {
        DISC_LOGE(DISC_COAP, "memcpy_s failed.");
        (void)SoftBusMutexUnlock(&g_discCoapInnerCbLock);
        return SOFTBUS_MEM_ERR;
    }
    (void)SoftBusMutexUnlock(&g_discCoapInnerCbLock);
    return SOFTBUS_OK;
}

int32_t DiscCoapRegisterCapability(uint32_t capabilityBitmapNum, uint32_t capabilityBitmap[])
{
    DISC_CHECK_AND_RETURN_RET_LOGE(capabilityBitmapNum != 0, SOFTBUS_INVALID_PARAM,
        DISC_COAP, "capabilityBitmapNum=0");

    if (NSTACKX_RegisterCapability(capabilityBitmapNum, capabilityBitmap) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "NSTACKX Register Capability failed");
        return SOFTBUS_DISCOVER_COAP_REGISTER_CAP_FAIL;
    }
    return SOFTBUS_OK;
}

int32_t DiscCoapSetFilterCapability(uint32_t capabilityBitmapNum, uint32_t capabilityBitmap[])
{
    DISC_CHECK_AND_RETURN_RET_LOGE(capabilityBitmapNum != 0, SOFTBUS_INVALID_PARAM,
        DISC_COAP, "capabilityBitmapNum=0");

    if (NSTACKX_SetFilterCapability(capabilityBitmapNum, capabilityBitmap) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "NSTACKX SetFilter Capability failed");
        SoftbusReportDiscFault(SOFTBUS_HISYSEVT_DISC_MEDIUM_COAP, SOFTBUS_HISYSEVT_DISCOVER_COAP_SET_FILTER_CAP_FAIL);
        return SOFTBUS_DISCOVER_COAP_SET_FILTER_CAP_FAIL;
    }
    return SOFTBUS_OK;
}

static int32_t RegisterServiceData()
{
    int32_t port = 0;
    char ip[IP_STR_MAX_LEN] = {0};
    int32_t cnt = 0;
    struct NSTACKX_ServiceData serviceData[MAX_IF + 1] = { 0 };
    for (uint32_t index = 0; index <= MAX_IF; index++) {
        if (g_LinkStatus[index] == LINK_STATUS_DOWN) {
            continue;
        }
 
        int32_t ret = 0;
        ret = LnnGetLocalNumInfoByIfnameIdx(NUM_KEY_AUTH_PORT, &port, index);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "get local port failed");
        ret = LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_IP, ip, IP_STR_MAX_LEN, index);
        DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "get local ip failed");
 
        if (strcpy_s(serviceData[cnt].ip, IP_STR_MAX_LEN, ip) != SOFTBUS_OK) {
            DISC_LOGE(DISC_COAP, "strcpy ip error.");
            return SOFTBUS_STRCPY_ERR;
        }
        DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
            DISC_COAP, "lock failed");
        if (sprintf_s(serviceData[cnt].serviceData, NSTACKX_MAX_SERVICE_DATA_LEN, "port:%d,%s",
            port, g_serviceData) < 0) {
            DISC_LOGE(DISC_COAP, "write service data failed.");
            (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
            return SOFTBUS_STRCPY_ERR;
        }
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        cnt++;
    }
    int32_t ret = NSTACKX_RegisterServiceDataV2(serviceData, cnt);
    DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "register servicedata to dfinder failed");
    return SOFTBUS_OK;
}

int32_t DiscCoapRegisterServiceData(const PublishOption *option, uint32_t allCap)
{
#ifdef DSOFTBUS_FEATURE_DISC_COAP
    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
    int32_t ret = DiscCoapFillServiceData(option, g_serviceData, NSTACKX_MAX_SERVICE_DATA_LEN, allCap);
    if (ret != SOFTBUS_OK) {
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        DISC_LOGE(DISC_COAP, "fill castJson failed. ret=%{public}d", ret);
        return ret;
    }
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
#endif /* DSOFTBUS_FEATURE_DISC_COAP */
    int32_t result = RegisterServiceData();
    DISC_CHECK_AND_RETURN_RET_LOGE(result == SOFTBUS_OK, result, DISC_COAP,
        "register service data to nstackx failed. result=%{public}d", result);
    return SOFTBUS_OK;
}

#ifdef DSOFTBUS_FEATURE_DISC_SHARE_COAP
int32_t DiscCoapRegisterCapabilityData(const unsigned char *capabilityData, uint32_t dataLen, uint32_t capability)
{
    if (capabilityData == NULL || dataLen == 0) {
        // no capability data, no need to parse and register
        return SOFTBUS_OK;
    }
    char *registerCapaData = (char *)SoftBusCalloc(MAX_CAPABILITYDATA_LEN);
    DISC_CHECK_AND_RETURN_RET_LOGE(registerCapaData, SOFTBUS_MALLOC_ERR, DISC_COAP, "malloc capability data failed");
    int32_t ret = DiscCoapAssembleCapData(capability, (const char *)capabilityData, dataLen, registerCapaData,
        DISC_MAX_CUST_DATA_LEN);
    if (ret == SOFTBUS_FUNC_NOT_SUPPORT) {
        DISC_LOGI(DISC_COAP, "the capability not support yet. capability=%{public}u", capability);
        SoftBusFree(registerCapaData);
        return SOFTBUS_OK;
    }
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "assemble the data of capability failed. capability=%{public}u", capability);
        SoftBusFree(registerCapaData);
        return ret;
    }

    if (NSTACKX_RegisterExtendServiceData(registerCapaData) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "register extend service data to nstackx failed");
        SoftBusFree(registerCapaData);
        return SOFTBUS_DISCOVER_COAP_REGISTER_CAP_DATA_FAIL;
    }
    DISC_LOGI(DISC_COAP, "register extend service data to nstackx succ.");
    SoftBusFree(registerCapaData);
    return SOFTBUS_OK;
}
#endif /* DSOFTBUS_FEATURE_DISC_SHARE_COAP */

static int32_t GetDiscFreq(int32_t freq, uint32_t *discFreq)
{
    uint32_t arrayFreq[FREQ_BUTT] = { 0 };
    int32_t ret = SoftbusGetConfig(SOFTBUS_INT_DISC_FREQ, (unsigned char *)arrayFreq, sizeof(arrayFreq));
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "disc get freq failed");
        return ret;
    }
    *discFreq = arrayFreq[freq];
    return SOFTBUS_OK;
}

static int32_t ConvertDiscoverySettings(NSTACKX_DiscoverySettings *discSet, const DiscCoapOption *option)
{
    if (option->mode == ACTIVE_PUBLISH) {
        discSet->discoveryMode = PUBLISH_MODE_PROACTIVE;
    } else {
        discSet->discoveryMode = DISCOVER_MODE;
    }
    uint32_t discFreq;
    int32_t ret = GetDiscFreq(option->freq, &discFreq);
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "get discovery freq config failed");
        return ret;
    }
    discSet->advertiseCount = discFreq & DISC_FREQ_COUNT_MASK;
    discSet->advertiseDuration = (discFreq >> DISC_FREQ_DURATION_BIT) * DISC_USECOND;
    ret = DiscFillBtype(option->capability, option->allCap, discSet);
    DISC_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, DISC_COAP, "unsupport capability");
    return SOFTBUS_OK;
}

static void FreeDiscSet(NSTACKX_DiscoverySettings *discSet)
{
    if (discSet != NULL) {
        SoftBusFree(discSet->businessData);
        SoftBusFree(discSet);
    }
}

int32_t DiscCoapStartDiscovery(DiscCoapOption *option)
{
    DISC_CHECK_AND_RETURN_RET_LOGE(option != NULL, SOFTBUS_INVALID_PARAM, DISC_COAP, "option is null.");
    DISC_CHECK_AND_RETURN_RET_LOGE(option->mode >= ACTIVE_PUBLISH && option->mode <= ACTIVE_DISCOVERY,
        SOFTBUS_INVALID_PARAM, DISC_COAP, "option->mode is invalid");
    DISC_CHECK_AND_RETURN_RET_LOGE(LOW <= option->freq && option->freq < FREQ_BUTT, SOFTBUS_INVALID_PARAM,
        DISC_COAP, "invalid freq. freq=%{public}d", option->freq);
    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
 
    if (g_currentLinkUpNums == 0) {
        DISC_LOGE(DISC_COAP, "netif not works");
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        return SOFTBUS_NETWORK_NOT_FOUND;
    }
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);

    NSTACKX_DiscoverySettings *discSet = (NSTACKX_DiscoverySettings *)SoftBusCalloc(sizeof(NSTACKX_DiscoverySettings));
    DISC_CHECK_AND_RETURN_RET_LOGE(discSet != NULL, SOFTBUS_MEM_ERR, DISC_COAP, "malloc disc settings failed");

    int32_t ret = ConvertDiscoverySettings(discSet, option);
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "set discovery settings failed");
        FreeDiscSet(discSet);
        return ret;
    }
    if (NSTACKX_StartDeviceDiscovery(discSet) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "start device discovery failed");
        FreeDiscSet(discSet);
        return (option->mode == ACTIVE_PUBLISH) ? SOFTBUS_DISCOVER_COAP_START_PUBLISH_FAIL :
            SOFTBUS_DISCOVER_COAP_START_DISCOVER_FAIL;
    }
    FreeDiscSet(discSet);
    return SOFTBUS_OK;
}

int32_t DiscCoapStopDiscovery(void)
{
    if (NSTACKX_StopDeviceFind() != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "stop device discovery failed");
        return SOFTBUS_DISCOVER_COAP_STOP_DISCOVER_FAIL;
    }

    return SOFTBUS_OK;
}

static char *GetDeviceId(void)
{
    char *formatString = NULL;
    char udid[UDID_BUF_LEN] = { 0 };
    int32_t ret = LnnGetLocalStrInfo(STRING_KEY_DEV_UDID, udid, sizeof(udid));
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "get udid failed, ret=%{public}d", ret);
        return NULL;
    }
    cJSON *deviceId = cJSON_CreateObject();
    DISC_CHECK_AND_RETURN_RET_LOGW(deviceId != NULL, NULL, DISC_COAP, "create json object failed: deviceId=NULL");

    if (!AddStringToJsonObject(deviceId, DEVICE_UDID, udid)) {
        DISC_LOGE(DISC_COAP, "add udid to device id json object failed.");
        goto GET_DEVICE_ID_END;
    }
    formatString = cJSON_PrintUnformatted(deviceId);
    if (formatString == NULL) {
        DISC_LOGE(DISC_COAP, "format device id json object failed.");
    }

GET_DEVICE_ID_END:
    cJSON_Delete(deviceId);
    return formatString;
}

static int32_t SetLocalDeviceInfo(int32_t ifnameIdx)
{
    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
    if (g_localDeviceInfo == NULL) {
        DISC_LOGE(DISC_COAP, "disc coap not init");
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        return SOFTBUS_DISCOVER_COAP_NOT_INIT;
    }
    int32_t res = memset_s(g_localDeviceInfo->localIfInfo, sizeof(NSTACKX_InterfaceInfo), 0,
        sizeof(NSTACKX_InterfaceInfo));
    if (res != EOK) {
        DISC_LOGE(DISC_COAP, "memset_s local device info failed");
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        return SOFTBUS_MEM_ERR;
    }

    int32_t deviceType = 0;
    int32_t ret = LnnGetLocalNumInfo(NUM_KEY_DEV_TYPE_ID, &deviceType);
    if (ret != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "get local device type failed.");
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        return ret;
    }
    g_localDeviceInfo->name = "";
    g_localDeviceInfo->deviceType = (uint32_t)deviceType;
    g_localDeviceInfo->businessType = (uint8_t)NSTACKX_BUSINESS_TYPE_NULL;
 
    if (LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_IP, g_localDeviceInfo->localIfInfo->networkIpAddr,
        sizeof(g_localDeviceInfo->localIfInfo->networkIpAddr), ifnameIdx) != SOFTBUS_OK ||
        LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_NET_IF_NAME, g_localDeviceInfo->localIfInfo->networkName,
        sizeof(g_localDeviceInfo->localIfInfo->networkName), ifnameIdx) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "get local device info from lnn failed.");
        (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
        return SOFTBUS_DISCOVER_GET_LOCAL_STR_FAILED;
    }
    g_localDeviceInfo->ifNums = 1;
 
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
    return SOFTBUS_OK;
}

void DiscCoapRecordLinkStatus(LinkStatus status, int32_t ifnameIdx)
{
    DISC_CHECK_AND_RETURN_LOGE(status == LINK_STATUS_UP || status == LINK_STATUS_DOWN, DISC_COAP,
        "invlaid link status, status=%{public}d.", status);
    DISC_CHECK_AND_RETURN_LOGE(ifnameIdx >= 0 && ifnameIdx <= MAX_IF, DISC_COAP,
        "invlaid ifnameIdx, ifnameIdx=%{public}d.", ifnameIdx);
    DISC_CHECK_AND_RETURN_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, DISC_COAP, "lock failed");
 
    g_LinkStatus[ifnameIdx] = status;
    int32_t cnt = 0;
    for (int32_t i = 0; i <= MAX_IF; i++) {
        if (g_LinkStatus[i] == LINK_STATUS_UP) {
            cnt++;
        }
    }
    g_currentLinkUpNums = cnt;
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
}

void DiscCoapModifyNstackThread(LinkStatus status, int32_t ifnameIdx)
{
    DISC_CHECK_AND_RETURN_LOGE(status == LINK_STATUS_UP || status == LINK_STATUS_DOWN, DISC_COAP,
        "invlaid link status, status=%{public}d.", status);
    DISC_CHECK_AND_RETURN_LOGE(ifnameIdx >= 0 && ifnameIdx <= MAX_IF, DISC_COAP,
        "invlaid ifnameIdx, ifnameIdx=%{public}d.", ifnameIdx);
    DISC_CHECK_AND_RETURN_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, DISC_COAP, "lock failed");
 
    if (status == LINK_STATUS_UP && g_currentLinkUpNums == 1) {
        int32_t ret = NSTACKX_ThreadInit();
        if (ret != SOFTBUS_OK) {
            (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
            DISC_LOGE(DISC_COAP, "init nstack thread failed, ret=%{public}d", ret);
            return;
        }
    } else if (status == LINK_STATUS_DOWN && g_currentLinkUpNums == 0) {
        NSTACKX_ThreadDeinit();
    }
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
}

void DiscCoapUpdateLocalIp(LinkStatus status, int32_t ifnameIdx)
{
    DISC_CHECK_AND_RETURN_LOGE(status == LINK_STATUS_UP || status == LINK_STATUS_DOWN, DISC_COAP,
        "invlaid link status, status=%{public}d.", status);
 
    DISC_CHECK_AND_RETURN_LOGE(SetLocalDeviceInfo(ifnameIdx) == SOFTBUS_OK, DISC_COAP,
        "link status change: set local device info failed");
 
    char *deviceIdStr = GetDeviceId();
    if (deviceIdStr == NULL) {
        DISC_LOGE(DISC_COAP, "get device id string failed.");
        return;
    }
    int64_t accountId = 0;
    int32_t ret = LnnGetLocalNum64Info(NUM_KEY_ACCOUNT_LONG, &accountId);
    DISC_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, DISC_COAP, "get local account failed");
    DISC_LOGI(DISC_COAP, "register ifname=%{public}s. status=%{public}s, accountInfo=%{public}s",
        g_localDeviceInfo->localIfInfo->networkName, status == LINK_STATUS_UP ? "up" : "down",
        accountId == 0 ? "without" : "with");
    DISC_CHECK_AND_RETURN_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, DISC_COAP, "lock failed");
    g_localDeviceInfo->deviceId = deviceIdStr;
    g_localDeviceInfo->deviceHash = (uint64_t)accountId;
    ret = NSTACKX_RegisterDeviceV2(g_localDeviceInfo);
    cJSON_free(deviceIdStr);
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
    DISC_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, DISC_COAP, "register local device info to dfinder failed");
    ret = RegisterServiceData();
    DISC_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, DISC_COAP, "register servicedata failed");
    DiscCoapUpdateDevName();
}

void DiscCoapUpdateDevName(void)
{
    char localDevName[DEVICE_NAME_BUF_LEN] = { 0 };
    int32_t ret = LnnGetLocalStrInfo(STRING_KEY_DEV_NAME, localDevName, sizeof(localDevName));
    DISC_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, DISC_COAP, "get local device name failed, ret=%{public}d.", ret);

    uint32_t truncateLen = 0;
    if (CalculateMbsTruncateSize((const char *)localDevName, NSTACKX_MAX_DEVICE_NAME_LEN - 1, &truncateLen)
        != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "truncate device name failed");
        return;
    }
    localDevName[truncateLen] = '\0';
    char *anonymizedName = NULL;
    Anonymize(localDevName, &anonymizedName);
    DISC_LOGI(DISC_COAP, "register new local device name. localDevName=%{public}s", AnonymizeWrapper(anonymizedName));
    AnonymizeFree(anonymizedName);
    ret = NSTACKX_RegisterDeviceName(localDevName);
    DISC_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, DISC_COAP, "register local device name failed, ret=%{public}d.", ret);
}

void DiscCoapUpdateAccount(void)
{
    DISC_LOGI(DISC_COAP, "accountId change, register new local accountId.");
    int64_t accountId = 0;
    int32_t ret = LnnGetLocalNum64Info(NUM_KEY_ACCOUNT_LONG, &accountId);
    DISC_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, DISC_COAP, "get local account failed");
    NSTACKX_RegisterDeviceHash((uint64_t)accountId);
}

static void DeinitLocalInfo(void)
{
    DISC_CHECK_AND_RETURN_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, DISC_COAP, "lock failed");
    if (g_localDeviceInfo != NULL && g_localDeviceInfo->localIfInfo != NULL) {
        SoftBusFree(g_localDeviceInfo->localIfInfo);
        g_localDeviceInfo->localIfInfo = NULL;
    }
    if (g_localDeviceInfo != NULL) {
        SoftBusFree(g_localDeviceInfo);
        g_localDeviceInfo = NULL;
    }
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);

    DISC_CHECK_AND_RETURN_LOGE(SoftBusMutexLock(&g_discCoapInnerCbLock) == SOFTBUS_OK, DISC_COAP, "lock failed");
    if (g_discCoapInnerCb != NULL) {
        SoftBusFree(g_discCoapInnerCb);
        g_discCoapInnerCb = NULL;
    }
    (void)SoftBusMutexUnlock(&g_discCoapInnerCbLock);

    (void)SoftBusMutexDestroy(&g_localDeviceInfoLock);
    (void)SoftBusMutexDestroy(&g_discCoapInnerCbLock);
}

static int32_t InitLocalInfo(void)
{
    if (SoftBusMutexInit(&g_localDeviceInfoLock, NULL) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "g_localDeviceInfoLock init failed");
        return SOFTBUS_NO_INIT;
    }
    if (SoftBusMutexInit(&g_discCoapInnerCbLock, NULL) != SOFTBUS_OK) {
        DISC_LOGE(DISC_COAP, "g_discCoapInnerCbLock init failed");
        (void)SoftBusMutexDestroy(&g_localDeviceInfoLock);
        return SOFTBUS_NO_INIT;
    }

    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
    if (g_localDeviceInfo == NULL) {
        g_localDeviceInfo = (NSTACKX_LocalDeviceInfoV2 *)SoftBusCalloc(sizeof(NSTACKX_LocalDeviceInfoV2));
        if (g_localDeviceInfo == NULL) {
            (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
            DeinitLocalInfo();
            return SOFTBUS_MEM_ERR;
        }
        g_localDeviceInfo->localIfInfo = (NSTACKX_InterfaceInfo *)SoftBusCalloc(sizeof(NSTACKX_InterfaceInfo));
        if (g_localDeviceInfo->localIfInfo == NULL) {
            (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);
            DISC_LOGE(DISC_COAP, "mem local If info failed");
            DeinitLocalInfo();
            return SOFTBUS_MEM_ERR;
        }
    }
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);

    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_discCoapInnerCbLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
    if (g_discCoapInnerCb == NULL) {
        g_discCoapInnerCb = (DiscInnerCallback *)SoftBusCalloc(sizeof(DiscInnerCallback));
        if (g_discCoapInnerCb == NULL) {
            (void)SoftBusMutexUnlock(&g_discCoapInnerCbLock);
            DeinitLocalInfo();
            return SOFTBUS_MEM_ERR;
        }
    }
    (void)SoftBusMutexUnlock(&g_discCoapInnerCbLock);
    return SOFTBUS_OK;
}

int32_t DiscNstackxInit(void)
{
    if (InitLocalInfo() != SOFTBUS_OK) {
        return SOFTBUS_DISCOVER_COAP_INIT_FAIL;
    }

    NSTACKX_DFinderRegisterLog(NstackxLogInnerImpl);
    if (SoftbusGetConfig(SOFTBUS_INT_DISC_COAP_MAX_DEVICE_NUM, (unsigned char *)&g_nstackxCallBack.maxDeviceNum,
        sizeof(g_nstackxCallBack.maxDeviceNum)) != SOFTBUS_OK) {
        DISC_LOGI(DISC_COAP, "get disc max device num config failed, use default %{public}u", DEFAULT_MAX_DEVICE_NUM);
        g_nstackxCallBack.maxDeviceNum = DEFAULT_MAX_DEVICE_NUM;
    }
    if (NSTACKX_Init(&g_nstackxCallBack) != SOFTBUS_OK) {
        DeinitLocalInfo();
        return SOFTBUS_DISCOVER_COAP_INIT_FAIL;
    }
    SoftBusRegDiscVarDump((char *)NSTACKX_LOCAL_DEV_INFO, &NstackxLocalDevInfoDump);
    return SOFTBUS_OK;
}

void DiscNstackxDeinit(void)
{
    NSTACKX_Deinit();
    DeinitLocalInfo();
}

static int32_t NstackxLocalDevInfoDump(int fd)
{
    char deviceId[NSTACKX_MAX_DEVICE_ID_LEN] = { 0 };
    DISC_CHECK_AND_RETURN_RET_LOGE(SoftBusMutexLock(&g_localDeviceInfoLock) == SOFTBUS_OK, SOFTBUS_LOCK_ERR,
        DISC_COAP, "lock failed");
    SOFTBUS_DPRINTF(fd, "\n-----------------NstackxLocalDevInfo-------------------\n");
    SOFTBUS_DPRINTF(fd, "name                                : %s\n", g_localDeviceInfo->name);
    DataMasking(g_localDeviceInfo->deviceId, NSTACKX_MAX_DEVICE_ID_LEN, ID_DELIMITER, deviceId);
    SOFTBUS_DPRINTF(fd, "deviceId                            : %s\n", deviceId);
    SOFTBUS_DPRINTF(fd, "localIfInfo networkName             : %s\n", g_localDeviceInfo->localIfInfo->networkName);
    SOFTBUS_DPRINTF(fd, "ifNums                              : %d\n", g_localDeviceInfo->ifNums);
    SOFTBUS_DPRINTF(fd, "deviceType                          : %d\n", g_localDeviceInfo->deviceType);
    SOFTBUS_DPRINTF(fd, "businessType                        : %d\n", g_localDeviceInfo->businessType);
    (void)SoftBusMutexUnlock(&g_localDeviceInfoLock);

    return SOFTBUS_OK;
}