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

#ifndef AUTH_SESSION_JSON_H
#define AUTH_SESSION_JSON_H

#include "auth_session_message.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define AUTH_VERSION_VALUE AUTH_VERSION_V2

char *PackDeviceIdJson(const AuthSessionInfo *info, int64_t authSeq);
int32_t UnpackDeviceIdJson(const char *msg, uint32_t len, AuthSessionInfo *info, int64_t authSeq);
bool GetUdidShortHash(const AuthSessionInfo *info, char *udidBuf, uint32_t bufLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* AUTH_SESSION_JSON_H */
