/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef AUTH_MESSAGE_H
#define AUTH_MESSAGE_H

#include "auth_manager.h"
#include "auth_session_fsm.h"
#include "auth_session_message_struct.h"
#include "softbus_json_utils.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

char *PackDeviceInfoMessage(const AuthConnInfo *connInfo, SoftBusVersion version, bool isMetaAuth,
    const char *remoteUuid, const AuthSessionInfo *info);
int32_t UnpackDeviceInfoMessage(const DevInfoData *devInfo, NodeInfo *nodeInfo, bool isMetaAuth,
    const AuthSessionInfo *info);

int32_t PostDeviceIdMessage(int64_t authSeq, const AuthSessionInfo *info);
int32_t ProcessDeviceIdMessage(AuthSessionInfo *info, const uint8_t *data, uint32_t len, int64_t authSeq);

int32_t PostDeviceInfoMessage(int64_t authSeq, const AuthSessionInfo *info);
int32_t ProcessDeviceInfoMessage(int64_t authSeq, AuthSessionInfo *info, const uint8_t *data, uint32_t len);

int32_t PostCloseAckMessage(int64_t authSeq, const AuthSessionInfo *info);
int32_t PostHichainAuthMessage(int64_t authSeq, const AuthSessionInfo *info, const uint8_t *data, uint32_t len);
int32_t PostDeviceMessage(
    const AuthManager *auth, int32_t flagRelay, AuthLinkType type, const DeviceMessageParse *messageParse);
bool IsDeviceMessagePacket(const AuthConnInfo *connInfo, const AuthDataHead *head, const uint8_t *data, bool isServer,
    DeviceMessageParse *messageParse);
int32_t UpdateLocalAuthState(int64_t authSeq, AuthSessionInfo *info);
int32_t TryUpdateLaneResourceLaneId(AuthSessionInfo *info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* AUTH_MESSAGE_H */
