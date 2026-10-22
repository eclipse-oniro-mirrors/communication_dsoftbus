/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef FAR_FIELD_PROXY_DFX_H
#define FAR_FIELD_PROXY_DFX_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t FarFieldProxyDfxInit(void);

void FarFieldDfxRecordConnectStart(uint32_t requestId, const char *brMac, bool isDirectly);
void FarFieldDfxRecordConnectSuccess(uint32_t requestId, const char *brMac, bool isDirectly);
void FarFieldDfxRecordConnectFail(uint32_t requestId, const char *brMac, int32_t reason, bool isDirectly);
void FarFieldDfxRecordDisconnected(uint32_t requestId, const char *brMac, int32_t reason);
void FarFieldDfxRecordClose(uint32_t requestId, const char *brMac, bool isClearReconnectEvent);
void FarFieldDfxRecordBrOpenSuccess(uint32_t requestId, const char *brMac, bool isSupportFarField);
void FarFieldDfxRemoveRecord(const char *brMac);

#ifdef __cplusplus
}
#endif

#endif // FAR_FIELD_PROXY_DFX_H
