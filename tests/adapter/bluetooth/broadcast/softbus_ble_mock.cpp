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

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <pthread.h>

#include "disc_log.h"
#include "securec.h"
#include "softbus_adapter_mem.h"
#include "softbus_broadcast_mgr_mock.h"
#include "softbus_error_code.h"
#include "softbus_adapter_sle_common_struct.h"

#define NS_PER_USECOND 1000

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

extern "C" bool DiscIsOsAccountConstraint()
{
    return ManagerMock::GetMock()->DiscIsOsAccountConstraint();
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
    ManagerMock::capturedIsIncludeDeviceName = (param != nullptr) ? param->isIncludeDeviceName : false;
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

static int32_t MockSendParamsToLpDevice(const uint8_t *data, uint32_t dataSize, int32_t type)
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
        .SendParamsToLpDevice = MockSendParamsToLpDevice,
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
    if (listenerId != nullptr) {
        *listenerId = 0;
    }
    return SOFTBUS_OK;
}

void SoftBusRemoveSleStateListenerPacked(int listenerId)
{
    DISC_LOGI(DISC_TEST, "enter");
    (void)listenerId;
}
}

// ---------------------------------------------------------------------------
// Configurable stub medium interface: used to drive error/edge branches of
// softbus_broadcast_mgr.c. Behaviour is controlled through g_stubMediumConfig.
// ---------------------------------------------------------------------------

StubMediumConfig g_stubMediumConfig;
SoftbusBroadcastMediumInterface g_stubMediumInterface;
SoftbusBroadcastMediumInterface g_stubSleMediumInterface;
const SoftbusBroadcastCallback *g_stubBcCallback;
const SoftbusScanCallback *g_stubScanCallback;

static int32_t g_stubBcIdCounter = STUB_BC_ID_BASE;
static int32_t g_callocFailCountdown = -1;
static int32_t g_condInitFailCountdown = -1;
static bool g_getTimeFail = false;

void SetStubGetTimeFail(bool fail)
{
    g_getTimeFail = fail;
}

extern "C" int32_t SoftBusGetTime(SoftBusSysTime *sysTime)
{
    if (g_getTimeFail) {
        return SOFTBUS_ERR;
    }
    if (sysTime == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    struct timespec time = {0};
    (void)clock_gettime(CLOCK_MONOTONIC, &time);
    sysTime->sec = time.tv_sec;
    sysTime->usec = time.tv_nsec / NS_PER_USECOND;
    return SOFTBUS_OK;
}

typedef int32_t (*StubVarDumpCb)(int32_t fd);
static StubVarDumpCb g_bcMgrDumpCb = nullptr;

extern "C" int32_t SoftBusRegBcMgrVarDump(const char *dumpVar, StubVarDumpCb cb)
{
    (void)dumpVar;
    g_bcMgrDumpCb = cb;
    return SOFTBUS_OK;
}

int32_t InvokeStubBcMgrDump(int32_t fd)
{
    if (g_bcMgrDumpCb == nullptr) {
        return SOFTBUS_INVALID_PARAM;
    }
    return g_bcMgrDumpCb(fd);
}

void SetStubCallocFailCountdown(int32_t countdown)
{
    g_callocFailCountdown = countdown;
}

void SetStubCondInitFailCountdown(int32_t countdown)
{
    g_condInitFailCountdown = countdown;
}

extern "C" void *SoftBusCalloc(unsigned int size)
{
    if (g_callocFailCountdown >= 0) {
        if (g_callocFailCountdown == 0) {
            g_callocFailCountdown = -1;
            return nullptr;
        }
        g_callocFailCountdown--;
    }
    if (size > MAX_MALLOC_SIZE) {
        return nullptr;
    }
    void *ptr = malloc(size);
    if (ptr != nullptr) {
        (void)memset_s(ptr, size, 0, size);
    }
    return ptr;
}

extern "C" int32_t SoftBusCondInit(SoftBusCond *cond)
{
    if (g_condInitFailCountdown >= 0) {
        if (g_condInitFailCountdown == 0) {
            g_condInitFailCountdown = -1;
            return SOFTBUS_ERR;
        }
        g_condInitFailCountdown--;
    }
    if (cond == nullptr) {
        return SOFTBUS_INVALID_PARAM;
    }
    pthread_cond_t *tempCond = (pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
    if (tempCond == nullptr) {
        return SOFTBUS_ERR;
    }
    if (pthread_cond_init(tempCond, nullptr) != 0) {
        free(tempCond);
        return SOFTBUS_ERR;
    }
    *cond = (uintptr_t)tempCond;
    return SOFTBUS_OK;
}

static int32_t StubMediumInit(void)
{
    return SOFTBUS_OK;
}

static int32_t StubMediumDeInit(void)
{
    return SOFTBUS_OK;
}

static int32_t StubRegisterBroadcaster(int32_t *bcId, const SoftbusBroadcastCallback *cb)
{
    g_stubBcCallback = cb;
    *bcId = g_stubBcIdCounter++;
    return g_stubMediumConfig.registerBroadcasterRet;
}

static int32_t StubUnRegisterBroadcaster(int32_t bcId)
{
    (void)bcId;
    return g_stubMediumConfig.unRegisterBroadcasterRet;
}

static int32_t StubRegisterScanListener(int32_t *scanerId, const SoftbusScanCallback *cb)
{
    g_stubScanCallback = cb;
    *scanerId = STUB_SCAN_ID;
    return g_stubMediumConfig.registerScanListenerRet;
}

static int32_t StubUnRegisterScanListener(int32_t scanerId)
{
    (void)scanerId;
    return SOFTBUS_OK;
}

static int32_t StubStartBroadcasting(int32_t bcId, const SoftbusBroadcastParam *param, SoftbusBroadcastData *data)
{
    (void)param;
    (void)data;
    if (g_stubMediumConfig.startBcRet == SOFTBUS_OK && g_stubMediumConfig.invokeStartBcCb &&
        g_stubBcCallback != NULL && g_stubBcCallback->OnStartBroadcastingCallback != NULL) {
        g_stubBcCallback->OnStartBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
            bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    }
    return g_stubMediumConfig.startBcRet;
}

static int32_t StubStopBroadcasting(int32_t bcId)
{
    if (g_stubMediumConfig.stopBcRet == SOFTBUS_OK && g_stubMediumConfig.invokeStopBcCb &&
        g_stubBcCallback != NULL && g_stubBcCallback->OnStopBroadcastingCallback != NULL) {
        g_stubBcCallback->OnStopBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
            bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    }
    return g_stubMediumConfig.stopBcRet;
}

static int32_t StubSetBroadcastingData(int32_t bcId, const SoftbusBroadcastData *data)
{
    (void)data;
    if (g_stubMediumConfig.setBcDataRet == SOFTBUS_OK && g_stubBcCallback != NULL &&
        g_stubBcCallback->OnSetBroadcastingCallback != NULL) {
        g_stubBcCallback->OnSetBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
            bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    }
    return g_stubMediumConfig.setBcDataRet;
}

static int32_t StubSetBroadcastingParam(int32_t bcId, const SoftbusBroadcastParam *param)
{
    (void)param;
    if (g_stubMediumConfig.setBcParamRet == SOFTBUS_OK && g_stubBcCallback != NULL &&
        g_stubBcCallback->OnSetBroadcastingParamCallback != NULL) {
        g_stubBcCallback->OnSetBroadcastingParamCallback(BROADCAST_PROTOCOL_BLE,
            bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    }
    return g_stubMediumConfig.setBcParamRet;
}

static int32_t StubEnableBroadcasting(int32_t bcId)
{
    if (g_stubMediumConfig.enableBcRet == SOFTBUS_OK && g_stubMediumConfig.invokeEnableBcCb &&
        g_stubBcCallback != NULL && g_stubBcCallback->OnEnableBroadcastingCallback != NULL) {
        g_stubBcCallback->OnEnableBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
            bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    }
    return g_stubMediumConfig.enableBcRet;
}

static int32_t StubDisableBroadcasting(int32_t bcId)
{
    if (g_stubMediumConfig.disableBcRet == SOFTBUS_OK && g_stubMediumConfig.invokeDisableBcCb &&
        g_stubBcCallback != NULL && g_stubBcCallback->OnDisableBroadcastingCallback != NULL) {
        g_stubBcCallback->OnDisableBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
            bcId, (int32_t)SOFTBUS_BC_STATUS_SUCCESS);
    }
    return g_stubMediumConfig.disableBcRet;
}

static int32_t StubStartScan(int32_t scanerId, const SoftBusBcScanParams *param,
    const SoftBusBcScanFilter *scanFilter, int32_t filterSize)
{
    (void)scanerId;
    (void)param;
    (void)scanFilter;
    (void)filterSize;
    return g_stubMediumConfig.startScanRet;
}

static int32_t StubStopScan(int32_t scanerId)
{
    (void)scanerId;
    return g_stubMediumConfig.stopScanRet;
}

static int32_t StubSetScanParams(int32_t scannerId, const SoftBusBcScanParams *param,
    const SoftBusBcScanFilter *scanFilter, int32_t filterSize, SoftbusSetFilterCmd cmdId)
{
    (void)scannerId;
    (void)param;
    (void)scanFilter;
    (void)filterSize;
    (void)cmdId;
    return g_stubMediumConfig.setScanParamsRet;
}

static bool StubIsLpDeviceAvailable(void)
{
    return true;
}

static bool StubSetAdvFilterParam(
    LpServerType type, const SoftBusLpBroadcastParam *bcParam, const SoftBusLpScanParam *scanParam)
{
    (void)type;
    (void)bcParam;
    (void)scanParam;
    return g_stubMediumConfig.setAdvFilterParamRet;
}

static int32_t StubGetBroadcastHandle(int32_t bcId, int32_t *bcHandle)
{
    (void)bcId;
    *bcHandle = 0;
    return SOFTBUS_OK;
}

static int32_t StubEnableSyncDataToLpDevice(void)
{
    return SOFTBUS_OK;
}

static int32_t StubDisableSyncDataToLpDevice(void)
{
    return SOFTBUS_OK;
}

static int32_t StubSetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    (void)scannerId;
    (void)enable;
    return g_stubMediumConfig.setScanReportChannelRet;
}

static int32_t StubSetLpDeviceParam(
    int32_t duration, int32_t maxExtAdvEvents, int32_t window, int32_t interval, int32_t bcHandle)
{
    (void)duration;
    (void)maxExtAdvEvents;
    (void)window;
    (void)interval;
    (void)bcHandle;
    return SOFTBUS_OK;
}

static int32_t StubSendParamsToLpDevice(const uint8_t *data, uint32_t dataSize, int32_t type)
{
    (void)data;
    (void)dataSize;
    (void)type;
    return SOFTBUS_OK;
}

static int8_t StubGetAdvPower(int32_t businessType)
{
    (void)businessType;
    return 0;
}

void ResetStubMediumInterfaceFuncs(void)
{
    g_stubMediumInterface = {
        .Init = StubMediumInit,
        .DeInit = StubMediumDeInit,
        .RegisterBroadcaster = StubRegisterBroadcaster,
        .UnRegisterBroadcaster = StubUnRegisterBroadcaster,
        .RegisterScanListener = StubRegisterScanListener,
        .UnRegisterScanListener = StubUnRegisterScanListener,
        .StartBroadcasting = StubStartBroadcasting,
        .SetScanParams = StubSetScanParams,
        .StopBroadcasting = StubStopBroadcasting,
        .EnableAdvertising = NULL,
        .DisableAdvertising = NULL,
        .SetBroadcastingData = StubSetBroadcastingData,
        .SetBroadcastingParam = StubSetBroadcastingParam,
        .EnableBroadcasting = StubEnableBroadcasting,
        .DisableBroadcasting = StubDisableBroadcasting,
        .UpdateBroadcasting = NULL,
        .StartScan = StubStartScan,
        .StopScan = StubStopScan,
        .IsLpDeviceAvailable = StubIsLpDeviceAvailable,
        .SetAdvFilterParam = StubSetAdvFilterParam,
        .GetBroadcastHandle = StubGetBroadcastHandle,
        .EnableSyncDataToLpDevice = StubEnableSyncDataToLpDevice,
        .DisableSyncDataToLpDevice = StubDisableSyncDataToLpDevice,
        .SetScanReportChannelToLpDevice = StubSetScanReportChannelToLpDevice,
        .SetLpDeviceParam = StubSetLpDeviceParam,
        .SendParamsToLpDevice = StubSendParamsToLpDevice,
        .GetAdvPower = StubGetAdvPower,
    };
}

static int32_t StubSleRegisterScanListener(int32_t *scanerId, const SoftbusScanCallback *cb)
{
    g_stubScanCallback = cb;
    *scanerId = STUB_SLE_SCAN_ID;
    return SOFTBUS_OK;
}

static void ResetStubSleMediumInterface(void)
{
    g_stubSleMediumInterface = {
        .Init = StubMediumInit,
        .DeInit = StubMediumDeInit,
        .RegisterScanListener = StubSleRegisterScanListener,
        .UnRegisterScanListener = StubUnRegisterScanListener,
        .SetScanParams = StubSetScanParams,
        .StartScan = StubStartScan,
        .StopScan = StubStopScan,
    };
}

void ResetStubMedium(void)
{
    g_stubMediumConfig = {
        .registerBroadcasterRet = SOFTBUS_OK,
        .unRegisterBroadcasterRet = SOFTBUS_OK,
        .registerScanListenerRet = SOFTBUS_OK,
        .startBcRet = SOFTBUS_OK,
        .stopBcRet = SOFTBUS_OK,
        .setBcDataRet = SOFTBUS_OK,
        .setBcParamRet = SOFTBUS_OK,
        .enableBcRet = SOFTBUS_OK,
        .disableBcRet = SOFTBUS_OK,
        .startScanRet = SOFTBUS_OK,
        .stopScanRet = SOFTBUS_OK,
        .setScanParamsRet = SOFTBUS_OK,
        .setScanReportChannelRet = SOFTBUS_OK,
        .setAdvFilterParamRet = true,
        .invokeStartBcCb = true,
        .invokeStopBcCb = true,
        .invokeEnableBcCb = true,
        .invokeDisableBcCb = true,
    };
    g_stubBcIdCounter = STUB_BC_ID_BASE;
    g_stubBcCallback = nullptr;
    g_stubScanCallback = nullptr;
    g_callocFailCountdown = -1;
    g_condInitFailCountdown = -1;
    g_getTimeFail = false;
    ResetStubMediumInterfaceFuncs();
    ResetStubSleMediumInterface();
}

void ActionOfRegisterStubMediumInterface(void)
{
    DISC_LOGI(DISC_TEST, "enter stub medium");
    ResetStubMedium();
    if (RegisterBroadcastMediumFunction(BROADCAST_PROTOCOL_BLE, &g_stubMediumInterface) != 0) {
        DISC_LOGE(DISC_TEST, "Register stub medium interface failed.");
    }
}