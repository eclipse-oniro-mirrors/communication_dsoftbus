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

#ifndef SOFTBUS_ADAPTER_RANGE_H
#define SOFTBUS_ADAPTER_RANGE_H

#include <stdint.h>

#include "ble_range.h"
#include "softbus_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define SOFTBUS_DEV_IDENTITY_LEN 96
// valid value of power is [-128, 10], 11 is considered to be illegal.
#define SOFTBUS_ILLEGAL_BLE_POWER 11
#define SOFTBUS_MODEL_ID_LEN (HEXIFY_LEN(3))
#define SOFTBUS_SUB_MODEL_ID_LEN (HEXIFY_LEN(1))
#define SOFTBUS_NEW_MODEL_ID_LEN (HEXIFY_LEN(4))
#define SOFTBUS_MODEL_NAME_LEN (HEXIFY_LEN(8))
#define NETWORK_ID_BUF_LEN 65

typedef enum {
    MODULE_BLE_DISCOVERY = 1,
    MODULE_BLE_HEARTBEAT,
    MODULE_BLE_APPROACH,
    MODULE_SLE_CONNECTION,
    MODULE_UWB,
} SoftBusRangeModule;

typedef enum {
    RANGE_SERVICE_DEFAULT = 0,
    RANGE_SERVICE_TOUCH,
    RANGE_SERVICE_VLINK,
    RANGE_SERVICE_BUTT,
} SoftBusRangeBusinessType;

typedef struct {
    int8_t power;
    bool isCycle;
    bool isSupportNearLink;
    char identity[SOFTBUS_DEV_IDENTITY_LEN];
    char modelId[SOFTBUS_MODEL_ID_LEN];
    char subModelId[SOFTBUS_SUB_MODEL_ID_LEN];
    char newModelId[SOFTBUS_NEW_MODEL_ID_LEN];
    char modelName[SOFTBUS_MODEL_NAME_LEN];
    int32_t rssi;
    SoftBusRangeBusinessType serviceType;
} SoftBusRangeParam;

typedef struct {
    SoftBusRangeModule module;
} SoftBusRangeHandle;

typedef struct {
    void (*onRangeResult)(SoftBusRangeHandle handle, const RangeResult *result);
} SoftBusRangeCallback;

int SoftBusBleRange(SoftBusRangeParam *param, int32_t *range);
int SoftBusGetBlePower(int8_t *power);
int32_t SoftBusGetAdvPower(int8_t *power);

int32_t SoftBusBleRangeAsync(const SoftBusRangeParam *param);
int32_t SoftBusRegRangeCb(SoftBusRangeModule module, const SoftBusRangeCallback *callback);
void SoftBusUnregRangeCb(SoftBusRangeModule module);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* SOFTBUS_ADAPTER_RANGE_H */
