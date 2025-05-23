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

#ifndef LNN_RANGING_MANAGER_H
#define LNN_RANGING_MANAGER_H

#include <stdint.h>

#include "ble_range.h"
#include "softbus_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLE_ADDR_LEN 65
#define MSG_FLAG_REQUEST 1
#define MES_FLAG_REPLY 2
#define DISTRIBUTED_CONFLICT 1
#define DISTRIBUTED_NOT_CONFLICT 2

#define LOCAL_START 1
#define REMOTE_START 2

typedef enum {
    SLE_NOT_SUPPORT,
    SLE_SUPPORT_G,
    SLE_SUPPORT_ALL,
} SleSupportType;

typedef enum {
    SLE_INIT,
    SLE_CONNECT,
    SLE_AUTH,
    SLE_COMMUNICATION_LOCAL_START,
    SLE_COMMUNICATION_REMOTE_START,
    SLE_LOCAL_START,
    SLE_REMOTE_START,
    SLE_STOP,
} SleState;

typedef struct {
    int32_t remote;
    char sleAddr[SLE_ADDR_LEN];
    SleState state;
    uint32_t requiredId;
} SleMgrInfo;

int32_t LnnStartRange(const RangeConfig *config);
int32_t LnnStopRange(const RangeConfig *config);
int32_t RegistAuthTransListener(void);
int32_t UnregistAuthTransListener(void);
int32_t SendAuthResult(AuthHandle authHandle, int64_t seq, const uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif
