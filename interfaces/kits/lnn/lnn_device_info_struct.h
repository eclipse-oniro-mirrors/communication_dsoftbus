/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef LNN_DEVICE_INFO_STRUCT_H
#define LNN_DEVICE_INFO_STRUCT_H

#include <stdint.h>
#include "../../../dfx/interface/include/form/lnn_event_form.h"
#include "softbus_common.h"
#include "softbus_bus_center.h"
#include "softbus_def.h"
#ifdef __cplusplus
extern "C" {
#endif

#define TYPE_UNKNOW_ID 0x00
#define TYPE_PHONE_ID 0x0E
#define TYPE_PAD_ID 0x11
#define TYPE_TV_ID 0x9C
#define TYPE_AUDIO_ID 0x0A
#define TYPE_CAR_ID 0x83
#define TYPE_WATCH_ID 0x6D
#define TYPE_IPCAMERA_ID 0X08
#define TYPE_PC_ID 0x0C
#define TYPE_SMART_DISPLAY_ID 0xA02
#define TYPE_2IN1_ID 0xA2F

typedef struct {
    uint16_t deviceTypeId;
    int32_t osType;
    char deviceName[DEVICE_NAME_BUF_LEN];
    char unifiedName[DEVICE_NAME_BUF_LEN];
    char nickName[DEVICE_NAME_BUF_LEN];
    char unifiedDefaultName[DEVICE_NAME_BUF_LEN];
    char deviceUdid[UDID_BUF_LEN];
    char osVersion[OS_VERSION_BUF_LEN];
    char deviceVersion[DEVICE_VERSION_SIZE_MAX];
    char productId[PRODUCT_ID_SIZE_MAX];
    char modelName[MODEL_NAME_SIZE_MAX];
} DeviceBasicInfo;

#ifdef __cplusplus
}
#endif

#endif // LNN_DEVICE_INFO_STRUCT_H
