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

#ifndef PERCEPTION_EXT_DEPS_MOCK_H
#define PERCEPTION_EXT_DEPS_MOCK_H

#include <gmock/gmock.h>

#include "broadcast_scheduler.h"
#include "lnn_async_callback_utils.h"
#include "lnn_perception.h"
#include "message_handler.h"
#include "softbus_broadcast_manager.h"

namespace OHOS {
class PerceptionExtDepsInterface {
public:
    virtual ~PerceptionExtDepsInterface() = default;

    virtual int32_t SchedulerRegisterBroadcaster(
        BroadcastProtocol protocol, BaseServiceType type, int32_t *bcId, const BroadcastCallback *cb) = 0;
    virtual int32_t SchedulerUnregisterBroadcaster(int32_t bcId) = 0;
    virtual int32_t SchedulerStartBroadcast(
        int32_t bcId, BroadcastContentType contentType, const BroadcastParam *param, const BroadcastPacket *packet) = 0;
    virtual int32_t SchedulerSetBroadcastParam(int32_t bcId, const BroadcastParam *param) = 0;
    virtual int32_t SchedulerSetBroadcastData(int32_t bcId, const BroadcastPacket *packet) = 0;
    virtual int32_t SchedulerStopBroadcast(int32_t bcId) = 0;
    virtual int32_t SchedulerRegisterScanListener(
        BroadcastProtocol protocol, BaseServiceType type, int32_t *listenerId, const ScanCallback *cb) = 0;
    virtual int32_t SchedulerUnregisterListener(int32_t listenerId) = 0;
    virtual int32_t SchedulerStartScan(int32_t listenerId, const BcScanParams *param) = 0;
    virtual int32_t SchedulerStopScan(int32_t listenerId) = 0;
    virtual int32_t SchedulerSetScanFilter(int32_t listenerId, const BcScanFilter *scanFilter, uint8_t filterNum) = 0;

    virtual int32_t PerceptionBuildAdvDataPacked(
        const PerceptionType *type, const PerceptionAdvParam *params, uint32_t paramCnt, BroadcastPacket *packet) = 0;
    virtual int32_t PerceptionBuildScanFilterPacked(BcScanFilter **filter, uint8_t *filterNum) = 0;
    virtual int32_t PerceptionGetDeviceInfoListPacked(
        PerceptionType type, PerceptionDeviceInfo **list, uint32_t *count) = 0;
    virtual int32_t PerceptionSyncStatePacked(PerceptionState state) = 0;
    virtual int32_t PerceptionSyncCyclePacked(PerceptionType type, uint32_t cycle) = 0;

    virtual bool BroadcastSetAdvDeviceParam(
        LpServerType type, const LpBroadcastParam *bcParam, const LpScanParam *scanParam) = 0;
    virtual int32_t BroadcastSetScanReportChannelToLpDevice(int32_t listenerId, bool enable) = 0;

    virtual int32_t LnnAsyncCallbackDelayHelper(
        SoftBusLooper *looper, LnnAsyncCallbackFunc callback, void *para, uint64_t delayMillis) = 0;
};

class PerceptionExtDepsMock : public PerceptionExtDepsInterface {
public:
    PerceptionExtDepsMock();
    ~PerceptionExtDepsMock() override;

    MOCK_METHOD4(SchedulerRegisterBroadcaster,
        int32_t(BroadcastProtocol, BaseServiceType, int32_t *, const BroadcastCallback *));
    MOCK_METHOD1(SchedulerUnregisterBroadcaster, int32_t(int32_t));
    MOCK_METHOD4(SchedulerStartBroadcast,
        int32_t(int32_t, BroadcastContentType, const BroadcastParam *, const BroadcastPacket *));
    MOCK_METHOD2(SchedulerSetBroadcastParam, int32_t(int32_t, const BroadcastParam *));
    MOCK_METHOD2(SchedulerSetBroadcastData, int32_t(int32_t, const BroadcastPacket *));
    MOCK_METHOD1(SchedulerStopBroadcast, int32_t(int32_t));
    MOCK_METHOD4(
        SchedulerRegisterScanListener, int32_t(BroadcastProtocol, BaseServiceType, int32_t *, const ScanCallback *));
    MOCK_METHOD1(SchedulerUnregisterListener, int32_t(int32_t));
    MOCK_METHOD2(SchedulerStartScan, int32_t(int32_t, const BcScanParams *));
    MOCK_METHOD1(SchedulerStopScan, int32_t(int32_t));
    MOCK_METHOD3(SchedulerSetScanFilter, int32_t(int32_t, const BcScanFilter *, uint8_t));

    MOCK_METHOD4(PerceptionBuildAdvDataPacked,
        int32_t(const PerceptionType *, const PerceptionAdvParam *, uint32_t, BroadcastPacket *));
    MOCK_METHOD2(PerceptionBuildScanFilterPacked, int32_t(BcScanFilter **, uint8_t *));
    MOCK_METHOD3(PerceptionGetDeviceInfoListPacked, int32_t(PerceptionType, PerceptionDeviceInfo **, uint32_t *));
    MOCK_METHOD1(PerceptionSyncStatePacked, int32_t(PerceptionState));
    MOCK_METHOD2(PerceptionSyncCyclePacked, int32_t(PerceptionType, uint32_t));

    MOCK_METHOD3(BroadcastSetAdvDeviceParam, bool(LpServerType, const LpBroadcastParam *, const LpScanParam *));
    MOCK_METHOD2(BroadcastSetScanReportChannelToLpDevice, int32_t(int32_t, bool));

    MOCK_METHOD4(LnnAsyncCallbackDelayHelper, int32_t(SoftBusLooper *, LnnAsyncCallbackFunc, void *, uint64_t));
};
} // namespace OHOS
#endif // PERCEPTION_EXT_DEPS_MOCK_H
