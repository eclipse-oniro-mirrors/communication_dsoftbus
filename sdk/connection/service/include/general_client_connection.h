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

#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <stdatomic.h>

#include "softbus_def.h"
#include "softbus_connection.h"

#ifdef __cplusplus
extern "C" {
#endif

enum EmulatorQueryState {
    EMULATOR_STATE_UNQUERIED = 0,
    EMULATOR_STATE_IS_EMULATOR = 1,
    EMULATOR_STATE_NOT_EMULATOR = 2,
};

extern atomic_int g_emulatorState;

int32_t ConnectionStateChange(uint32_t handle, int32_t state, int32_t reason);
int32_t AcceptConnect(const char *name, uint32_t handle);
void DataReceived(uint32_t handle, const uint8_t *data, uint32_t len);
void ConnectionDeathNotify(void);
int32_t ServerStopped(const char *name);
bool GeneralIsEmulatorEnv(void);

#ifdef __cplusplus
}
#endif
#endif // CLIENT_CONNECTION_H
