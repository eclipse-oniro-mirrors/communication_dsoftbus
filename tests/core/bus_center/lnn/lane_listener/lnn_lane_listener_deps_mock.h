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

#ifndef LNN_LANE_LISTENER_DEPS_MOCK_H
#define LNN_LANE_LISTENER_DEPS_MOCK_H

#include <gmock/gmock.h>

#include "lnn_distributed_net_ledger.h"
#include "lnn_lane_link.h"
#include "lnn_lane_listener.h"
#include "lnn_node_info.h"

namespace OHOS {
class LaneListenerDepsInterface {
public:
    LaneListenerDepsInterface() {};
    virtual ~LaneListenerDepsInterface() {};
    virtual bool CompLaneResource(const LaneResource *src, const LaneResource *dst) = 0;
    virtual int32_t ConvertToLaneResource(const LaneLinkInfo *linkInfo, LaneResource *laneResourceInfo) = 0;
    virtual int32_t FreeLaneResource(const LaneResource *resourceItem) = 0;
    virtual int32_t LaneInfoProcess(const LaneLinkInfo *linkInfo, LaneConnInfo *connInfo,
        LaneProfile *profile) = 0;
    virtual int32_t LnnGetRemoteNodeInfoById(const char *id, IdCategory type, NodeInfo *info) = 0;
    virtual int32_t PostLaneStateChangeMessage(LaneState state, const char *peerUdid,
        const LaneLinkInfo *laneLinkInfo) = 0;
};

class LaneListenerDepsInterfaceMock : public LaneListenerDepsInterface {
public:
    LaneListenerDepsInterfaceMock();
    ~LaneListenerDepsInterfaceMock() override;

    MOCK_METHOD2(CompLaneResource, bool (const LaneResource *src, const LaneResource *dst));
    MOCK_METHOD2(ConvertToLaneResource, int32_t (const LaneLinkInfo *linkInfo, LaneResource *laneResourceInfo));
    MOCK_METHOD1(FreeLaneResource, int32_t (const LaneResource *resourceItem));
    MOCK_METHOD3(LaneInfoProcess, int32_t (const LaneLinkInfo *linkInfo, LaneConnInfo *connInfo,
        LaneProfile *profile));
    MOCK_METHOD3(LnnGetRemoteNodeInfoById, int32_t (const char *id, IdCategory type, NodeInfo *info));
    MOCK_METHOD3(PostLaneStateChangeMessage, int32_t (LaneState state, const char *peerUdid,
        const LaneLinkInfo *laneLinkInfo));
};
} // namespace OHOS
#endif // LNN_LANE_LISTENER_DEPS_MOCK_H