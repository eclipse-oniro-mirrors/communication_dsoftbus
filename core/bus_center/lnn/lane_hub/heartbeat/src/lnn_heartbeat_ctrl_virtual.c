/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "lnn_heartbeat_ctrl.h"
#include "lnn_heartbeat_utils.h"
#include "lnn_log.h"
#include "softbus_error_code.h"
#include "softbus_def.h"

SOFTBUS_VISIBILITY int32_t LnnStartHeartbeatFrameDelay(void)
{
    LNN_LOGI(LNN_HEART_BEAT, "heartbeat(HB) stub process start.");
    return SOFTBUS_OK;
}

SOFTBUS_VISIBILITY int32_t LnnSetHeartbeatMediumParam(const LnnHeartbeatMediumParam *param)
{
    (void)param;

    LNN_LOGI(LNN_HEART_BEAT, "heartbeat stub set medium param");
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY int32_t LnnOfflineTimingByHeartbeat(const char *networkId, ConnectionAddrType addrType)
{
    (void)networkId;
    (void)addrType;

    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY void LnnStopOfflineTimingByHeartbeat(const char *networkId, ConnectionAddrType addrType)
{
    (void)networkId;
    (void)addrType;

    LNN_LOGI(LNN_HEART_BEAT, "heartbeat stop offline timing by heartbeat");
}

SOFTBUS_VISIBILITY
int32_t LnnShiftLNNGear(const char *pkgName, const char *callerId, const char *targetNetworkId, const GearMode *mode)
{
    (void)pkgName;
    (void)callerId;
    (void)targetNetworkId;
    (void)mode;

    LNN_LOGI(LNN_HEART_BEAT, "heartbeat stub ShiftLNNGear");
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY void LnnUpdateHeartbeatInfo(LnnHeartbeatUpdateInfoType type)
{
    (void)type;

    LNN_LOGI(LNN_HEART_BEAT, "heartbeat stub update send info");
}

SOFTBUS_VISIBILITY void LnnHbOnTrustedRelationIncreased(int32_t groupType)
{
    (void)groupType;

    LNN_LOGI(LNN_HEART_BEAT, "heartbeat stub process auth group created");
}

SOFTBUS_VISIBILITY void LnnHbOnTrustedRelationReduced(void)
{
    LNN_LOGI(LNN_HEART_BEAT, "heartbeat stub process auth group deleted");
}

SOFTBUS_VISIBILITY int32_t LnnInitHeartbeat(void)
{
    LNN_LOGI(LNN_INIT, "heartbeat(HB) stub init success");
    return SOFTBUS_OK;
}

SOFTBUS_VISIBILITY void LnnDeinitHeartbeat(void) { }

SOFTBUS_VISIBILITY SoftBusScreenState GetScreenState(void)
{
    return SOFTBUS_SCREEN_UNKNOWN;
}

SOFTBUS_VISIBILITY
int32_t HmosShiftLNNGear(const char *callerId, const GearMode *mode, LnnHeartbeatStrategyType strategyType)
{
    (void)callerId;
    (void)mode;
    (void)strategyType;
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY int32_t LnnTriggerHbRangeForMsdp(const char *pkgName, const RangeConfig *config)
{
    (void)pkgName;
    (void)config;
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY int32_t LnnTriggerDataLevelHeartbeat(void)
{
    return SOFTBUS_OK;
}

SOFTBUS_VISIBILITY int32_t LnnTriggerCloudSyncHeartbeat(void)
{
    return SOFTBUS_OK;
}

SOFTBUS_VISIBILITY void LnnRegDataLevelChangeCb(const IDataLevelChangeCallback *callback)
{
    (void)callback;
}

SOFTBUS_VISIBILITY void LnnUnregDataLevelChangeCb(void) { }

SOFTBUS_VISIBILITY bool LnnIsCloudSyncEnd(void)
{
    return true;
}

int32_t __attribute__((weak)) LnnClearAllNode(void)
{
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY bool IsHeartbeatEnable(void)
{
    return false;
}

SOFTBUS_VISIBILITY bool LnnIsNeedInterceptBroadcast(bool disableGlass)
{
    (void)disableGlass;
    return false;
}

SOFTBUS_VISIBILITY SoftBusScreenLockState GetScreenLockState(void)
{
    return SOFTBUS_SCREEN_LOCK_UNKNOWN;
}

SOFTBUS_VISIBILITY void HbEnableDiscovery(void) { }

SOFTBUS_VISIBILITY int32_t LnnOfflineTimingBySleHb(const char *networkId, ConnectionAddrType addrType)
{
    (void)networkId;
    (void)addrType;
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY void LnnRegBleRangeCb(const IBleRangeInnerCallback *callback)
{
    (void)callback;
}

SOFTBUS_VISIBILITY void LnnRequestBleDiscoveryProcess(int32_t strategy, int64_t timeout)
{
    (void)strategy;
    (void)timeout;
}

SOFTBUS_VISIBILITY void LnnStopOfflineTimingBySleHb(const char *networkId, ConnectionAddrType addrType)
{
    (void)networkId;
    (void)addrType;
}

SOFTBUS_VISIBILITY int32_t LnnStopSleHeartbeat(void)
{
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY int32_t LnnTriggerDirectHeartbeat(const char *networkId, uint64_t timeout)
{
    (void)networkId;
    (void)timeout;
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY int32_t LnnTriggerSleHeartbeat(void)
{
    return SOFTBUS_NOT_IMPLEMENT;
}

SOFTBUS_VISIBILITY void LnnUnregBleRangeCb(void) { }

SOFTBUS_VISIBILITY void SetScreenState(SoftBusScreenState state)
{
    (void)state;
}

