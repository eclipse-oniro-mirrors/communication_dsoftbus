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

#include <gtest/gtest.h>
#include <securec.h>
#include <stdatomic.h>

#include <memory>

#include "perception_ext_deps_mock.h"
#include "perception_scanner.c"
#include "softbus_adapter_mem.h"
#include "softbus_error_code.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace {
constexpr int32_t TEST_LISTENER_ID = 2;

static void FakePostMessageDelay(const SoftBusLooper *, SoftBusMessage *, uint64_t) { }
static void FakeRemoveMessage(const SoftBusLooper *, const SoftBusHandler *, int32_t) { }

static SoftBusLooper *GetFakeLooper()
{
    static SoftBusLooper looper;
    looper.context = nullptr;
    looper.queue = nullptr;
    looper.dumpable = false;
    looper.PostMessage = nullptr;
    looper.PostMessageDelay = FakePostMessageDelay;
    looper.RemoveMessage = FakeRemoveMessage;
    looper.RemoveMessageCustom = nullptr;
    return &looper;
}
} // namespace

class PerceptionScanMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<PerceptionExtDepsMock> mock_;
    static void InstallDefaults(PerceptionExtDepsMock &m);
    void ResetScanRuntimeState();
};

void PerceptionScanMockTest::SetUpTestSuite()
{
    SetLooper(LOOP_TYPE_DEFAULT, GetFakeLooper());
}

void PerceptionScanMockTest::TearDownTestSuite()
{
    SetLooper(LOOP_TYPE_DEFAULT, nullptr);
}

void PerceptionScanMockTest::SetUp()
{
    mock_ = std::make_unique<PerceptionExtDepsMock>();
    InstallDefaults(*mock_);
    atomic_store(&g_scanInited, false);
    ASSERT_EQ(PerceptionScannerInit(), SOFTBUS_OK);
    ResetScanRuntimeState();
}

void PerceptionScanMockTest::TearDown()
{
    PerceptionScannerDeinit();
    mock_.reset();
}

void PerceptionScanMockTest::InstallDefaults(PerceptionExtDepsMock &m)
{
    ON_CALL(m, SchedulerRegisterScanListener)
        .WillByDefault([](BroadcastProtocol, BaseServiceType, int32_t *listenerId, const ScanCallback *) {
            *listenerId = TEST_LISTENER_ID;
            return SOFTBUS_OK;
        });
    ON_CALL(m, SchedulerUnregisterListener).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerStartScan).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerStopScan).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerSetScanFilter).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, PerceptionBuildScanFilterPacked).WillByDefault([](BcScanFilter **filter, uint8_t *filterNum) {
        *filter = static_cast<BcScanFilter *>(SoftBusCalloc(sizeof(BcScanFilter)));
        *filterNum = 1;
        return SOFTBUS_OK;
    });
    ON_CALL(m, PerceptionGetDeviceInfoListPacked).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, PerceptionSyncStatePacked).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, PerceptionSyncCyclePacked).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, BroadcastSetAdvDeviceParam).WillByDefault(Return(true));
    ON_CALL(m, BroadcastSetScanReportChannelToLpDevice).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, LnnAsyncCallbackDelayHelper)
        .WillByDefault([](SoftBusLooper *, LnnAsyncCallbackFunc, void *para, uint64_t) {
            SoftBusFree(para);
            return SOFTBUS_OK;
        });
}

void PerceptionScanMockTest::ResetScanRuntimeState()
{
    atomic_store(&g_scanCtx.isScanning, false);
    atomic_store(&g_isScreenOn, true);
    g_scanCtx.activeType = PERCEPTION_TYPE_BUTT;
}

/*
 * @tc.name: ScanInit_AlreadyInited_Test_001
 * @tc.desc: PerceptionScannerInit returns SOFTBUS_OK when already initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanInit_AlreadyInited_Test_001, TestSize.Level1)
{
    EXPECT_EQ(PerceptionScannerInit(), SOFTBUS_OK);
}

/*
 * @tc.name: ScanInit_RegisterFail_Test_001
 * @tc.desc: PerceptionScannerInit returns error when SchedulerRegisterScanListener fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanInit_RegisterFail_Test_001, TestSize.Level1)
{
    PerceptionScannerDeinit();
    EXPECT_CALL(*mock_, SchedulerRegisterScanListener)
        .WillOnce([](BroadcastProtocol, BaseServiceType, int32_t *, const ScanCallback *) {
            return SOFTBUS_ERR;
        });
    EXPECT_EQ(PerceptionScannerInit(), SOFTBUS_ERR);
}

/*
 * @tc.name: ScanDeinit_NotInited_Test_001
 * @tc.desc: PerceptionScannerDeinit is a no-op when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanDeinit_NotInited_Test_001, TestSize.Level1)
{
    PerceptionScannerDeinit();
    atomic_store(&g_scanInited, false);
    PerceptionScannerDeinit();
    ASSERT_EQ(PerceptionScannerInit(), SOFTBUS_OK);
}

/*
 * @tc.name: CycleToSeconds_Test_001
 * @tc.desc: PerceptionCycleToSeconds maps each cycle to the expected seconds value
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, CycleToSeconds_Test_001, TestSize.Level1)
{
    EXPECT_EQ(PerceptionCycleToSeconds(PERCEPTION_CYCLE_LOW), static_cast<uint32_t>(PERCEPTION_CYCLE_LOW_SEC));
    EXPECT_EQ(PerceptionCycleToSeconds(PERCEPTION_CYCLE_MEDIUM), static_cast<uint32_t>(PERCEPTION_CYCLE_MEDIUM_SEC));
    EXPECT_EQ(PerceptionCycleToSeconds(PERCEPTION_CYCLE_HIGH), static_cast<uint32_t>(PERCEPTION_CYCLE_HIGH_SEC));
    EXPECT_EQ(PerceptionCycleToSeconds(PERCEPTION_CYCLE_BUTT), 0u);
}

/*
 * @tc.name: ScanStart_InvalidType_Test_001
 * @tc.desc: PerceptionScanStart returns SOFTBUS_INVALID_PARAM for invalid type
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_InvalidType_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_BUTT, PERCEPTION_CYCLE_LOW), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(PerceptionScanStart(static_cast<PerceptionType>(-1), PERCEPTION_CYCLE_LOW), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: ScanStart_NotInited_Test_001
 * @tc.desc: PerceptionScanStart returns SOFTBUS_NO_INIT when scanner is not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanInited, false);
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_NO_INIT);
}

/*
 * @tc.name: ScanStart_AlreadyScanning_Test_001
 * @tc.desc: PerceptionScanStart returns SOFTBUS_OK when already scanning
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_AlreadyScanning_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanCtx.isScanning, true);
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_OK);
}

/*
 * @tc.name: ScanStart_FilterBuildFail_Test_001
 * @tc.desc: PerceptionScanStart fails when PerceptionBuildScanFilterPacked fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_FilterBuildFail_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, PerceptionBuildScanFilterPacked).WillOnce([](BcScanFilter **filter, uint8_t *filterNum) {
        *filter = nullptr;
        *filterNum = 0;
        return SOFTBUS_ERR;
    });
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_ERR);
}

/*
 * @tc.name: ScanStart_FilterInvalid_Test_001
 * @tc.desc: PerceptionScanStart fails when scan filter is null or has invalid count
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_FilterInvalid_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, PerceptionBuildScanFilterPacked).WillOnce([](BcScanFilter **filter, uint8_t *filterNum) {
        *filter = nullptr;
        *filterNum = 1;
        return SOFTBUS_OK;
    });
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_INVALID_PARAM);

    EXPECT_CALL(*mock_, PerceptionBuildScanFilterPacked).WillOnce([](BcScanFilter **filter, uint8_t *filterNum) {
        *filter = static_cast<BcScanFilter *>(SoftBusCalloc(sizeof(BcScanFilter)));
        *filterNum = 0;
        return SOFTBUS_OK;
    });
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_INVALID_PARAM);

    EXPECT_CALL(*mock_, PerceptionBuildScanFilterPacked).WillOnce([](BcScanFilter **filter, uint8_t *filterNum) {
        *filter = static_cast<BcScanFilter *>(SoftBusCalloc(sizeof(BcScanFilter) * 3));
        *filterNum = 3;
        return SOFTBUS_OK;
    });
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: ScanStart_SetScanFilterFail_Test_001
 * @tc.desc: PerceptionScanStart fails when SchedulerSetScanFilter fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_SetScanFilterFail_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerSetScanFilter).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_ERR);
}

/*
 * @tc.name: ScanStart_StartScanFail_Test_001
 * @tc.desc: PerceptionScanStart fails when SchedulerStartScan fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_StartScanFail_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerStartScan).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_LOW), SOFTBUS_ERR);
}

/*
 * @tc.name: ScanStart_Success_Test_001
 * @tc.desc: PerceptionScanStart succeeds end to end
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStart_Success_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerStartScan(TEST_LISTENER_ID, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, PerceptionSyncStatePacked(PERCEPTION_STATE_START)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionScanStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, PERCEPTION_CYCLE_MEDIUM), SOFTBUS_OK);
    EXPECT_EQ(g_scanCtx.activeType, PERCEPTION_TYPE_COLLABORATIVE_WAKE);
}

/*
 * @tc.name: ScanStop_NotInited_Test_001
 * @tc.desc: PerceptionScanStop returns SOFTBUS_NO_INIT when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, ScanStop_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanInited, false);
    EXPECT_EQ(PerceptionScanStop(), SOFTBUS_NO_INIT);
}

/*
 * @tc.name: GetDeviceList_InvalidParam_Test_001
 * @tc.desc: PerceptionScanGetDeviceList returns SOFTBUS_INVALID_PARAM for null out params
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, GetDeviceList_InvalidParam_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, PerceptionGetDeviceInfoListPacked).Times(0);
    EXPECT_EQ(PerceptionScanGetDeviceList(PERCEPTION_TYPE_COLLABORATIVE_WAKE, nullptr, nullptr), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: GetDeviceList_NotScanning_Test_001
 * @tc.desc: PerceptionScanGetDeviceList returns SOFTBUS_PERCEPTION_SCAN_NOT_START when not scanning
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, GetDeviceList_NotScanning_Test_001, TestSize.Level1)
{
    PerceptionDeviceInfo *list = nullptr;
    uint32_t count = 0;
    EXPECT_EQ(PerceptionScanGetDeviceList(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &list, &count),
        SOFTBUS_PERCEPTION_SCAN_NOT_START);
}

/*
 * @tc.name: GetDeviceList_Success_Test_001
 * @tc.desc: PerceptionScanGetDeviceList delegates to PerceptionGetDeviceInfoListPacked when scanning
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, GetDeviceList_Success_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanCtx.isScanning, true);
    PerceptionDeviceInfo *list = nullptr;
    uint32_t count = 0;
    EXPECT_CALL(*mock_, PerceptionGetDeviceInfoListPacked(PERCEPTION_TYPE_COLLABORATIVE_WAKE, _, _))
        .WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionScanGetDeviceList(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &list, &count), SOFTBUS_OK);
}

/*
 * @tc.name: OnScreenState_NotScanning_Test_001
 * @tc.desc: PerceptionScanOnScreenStateChanged only updates screen flag when not scanning
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, OnScreenState_NotScanning_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionScanOnScreenStateChanged(false));
    EXPECT_FALSE(atomic_load(&g_isScreenOn));
}

/*
 * @tc.name: OnBtState_NotInited_Test_001
 * @tc.desc: PerceptionScanOnBtStateChanged is a no-op when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, OnBtState_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanInited, false);
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionScanOnBtStateChanged(true));
}

/*
 * @tc.name: OnBtState_BtOff_Test_001
 * @tc.desc: PerceptionScanOnBtStateChanged clears scanning flag when bt turns off
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, OnBtState_BtOff_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanCtx.isScanning, true);
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionScanOnBtStateChanged(false));
    EXPECT_FALSE(atomic_load(&g_scanCtx.isScanning));
}

/*
 * @tc.name: OnBtState_BtOnNoActiveType_Test_001
 * @tc.desc: PerceptionScanOnBtStateChanged does nothing when bt turns on without active type
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, OnBtState_BtOnNoActiveType_Test_001, TestSize.Level1)
{
    g_scanCtx.activeType = PERCEPTION_TYPE_BUTT;
    EXPECT_CALL(*mock_, SchedulerStartScan).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionScanOnBtStateChanged(true));
}

/*
 * @tc.name: OnBtState_BtOnRecover_Test_001
 * @tc.desc: PerceptionScanOnBtStateChanged recovers scan when bt turns on with active type
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, OnBtState_BtOnRecover_Test_001, TestSize.Level1)
{
    g_scanCtx.activeType = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    EXPECT_CALL(*mock_, SchedulerStartScan(TEST_LISTENER_ID, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(PerceptionScanOnBtStateChanged(true));
}

/*
 * @tc.name: OnBtState_BtOnRecoverFail_Test_001
 * @tc.desc: PerceptionScanOnBtStateChanged recover path tolerates SchedulerStartScan failure
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, OnBtState_BtOnRecoverFail_Test_001, TestSize.Level1)
{
    g_scanCtx.activeType = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    EXPECT_CALL(*mock_, SchedulerStartScan).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_NO_FATAL_FAILURE(PerceptionScanOnBtStateChanged(true));
}

/*
 * @tc.name: MlpsState_Skip_Test_001
 * @tc.desc: PerceptionMlpsServicesStateChange skips when isStart is false or not scanning
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, MlpsState_Skip_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, LnnAsyncCallbackDelayHelper).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionMlpsServicesStateChange(false));
    EXPECT_NO_FATAL_FAILURE(PerceptionMlpsServicesStateChange(true));
}

/*
 * @tc.name: MlpsState_PostFail_Test_001
 * @tc.desc: PerceptionMlpsServicesStateChange frees para when LnnAsyncCallbackDelayHelper fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, MlpsState_PostFail_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanCtx.isScanning, true);
    EXPECT_CALL(*mock_, LnnAsyncCallbackDelayHelper).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_NO_FATAL_FAILURE(PerceptionMlpsServicesStateChange(true));
}

/*
 * @tc.name: MlpsState_Success_Test_001
 * @tc.desc: PerceptionMlpsServicesStateChange posts switch-to-SH callback when scanning
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, MlpsState_Success_Test_001, TestSize.Level1)
{
    atomic_store(&g_scanCtx.isScanning, true);
    EXPECT_CALL(*mock_, LnnAsyncCallbackDelayHelper(_, _, _, PERCEPTION_SCAN_TO_SH_DELAY_MS)).Times(1);
    EXPECT_NO_FATAL_FAILURE(PerceptionMlpsServicesStateChange(true));
}

/*
 * @tc.name: SwitchScanToSH_SetAdvFail_Test_001
 * @tc.desc: PerceptionSwitchScanToSH returns when BroadcastSetAdvDeviceParam fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, SwitchScanToSH_SetAdvFail_Test_001, TestSize.Level1)
{
    int32_t *para = static_cast<int32_t *>(SoftBusMalloc(sizeof(int32_t)));
    ASSERT_NE(para, nullptr);
    *para = TEST_LISTENER_ID;
    EXPECT_CALL(*mock_, BroadcastSetAdvDeviceParam).WillOnce(Return(false));
    EXPECT_CALL(*mock_, BroadcastSetScanReportChannelToLpDevice).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionSwitchScanToSH(para));
}

/*
 * @tc.name: SwitchScanToSH_SetChannelFail_Test_001
 * @tc.desc: PerceptionSwitchScanToSH returns when BroadcastSetScanReportChannelToLpDevice fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, SwitchScanToSH_SetChannelFail_Test_001, TestSize.Level1)
{
    int32_t *para = static_cast<int32_t *>(SoftBusMalloc(sizeof(int32_t)));
    ASSERT_NE(para, nullptr);
    *para = TEST_LISTENER_ID;
    EXPECT_CALL(*mock_, BroadcastSetAdvDeviceParam).WillOnce(Return(true));
    EXPECT_CALL(*mock_, BroadcastSetScanReportChannelToLpDevice(TEST_LISTENER_ID, true)).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_NO_FATAL_FAILURE(PerceptionSwitchScanToSH(para));
}

/*
 * @tc.name: SwitchScanToSH_Success_Test_001
 * @tc.desc: PerceptionSwitchScanToSH succeeds end to end
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, SwitchScanToSH_Success_Test_001, TestSize.Level1)
{
    int32_t *para = static_cast<int32_t *>(SoftBusMalloc(sizeof(int32_t)));
    ASSERT_NE(para, nullptr);
    *para = TEST_LISTENER_ID;
    EXPECT_CALL(*mock_, BroadcastSetAdvDeviceParam).WillOnce(Return(true));
    EXPECT_CALL(*mock_, BroadcastSetScanReportChannelToLpDevice).WillOnce(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(PerceptionSwitchScanToSH(para));
}

/*
 * @tc.name: StartScanCallback_Test_001
 * @tc.desc: Verify OnStartScanCallback mismatch/fail/success and OnStopScanCallback paths
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, StartScanCallback_Test_001, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStartScanCallback(TEST_LISTENER_ID + 1, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_FALSE(atomic_load(&g_scanCtx.isScanning));

    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStartScanCallback(TEST_LISTENER_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_FAIL)));
    EXPECT_FALSE(atomic_load(&g_scanCtx.isScanning));

    EXPECT_CALL(*mock_, LnnAsyncCallbackDelayHelper(_, _, _, PERCEPTION_SCAN_POST_START_DELAY_MS)).Times(1);
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStartScanCallback(TEST_LISTENER_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_TRUE(atomic_load(&g_scanCtx.isScanning));

    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStopScanCallback(TEST_LISTENER_ID + 1, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStopScanCallback(TEST_LISTENER_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_FALSE(atomic_load(&g_scanCtx.isScanning));
}

/*
 * @tc.name: StartScanCallback_PostFail_Test_001
 * @tc.desc: OnStartScanCallback success path frees para when LnnAsyncCallbackDelayHelper fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, StartScanCallback_PostFail_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, LnnAsyncCallbackDelayHelper).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStartScanCallback(TEST_LISTENER_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_TRUE(atomic_load(&g_scanCtx.isScanning));
}

/*
 * @tc.name: FreeFilterArray_Test_001
 * @tc.desc: FreePerceptionScanFilterArray is safe with null input and frees allocated fields
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionScanMockTest, FreeFilterArray_Test_001, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(FreePerceptionScanFilterArray(nullptr, 0));
    BcScanFilter *filter = static_cast<BcScanFilter *>(SoftBusCalloc(sizeof(BcScanFilter)));
    ASSERT_NE(filter, nullptr);
    filter[0].address = static_cast<int8_t *>(SoftBusMalloc(4));
    filter[0].serviceData = static_cast<uint8_t *>(SoftBusMalloc(4));
    EXPECT_NO_FATAL_FAILURE(FreePerceptionScanFilterArray(filter, 1));
}
