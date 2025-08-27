/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef LNN_LANE_LINK_P2P_H
#define LNN_LANE_LINK_P2P_H

#include "lnn_lane_link.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LANE_ACTIVE_AUTH_TRIGGER = 0x0,
    LANE_BLE_TRIGGER,
    LANE_ACTIVE_AUTH_NEGO,
    LANE_ACTIVE_BR_NEGO,
    LANE_PROXY_AUTH_NEGO,
    LANE_NEW_AUTH_NEGO,
    LANE_ACTION_TRIGGER,
    LANE_SPARKLINK_TRIGGER,
    LANE_CHANNEL_BUTT,
} WdGuideType;

int32_t LnnConnectP2p(const LinkRequest *request, uint32_t laneReqId, const LaneLinkCb *callback);
int32_t LnnDisconnectP2p(const char *networkId, uint32_t laneReqId);
void LnnDestroyP2p(void);
void LnnCancelWifiDirect(uint32_t laneReqId);
int32_t CheckIsAuthSessionServer(const char *peerIp, bool *isServer);
int32_t RemoveAuthSessionServer(const char *peerIp);
void LnnDisconnectP2pWithoutLnn(uint32_t laneReqId);
void NotifyLinkFailForForceDown(uint32_t requestId, int32_t reason);
void RecycleP2pLinkedReqByLinkType(const char *peerNetworkId, LaneLinkType linkType);
int32_t WifiDirectReconnectDevice(uint32_t p2pRequestId);
int32_t LnnInitPtkSyncListener(void);

#ifdef __cplusplus
}
#endif
#endif // LNN_LANE_LINK_P2P_H