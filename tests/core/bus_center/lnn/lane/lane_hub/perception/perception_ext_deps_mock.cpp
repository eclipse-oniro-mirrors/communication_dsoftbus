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

#include "perception_ext_deps_mock.h"

#include "softbus_error_code.h"

namespace OHOS {
void *g_perceptionExtDepsInterface = nullptr;

PerceptionExtDepsMock::PerceptionExtDepsMock()
{
    g_perceptionExtDepsInterface = reinterpret_cast<void *>(this);
}

PerceptionExtDepsMock::~PerceptionExtDepsMock()
{
    g_perceptionExtDepsInterface = nullptr;
}

static PerceptionExtDepsInterface *GetExtDepsInterface()
{
    if (g_perceptionExtDepsInterface == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<PerceptionExtDepsInterface *>(g_perceptionExtDepsInterface);
}

extern "C" {
int32_t SchedulerRegisterBroadcaster(
    BroadcastProtocol protocol, BaseServiceType type, int32_t *bcId, const BroadcastCallback *cb)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerRegisterBroadcaster(protocol, type, bcId, cb);
}

int32_t SchedulerUnregisterBroadcaster(int32_t bcId)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerUnregisterBroadcaster(bcId);
}

int32_t SchedulerStartBroadcast(
    int32_t bcId, BroadcastContentType contentType, const BroadcastParam *param, const BroadcastPacket *packet)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerStartBroadcast(bcId, contentType, param, packet);
}

int32_t SchedulerSetBroadcastParam(int32_t bcId, const BroadcastParam *param)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerSetBroadcastParam(bcId, param);
}

int32_t SchedulerSetBroadcastData(int32_t bcId, const BroadcastPacket *packet)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerSetBroadcastData(bcId, packet);
}

int32_t SchedulerStopBroadcast(int32_t bcId)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerStopBroadcast(bcId);
}

int32_t SchedulerRegisterScanListener(
    BroadcastProtocol protocol, BaseServiceType type, int32_t *listenerId, const ScanCallback *cb)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerRegisterScanListener(protocol, type, listenerId, cb);
}

int32_t SchedulerUnregisterListener(int32_t listenerId)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerUnregisterListener(listenerId);
}

int32_t SchedulerStartScan(int32_t listenerId, const BcScanParams *param)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerStartScan(listenerId, param);
}

int32_t SchedulerStopScan(int32_t listenerId)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerStopScan(listenerId);
}

int32_t SchedulerSetScanFilter(int32_t listenerId, const BcScanFilter *scanFilter, uint8_t filterNum)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->SchedulerSetScanFilter(listenerId, scanFilter, filterNum);
}

int32_t PerceptionBuildAdvDataPacked(
    const PerceptionType *type, const PerceptionAdvParam *params, uint32_t paramCnt, BroadcastPacket *packet)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_NOT_IMPLEMENT : p->PerceptionBuildAdvDataPacked(type, params, paramCnt, packet);
}

int32_t PerceptionBuildScanFilterPacked(BcScanFilter **filter, uint8_t *filterNum)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_NOT_IMPLEMENT : p->PerceptionBuildScanFilterPacked(filter, filterNum);
}

int32_t PerceptionGetDeviceInfoListPacked(PerceptionType type, PerceptionDeviceInfo **list, uint32_t *count)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_NOT_IMPLEMENT : p->PerceptionGetDeviceInfoListPacked(type, list, count);
}

int32_t PerceptionSyncStatePacked(PerceptionState state)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_NOT_IMPLEMENT : p->PerceptionSyncStatePacked(state);
}

int32_t PerceptionSyncCyclePacked(PerceptionType type, uint32_t cycle)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_NOT_IMPLEMENT : p->PerceptionSyncCyclePacked(type, cycle);
}

bool BroadcastSetAdvDeviceParam(LpServerType type, const LpBroadcastParam *bcParam, const LpScanParam *scanParam)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? false : p->BroadcastSetAdvDeviceParam(type, bcParam, scanParam);
}

int32_t BroadcastSetScanReportChannelToLpDevice(int32_t listenerId, bool enable)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->BroadcastSetScanReportChannelToLpDevice(listenerId, enable);
}

int32_t LnnAsyncCallbackDelayHelper(
    SoftBusLooper *looper, LnnAsyncCallbackFunc callback, void *para, uint64_t delayMillis)
{
    auto *p = GetExtDepsInterface();
    return p == nullptr ? SOFTBUS_ERR : p->LnnAsyncCallbackDelayHelper(looper, callback, para, delayMillis);
}
}
} // namespace OHOS
