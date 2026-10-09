/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "napi_softbus_base.h"

#include <map>
#include <string>

#include "accesstoken_kit.h"
#include "access_token.h"
#include "comm_log.h"
#include "ipc_skeleton.h"
#include "securec.h"
#include "softbus_adapter_mem.h"
#include "softbus_common.h"
#include "softbus_error_code.h"
#include "tokenid_kit.h"

namespace Communication {
namespace OHOS::Softbus {

/* Permission required by @ohos.distributed.softbusBase.d.ts. */
#ifndef OHOS_PERMISSION_ACCESS_SOFTBUS_SYS_HAP
#define OHOS_PERMISSION_ACCESS_SOFTBUS_SYS_HAP "ohos.permission.ACCESS_SOFTBUS_SYS_HAP"
#endif

static std::map<int32_t, std::string> g_errMsgMap {
    {SOFTBUS_BASE_PERMISSION_ERR, "Permission denied."},
    {SOFTBUS_BASE_PERMISSION_SYSTEMAPI_ERR, "Permission denied. A non-system application calls a system API."},
    {SOFTBUS_BASE_INVALID_PARAM, "Invalid argument."},
    {SOFTBUS_BASE_NOT_SUPPORT, "Capability not supported."},
    {SOFTBUS_BASE_INTERNAL_ERR, "Internal error."},
    {SOFTBUS_BASE_CALLER_ERR, "Caller error. The caller did not call the API in the specified order."},
    {SOFTBUS_BASE_TEMP_ERR, "Temporary error. The request failed due to a temporary error and can be retried."},
    {SOFTBUS_BASE_UNDERLYING_ERR, "Underlying module error."},
};

int32_t ConvertToJsErrcode(int32_t err)
{
    switch (err) {
        case SOFTBUS_OK:
            return SOFTBUS_BASE_OK;
        case SOFTBUS_PERMISSION_DENIED:
            return SOFTBUS_BASE_PERMISSION_ERR;
        case SOFTBUS_FUNC_NOT_SUPPORT:
            return SOFTBUS_BASE_NOT_SUPPORT;
        case SOFTBUS_INVALID_PARAM:
            return SOFTBUS_BASE_INVALID_PARAM;
        case SOFTBUS_PERCEPTION_SCAN_NOT_START:
            /* getPerceptionDeviceList called before startPerceptionScan (call-order error). */
            return SOFTBUS_BASE_CALLER_ERR;
        case SOFTBUS_NO_INIT:
            return SOFTBUS_BASE_TEMP_ERR;
        case SOFTBUS_BLUETOOTH_OFF:
            return SOFTBUS_BASE_OK;
        case SOFTBUS_NETWORK_SEND_MSG_TO_MLPS_FAILED:
            return SOFTBUS_BASE_UNDERLYING_ERR;
        default:
            return SOFTBUS_BASE_INTERNAL_ERR;
    }
}

void ThrowBusinessError(napi_env env, int32_t errCode)
{
    if (errCode == SOFTBUS_BASE_OK) {
        return;
    }
    std::string errMsg = "Internal error.";
    auto iter = g_errMsgMap.find(errCode);
    if (iter != g_errMsgMap.end()) {
        errMsg = iter->second;
    }
    napi_status status = napi_throw_error(env, std::to_string(errCode).c_str(), errMsg.c_str());
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "throw error failed, errCode=%{public}d", errCode);
    }
}

napi_value CreateBusinessErrorValue(napi_env env, int32_t errCode)
{
    if (errCode == SOFTBUS_BASE_OK) {
        napi_value undefined = nullptr;
        if (napi_get_undefined(env, &undefined) != napi_ok) {
            COMM_LOGE(COMM_SDK, "get undefined failed");
            return nullptr;
        }
        return undefined;
    }
    std::string errMsg = "Internal error.";
    auto iter = g_errMsgMap.find(errCode);
    if (iter != g_errMsgMap.end()) {
        errMsg = iter->second;
    }
    napi_value code = nullptr;
    if (napi_create_int32(env, errCode, &code) != napi_ok) {
        COMM_LOGE(COMM_SDK, "create code failed");
        return nullptr;
    }
    napi_value msg = nullptr;
    if (napi_create_string_utf8(env, errMsg.c_str(), errMsg.size(), &msg) != napi_ok) {
        COMM_LOGE(COMM_SDK, "create msg failed");
        return nullptr;
    }
    napi_value error = nullptr;
    if (napi_create_error(env, code, msg, &error) != napi_ok) {
        COMM_LOGE(COMM_SDK, "create error failed");
        return nullptr;
    }
    return error;
}

bool IsSystemApp(void)
{
    uint64_t tokenId = ::OHOS::IPCSkeleton::GetSelfTokenID();
    return ::OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(tokenId);
}

bool CheckPermission(void)
{
    uint32_t tokenId = static_cast<uint32_t>(::OHOS::IPCSkeleton::GetSelfTokenID());
    if (::OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(
        tokenId, OHOS_PERMISSION_ACCESS_SOFTBUS_SYS_HAP) != ::OHOS::Security::AccessToken::PERMISSION_GRANTED) {
        COMM_LOGE(COMM_SVC, "permission %{public}s denied.", OHOS_PERMISSION_ACCESS_SOFTBUS_SYS_HAP);
        return false;
    }
    if (::OHOS::Security::AccessToken::AccessTokenKit::VerifyAccessToken(
        tokenId, OHOS_PERMISSION_DISTRIBUTED_DATASYNC) != ::OHOS::Security::AccessToken::PERMISSION_GRANTED) {
        COMM_LOGE(COMM_SVC, "permission %{public}s denied.", OHOS_PERMISSION_DISTRIBUTED_DATASYNC);
        return false;
    }
    return true;
}

bool ParsePerceptionType(napi_env env, napi_value arg, PerceptionType &type)
{
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, arg, &value);
    if (status != napi_ok || value < 0 || value >= (int32_t)PERCEPTION_TYPE_BUTT) {
        COMM_LOGE(COMM_SDK, "invalid perception type=%{public}d", value);
        return false;
    }
    type = (PerceptionType)value;
    return true;
}

bool ParsePerceptionCycle(napi_env env, napi_value arg, PerceptionCycle &cycle)
{
    int32_t value = 0;
    napi_status status = napi_get_value_int32(env, arg, &value);
    if (status != napi_ok || value < 0 || value >= (int32_t)PERCEPTION_CYCLE_BUTT) {
        COMM_LOGE(COMM_SDK, "invalid perception cycle=%{public}d", value);
        return false;
    }
    cycle = (PerceptionCycle)value;
    return true;
}

bool ParseCustomData(napi_env env, napi_value arg, PerceptionAdvParam &param)
{
    param.customDataLen = 0;
    /* customData is optional; undefined/null means empty payload. */
    napi_valuetype valueType = napi_undefined;
    napi_status status = napi_typeof(env, arg, &valueType);
    if (status != napi_ok || valueType == napi_undefined || valueType == napi_null) {
        return true;
    }
    bool isArrayBuffer = false;
    status = napi_is_arraybuffer(env, arg, &isArrayBuffer);
    if (status != napi_ok || !isArrayBuffer) {
        COMM_LOGE(COMM_SDK, "customData is not arraybuffer");
        return false;
    }
    void *data = nullptr;
    size_t byteLen = 0;
    status = napi_get_arraybuffer_info(env, arg, &data, &byteLen);
    if (status != napi_ok || data == nullptr) {
        COMM_LOGE(COMM_SDK, "get arraybuffer info failed");
        return false;
    }
    if (byteLen > PERCEPTION_CUSTOM_DATA_MAX_LEN) {
        COMM_LOGE(COMM_SDK, "customData too long, len=%{public}zu", byteLen);
        return false;
    }
    if (byteLen > 0 && memcpy_s(param.customData, PERCEPTION_CUSTOM_DATA_MAX_LEN, data, byteLen) != EOK) {
        COMM_LOGE(COMM_SDK, "memcpy customData failed");
        return false;
    }
    param.customDataLen = static_cast<uint32_t>(byteLen);
    return true;
}

static napi_value BuildDeviceObject(napi_env env, const PerceptionDeviceInfo *info)
{
    napi_value jsDevice = nullptr;
    if (napi_create_object(env, &jsDevice) != napi_ok) {
        COMM_LOGE(COMM_SDK, "create device object failed");
        return nullptr;
    }
    napi_value deviceType = nullptr;
    if (napi_create_int32(env, info->deviceType, &deviceType) != napi_ok ||
        napi_set_named_property(env, jsDevice, "deviceType", deviceType) != napi_ok) {
        COMM_LOGE(COMM_SDK, "set deviceType failed");
        return nullptr;
    }
    void *devIdBuf = nullptr;
    napi_value deviceId = nullptr;
    if (napi_create_arraybuffer(env, PERCEPTION_DEVICE_ID_LEN, &devIdBuf, &deviceId) != napi_ok ||
        memcpy_s(devIdBuf, PERCEPTION_DEVICE_ID_LEN, info->deviceId, PERCEPTION_DEVICE_ID_LEN) != EOK ||
        napi_set_named_property(env, jsDevice, "deviceId", deviceId) != napi_ok) {
        COMM_LOGE(COMM_SDK, "set deviceId failed");
        return nullptr;
    }
    uint32_t custLen = info->customDataLen;
    if (custLen > PERCEPTION_CUSTOM_DATA_MAX_LEN) {
        custLen = PERCEPTION_CUSTOM_DATA_MAX_LEN;
    }
    void *custBuf = nullptr;
    napi_value customData = nullptr;
    if (napi_create_arraybuffer(env, custLen, &custBuf, &customData) != napi_ok ||
        (custLen > 0 && memcpy_s(custBuf, custLen, info->customData, custLen) != EOK) ||
        napi_set_named_property(env, jsDevice, "customData", customData) != napi_ok) {
        COMM_LOGE(COMM_SDK, "set customData failed");
        return nullptr;
    }
    return jsDevice;
}

napi_value BuildDeviceListResult(napi_env env, PerceptionDeviceInfo *list, uint32_t count)
{
    napi_value resultArray = nullptr;
    if (napi_create_array(env, &resultArray) != napi_ok) {
        COMM_LOGE(COMM_SDK, "create array failed");
        return nullptr;
    }
    if (list == nullptr || count == 0) {
        return resultArray;
    }
    for (uint32_t i = 0; i < count; ++i) {
        napi_value jsDevice = BuildDeviceObject(env, &list[i]);
        if (jsDevice == nullptr || napi_set_element(env, resultArray, i, jsDevice) != napi_ok) {
            COMM_LOGE(COMM_SDK, "build/set element failed, idx=%{public}u", i);
            return nullptr;
        }
    }
    return resultArray;
}

} // namespace Softbus
} // namespace Communication
