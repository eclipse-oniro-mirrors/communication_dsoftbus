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

#include <cstring>
#include <string>

#include "comm_log.h"
#include "securec.h"
#include "softbus_error_code.h"

namespace Communication {
namespace OHOS::Softbus {

#define ARGS_ONE 1
#define ARGS_TWO 2

/* ---- async work engine ---- */

/* Build the package name locally from PerceptionType (no access token / no special permission
   needed). The same type always maps to the same pkgName, so start/stop/query stay consistent
   across calls without the NAPI extracting the caller's bundle name. */
static const char *BuildPkgNameFromType(PerceptionType type)
{
    switch (type) {
        case PERCEPTION_TYPE_COLLABORATIVE_WAKE:
            return "perception_collaborative_wake";
        default:
            return "perception_unknown";
    }
}

static void ExecutePerception(napi_env env, void *data)
{
    PerceptionContext *ctx = static_cast<PerceptionContext *>(data);
    (void)env;
    int32_t ret = SOFTBUS_OK;
    const char *pkg = BuildPkgNameFromType(ctx->type);
    switch (ctx->opType) {
        case PERCEPTION_OP_START_ADV:
            ret = StartPerceptionAdv(pkg, ctx->type, &ctx->advParam);
            break;
        case PERCEPTION_OP_SET_HIGH_FREQ:
            ret = SetPerceptionAdvHighFreq(pkg, ctx->type, &ctx->advParam);
            break;
        case PERCEPTION_OP_STOP_ADV:
            ret = StopPerceptionAdv(pkg, ctx->type);
            break;
        case PERCEPTION_OP_START_SCAN:
            ret = StartPerceptionScan(pkg, ctx->type, ctx->cycle);
            break;
        case PERCEPTION_OP_STOP_SCAN:
            ret = StopPerceptionScan(pkg, ctx->type);
            break;
        case PERCEPTION_OP_GET_DEVICE_LIST:
            ret = GetPerceptionDeviceList(pkg, ctx->type, &ctx->deviceList, &ctx->deviceCount);
            break;
        default:
            ret = SOFTBUS_INVALID_PARAM;
            break;
    }
    ctx->resultCode = ConvertToJsErrcode(ret);
    COMM_LOGI(COMM_SDK, "perception op=%{public}d ret=%{public}d jsCode=%{public}d",
        ctx->opType, ret, ctx->resultCode);
}

static void CompletePerception(napi_env env, napi_status status, void *data)
{
    PerceptionContext *ctx = static_cast<PerceptionContext *>(data);
    if (ctx->resultCode != SOFTBUS_BASE_OK) {
        COMM_LOGE(COMM_SDK, "reject op=%{public}d code=%{public}d", ctx->opType, ctx->resultCode);
        napi_reject_deferred(env, ctx->deferred, CreateBusinessErrorValue(env, ctx->resultCode));
        goto cleanup;
    }
    if (ctx->opType == PERCEPTION_OP_GET_DEVICE_LIST) {
        napi_value result = BuildDeviceListResult(env, ctx->deviceList, ctx->deviceCount);
        if (result == nullptr) {
            napi_reject_deferred(env, ctx->deferred, CreateBusinessErrorValue(env, SOFTBUS_BASE_INTERNAL_ERR));
        } else {
            napi_resolve_deferred(env, ctx->deferred, result);
        }
    } else {
        napi_value undefined = nullptr;
        if (napi_get_undefined(env, &undefined) != napi_ok) {
            napi_reject_deferred(env, ctx->deferred, CreateBusinessErrorValue(env, SOFTBUS_BASE_INTERNAL_ERR));
        } else {
            napi_resolve_deferred(env, ctx->deferred, undefined);
        }
    }

cleanup:
    if (ctx->deviceList != nullptr) {
        FreePerceptionDeviceList(ctx->deviceList);
        ctx->deviceList = nullptr;
    }
    napi_delete_async_work(env, ctx->work);
    delete ctx;
}

static napi_value CreatePerceptionAsyncWork(napi_env env, PerceptionContext *ctx, const char *name)
{
    napi_value promise = nullptr;
    napi_value resourceName = nullptr;
    napi_status status = napi_create_promise(env, &ctx->deferred, &promise);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "create promise failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        delete ctx;
        return nullptr;
    }
    status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &resourceName);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "create resource name failed");
        napi_reject_deferred(env, ctx->deferred, CreateBusinessErrorValue(env, SOFTBUS_BASE_INTERNAL_ERR));
        delete ctx;
        return promise;
    }
    status = napi_create_async_work(env, nullptr, resourceName, ExecutePerception, CompletePerception, ctx,
        &ctx->work);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "create async work failed");
        napi_reject_deferred(env, ctx->deferred, CreateBusinessErrorValue(env, SOFTBUS_BASE_INTERNAL_ERR));
        delete ctx;
        return promise;
    }
    status = napi_queue_async_work(env, ctx->work);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "queue async work failed");
        napi_delete_async_work(env, ctx->work);
        ctx->work = nullptr;
        napi_reject_deferred(env, ctx->deferred, CreateBusinessErrorValue(env, SOFTBUS_BASE_INTERNAL_ERR));
        delete ctx;
        return promise;
    }
    return promise;
}

/* Common pre-checks: system application (202) + permission (201). */
static bool PreCheck(napi_env env)
{
    if (!IsSystemApp()) {
        COMM_LOGE(COMM_SDK, "not system app");
        ThrowBusinessError(env, SOFTBUS_BASE_PERMISSION_SYSTEMAPI_ERR);
        return false;
    }
    if (!CheckPermission()) {
        COMM_LOGE(COMM_SDK, "permission denied");
        ThrowBusinessError(env, SOFTBUS_BASE_PERMISSION_ERR);
        return false;
    }
    return true;
}

/* ---- promise entries (match @ohos.distributed.softbusBase.d.ts) ---- */

static napi_value NapiStartPerceptionAdv(napi_env env, napi_callback_info info)
{
    COMM_LOGI(COMM_SDK, "start");
    if (!PreCheck(env)) {
        return nullptr;
    }
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "get cb info failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    if (argc < ARGS_ONE) {
        COMM_LOGE(COMM_SDK, "invalid argc=%{public}zu", argc);
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    auto *ctx = new (std::nothrow) PerceptionContext();
    if (ctx == nullptr) {
        COMM_LOGE(COMM_SDK, "alloc ctx failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        return nullptr;
    }
    ctx->opType = PERCEPTION_OP_START_ADV;
    if (!ParsePerceptionType(env, argv[0], ctx->type)) {
        COMM_LOGE(COMM_SDK, "parse perception type failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    if (argc >= ARGS_TWO && !ParseCustomData(env, argv[1], ctx->advParam)) {
        COMM_LOGE(COMM_SDK, "parse customData failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    return CreatePerceptionAsyncWork(env, ctx, "StartPerceptionAdv");
}

static napi_value NapiSetPerceptionAdvHighFreq(napi_env env, napi_callback_info info)
{
    COMM_LOGI(COMM_SDK, "start");
    if (!PreCheck(env)) {
        return nullptr;
    }
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "get cb info failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    if (argc < ARGS_ONE) {
        COMM_LOGE(COMM_SDK, "invalid argc=%{public}zu", argc);
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    auto *ctx = new (std::nothrow) PerceptionContext();
    if (ctx == nullptr) {
        COMM_LOGE(COMM_SDK, "alloc ctx failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        return nullptr;
    }
    ctx->opType = PERCEPTION_OP_SET_HIGH_FREQ;
    if (!ParsePerceptionType(env, argv[0], ctx->type)) {
        COMM_LOGE(COMM_SDK, "parse perception type failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    if (argc >= ARGS_TWO && !ParseCustomData(env, argv[1], ctx->advParam)) {
        COMM_LOGE(COMM_SDK, "parse customData failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    return CreatePerceptionAsyncWork(env, ctx, "SetPerceptionAdvHighFreq");
}

static napi_value NapiStopPerceptionAdv(napi_env env, napi_callback_info info)
{
    COMM_LOGI(COMM_SDK, "start");
    if (!PreCheck(env)) {
        return nullptr;
    }
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "get cb info failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    if (argc < ARGS_ONE) {
        COMM_LOGE(COMM_SDK, "invalid argc=%{public}zu", argc);
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    auto *ctx = new (std::nothrow) PerceptionContext();
    if (ctx == nullptr) {
        COMM_LOGE(COMM_SDK, "alloc ctx failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        return nullptr;
    }
    ctx->opType = PERCEPTION_OP_STOP_ADV;
    if (!ParsePerceptionType(env, argv[0], ctx->type)) {
        COMM_LOGE(COMM_SDK, "parse perception type failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    return CreatePerceptionAsyncWork(env, ctx, "StopPerceptionAdv");
}

static napi_value NapiStartPerceptionScan(napi_env env, napi_callback_info info)
{
    COMM_LOGI(COMM_SDK, "start");
    if (!PreCheck(env)) {
        return nullptr;
    }
    size_t argc = ARGS_TWO;
    napi_value argv[ARGS_TWO] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "get cb info failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    if (argc < ARGS_TWO) {
        COMM_LOGE(COMM_SDK, "invalid argc=%{public}zu", argc);
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    auto *ctx = new (std::nothrow) PerceptionContext();
    if (ctx == nullptr) {
        COMM_LOGE(COMM_SDK, "alloc ctx failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        return nullptr;
    }
    ctx->opType = PERCEPTION_OP_START_SCAN;
    if (!ParsePerceptionType(env, argv[0], ctx->type)) {
        COMM_LOGE(COMM_SDK, "parse perception type failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    if (!ParsePerceptionCycle(env, argv[1], ctx->cycle)) {
        COMM_LOGE(COMM_SDK, "parse perception cycle failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    return CreatePerceptionAsyncWork(env, ctx, "StartPerceptionScan");
}

static napi_value NapiStopPerceptionScan(napi_env env, napi_callback_info info)
{
    COMM_LOGI(COMM_SDK, "start");
    if (!PreCheck(env)) {
        return nullptr;
    }
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "get cb info failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    if (argc < ARGS_ONE) {
        COMM_LOGE(COMM_SDK, "invalid argc=%{public}zu", argc);
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    auto *ctx = new (std::nothrow) PerceptionContext();
    if (ctx == nullptr) {
        COMM_LOGE(COMM_SDK, "alloc ctx failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        return nullptr;
    }
    ctx->opType = PERCEPTION_OP_STOP_SCAN;
    if (!ParsePerceptionType(env, argv[0], ctx->type)) {
        COMM_LOGE(COMM_SDK, "parse perception type failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    return CreatePerceptionAsyncWork(env, ctx, "StopPerceptionScan");
}

static napi_value NapiGetPerceptionDeviceList(napi_env env, napi_callback_info info)
{
    COMM_LOGI(COMM_SDK, "start");
    if (!PreCheck(env)) {
        return nullptr;
    }
    size_t argc = ARGS_ONE;
    napi_value argv[ARGS_ONE] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "get cb info failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    if (argc < ARGS_ONE) {
        COMM_LOGE(COMM_SDK, "invalid argc=%{public}zu", argc);
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        return nullptr;
    }
    auto *ctx = new (std::nothrow) PerceptionContext();
    if (ctx == nullptr) {
        COMM_LOGE(COMM_SDK, "alloc ctx failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INTERNAL_ERR);
        return nullptr;
    }
    ctx->opType = PERCEPTION_OP_GET_DEVICE_LIST;
    if (!ParsePerceptionType(env, argv[0], ctx->type)) {
        COMM_LOGE(COMM_SDK, "parse perception type failed");
        ThrowBusinessError(env, SOFTBUS_BASE_INVALID_PARAM);
        delete ctx;
        return nullptr;
    }
    return CreatePerceptionAsyncWork(env, ctx, "GetPerceptionDeviceList");
}

/* Expose d.ts enums (PerceptionType / PerceptionCycle) on the exports object. */
static void SetEnumInt32(napi_env env, napi_value obj, const char *name, int32_t value)
{
    napi_value val = nullptr;
    if (napi_create_int32(env, value, &val) == napi_ok) {
        (void)napi_set_named_property(env, obj, name, val);
    }
}

static napi_value CreatePerceptionTypeEnum(napi_env env)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    SetEnumInt32(env, obj, "PERCEPTION_TYPE_COLLABORATIVE_WAKE", (int32_t)PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    return obj;
}

static napi_value CreatePerceptionCycleEnum(napi_env env)
{
    napi_value obj = nullptr;
    (void)napi_create_object(env, &obj);
    SetEnumInt32(env, obj, "PERCEPTION_CYCLE_LOW", (int32_t)PERCEPTION_CYCLE_LOW);
    SetEnumInt32(env, obj, "PERCEPTION_CYCLE_MEDIUM", (int32_t)PERCEPTION_CYCLE_MEDIUM);
    SetEnumInt32(env, obj, "PERCEPTION_CYCLE_HIGH", (int32_t)PERCEPTION_CYCLE_HIGH);
    return obj;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startPerceptionAdv", NapiStartPerceptionAdv),
        DECLARE_NAPI_FUNCTION("setPerceptionAdvHighFreq", NapiSetPerceptionAdvHighFreq),
        DECLARE_NAPI_FUNCTION("stopPerceptionAdv", NapiStopPerceptionAdv),
        DECLARE_NAPI_FUNCTION("startPerceptionScan", NapiStartPerceptionScan),
        DECLARE_NAPI_FUNCTION("stopPerceptionScan", NapiStopPerceptionScan),
        DECLARE_NAPI_FUNCTION("getPerceptionDeviceList", NapiGetPerceptionDeviceList),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    if (status != napi_ok) {
        COMM_LOGE(COMM_SDK, "define properties failed");
        return nullptr;
    }
    (void)napi_set_named_property(env, exports, "PerceptionType", CreatePerceptionTypeEnum(env));
    (void)napi_set_named_property(env, exports, "PerceptionCycle", CreatePerceptionCycleEnum(env));
    return exports;
}
EXTERN_C_END

static napi_module softbusBaseModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "distributed.softbusbase",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterSoftbusBaseModule(void)
{
    COMM_LOGI(COMM_SDK, "register softbusBase module");
    napi_module_register(&softbusBaseModule);
}

} // namespace Softbus
} // namespace Communication
