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

#include "bluetooth_mock.h"
#include "c_header/ohos_bt_gatt.h"
#include "disc_log.h"
#include "softbus_adapter_mem.h"
#include "softbus_ble_gatt_public.h"
#include "softbus_ble_gatt_test_common.h"
#include "softbus_broadcast_type.h"
#include "softbus_error_code.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing::ext;
using ::testing::Return;

namespace OHOS {

// Fixture for the BleGattCover coverage tests. Re-initializing the adapter is
// idempotent: it re-registers the same static interface and resets channel ids.
class SoftbusBleGattCoverTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        MockBluetooth mocker;
        SoftbusBleAdapterInit();
        MockBluetooth::interface->Init();
    }
};

static void CoverFakeBcBleCallback(BroadcastProtocol protocol, int32_t adapterBcld, int32_t status)
{
    (void)protocol;
    (void)adapterBcld;
    (void)status;
}

static SoftbusBroadcastCallback g_coverSoftbusBcBleCbTest = {
    .OnStartBroadcastingCallback = CoverFakeBcBleCallback,
    .OnStopBroadcastingCallback = CoverFakeBcBleCallback,
    .OnUpdateBroadcastingCallback = CoverFakeBcBleCallback,
    .OnSetBroadcastingCallback = CoverFakeBcBleCallback,
    .OnSetBroadcastingParamCallback = CoverFakeBcBleCallback,
    .OnDisableBroadcastingCallback = CoverFakeBcBleCallback,
    .OnEnableBroadcastingCallback = CoverFakeBcBleCallback,
};

static void CoverStubOnScanResult(BroadcastProtocol protocol, int32_t scannerId, const SoftBusBcScanResult *reportData)
{
    (void)protocol;
    (void)scannerId;
    (void)reportData;
}

static SoftbusScanCallback *GetCoverStubScanListener()
{
    static SoftbusScanCallback listener = { .OnStartScanCallback = nullptr,
        .OnStopScanCallback = nullptr,
        .OnReportScanDataCallback = CoverStubOnScanResult,
        .OnScanStateChanged = nullptr };
    return &listener;
}


// ---------------------------------------------------------------------------
// Coverage tests driven by the configurable stubs in bluetooth_mock.cpp.
// ---------------------------------------------------------------------------
#define BLE_GATT_ADV_CH_NUM 21
#define BLE_GATT_SCAN_CH_NUM 8
#define LONG_SCAN_PAYLOAD_LEN 25
#define LONG_SCAN_RSP_LEN 28

// Reset all adv/scan channels to a clean state and recreate the locks.
static void ResetBleGattChannels(void)
{
    (void)MockBluetooth::interface->DeInit();
    (void)MockBluetooth::interface->Init();
    for (int32_t i = 0; i < BLE_GATT_ADV_CH_NUM; i++) {
        (void)MockBluetooth::interface->UnRegisterBroadcaster(i);
    }
    for (int32_t i = 0; i < BLE_GATT_SCAN_CH_NUM; i++) {
        (void)MockBluetooth::interface->UnRegisterScanListener(i);
    }
}

static SoftbusScanCallback *GetStubScanListenerNoReport()
{
    static SoftbusScanCallback listener = { .OnStartScanCallback = nullptr,
        .OnStopScanCallback = nullptr,
        .OnReportScanDataCallback = nullptr,
        .OnScanStateChanged = nullptr };
    return &listener;
}

static SoftbusScanCallback *GetStubScanListenerNoLp()
{
    static SoftbusScanCallback listener = { .OnStartScanCallback = nullptr,
        .OnStopScanCallback = nullptr,
        .OnReportScanDataCallback = nullptr,
        .OnScanStateChanged = nullptr,
        .OnLpDeviceInfoCallback = nullptr };
    return &listener;
}

static void InvokeScanResultWithValidData()
{
    const unsigned char scanDataExample[] = { 0x02, 0x01, 0x02, 0x15, 0x16, 0xEE, 0xFD, 0x04, 0x05, 0x90, 0x00, 0x00,
        0x04, 0x00, 0x18, 0x33, 0x39, 0x36, 0x62, 0x33, 0x61, 0x33, 0x31, 0x21, 0x00, 0x02, 0x0A, 0xEF, 0x03, 0xFF,
        0x7D, 0x02 };
    BtScanResultData mockScanResult = { 0 };
    mockScanResult.advLen = sizeof(scanDataExample);
    mockScanResult.advData = const_cast<unsigned char *>(scanDataExample);
    MockBluetooth::bleScanCallback->scanResultCb(&mockScanResult);
}

/*
 * @tc.name: BleGattCoverMutexInitFail001
 * @tc.desc: Cover SoftBusMutexInit failure branches in SoftbusGattInit
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverMutexInitFail001, TestSize.Level1)
{
    MockBluetooth mocker;
    (void)MockBluetooth::interface->DeInit();

    // g_advLock init fails
    SetBleGattMutexInitFailCountdown(0);
    int32_t ret = MockBluetooth::interface->Init();
    EXPECT_EQ(ret, SOFTBUS_NO_INIT);

    // g_scannerLock init fails
    SetBleGattMutexInitFailCountdown(1);
    ret = MockBluetooth::interface->Init();
    EXPECT_EQ(ret, SOFTBUS_NO_INIT);

    SetBleGattMutexInitFailCountdown(-1);
    ret = MockBluetooth::interface->Init();
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverWrapperAdvCb001
 * @tc.desc: Cover the lock check line of all adv wrapper callbacks and SoftbusUnRegisterAdvCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverWrapperAdvCb001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t advId = -1;
    int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advId, &g_coverSoftbusBcBleCbTest);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ASSERT_NE(MockBluetooth::btGattCallback, nullptr);

    MockBluetooth::btGattCallback->advEnableCb(0, OHOS_BT_STATUS_SUCCESS);
    MockBluetooth::btGattCallback->advDisableCb(0, OHOS_BT_STATUS_SUCCESS);
    MockBluetooth::btGattCallback->advDataCb(0, OHOS_BT_STATUS_SUCCESS);
    MockBluetooth::btGattCallback->advUpdateCb(0, OHOS_BT_STATUS_SUCCESS);
    MockBluetooth::btGattCallback->advChangeCb(0, OHOS_BT_STATUS_SUCCESS);
    MockBluetooth::btGattCallback->onEnableExCb(0, OHOS_BT_STATUS_SUCCESS);
    MockBluetooth::btGattCallback->onDisableExCb(0, OHOS_BT_STATUS_SUCCESS);

    ret = MockBluetooth::interface->UnRegisterBroadcaster(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverNoAdvChannel001
 * @tc.desc: Cover "no available adv channel" branch in SoftbusRegisterAdvCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverNoAdvChannel001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t advIds[BLE_GATT_ADV_CH_NUM] = {};
    for (int32_t i = 0; i < BLE_GATT_ADV_CH_NUM; i++) {
        advIds[i] = -1;
        int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advIds[i], &g_coverSoftbusBcBleCbTest);
        EXPECT_EQ(ret, SOFTBUS_OK);
    }
    int32_t advId = -1;
    int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advId, &g_coverSoftbusBcBleCbTest);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_REGISTER_FAIL);

    for (int32_t i = 0; i < BLE_GATT_ADV_CH_NUM; i++) {
        EXPECT_EQ(MockBluetooth::interface->UnRegisterBroadcaster(advIds[i]), SOFTBUS_OK);
    }
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanResultNull001
 * @tc.desc: Cover null-data branch and lock check line in WrapperScanResultCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanResultNull001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);
    ASSERT_NE(MockBluetooth::bleScanCallback, nullptr);

    MockBluetooth::bleScanCallback->scanResultCb(nullptr);
    InvokeScanResultWithValidData();

    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanResultNotScanning001
 * @tc.desc: Cover "scanner is not in used/scanning" branch in WrapperScanResultCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanResultNotScanning001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);
    ASSERT_NE(MockBluetooth::bleScanCallback, nullptr);

    // registered but not scanning
    InvokeScanResultWithValidData();

    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanResultCbNull001
 * @tc.desc: Cover "scanner callback is null" branch in WrapperScanResultCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanResultCbNull001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetStubScanListenerNoReport());
    EXPECT_EQ(ret, SOFTBUS_OK);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusBcScanParams scanParam = {
        .scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10,
        .scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10,
        .scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE,
        .scanPhy = SOFTBUS_BC_SCAN_PHY_1M,
        .scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL,
    };
    EXPECT_CALL(mocker, BleStartScanEx).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ret = MockBluetooth::interface->StartScan(scannerId, &scanParam, filter, 1);
    EXPECT_EQ(ret, SOFTBUS_OK);

    InvokeScanResultWithValidData();

    EXPECT_CALL(mocker, BleStopScan).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    (void)MockBluetooth::interface->StopScan(scannerId);
    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanResultTooLong001
 * @tc.desc: Cover bc/rsp payload too long branches in WrapperScanResultCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanResultTooLong001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusBcScanParams scanParam = {
        .scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10,
        .scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10,
        .scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE,
        .scanPhy = SOFTBUS_BC_SCAN_PHY_1M,
        .scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL,
    };
    EXPECT_CALL(mocker, BleStartScanEx).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ret = MockBluetooth::interface->StartScan(scannerId, &scanParam, filter, 1);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // flag + service AD (25 bytes payload, > BC_DATA_MAX_LEN) + manufacturer AD (28 bytes payload, > RSP_DATA_MAX_LEN)
    uint8_t longScanData[3 + 29 + 32] = { 0 };
    uint32_t offset = 0;
    longScanData[offset++] = 0x02;
    longScanData[offset++] = 0x01;
    longScanData[offset++] = 0x02;
    longScanData[offset++] = 0x1C;
    longScanData[offset++] = 0x16;
    longScanData[offset++] = 0xEE;
    longScanData[offset++] = 0xFD;
    for (uint32_t i = 0; i < LONG_SCAN_PAYLOAD_LEN; i++) {
        longScanData[offset++] = (uint8_t)i;
    }
    longScanData[offset++] = 0x1F;
    longScanData[offset++] = 0xFF;
    longScanData[offset++] = 0x7D;
    longScanData[offset++] = 0x02;
    for (uint32_t i = 0; i < LONG_SCAN_RSP_LEN; i++) {
        longScanData[offset++] = (uint8_t)i;
    }
    BtScanResultData mockScanResult = { 0 };
    mockScanResult.advLen = static_cast<unsigned char>(offset);
    mockScanResult.advData = longScanData;
    MockBluetooth::bleScanCallback->scanResultCb(&mockScanResult);

    EXPECT_CALL(mocker, BleStopScan).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    (void)MockBluetooth::interface->StopScan(scannerId);
    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanStateCbAbnormal001
 * @tc.desc: Cover not-used and null-callback branches in WrapperScanStateChangeCb/WrapperLpDeviceInfoCb
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanStateCbAbnormal001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetStubScanListenerNoLp());
    EXPECT_EQ(ret, SOFTBUS_OK);
    ASSERT_NE(MockBluetooth::bleScanCallback, nullptr);

    BtUuid uuid = {};
    uint8_t data = 0x01;
    // scanCallback has null OnScanStateChanged/OnLpDeviceInfoCallback
    MockBluetooth::bleScanCallback->scanStateChangeCb(0, true);
    MockBluetooth::bleScanCallback->lpDeviceInfoCb(&uuid, 0, &data, sizeof(data));

    // channel not used after unregister
    BleScanCallbacks *savedCb = MockBluetooth::bleScanCallback;
    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    savedCb->scanStateChangeCb(0, true);
    savedCb->lpDeviceInfoCb(&uuid, 0, &data, sizeof(data));

    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverChannelCb001
 * @tc.desc: Cover channel specific wrapper callbacks (STEADY/SHARE/UNSTEADY/VLINK)
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverChannelCb001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerIds[5] = { -1, -1, -1, -1, -1 };
    BtUuid uuid = {};
    uint8_t data = 0x01;
    for (int32_t i = 0; i < 5; i++) {
        int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerIds[i], GetStubScanListenerNoLp());
        EXPECT_EQ(ret, SOFTBUS_OK);
        ASSERT_NE(MockBluetooth::bleScanCallback, nullptr);
        MockBluetooth::bleScanCallback->scanStateChangeCb(0, true);
        MockBluetooth::bleScanCallback->lpDeviceInfoCb(&uuid, 0, &data, sizeof(data));
        InvokeScanResultWithValidData();
    }

    for (int32_t i = 0; i < 5; i++) {
        EXPECT_EQ(MockBluetooth::interface->UnRegisterScanListener(scannerIds[i]), SOFTBUS_OK);
    }
    int32_t ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverStartAdvAssembleFail001
 * @tc.desc: Cover StartBleAdv assemble failure and adapter failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverStartAdvAssembleFail001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t advId = -1;
    int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advId, &g_coverSoftbusBcBleCbTest);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftbusBroadcastParam params = {};
    SoftbusBroadcastData data = BuildBcData();

    // AssembleAdvData calloc failure
    SetBleGattCallocFailCountdown(0);
    ret = MockBluetooth::interface->StartBroadcasting(advId, &params, &data);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_ASSEMBLE_FAIL);

    // AssembleRspData calloc failure
    SetBleGattCallocFailCountdown(1);
    ret = MockBluetooth::interface->StartBroadcasting(advId, &params, &data);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_ASSEMBLE_FAIL);

    // adapter BleStartAdvEx failure
    EXPECT_CALL(mocker, BleStartAdvEx).WillRepeatedly(Return(OHOS_BT_STATUS_FAIL));
    ret = MockBluetooth::interface->StartBroadcasting(advId, &params, &data);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);

    ret = MockBluetooth::interface->UnRegisterBroadcaster(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverSetAdvData001
 * @tc.desc: Cover SoftbusSetAdvData assemble failure branches and rsp assemble path
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverSetAdvData001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t advId = -1;
    int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advId, &g_coverSoftbusBcBleCbTest);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftbusBroadcastParam params = {};
    SoftbusBroadcastData data = BuildBcData();
    EXPECT_CALL(mocker, BleStartAdvEx).WillRepeatedly(MockBluetooth::ActionBleStartAdvEx);
    ret = MockBluetooth::interface->StartBroadcasting(advId, &params, &data);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // rsp assemble success path
    ret = MockBluetooth::interface->SetBroadcastingData(advId, &data);
    EXPECT_EQ(ret, SOFTBUS_OK);
    // AssembleAdvData calloc failure
    SetBleGattCallocFailCountdown(0);
    ret = MockBluetooth::interface->SetBroadcastingData(advId, &data);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_ASSEMBLE_FAIL);
    // AssembleRspData calloc failure
    SetBleGattCallocFailCountdown(1);
    ret = MockBluetooth::interface->SetBroadcastingData(advId, &data);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_ASSEMBLE_FAIL);

    (void)MockBluetooth::interface->StopBroadcasting(advId);
    ret = MockBluetooth::interface->UnRegisterBroadcaster(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverSetAdvParam001
 * @tc.desc: Cover SoftbusSetAdvParamter not-disabled branch and success path
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverSetAdvParam001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t advId = -1;
    int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advId, &g_coverSoftbusBcBleCbTest);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftbusBroadcastParam params = {};
    SoftbusBroadcastData data = BuildBcData();
    EXPECT_CALL(mocker, BleStartAdvEx).WillRepeatedly(MockBluetooth::ActionBleStartAdvEx);
    ret = MockBluetooth::interface->StartBroadcasting(advId, &params, &data);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // adv is not disabled
    ret = MockBluetooth::interface->SetBroadcastingParam(advId, &params);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_DISABLED_FAIL);

    SetBleAdvExReturnValue(OHOS_BT_STATUS_SUCCESS, OHOS_BT_STATUS_SUCCESS);
    ret = MockBluetooth::interface->DisableBroadcasting(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->SetBroadcastingParam(advId, &params);
    EXPECT_EQ(ret, SOFTBUS_OK);

    (void)MockBluetooth::interface->StopBroadcasting(advId);
    ret = MockBluetooth::interface->UnRegisterBroadcaster(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverEnableDisableAdv001
 * @tc.desc: Cover enable/disable broadcasting already-state and adapter failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverEnableDisableAdv001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t advId = -1;
    int32_t ret = MockBluetooth::interface->RegisterBroadcaster(&advId, &g_coverSoftbusBcBleCbTest);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftbusBroadcastParam params = {};
    SoftbusBroadcastData data = BuildBcData();
    EXPECT_CALL(mocker, BleStartAdvEx).WillRepeatedly(MockBluetooth::ActionBleStartAdvEx);
    ret = MockBluetooth::interface->StartBroadcasting(advId, &params, &data);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // already enabled
    ret = MockBluetooth::interface->EnableBroadcasting(advId);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_ENABLED_FAIL);

    // disable success, then already disabled
    SetBleAdvExReturnValue(OHOS_BT_STATUS_SUCCESS, OHOS_BT_STATUS_SUCCESS);
    ret = MockBluetooth::interface->DisableBroadcasting(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DisableBroadcasting(advId);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_DISABLED_FAIL);

    // enable adapter failure, then enable success
    SetBleAdvExReturnValue(OHOS_BT_STATUS_FAIL, OHOS_BT_STATUS_SUCCESS);
    ret = MockBluetooth::interface->EnableBroadcasting(advId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    // failed enable left isDisabled false, disable again to restore it
    SetBleAdvExReturnValue(OHOS_BT_STATUS_SUCCESS, OHOS_BT_STATUS_SUCCESS);
    ret = MockBluetooth::interface->DisableBroadcasting(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->EnableBroadcasting(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // disable adapter failure
    SetBleAdvExReturnValue(OHOS_BT_STATUS_SUCCESS, OHOS_BT_STATUS_FAIL);
    ret = MockBluetooth::interface->DisableBroadcasting(advId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);

    (void)MockBluetooth::interface->StopBroadcasting(advId);
    ret = MockBluetooth::interface->UnRegisterBroadcaster(advId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanChannelCheck001
 * @tc.desc: Cover CheckScanChannelInUsed not-used branch
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanChannelCheck001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t ret = MockBluetooth::interface->StopScan(BLE_GATT_SCAN_CH_NUM - 1);
    EXPECT_EQ(ret, SOFTBUS_BC_ADAPTER_NOT_IN_USED_FAIL);

    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverSetScanParams001
 * @tc.desc: Cover SoftbusSetScanParams filter/no-filter, malloc failure and adapter failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverSetScanParams001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusBcScanParams scanParam = {
        .scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10,
        .scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10,
        .scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE,
        .scanPhy = SOFTBUS_BC_SCAN_PHY_1M,
        .scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL,
    };

    EXPECT_CALL(mocker, BleChangeScanParams).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ret = MockBluetooth::interface->SetScanParams(scannerId, &scanParam, filter, 1, SOFTBUS_SCAN_FILTER_CMD_ADD);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // no filter branch
    ret = MockBluetooth::interface->SetScanParams(scannerId, &scanParam, nullptr, 0, SOFTBUS_SCAN_FILTER_CMD_NONE);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // adapter failure branch
    EXPECT_CALL(mocker, BleChangeScanParams).WillRepeatedly(Return(OHOS_BT_STATUS_FAIL));
    ret = MockBluetooth::interface->SetScanParams(scannerId, &scanParam, filter, 1, SOFTBUS_SCAN_FILTER_CMD_MODIFY);
    EXPECT_EQ(ret, SOFTBUS_DISCOVER_BLE_SET_SCAN_PARAM_FAIL);

    // native filter calloc failure branch
    SetBleGattCallocFailCountdown(0);
    ret = MockBluetooth::interface->SetScanParams(scannerId, &scanParam, filter, 1, SOFTBUS_SCAN_FILTER_CMD_ADD);
    EXPECT_EQ(ret, SOFTBUS_DISCOVER_BLE_END_SCAN_FAIL);

    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverStartScan001
 * @tc.desc: Cover SoftbusStartScan already-scanning and malloc failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverStartScan001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusBcScanParams scanParam = {
        .scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10,
        .scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10,
        .scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE,
        .scanPhy = SOFTBUS_BC_SCAN_PHY_1M,
        .scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL,
    };
    EXPECT_CALL(mocker, BleStartScanEx).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ret = MockBluetooth::interface->StartScan(scannerId, &scanParam, filter, 1);
    EXPECT_EQ(ret, SOFTBUS_OK);
    // already scanning
    ret = MockBluetooth::interface->StartScan(scannerId, &scanParam, filter, 1);
    EXPECT_EQ(ret, SOFTBUS_ALREADY_TRIGGERED);
    EXPECT_CALL(mocker, BleStopScan).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    (void)MockBluetooth::interface->StopScan(scannerId);

    // native filter calloc failure
    SetBleGattCallocFailCountdown(0);
    ret = MockBluetooth::interface->StartScan(scannerId, &scanParam, filter, 1);
    EXPECT_EQ(ret, SOFTBUS_MALLOC_ERR);
    (void)MockBluetooth::interface->StopScan(scannerId);

    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverStopScan001
 * @tc.desc: Cover SoftbusStopScan already-stopped and adapter failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverStopScan001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t scannerId = -1;
    int32_t ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);

    // already stopped
    ret = MockBluetooth::interface->StopScan(scannerId);
    EXPECT_EQ(ret, SOFTBUS_ALREADY_TRIGGERED);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusBcScanParams scanParam = {
        .scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10,
        .scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10,
        .scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE,
        .scanPhy = SOFTBUS_BC_SCAN_PHY_1M,
        .scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL,
    };
    EXPECT_CALL(mocker, BleStartScanEx).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ret = MockBluetooth::interface->StartScan(scannerId, &scanParam, filter, 1);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // adapter failure
    EXPECT_CALL(mocker, BleStopScan).WillRepeatedly(Return(OHOS_BT_STATUS_FAIL));
    ret = MockBluetooth::interface->StopScan(scannerId);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    EXPECT_CALL(mocker, BleStopScan).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    (void)MockBluetooth::interface->StopScan(scannerId);

    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverIsLpAvailable001
 * @tc.desc: Cover IsLpAvailable false branch
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverIsLpAvailable001, TestSize.Level1)
{
    MockBluetooth mocker;
    int32_t ret = MockBluetooth::interface->Init();
    EXPECT_EQ(ret, SOFTBUS_OK);

    SetIsLpAvailable(false);
    EXPECT_FALSE(MockBluetooth::interface->IsLpDeviceAvailable());
    SetIsLpAvailable(true);
    EXPECT_TRUE(MockBluetooth::interface->IsLpDeviceAvailable());

    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverSetLpParam001
 * @tc.desc: Cover SoftbusSetLpParam uuid mapping, assemble failure and manufacture filter branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverSetLpParam001, TestSize.Level1)
{
    MockBluetooth mocker;
    int32_t ret = MockBluetooth::interface->Init();
    EXPECT_EQ(ret, SOFTBUS_OK);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusLpScanParam scanParam = {};
    scanParam.filterSize = 1;
    scanParam.filter = filter;
    scanParam.scanParam.scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10;
    scanParam.scanParam.scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10;

    SoftBusLpBroadcastParam bcParam = {};
    bcParam.advHandle = 1;
    bcParam.advData = BuildBcData();

    // heartbeat type: uuid mapping and manufacture filter branches, AssembleAdvDataWithRsp success
    (void)MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_HEARTBEAT_TYPE, &bcParam, &scanParam);

    // AssembleRspData calloc failure
    SetBleGattCallocFailCountdown(1);
    EXPECT_FALSE(MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_HEARTBEAT_TYPE, &bcParam, &scanParam));

    // filter calloc failure (no rsp data)
    SoftBusLpBroadcastParam bcParamNoRsp = {};
    bcParamNoRsp.advHandle = 1;
    bcParamNoRsp.advData = BuildBcData();
    bcParamNoRsp.advData.rspData.payload = nullptr;
    bcParamNoRsp.advData.rspData.payloadLen = 0;
    SetBleGattCallocFailCountdown(1);
    EXPECT_FALSE(MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_HEARTBEAT_TYPE, &bcParamNoRsp, &scanParam));

    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverScanLpParam001
 * @tc.desc: Cover SoftbusSetScanLpParam filterSize/uuid checks and malloc failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverScanLpParam001, TestSize.Level1)
{
    MockBluetooth mocker;
    int32_t ret = MockBluetooth::interface->Init();
    EXPECT_EQ(ret, SOFTBUS_OK);

    auto filter = CreateScanFilter();
    ASSERT_NE(filter, nullptr);
    SoftBusLpBroadcastParam bcParam = {};
    bcParam.advHandle = -1;
    SoftBusLpScanParam scanParam = {};
    scanParam.filterSize = 1;
    scanParam.filter = filter;
    scanParam.scanParam.scanInterval = SOFTBUS_BC_SCAN_WINDOW_P10;
    scanParam.scanParam.scanWindow = SOFTBUS_BC_SCAN_INTERVAL_P10;

    // perception type uuid mapping
    (void)MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_PERCEPTION_TYPE, &bcParam, &scanParam);

    // invalid filterSize
    scanParam.filterSize = 17;
    EXPECT_FALSE(MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_PERCEPTION_TYPE, &bcParam, &scanParam));
    scanParam.filterSize = 1;

    // invalid server type
    EXPECT_FALSE(MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_UNKNOW_TYPE, &bcParam, &scanParam));

    // filter calloc failure
    SetBleGattCallocFailCountdown(0);
    EXPECT_FALSE(MockBluetooth::interface->SetAdvFilterParam(SOFTBUS_PERCEPTION_TYPE, &bcParam, &scanParam));

    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BleGattCoverSyncLp001
 * @tc.desc: Cover sync-data/scan-report-channel/lp-adv-param adapter failure branches
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(SoftbusBleGattCoverTest, BleGattCoverSyncLp001, TestSize.Level1)
{
    MockBluetooth mocker;
    ResetBleGattChannels();

    int32_t bcHandle = 0;
    (void)MockBluetooth::interface->GetBroadcastHandle(0, &bcHandle);

    // sync data adapter failure
    SetSyncDataToLpReturnValue(OHOS_BT_STATUS_FAIL, OHOS_BT_STATUS_FAIL);
    int32_t ret = MockBluetooth::interface->EnableSyncDataToLpDevice();
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    ret = MockBluetooth::interface->DisableSyncDataToLpDevice();
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    SetSyncDataToLpReturnValue(OHOS_BT_STATUS_SUCCESS, OHOS_BT_STATUS_SUCCESS);
    EXPECT_EQ(MockBluetooth::interface->EnableSyncDataToLpDevice(), SOFTBUS_OK);
    EXPECT_EQ(MockBluetooth::interface->DisableSyncDataToLpDevice(), SOFTBUS_OK);

    // scan report channel adapter failure and success
    int32_t scannerId = -1;
    ret = MockBluetooth::interface->RegisterScanListener(&scannerId, GetCoverStubScanListener());
    EXPECT_EQ(ret, SOFTBUS_OK);
    SetScanReportChannelReturnValue(OHOS_BT_STATUS_FAIL);
    ret = MockBluetooth::interface->SetScanReportChannelToLpDevice(scannerId, true);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    SetScanReportChannelReturnValue(OHOS_BT_STATUS_SUCCESS);
    ret = MockBluetooth::interface->SetScanReportChannelToLpDevice(scannerId, true);
    EXPECT_EQ(ret, SOFTBUS_OK);

    // lp adv param adapter failure and success
    SetLpDeviceAdvParamReturnValue(OHOS_BT_STATUS_FAIL);
    ret = MockBluetooth::interface->SetLpDeviceParam(0, 0, 0, 0, 0);
    EXPECT_EQ(ret, OHOS_BT_STATUS_FAIL);
    SetLpDeviceAdvParamReturnValue(OHOS_BT_STATUS_SUCCESS);
    ret = MockBluetooth::interface->SetLpDeviceParam(0, 0, 0, 0, 0);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = MockBluetooth::interface->UnRegisterScanListener(scannerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = MockBluetooth::interface->DeInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
}
} // namespace OHOS
