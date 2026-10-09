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
#include "lnn_perception.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_softbus_base_error_code.h"

#include <string>

namespace Communication {
namespace OHOS::Softbus {

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

static napi_value ThrowNotSupport(napi_env env, napi_callback_info info)
{
    (void)info;
    napi_value undefined = nullptr;
    (void)napi_get_undefined(env, &undefined);
    (void)napi_throw_error(env, std::to_string(SOFTBUS_BASE_NOT_SUPPORT).c_str(), "Capability not supported.");
    return undefined;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startPerceptionAdv", ThrowNotSupport),
        DECLARE_NAPI_FUNCTION("setPerceptionAdvHighFreq", ThrowNotSupport),
        DECLARE_NAPI_FUNCTION("stopPerceptionAdv", ThrowNotSupport),
        DECLARE_NAPI_FUNCTION("startPerceptionScan", ThrowNotSupport),
        DECLARE_NAPI_FUNCTION("stopPerceptionScan", ThrowNotSupport),
        DECLARE_NAPI_FUNCTION("getPerceptionDeviceList", ThrowNotSupport),
    };
    (void)napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
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
    napi_module_register(&softbusBaseModule);
}

} // namespace Softbus
} // namespace Communication
