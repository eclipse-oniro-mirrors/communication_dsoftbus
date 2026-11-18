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

#include "perception_advertiser.c"
#include "perception_ext_deps_mock.h"
#include "softbus_error_code.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;

namespace {
constexpr int32_t TEST_BC_ID = 1;

static void FakePostMessageDelay(const SoftBusLooper *, SoftBusMessage *msg, uint64_t)
{
    FreeMessage(msg);
}
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

static PerceptionAdvParam MakeParam(uint32_t len)
{
    PerceptionAdvParam p;
    (void)memset_s(&p, sizeof(p), 0, sizeof(p));
    p.customDataLen = len;
    return p;
}
} // namespace

class PerceptionAdvMockTest : public testing::Test {
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();
    void SetUp() override;
    void TearDown() override;

protected:
    std::unique_ptr<PerceptionExtDepsMock> mock_;
    static void InstallDefaults(PerceptionExtDepsMock &m);
    void ResetAdvRuntimeState();
};

void PerceptionAdvMockTest::SetUpTestSuite()
{
    SetLooper(LOOP_TYPE_DEFAULT, GetFakeLooper());
}

void PerceptionAdvMockTest::TearDownTestSuite()
{
    SetLooper(LOOP_TYPE_DEFAULT, nullptr);
}

void PerceptionAdvMockTest::SetUp()
{
    mock_ = std::make_unique<PerceptionExtDepsMock>();
    InstallDefaults(*mock_);
    atomic_store(&g_advInited, false);
    ASSERT_EQ(PerceptionAdvertiserInit(), SOFTBUS_OK);
    ResetAdvRuntimeState();
}

void PerceptionAdvMockTest::TearDown()
{
    PerceptionAdvertiserDeinit();
    mock_.reset();
}

void PerceptionAdvMockTest::InstallDefaults(PerceptionExtDepsMock &m)
{
    ON_CALL(m, SchedulerRegisterBroadcaster)
        .WillByDefault([](BroadcastProtocol, BaseServiceType, int32_t *bcId, const BroadcastCallback *) {
            *bcId = TEST_BC_ID;
            return SOFTBUS_OK;
        });
    ON_CALL(m, SchedulerUnregisterBroadcaster).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerStartBroadcast).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerSetBroadcastParam).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerSetBroadcastData).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, SchedulerStopBroadcast).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(m, PerceptionBuildAdvDataPacked)
        .WillByDefault([](const PerceptionType *, const PerceptionAdvParam *, uint32_t, BroadcastPacket *) {
            return SOFTBUS_OK;
        });
}

void PerceptionAdvMockTest::ResetAdvRuntimeState()
{
    atomic_store(&g_advCtx.isAdvertising, false);
    g_advCtx.currentType = PERCEPTION_TYPE_BUTT;
    (void)memset_s(&g_advCtx.currentParam, sizeof(g_advCtx.currentParam), 0, sizeof(g_advCtx.currentParam));
}

/*
 * @tc.name: Init_AlreadyInited_Test_001
 * @tc.desc: PerceptionAdvertiserInit returns SOFTBUS_OK when already initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, Init_AlreadyInited_Test_001, TestSize.Level1)
{
    EXPECT_EQ(PerceptionAdvertiserInit(), SOFTBUS_OK);
}

/*
 * @tc.name: Init_LooperUnavailable_Test_001
 * @tc.desc: PerceptionAdvertiserInit returns SOFTBUS_LOOPER_ERR when default looper is unavailable
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, Init_LooperUnavailable_Test_001, TestSize.Level1)
{
    PerceptionAdvertiserDeinit();
    SetLooper(LOOP_TYPE_DEFAULT, nullptr);
    EXPECT_CALL(*mock_, SchedulerRegisterBroadcaster).Times(0);
    EXPECT_EQ(PerceptionAdvertiserInit(), SOFTBUS_LOOPER_ERR);
    SetLooper(LOOP_TYPE_DEFAULT, GetFakeLooper());
}

/*
 * @tc.name: Init_RegisterFail_Test_001
 * @tc.desc: PerceptionAdvertiserInit returns error when SchedulerRegisterBroadcaster fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, Init_RegisterFail_Test_001, TestSize.Level1)
{
    PerceptionAdvertiserDeinit();
    EXPECT_CALL(*mock_, SchedulerRegisterBroadcaster)
        .WillOnce([](BroadcastProtocol, BaseServiceType, int32_t *, const BroadcastCallback *) {
            return SOFTBUS_ERR;
        });
    EXPECT_EQ(PerceptionAdvertiserInit(), SOFTBUS_ERR);
}

/*
 * @tc.name: Deinit_NotInited_Test_001
 * @tc.desc: PerceptionAdvertiserDeinit is a no-op when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, Deinit_NotInited_Test_001, TestSize.Level1)
{
    PerceptionAdvertiserDeinit();
    atomic_store(&g_advInited, false);
    PerceptionAdvertiserDeinit();
    ASSERT_EQ(PerceptionAdvertiserInit(), SOFTBUS_OK);
}

/*
 * @tc.name: AdvStart_InvalidParam_Test_001
 * @tc.desc: PerceptionAdvStart returns SOFTBUS_INVALID_PARAM for null param and invalid type
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_InvalidParam_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).Times(0);
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, nullptr), SOFTBUS_INVALID_PARAM);
    auto param = MakeParam(0);
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_BUTT, &param), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(PerceptionAdvStart(static_cast<PerceptionType>(-1), &param), SOFTBUS_INVALID_PARAM);
    auto big = MakeParam(PERCEPTION_CUSTOM_DATA_MAX_LEN + 1);
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &big), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: AdvStart_NotInited_Test_001
 * @tc.desc: PerceptionAdvStart returns SOFTBUS_NO_INIT when advertiser is not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_advInited, false);
    auto param = MakeParam(0);
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_NO_INIT);
}

/*
 * @tc.name: AdvStart_Success_Test_001
 * @tc.desc: PerceptionAdvStart starts broadcast when not advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_Success_Test_001, TestSize.Level1)
{
    auto param = MakeParam(3);
    param.customData[0] = 0xAB;
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerStartBroadcast(TEST_BC_ID, BC_TYPE_PERCEPTION, _, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_OK);
    EXPECT_EQ(g_advCtx.currentType, PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    EXPECT_EQ(g_advCtx.currentParam.customDataLen, 3u);
    EXPECT_EQ(g_advCtx.currentParam.customData[0], 0xAB);
}

/*
 * @tc.name: AdvStart_BuildPacketFail_Test_001
 * @tc.desc: PerceptionAdvStart fails when PerceptionBuildAdvDataPacked fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_BuildPacketFail_Test_001, TestSize.Level1)
{
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).Times(0);
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_ERR);
}

/*
 * @tc.name: AdvStart_StartBroadcastFail_Test_001
 * @tc.desc: PerceptionAdvStart fails when SchedulerStartBroadcast fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_StartBroadcastFail_Test_001, TestSize.Level1)
{
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_ERR);
}

/*
 * @tc.name: AdvStart_UpdateWhenAdvertising_Test_001
 * @tc.desc: PerceptionAdvStart updates data when already advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_UpdateWhenAdvertising_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    auto param = MakeParam(2);
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).Times(0);
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastData(TEST_BC_ID, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_OK);
}

/*
 * @tc.name: AdvStart_UpdateDataFail_Test_001
 * @tc.desc: PerceptionAdvStart update path returns error when SchedulerSetBroadcastData fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStart_UpdateDataFail_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastData).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_EQ(PerceptionAdvStart(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_ERR);
}

/*
 * @tc.name: SetHighFreq_NotAdvertising_Test_001
 * @tc.desc: PerceptionAdvSetHighFreq starts broadcast with high freq when not advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, SetHighFreq_NotAdvertising_Test_001, TestSize.Level1)
{
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastParam).Times(0);
    EXPECT_EQ(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_OK);
}

/*
 * @tc.name: SetHighFreq_Advertising_Test_001
 * @tc.desc: PerceptionAdvSetHighFreq updates data and switches to high freq when advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, SetHighFreq_Advertising_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastData).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastParam(TEST_BC_ID, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_OK);
}

/*
 * @tc.name: SetHighFreq_UpdateFailSkipsSwitch_Test_001
 * @tc.desc: PerceptionAdvSetHighFreq skips SwitchAdvFreq when UpdateAdvDataLocked fails
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, SetHighFreq_UpdateFailSkipsSwitch_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastData).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(*mock_, SchedulerSetBroadcastParam).Times(0);
    EXPECT_NE(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_OK);
}

/*
 * @tc.name: SetHighFreq_InvalidParam_Test_001
 * @tc.desc: PerceptionAdvSetHighFreq returns SOFTBUS_INVALID_PARAM for invalid input
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, SetHighFreq_InvalidParam_Test_001, TestSize.Level1)
{
    EXPECT_EQ(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_COLLABORATIVE_WAKE, nullptr), SOFTBUS_INVALID_PARAM);
    auto param = MakeParam(0);
    EXPECT_EQ(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_BUTT, &param), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: SetHighFreq_NotInited_Test_001
 * @tc.desc: PerceptionAdvSetHighFreq returns SOFTBUS_NO_INIT when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, SetHighFreq_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_advInited, false);
    auto param = MakeParam(0);
    EXPECT_EQ(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_NO_INIT);
}

/*
 * @tc.name: AdvStop_NotAdvertising_Test_001
 * @tc.desc: PerceptionAdvStop returns SOFTBUS_OK when already stopped
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStop_NotAdvertising_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerStopBroadcast).Times(0);
    EXPECT_EQ(PerceptionAdvStop(), SOFTBUS_OK);
}

/*
 * @tc.name: AdvStop_NotInited_Test_001
 * @tc.desc: PerceptionAdvStop returns SOFTBUS_NO_INIT when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStop_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_advInited, false);
    EXPECT_EQ(PerceptionAdvStop(), SOFTBUS_NO_INIT);
}

/*
 * @tc.name: AdvStop_Success_Test_001
 * @tc.desc: PerceptionAdvStop stops broadcast and clears state when advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStop_Success_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    g_advCtx.currentType = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    EXPECT_CALL(*mock_, SchedulerStopBroadcast(TEST_BC_ID)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionAdvStop(), SOFTBUS_OK);
    EXPECT_FALSE(atomic_load(&g_advCtx.isAdvertising));
    EXPECT_EQ(g_advCtx.currentType, PERCEPTION_TYPE_BUTT);
}

/*
 * @tc.name: AdvStop_SchedulerFail_Test_001
 * @tc.desc: PerceptionAdvStop propagates SchedulerStopBroadcast failure but still clears state
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, AdvStop_SchedulerFail_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    EXPECT_CALL(*mock_, SchedulerStopBroadcast).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_EQ(PerceptionAdvStop(), SOFTBUS_ERR);
    EXPECT_FALSE(atomic_load(&g_advCtx.isAdvertising));
}

/*
 * @tc.name: OnBtStateChanged_NotInited_Test_001
 * @tc.desc: PerceptionAdvOnBtStateChanged is a no-op when not initialized
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, OnBtStateChanged_NotInited_Test_001, TestSize.Level1)
{
    atomic_store(&g_advInited, false);
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionAdvOnBtStateChanged(true));
}

/*
 * @tc.name: OnBtStateChanged_BtOff_Test_001
 * @tc.desc: PerceptionAdvOnBtStateChanged clears advertising flag when bt turns off
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, OnBtStateChanged_BtOff_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    g_advCtx.currentType = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionAdvOnBtStateChanged(false));
    EXPECT_FALSE(atomic_load(&g_advCtx.isAdvertising));
}

/*
 * @tc.name: OnBtStateChanged_BtOnRecover_Test_001
 * @tc.desc: PerceptionAdvOnBtStateChanged restarts broadcast when bt turns on with retained type
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, OnBtStateChanged_BtOnRecover_Test_001, TestSize.Level1)
{
    g_advCtx.currentType = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    EXPECT_CALL(*mock_, PerceptionBuildAdvDataPacked).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).WillOnce(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(PerceptionAdvOnBtStateChanged(true));
}

/*
 * @tc.name: OnBtStateChanged_BtOnNoType_Test_001
 * @tc.desc: PerceptionAdvOnBtStateChanged does nothing when bt turns on without retained type
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, OnBtStateChanged_BtOnNoType_Test_001, TestSize.Level1)
{
    g_advCtx.currentType = PERCEPTION_TYPE_BUTT;
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).Times(0);
    EXPECT_NO_FATAL_FAILURE(PerceptionAdvOnBtStateChanged(true));
}

/*
 * @tc.name: Callbacks_StartStopSet_Test_001
 * @tc.desc: Verify OnStartBroadcastingCallback/OnStopBroadcastingCallback/OnSetBroadcastingCallback behavior
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, Callbacks_StartStopSet_Test_001, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStartBroadcastingCallback(TEST_BC_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_FAIL)));
    EXPECT_FALSE(atomic_load(&g_advCtx.isAdvertising));
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStartBroadcastingCallback(TEST_BC_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_TRUE(atomic_load(&g_advCtx.isAdvertising));
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnStopBroadcastingCallback(TEST_BC_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
    EXPECT_FALSE(atomic_load(&g_advCtx.isAdvertising));
    EXPECT_NO_FATAL_FAILURE(
        PerceptionOnSetBroadcastingCallback(TEST_BC_ID, static_cast<int32_t>(SOFTBUS_BC_STATUS_SUCCESS)));
}

/*
 * @tc.name: HandleAdvMessage_Invalid_Test_001
 * @tc.desc: HandleAdvMessage ignores null and wrong-what messages
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, HandleAdvMessage_Invalid_Test_001, TestSize.Level1)
{
    EXPECT_CALL(*mock_, SchedulerSetBroadcastParam).Times(0);
    EXPECT_NO_FATAL_FAILURE(HandleAdvMessage(nullptr));
    SoftBusMessage msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    msg.what = 999;
    EXPECT_NO_FATAL_FAILURE(HandleAdvMessage(&msg));
}

/*
 * @tc.name: HandleAdvMessage_RecoverLow_Test_001
 * @tc.desc: HandleAdvMessage switches to low freq when advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, HandleAdvMessage_RecoverLow_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, true);
    SoftBusMessage msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    msg.what = PERCEPTION_ADV_MSG_RECOVER_LOW;
    EXPECT_CALL(*mock_, SchedulerSetBroadcastParam(TEST_BC_ID, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(HandleAdvMessage(&msg));
}

/*
 * @tc.name: HandleAdvMessage_RecoverLowNotAdvertising_Test_001
 * @tc.desc: HandleAdvMessage skips SwitchAdvFreq when not advertising
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, HandleAdvMessage_RecoverLowNotAdvertising_Test_001, TestSize.Level1)
{
    atomic_store(&g_advCtx.isAdvertising, false);
    SoftBusMessage msg;
    (void)memset_s(&msg, sizeof(msg), 0, sizeof(msg));
    msg.what = PERCEPTION_ADV_MSG_RECOVER_LOW;
    EXPECT_CALL(*mock_, SchedulerSetBroadcastParam).Times(0);
    EXPECT_NO_FATAL_FAILURE(HandleAdvMessage(&msg));
}

/*
 * @tc.name: PostRecoverLow_HandlerNotReady_Test_001
 * @tc.desc: PerceptionAdvSetHighFreq returns SOFTBUS_LOOPER_ERR path when handler looper is absent
 * @tc.type: FUNC
 * @tc.level: Level1
 */
HWTEST_F(PerceptionAdvMockTest, PostRecoverLow_HandlerNotReady_Test_001, TestSize.Level1)
{
    SoftBusLooper *saved = g_advHandler.looper;
    g_advHandler.looper = nullptr;
    auto param = MakeParam(0);
    EXPECT_CALL(*mock_, SchedulerStartBroadcast).WillOnce(Return(SOFTBUS_OK));
    EXPECT_EQ(PerceptionAdvSetHighFreq(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param), SOFTBUS_OK);
    g_advHandler.looper = saved;
}
