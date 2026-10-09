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

#ifndef NAPI_SOFTBUS_BASE_H_
#define NAPI_SOFTBUS_BASE_H_

#include <string>

#include "lnn_perception.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_softbus_base_error_code.h"

namespace Communication {
namespace OHOS::Softbus {

/* Operation selector carried by the async context. */
enum PerceptionOpType {
    PERCEPTION_OP_START_ADV = 0,
    PERCEPTION_OP_SET_HIGH_FREQ,
    PERCEPTION_OP_STOP_ADV,
    PERCEPTION_OP_START_SCAN,
    PERCEPTION_OP_STOP_SCAN,
    PERCEPTION_OP_GET_DEVICE_LIST,
};

/* Shared async-work context for all softbusBase promise APIs. */
struct PerceptionContext {
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;

    PerceptionType type = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    PerceptionCycle cycle = PERCEPTION_CYCLE_LOW;
    PerceptionAdvParam advParam {};
    PerceptionOpType opType = PERCEPTION_OP_START_ADV;

    /* Output of GetPerceptionDeviceList, ownership transferred to NAPI for marshalling. */
    PerceptionDeviceInfo *deviceList = nullptr;
    uint32_t deviceCount = 0;

    int32_t resultCode = SOFTBUS_BASE_OK;

    /* Destructor: ensure deviceList is freed on any exit path (firstsummer #2). */
    ~PerceptionContext()
    {
        if (deviceList != nullptr) {
            FreePerceptionDeviceList(deviceList);
            deviceList = nullptr;
        }
    }
};

/* ---- utils (implemented in napi_softbus_base_utils.cpp) ---- */
int32_t ConvertToJsErrcode(int32_t err);
void ThrowBusinessError(napi_env env, int32_t errCode);
napi_value CreateBusinessErrorValue(napi_env env, int32_t errCode);
bool IsSystemApp(void);
bool CheckPermission(void);
bool ParsePerceptionType(napi_env env, napi_value arg, PerceptionType &type);
bool ParsePerceptionCycle(napi_env env, napi_value arg, PerceptionCycle &cycle);
bool ParseCustomData(napi_env env, napi_value arg, PerceptionAdvParam &param);
napi_value BuildDeviceListResult(napi_env env, PerceptionDeviceInfo *list, uint32_t count);

} // namespace Softbus
} // namespace Communication
#endif /* NAPI_SOFTBUS_BASE_H_ */
