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

#include "lnn_lane_link_p2p.h"
#include "softbus_errcode.h"

int32_t LnnConnectP2p(const LinkRequest *request, uint32_t laneLinkReqId, const LaneLinkCb *callback)
{
    (void)request;
    (void)laneLinkReqId;
    (void)callback;
    return SOFTBUS_P2P_NOT_SUPPORT;
}
void LnnDisconnectP2p(const char *networkId, int32_t pid, uint32_t laneLinkReqId)
{
    (void)networkId;
    (void)pid;
    (void)laneLinkReqId;
}
void LnnDestroyP2p(void)
{
}