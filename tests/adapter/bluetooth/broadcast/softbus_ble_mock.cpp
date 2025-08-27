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

#include "softbus_ble_mock.h"

#include "disc_log.h"
#include "softbus_error_code.h"
#include "softbus_adapter_sle_common_struct.h"

ManagerMock *ManagerMock::managerMock = nullptr;
const SoftbusBroadcastCallback *ManagerMock::broadcastCallback = nullptr;
const SoftbusScanCallback *ManagerMock::scanCallback = nullptr;

static int32_t g_advId = 0;
static int32_t g_listenerId = 0;
static void ActionOfSoftbusBleAdapterInit(void);

int32_t ActionOfSoftBusAddBtStateListener(const SoftBusBtStateListener *listener, int32_t *listenerId)
{
    return SOFTBUS_OK;
}

int32_t ActionOfSoftBusRemoveBtStateListener(int32_t listenerId)
{
    return SOFTBUS_OK;
}

ManagerMock::ManagerMock()
{
    ManagerMock::managerMock = this;
    EXPECT_CALL(*this, SoftbusBleAdapterInit).WillRepeatedly(ActionOfSoftbusBleAdapterInit);
    EXPECT_CALL(*this, SoftBusAddBtStateListener).WillRepeatedly(ActionOfSoftBusAddBtStateListener);
    EXPECT_CALL(*this, SoftBusRemoveBtStateListener).WillRepeatedly(ActionOfSoftBusRemoveBtStateListener);
}

ManagerMock::~ManagerMock()
{
    ManagerMock::managerMock = nullptr;
}

ManagerMock *ManagerMock::GetMock()
{
    return managerMock;
}

void SoftbusBleAdapterInit()
{
    ManagerMock::GetMock()->SoftbusBleAdapterInit();
}

static int32_t MockInit(void)
{
    return SOFTBUS_OK;
}

static int32_t MockDeInit(void)
{
    return SOFTBUS_OK;
}

int32_t SoftBusAddBtStateListener(const SoftBusBtStateListener *listener, int32_t *listenerId)
{
    return ManagerMock::GetMock()->SoftBusAddBtStateListener(listener, listenerId);
}

int32_t SoftBusRemoveBtStateListener(int32_t listenerId)
{
    return ManagerMock::GetMock()->SoftBusRemoveBtStateListener(listenerId);
}

int32_t BleAsyncCallbackDelayHelper(SoftBusLooper *looper, BleAsyncCallbackFunc callback,
    void *para, uint64_t delayMillis)
{
    return ManagerMock::GetMock()->BleAsyncCallbackDelayHelper(looper, callback, para, delayMillis);
}

int32_t SoftBusCondWait(SoftBusCond *cond, SoftBusMutex *mutex, SoftBusSysTime *time)
{
    return ManagerMock::GetMock()->SoftBusCondWait(cond, mutex, time);
}

static int32_t MockRegisterBroadcaster(int32_t *bcId, const SoftbusBroadcastCallback *cb)
{
    ManagerMock::broadcastCallback = cb;
    *bcId = g_advId;
    g_advId++;
    return SOFTBUS_OK;
}

static int32_t MockUnRegisterBroadcaster(int32_t bcId)
{
    ManagerMock::broadcastCallback = nullptr;
    g_advId--;
    return SOFTBUS_OK;
}

static int32_t MockRegisterScanListener(int32_t *scanerId, const SoftbusScanCallback *cb)
{
    ManagerMock::scanCallback = cb;
    *scanerId = g_listenerId;
    g_listenerId++;
    return SOFTBUS_OK;
}

static int32_t MockUnRegisterScanListener(int32_t scanerId)
{
    ManagerMock::scanCallback = nullptr;
    g_listenerId--;
    return SOFTBUS_OK;
}

static int32_t MockStartBroadcasting(int32_t bcId, const SoftbusBroadcastParam *param, SoftbusBroadcastData *data)
{
    ManagerMock::broadcastCallback->OnStartBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockStopBroadcasting(int32_t bcId)
{
    ManagerMock::broadcastCallback->OnStopBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockSetBroadcastingData(int32_t bcId, const SoftbusBroadcastData *data)
{
    ManagerMock::broadcastCallback->OnSetBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockSetBroadcastingParam(int32_t bcId, const SoftbusBroadcastParam *param)
{
    ManagerMock::broadcastCallback->OnSetBroadcastingParamCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockEnableBroadcasting(int32_t bcId)
{
    ManagerMock::broadcastCallback->OnEnableBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockDisableBroadcasting(int32_t bcId)
{
    ManagerMock::broadcastCallback->OnDisableBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockUpdateBroadcasting(
    int32_t bcId, const SoftbusBroadcastParam *param, SoftbusBroadcastData *data)
{
    ManagerMock::broadcastCallback->OnUpdateBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    return SOFTBUS_OK;
}

static int32_t MockStartScan(
    int32_t scanerId, const SoftBusBcScanParams *param, const SoftBusBcScanFilter *scanFilter, int32_t filterSize)
{
    return SOFTBUS_OK;
}

static int32_t MockStopScan(int32_t scanerId)
{
    return SOFTBUS_OK;
}

static int32_t MockSetScanParams(int32_t scannerId, const SoftBusBcScanParams *param,
    const SoftBusBcScanFilter *scanFilter, int32_t filterSize, SoftbusSetFilterCmd cmdId)
{
    return SOFTBUS_OK;
}

static bool MockIsLpDeviceAvailable(void)
{
    return true;
}

static bool MockSetAdvFilterParam(
    LpServerType type, const SoftBusLpBroadcastParam *bcParam, const SoftBusLpScanParam *scanParam)
{
    return SOFTBUS_OK;
}

static int32_t MockGetBroadcastHandle(int32_t bcId, int32_t *bcHandle)
{
    return SOFTBUS_OK;
}

static int32_t MockEnableSyncDataToLpDevice(void)
{
    return SOFTBUS_OK;
}

static int32_t MockDisableSyncDataToLpDevice(void)
{
    return SOFTBUS_OK;
}

static int32_t MockSetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    return SOFTBUS_OK;
}

static int32_t MockSetLpDeviceParam(
    int32_t duration, int32_t maxExtAdvEvents, int32_t window, int32_t interval, int32_t bcHandle)
{
    return SOFTBUS_OK;
}

static void ActionOfSoftbusBleAdapterInit()
{
    DISC_LOGI(DISC_TEST, "enter");
    static SoftbusBroadcastMediumInterface interface = {
        .Init = MockInit,
        .DeInit = MockDeInit,
        .RegisterBroadcaster = MockRegisterBroadcaster,
        .UnRegisterBroadcaster = MockUnRegisterBroadcaster,
        .RegisterScanListener = MockRegisterScanListener,
        .UnRegisterScanListener = MockUnRegisterScanListener,
        .StartBroadcasting = MockStartBroadcasting,
        .StopBroadcasting = MockStopBroadcasting,
        .SetBroadcastingData = MockSetBroadcastingData,
        .SetBroadcastingParam = MockSetBroadcastingParam,
        .EnableBroadcasting = MockEnableBroadcasting,
        .DisableBroadcasting = MockDisableBroadcasting,
        .UpdateBroadcasting = MockUpdateBroadcasting,
        .StartScan = MockStartScan,
        .StopScan = MockStopScan,
        .SetScanParams = MockSetScanParams,
        .IsLpDeviceAvailable = MockIsLpDeviceAvailable,
        .SetAdvFilterParam = MockSetAdvFilterParam,
        .GetBroadcastHandle = MockGetBroadcastHandle,
        .EnableSyncDataToLpDevice = MockEnableSyncDataToLpDevice,
        .DisableSyncDataToLpDevice = MockDisableSyncDataToLpDevice,
        .SetScanReportChannelToLpDevice = MockSetScanReportChannelToLpDevice,
        .SetLpDeviceParam = MockSetLpDeviceParam,
    };
    if (RegisterBroadcastMediumFunction(BROADCAST_PROTOCOL_BLE, &interface) != 0) {
        DISC_LOGE(DISC_TEST, "Register gatt interface failed.");
    }
}

void SoftbusBleAdapterDeInit(void)
{
    DISC_LOGI(DISC_TEST, "enter");
}

extern "C" {
void SoftbusSleAdapterInitPacked(void)
{
    DISC_LOGI(DISC_TEST, "enter");
}

void SoftbusSleAdapterDeInitPacked(void)
{
    DISC_LOGI(DISC_TEST, "enter");
}

int32_t SoftBusAddSleStateListenerPacked(const SoftBusSleStateListener *listener, int32_t *listenerId)
{
    DISC_LOGI(DISC_TEST, "enter");
    (void)listener;
    (void)listenerId;
    return SOFTBUS_OK;
}

void SoftBusRemoveSleStateListenerPacked(int listenerId)
{
    DISC_LOGI(DISC_TEST, "enter");
    (void)listenerId;
}
}