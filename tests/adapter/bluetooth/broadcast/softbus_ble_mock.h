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

#ifndef SOFTBUS_BLE_MGR_MOCK_H
#define SOFTBUS_BLE_MGR_MOCK_H

#include "gmock/gmock.h"

#include "softbus_adapter_bt_common.h"
#include "softbus_ble_gatt_public.h"
#include "softbus_broadcast_adapter_interface.h"
#include "softbus_broadcast_manager.h"
#include "softbus_broadcast_mgr_utils.h"

class BleGattInterface {
public:
    virtual void SoftbusBleAdapterInit() = 0;
    virtual int32_t SoftBusAddBtStateListener(const SoftBusBtStateListener *listener, int32_t *listenerId) = 0;
    virtual int32_t SoftBusRemoveBtStateListener(int32_t listenerId) = 0;
    virtual int32_t BleAsyncCallbackDelayHelper(SoftBusLooper *looper, BleAsyncCallbackFunc callback,
        void *para, uint64_t delayMillis) = 0;
    virtual int32_t SoftBusCondWait(SoftBusCond *cond, SoftBusMutex *mutex, SoftBusSysTime *time) = 0;
};

class ManagerMock : public BleGattInterface {
public:
    static ManagerMock *GetMock();

    ManagerMock();
    ~ManagerMock();

    MOCK_METHOD(void, SoftbusBleAdapterInit, (), (override));
    MOCK_METHOD(int32_t, SoftBusAddBtStateListener,
        (const SoftBusBtStateListener *listener, int32_t *listenerId), (override));
    MOCK_METHOD(int32_t, SoftBusRemoveBtStateListener, (int32_t listenerId), (override));
    MOCK_METHOD(int32_t, BleAsyncCallbackDelayHelper,
        (SoftBusLooper *looper, BleAsyncCallbackFunc callback,
            void *para, uint64_t delayMillis), (override));
    MOCK_METHOD(int32_t, SoftBusCondWait,
        (SoftBusCond *cond, SoftBusMutex *mutex, SoftBusSysTime *time), (override));

    static const SoftbusBroadcastCallback *broadcastCallback;
    static const SoftbusScanCallback *scanCallback;
    static inline const SoftbusScanCallback *softbusScanCallback {};
    static inline const SoftbusBroadcastCallback *softbusBroadcastCallback {};

private:
    static ManagerMock *managerMock;
};

#endif /* SOFTBUS_BLE_MGR_MOCK_H */