/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 
#ifndef BROADCAST_DFX_EVENT_H
#define BROADCAST_DFX_EVENT_H
 
#include "disc_event.h"
#include "broadcast_dfx_event_struct.h"
 
#ifdef __cplusplus
extern "C" {
#endif

void BroadcastDiscEvent(int32_t eventScene, int32_t eventStage, DiscEventExtra *extra, int32_t size);
void BroadcastScanEvent(int32_t eventScene, int32_t eventStage, DiscEventExtra *extra, int32_t size);
 
#ifdef __cplusplus
}
#endif /* __cplusplus */
 
#endif // BROADCAST_DFX_EVENT_H
