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

#ifndef HEARTBEAT_FSM_STRATEGY_H
#define HEARTBEAT_FSM_STRATEGY_H

#include <gmock/gmock.h>
#include <mutex>

#include "bus_center_manager.h"
#include "lnn_feature_capability.h"
#include "lnn_heartbeat_ctrl.h"
#include "lnn_heartbeat_fsm.h"
#include "lnn_heartbeat_medium_mgr.h"
#include "lnn_heartbeat_utils.h"
#include "lnn_state_machine.h"
#include "softbus_adapter_thread.h"

namespace OHOS {
class HeartBeatFSMStrategyInterface {
public:
    HeartBeatFSMStrategyInterface() {};
    virtual ~HeartBeatFSMStrategyInterface() {};

    virtual int32_t LnnPostSendEndMsgToHbFsm(
        LnnHeartbeatFsm *hbFsm, LnnHeartbeatSendEndData *custData, uint64_t delayMillis) = 0;
    virtual int32_t LnnPostSendBeginMsgToHbFsm(LnnHeartbeatFsm *hbFsm, LnnHeartbeatType type, bool wakeupFlag,
        LnnProcessSendOnceMsgPara *msgPara, uint64_t delayMillis) = 0;
    virtual SoftBusScreenState GetScreenState(void) = 0;
    virtual bool LnnCheckSupportedHbType(LnnHeartbeatType *srcType, LnnHeartbeatType *dstType) = 0;
    virtual int32_t LnnPostStartMsgToHbFsm(LnnHeartbeatFsm *hbFsm, uint64_t delayMillis) = 0;
    virtual bool LnnVisitHbTypeSet(VisitHbTypeCb callback, LnnHeartbeatType *typeSet, void *data) = 0;
    virtual LnnHeartbeatType LnnConvertConnAddrTypeToHbType(ConnectionAddrType addrType) = 0;
    virtual int32_t LnnConvertHbTypeToId(LnnHeartbeatType type) = 0;
    virtual bool LnnIsSupportBurstFeature(const char *networkId) = 0;
    virtual int32_t LnnPostSetMediumParamMsgToHbFsm(LnnHeartbeatFsm *hbFsm, const LnnHeartbeatMediumParam *para) = 0;
    virtual int32_t LnnPostCheckDevStatusMsgToHbFsm(
        LnnHeartbeatFsm *hbFsm, const LnnCheckDevStatusMsgPara *para, uint64_t delayMillis) = 0;
    virtual int32_t LnnPostScreenOffCheckDevMsgToHbFsm(
        LnnHeartbeatFsm *hbFsm, const LnnCheckDevStatusMsgPara *para, uint64_t delayMillis) = 0;
    virtual LnnHeartbeatFsm *LnnCreateHeartbeatFsm(void) = 0;
    virtual int32_t LnnStartHeartbeatFsm(LnnHeartbeatFsm *hbFsm) = 0;
    virtual void LnnRemoveScreenOffCheckStatusMsg(LnnHeartbeatFsm *hbFsm, LnnCheckDevStatusMsgPara *msgPara) = 0;
    virtual void LnnRemoveCheckDevStatusMsg(LnnHeartbeatFsm *hbFsm, LnnCheckDevStatusMsgPara *msgPara) = 0;
    virtual int32_t LnnPostStopMsgToHbFsm(LnnHeartbeatFsm *hbFsm, LnnHeartbeatType type) = 0;
    virtual int32_t LnnStopHeartbeatFsm(LnnHeartbeatFsm *hbFsm) = 0;
    virtual void LnnRemoveSendEndMsg(
        LnnHeartbeatFsm *hbFsm, LnnProcessSendOnceMsgPara *msg, bool wakeupFlag, bool *isRemoved) = 0;
    virtual int32_t LnnPostNextSendOnceMsgToHbFsm(
        LnnHeartbeatFsm *hbFsm, const LnnProcessSendOnceMsgPara *para, uint64_t delayMillis) = 0;
    virtual void LnnRemoveProcessSendOnceMsg(
        LnnHeartbeatFsm *hbFsm, LnnHeartbeatType hbType, LnnHeartbeatStrategyType strategyType) = 0;
    virtual void LnnHbClearRecvList(void) = 0;
    virtual int32_t LnnFsmRemoveMessage(FsmStateMachine *fsm, int32_t msgType) = 0;
    virtual int32_t LnnGetLocalNumU64Info(InfoKey key, uint64_t *info) = 0;
    virtual int32_t LnnGetRemoteNumU64Info(const char *networkId, InfoKey key, uint64_t *info) = 0;
    virtual bool IsFeatureSupport(uint64_t feature, FeatureCapability capaBit) = 0;
    virtual uint32_t GenerateRandomNumForHb(uint32_t randMin, uint32_t randMax) = 0;
    virtual bool LnnIsMultiDeviceOnline(void) = 0;
    virtual int32_t SoftBusMutexLockInner(SoftBusMutex *mutex) = 0;
    virtual int32_t LnnPostTransStateMsgToHbFsm(LnnHeartbeatFsm *hbFsm, LnnHeartbeatEventType evtType) = 0;
    virtual int32_t LnnPostUpdateSendInfoMsgToHbFsm(LnnHeartbeatFsm *hbFsm, LnnHeartbeatUpdateInfoType type) = 0;
    virtual int32_t LnnGetLocalNumInfo(InfoKey key, int32_t *info);
    virtual bool LnnIsNeedInterceptBroadcast(void);
};

class HeartBeatFSMStrategyInterfaceMock : public HeartBeatFSMStrategyInterface {
public:
    HeartBeatFSMStrategyInterfaceMock();
    ~HeartBeatFSMStrategyInterfaceMock() override;

    MOCK_METHOD3(LnnPostSendEndMsgToHbFsm, int32_t(LnnHeartbeatFsm *, LnnHeartbeatSendEndData *, uint64_t));
    MOCK_METHOD5(LnnPostSendBeginMsgToHbFsm,
        int32_t(LnnHeartbeatFsm *, LnnHeartbeatType, bool, LnnProcessSendOnceMsgPara *, uint64_t));
    MOCK_METHOD0(GetScreenState, SoftBusScreenState(void));
    MOCK_METHOD2(LnnCheckSupportedHbType, bool(LnnHeartbeatType *, LnnHeartbeatType *));
    MOCK_METHOD2(LnnPostStartMsgToHbFsm, int32_t(LnnHeartbeatFsm *, uint64_t));
    MOCK_METHOD3(LnnVisitHbTypeSet, bool(VisitHbTypeCb, LnnHeartbeatType *, void *));
    MOCK_METHOD1(LnnConvertConnAddrTypeToHbType, LnnHeartbeatType(ConnectionAddrType));
    MOCK_METHOD1(LnnConvertHbTypeToId, int32_t(LnnHeartbeatType));
    MOCK_METHOD1(LnnIsSupportBurstFeature, bool(const char *));
    MOCK_METHOD2(LnnPostSetMediumParamMsgToHbFsm, int32_t(LnnHeartbeatFsm *, const LnnHeartbeatMediumParam *));
    MOCK_METHOD3(
        LnnPostCheckDevStatusMsgToHbFsm, int32_t(LnnHeartbeatFsm *, const LnnCheckDevStatusMsgPara *, uint64_t));
    MOCK_METHOD3(
        LnnPostScreenOffCheckDevMsgToHbFsm, int32_t(LnnHeartbeatFsm *, const LnnCheckDevStatusMsgPara *, uint64_t));
    MOCK_METHOD0(LnnCreateHeartbeatFsm, LnnHeartbeatFsm *());
    MOCK_METHOD1(LnnStartHeartbeatFsm, int32_t(LnnHeartbeatFsm *));
    MOCK_METHOD2(LnnRemoveScreenOffCheckStatusMsg, void(LnnHeartbeatFsm *, LnnCheckDevStatusMsgPara *));
    MOCK_METHOD2(LnnRemoveCheckDevStatusMsg, void(LnnHeartbeatFsm *, LnnCheckDevStatusMsgPara *));
    MOCK_METHOD2(LnnPostStopMsgToHbFsm, int32_t(LnnHeartbeatFsm *, LnnHeartbeatType));
    MOCK_METHOD1(LnnStopHeartbeatFsm, int32_t(LnnHeartbeatFsm *));
    MOCK_METHOD4(LnnRemoveSendEndMsg, void(LnnHeartbeatFsm *, LnnProcessSendOnceMsgPara *, bool, bool *));
    MOCK_METHOD3(
        LnnPostNextSendOnceMsgToHbFsm, int32_t(LnnHeartbeatFsm *, const LnnProcessSendOnceMsgPara *, uint64_t));
    MOCK_METHOD3(LnnRemoveProcessSendOnceMsg, void(LnnHeartbeatFsm *, LnnHeartbeatType, LnnHeartbeatStrategyType));
    MOCK_METHOD0(LnnHbClearRecvList, void());
    MOCK_METHOD2(LnnFsmRemoveMessage, int32_t(FsmStateMachine *, int32_t));
    MOCK_METHOD2(LnnGetLocalNumU64Info, int32_t(InfoKey, uint64_t *));
    MOCK_METHOD3(LnnGetRemoteNumU64Info, int32_t(const char *, InfoKey, uint64_t *));
    MOCK_METHOD2(IsFeatureSupport, bool(uint64_t, FeatureCapability));
    MOCK_METHOD2(GenerateRandomNumForHb, uint32_t(uint32_t, uint32_t));
    MOCK_METHOD0(LnnIsMultiDeviceOnline, bool());
    MOCK_METHOD1(SoftBusMutexLockInner, int32_t(SoftBusMutex *));
    MOCK_METHOD2(LnnPostTransStateMsgToHbFsm, int32_t(LnnHeartbeatFsm *, LnnHeartbeatEventType));
    MOCK_METHOD2(LnnPostUpdateSendInfoMsgToHbFsm, int32_t(LnnHeartbeatFsm *, LnnHeartbeatUpdateInfoType));
    MOCK_METHOD2(LnnGetLocalNumInfo, int32_t(InfoKey key, int32_t *info));
    MOCK_METHOD0(LnnIsNeedInterceptBroadcast, bool());
};
} // namespace OHOS
#endif // HEARTBEAT_FSM_STRATEGY_H
