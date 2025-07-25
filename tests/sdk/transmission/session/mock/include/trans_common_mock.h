/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef TRANS_COMMON_MOCK_H
#define TRANS_COMMON_MOCK_H

#include <gmock/gmock.h>

#include "g_enhance_sdk_func.h"
#include "softbus_bus_center.h"
#include "softbus_config_type.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "softbus_utils.h"

namespace OHOS {
class TransCommInterface {
public:
    TransCommInterface() {};
    virtual ~TransCommInterface() {};

    virtual int SoftbusGetConfig(ConfigType type, unsigned char *val, uint32_t len) = 0;
    virtual ClientEnhanceFuncList *ClientEnhanceFuncListGet(void) = 0;
    virtual int32_t WriteInt32ToBuf(uint8_t *buf, uint32_t dataLen, int32_t *offSet, int32_t data) = 0;
    virtual int32_t WriteUint64ToBuf(uint8_t *buf, uint32_t bufLen, int32_t *offSet, uint64_t data) = 0;
    virtual int32_t WriteStringToBuf(uint8_t *buf, uint32_t bufLen, int32_t *offSet, char *data, uint32_t dataLen) = 0;
    virtual int32_t ServerIpcProcessInnerEvent(int32_t eventType, uint8_t *buf, uint32_t len) = 0;
    virtual SoftBusList *CreateSoftBusList(void) = 0;
    virtual int32_t TransServerProxyInit(void) = 0;
    virtual int32_t ClientTransChannelInit(void) = 0;
    virtual int32_t RegisterTimeoutCallback(int32_t timerFunId, TimerFunCallback callback) = 0;
    virtual int32_t RegNodeDeviceStateCbInner(const char *pkgName, INodeStateCb *callback) = 0;
    virtual int32_t SoftBusCondSignal(SoftBusCond *cond) = 0;
    virtual int32_t SoftBusGetTime(SoftBusSysTime *sysTime) = 0;
    virtual int32_t SoftBusCondWait(SoftBusCond *cond, SoftBusMutex *mutex, SoftBusSysTime *time) = 0;
};

class TransCommInterfaceMock : public TransCommInterface {
public:
    TransCommInterfaceMock();
    ~TransCommInterfaceMock() override;

    MOCK_METHOD3(SoftbusGetConfig, int(ConfigType type, unsigned char *val, uint32_t len));
    MOCK_METHOD0(ClientEnhanceFuncListGet, ClientEnhanceFuncList *(void));
    MOCK_METHOD4(WriteInt32ToBuf, int32_t(uint8_t *buf, uint32_t dataLen, int32_t *offSet, int32_t data));
    MOCK_METHOD4(WriteUint64ToBuf, int32_t(uint8_t *buf, uint32_t bufLen, int32_t *offSet, uint64_t data));
    MOCK_METHOD5(WriteStringToBuf, int32_t(
        uint8_t *buf, uint32_t bufLen, int32_t *offSet, char *data, uint32_t dataLen));
    MOCK_METHOD3(ServerIpcProcessInnerEvent, int32_t(int32_t eventType, uint8_t *buf, uint32_t len));
    MOCK_METHOD0(CreateSoftBusList, SoftBusList *(void));
    MOCK_METHOD0(TransServerProxyInit, int32_t(void));
    MOCK_METHOD0(ClientTransChannelInit, int32_t(void));
    MOCK_METHOD2(RegisterTimeoutCallback, int32_t(int32_t timerFunId, TimerFunCallback callback));
    MOCK_METHOD2(RegNodeDeviceStateCbInner, int32_t(const char *pkgName, INodeStateCb *callback));
    MOCK_METHOD1(SoftBusCondSignal, int32_t(SoftBusCond *cond));
    MOCK_METHOD1(SoftBusGetTime, int32_t(SoftBusSysTime *sysTime));
    MOCK_METHOD3(SoftBusCondWait, int32_t(SoftBusCond *cond, SoftBusMutex *mutex, SoftBusSysTime *time));

    static int ActionOfSoftbusGetConfig(ConfigType type, unsigned char *val, uint32_t len);
};

} // namespace OHOS
#endif // TRANS_COMMON_MOCK_H
