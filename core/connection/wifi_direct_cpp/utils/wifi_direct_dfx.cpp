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

#include "wifi_direct_dfx.h"
#include "conn_log.h"
#include "duration_statistic.h"
#include "softbus_conn_interface.h"
#include "wifi_direct_utils.h"

namespace OHOS::SoftBus {

void WifiDirectDfx::DfxRecord(bool success, int32_t reason, const ConnectInfo &connectInfo)
{
    auto wifiDirectConnectInfo = connectInfo.info_;
    if (success) {
        DurationStatistic::GetInstance().Record(wifiDirectConnectInfo.requestId, TotalEnd);
        DurationStatistic::GetInstance().End(wifiDirectConnectInfo.requestId);
        DurationStatistic::GetInstance().Clear(wifiDirectConnectInfo.requestId);
        WifiDirectDfx::GetInstance().Clear(wifiDirectConnectInfo.requestId);

        ConnEventExtra extra = {
            .result = EVENT_STAGE_RESULT_OK,
            .requestId = static_cast<int32_t>(wifiDirectConnectInfo.requestId),
        };
        ReportConnEventExtra(extra, connectInfo);
    } else {
        DurationStatistic::GetInstance().Clear(wifiDirectConnectInfo.requestId);
        ConnEventExtra extra = {
            .result = EVENT_STAGE_RESULT_FAILED,
            .errcode = reason,
            .requestId = static_cast<int32_t>(wifiDirectConnectInfo.requestId),
        };
        ReportConnEventExtra(extra, connectInfo);
    }
}

void WifiDirectDfx::Record(uint32_t requestId, uint16_t challengeCode)
{
    std::lock_guard lock(mutex_);
    challengeCodeMap_.insert(std::make_pair(requestId, challengeCode));
}

void WifiDirectDfx::Clear(uint32_t requestId)
{
    std::lock_guard lock(mutex_);
    challengeCodeMap_.erase(requestId);
}

void WifiDirectDfx::ReportConnEventExtra(ConnEventExtra &extra, const ConnectInfo &info)
{
    CONN_LOGI(CONN_WIFI_DIRECT, "FillConnEventExtra enter");
    auto wifiDirectConnectInfo = info.info_;
    enum StatisticLinkType type = wifiDirectConnectInfo.dfxInfo.linkType;
    if (type == STATISTIC_P2P) {
        extra.linkType = CONNECT_P2P;
    } else if (type == STATISTIC_HML) {
        extra.linkType = CONNECT_HML;
    } else {
        extra.linkType = CONNECT_TRIGGER_HML;
    }

    auto requestId = wifiDirectConnectInfo.requestId;
    {
        std::shared_lock lock(mutex_);
        if (challengeCodeMap_.find(requestId) != challengeCodeMap_.end()) {
            auto challengeCodeStr = std::to_string(challengeCodeMap_[requestId]);
            extra.challengeCode = challengeCodeStr.c_str();
        }
    }

    extra.peerNetworkId = wifiDirectConnectInfo.remoteNetworkId;
    extra.localNetworkId = WifiDirectUtils::GetLocalNetworkId().c_str();
    auto stateMapElement = DurationStatistic::GetInstance().GetStateTimeMapElement(requestId);
    uint64_t startTime = stateMapElement[TotalStart];
    uint64_t endTime = stateMapElement[TotalEnd];
    if (startTime != 0 && endTime != 0) {
        extra.costTime = int32_t(endTime - startTime);
        extra.negotiateTime = int32_t(endTime - startTime);
    }
    auto dfxInfo = wifiDirectConnectInfo.dfxInfo;
    extra.bootLinkType = dfxInfo.bootLinkType;
    CONN_EVENT(EVENT_SCENE_CONNECT, EVENT_STAGE_CONNECT_END, extra);
}
} // namespace OHOS::SoftBus