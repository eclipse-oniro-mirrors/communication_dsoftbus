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
#include <functional>
#include <gtest/gtest.h>

#include "lnn_lane_interface_struct.h"
#include "softbus_error_code.h"
#include "trans_channel_common.h"
#include "trans_channel_common_mock.h"
#include "wifi_direct_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace {
std::function<int32_t(int, int)> g_setEdcaParamHook;

int32_t MockSetEdcaParam(int scene, int level)
{
    if (g_setEdcaParamHook) {
        return g_setEdcaParamHook(scene, level);
    }
    return SOFTBUS_OK;
}

struct WifiDirectManager g_wifiDirectManager = {
    .setEdcaParam = MockSetEdcaParam,
};
} // namespace

extern "C" struct WifiDirectManager *GetWifiDirectManager(void)
{
    return &g_wifiDirectManager;
}

class TransChannelCommonTest : public testing::Test {
public:
    TransChannelCommonTest() { }
    ~TransChannelCommonTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override { g_setEdcaParamHook = nullptr; }
    void TearDown() override { g_setEdcaParamHook = nullptr; }
};

void TransChannelCommonTest::SetUpTestCase(void) { }

void TransChannelCommonTest::TearDownTestCase(void) { }

/*
 * @tc.name: TransSetWakeUpInfoTest001
 * @tc.desc: TransSetWakeUpInfo with TCP_DIRECT channel passes through TransTdcSetWakeUpInfo return value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetWakeUpInfoTest001, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 0;
    bool needFastWakeUp = true;
    EXPECT_CALL(mock, TransTdcSetWakeUpInfo).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransSetWakeUpInfo(CHANNEL_TYPE_TCP_DIRECT, channelId, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_CALL(mock, TransTdcSetWakeUpInfo).WillOnce(Return(SOFTBUS_ERR));
    ret = TransSetWakeUpInfo(CHANNEL_TYPE_TCP_DIRECT, channelId, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransSetWakeUpInfoTest002
 * @tc.desc: TransSetWakeUpInfo with UDP channel passes through TransUdpSetWakeUpInfo return value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetWakeUpInfoTest002, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 0;
    bool needFastWakeUp = true;
    EXPECT_CALL(mock, TransUdpSetWakeUpInfo).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransSetWakeUpInfo(CHANNEL_TYPE_UDP, channelId, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_CALL(mock, TransUdpSetWakeUpInfo).WillOnce(Return(SOFTBUS_ERR));
    ret = TransSetWakeUpInfo(CHANNEL_TYPE_UDP, channelId, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransSetWakeUpInfoTest003
 * @tc.desc: TransSetWakeUpInfo with unsupported channel type returns SOFTBUS_TRANS_FUNC_NOT_SUPPORT.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetWakeUpInfoTest003, TestSize.Level1)
{
    int32_t channelId = 0;
    bool needFastWakeUp = true;
    int32_t ret = TransSetWakeUpInfo(CHANNEL_TYPE_UNDEFINED, channelId, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_TRANS_FUNC_NOT_SUPPORT, ret);
    ret = TransSetWakeUpInfo(CHANNEL_TYPE_PROXY, channelId, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_TRANS_FUNC_NOT_SUPPORT, ret);
}

/*
 * @tc.name: TransGetWakeUpInfoTest001
 * @tc.desc: TransGetWakeUpInfo with TCP_DIRECT channel passes through TransTdcGetWakeUpInfo return value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransGetWakeUpInfoTest001, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 0;
    char *uuid = nullptr;
    int32_t uuidLen = 0;
    bool *needFastWakeUp = nullptr;
    EXPECT_CALL(mock, TransTdcGetWakeUpInfo).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransGetWakeUpInfo(CHANNEL_TYPE_TCP_DIRECT, channelId, uuid, uuidLen, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_CALL(mock, TransTdcGetWakeUpInfo).WillOnce(Return(SOFTBUS_ERR));
    ret = TransGetWakeUpInfo(CHANNEL_TYPE_TCP_DIRECT, channelId, uuid, uuidLen, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransGetWakeUpInfoTest002
 * @tc.desc: TransGetWakeUpInfo with UDP channel passes through TransUdpGetWakeUpInfo return value.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransGetWakeUpInfoTest002, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 0;
    char *uuid = nullptr;
    int32_t uuidLen = 0;
    bool *needFastWakeUp = nullptr;
    EXPECT_CALL(mock, TransUdpGetWakeUpInfo).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransGetWakeUpInfo(CHANNEL_TYPE_UDP, channelId, uuid, uuidLen, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_CALL(mock, TransUdpGetWakeUpInfo).WillOnce(Return(SOFTBUS_ERR));
    ret = TransGetWakeUpInfo(CHANNEL_TYPE_UDP, channelId, uuid, uuidLen, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransGetWakeUpInfoTest003
 * @tc.desc: TransGetWakeUpInfo with unsupported channel type returns SOFTBUS_TRANS_FUNC_NOT_SUPPORT.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransGetWakeUpInfoTest003, TestSize.Level1)
{
    int32_t channelId = 0;
    char *uuid = nullptr;
    int32_t uuidLen = 0;
    bool *needFastWakeUp = nullptr;
    int32_t ret = TransGetWakeUpInfo(CHANNEL_TYPE_UNDEFINED, channelId, uuid, uuidLen, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_TRANS_FUNC_NOT_SUPPORT, ret);
    ret = TransGetWakeUpInfo(CHANNEL_TYPE_PROXY, channelId, uuid, uuidLen, needFastWakeUp);
    EXPECT_EQ(SOFTBUS_TRANS_FUNC_NOT_SUPPORT, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest001
 * @tc.desc: invalid scene/level or unsupported channelType returns SOFTBUS_INVALID_PARAM /
 *           SOFTBUS_TRANS_INVALID_CHANNEL_TYPE.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT, -1, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT, EDCA_SCENE_BUTT, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT, EDCA_SCENE_DATA_CLONE, -1);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT, EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_BUTT);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_PROXY, EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_TYPE, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest002
 * @tc.desc: TCP_DIRECT channel not found returns SOFTBUS_NOT_FIND.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest002, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 100;
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    int32_t linkType = LANE_HML;
    EXPECT_CALL(mock, TransTdcGetChannelEdcaInfo(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(edca), SetArgPointee<2>(linkType), Return(SOFTBUS_ERR)));
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest003
 * @tc.desc: UDP channel not found returns SOFTBUS_NOT_FIND.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest003, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 100;
    EXPECT_CALL(mock, TransGetUdpChannelById(_, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_UDP,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest004
 * @tc.desc: TCP_DIRECT non-HML linkType returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest004, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 1;
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    int32_t linkType = LANE_WLAN_5G;
    EXPECT_CALL(mock, TransTdcGetChannelEdcaInfo(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(edca), SetArgPointee<2>(linkType), Return(SOFTBUS_OK)));
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest005
 * @tc.desc: TCP_DIRECT already DISABLE and level=DISABLE returns SOFTBUS_OK (idempotent), no update/driver calls.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest005, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 1;
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    int32_t linkType = LANE_HML;
    EXPECT_CALL(mock, TransTdcGetChannelEdcaInfo(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(edca), SetArgPointee<2>(linkType), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, TransTdcUpdateEdcaParam(_, _)).Times(0);
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_DISABLE);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest006
 * @tc.desc: TCP_DIRECT HML link, enable EDCA succeeds; TransUdpUpdateEdcaParam not called.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest006, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 1;
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    int32_t linkType = LANE_HML;
    EXPECT_CALL(mock, TransTdcGetChannelEdcaInfo(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(edca), SetArgPointee<2>(linkType), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, TransTdcUpdateEdcaParam(_, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, TransUdpUpdateEdcaParam(_, _)).Times(0);
    g_setEdcaParamHook = [](int scene, int level) { return SOFTBUS_OK; };
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest007
 * @tc.desc: UDP HML link, enable EDCA succeeds; TransTdcUpdateEdcaParam not called.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest007, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 2;
    UdpChannelInfo channel;
    (void)memset_s(&channel, sizeof(channel), 0, sizeof(channel));
    channel.info.linkType = LANE_HML;
    channel.info.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    channel.info.edcaParam.level = EDCA_LEVEL_DISABLE;
    EXPECT_CALL(mock, TransGetUdpChannelById(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(channel), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, TransUdpUpdateEdcaParam(_, _)).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, TransTdcUpdateEdcaParam(_, _)).Times(0);
    g_setEdcaParamHook = [](int scene, int level) { return SOFTBUS_OK; };
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_UDP,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_FULL);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest008
 * @tc.desc: TCP_DIRECT HML link, TransTdcUpdateEdcaParam fails; returns the error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest008, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 1;
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    int32_t linkType = LANE_HML;
    EXPECT_CALL(mock, TransTdcGetChannelEdcaInfo(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(edca), SetArgPointee<2>(linkType), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, TransTdcUpdateEdcaParam(_, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest009
 * @tc.desc: TCP_DIRECT HML link, driver setEdcaParam fails; returns the error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest009, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 1;
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    int32_t linkType = LANE_HML;
    EXPECT_CALL(mock, TransTdcGetChannelEdcaInfo(_, _, _))
        .WillOnce(DoAll(SetArgPointee<1>(edca), SetArgPointee<2>(linkType), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, TransTdcUpdateEdcaParam(_, _))
        .WillOnce(Return(SOFTBUS_OK))
        .WillOnce(Return(SOFTBUS_OK));
    g_setEdcaParamHook = [](int scene, int level) { return SOFTBUS_ERR; };
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_TCP_DIRECT,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest010
 * @tc.desc: UDP HML link, current level non-DISABLE, disable EDCA succeeds.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest010, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 3;
    UdpChannelInfo channel;
    (void)memset_s(&channel, sizeof(channel), 0, sizeof(channel));
    channel.info.linkType = LANE_HML;
    channel.info.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    channel.info.edcaParam.level = EDCA_LEVEL_FULL;
    EXPECT_CALL(mock, TransGetUdpChannelById(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(channel), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, TransUdpUpdateEdcaParam(_, _)).WillOnce(Return(SOFTBUS_OK));
    g_setEdcaParamHook = [](int scene, int level) {
        EXPECT_EQ(EDCA_LEVEL_DISABLE, level);
        return SOFTBUS_OK;
    };
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_UDP,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_DISABLE);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransSetEdcaParamForChannelTest011
 * @tc.desc: UDP non-HML link returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransSetEdcaParamForChannelTest011, TestSize.Level1)
{
    TransChannelCommonMock mock;
    int32_t channelId = 4;
    UdpChannelInfo channel;
    (void)memset_s(&channel, sizeof(channel), 0, sizeof(channel));
    channel.info.linkType = LANE_P2P;
    channel.info.edcaParam.level = EDCA_LEVEL_DISABLE;
    EXPECT_CALL(mock, TransGetUdpChannelById(_, _))
        .WillOnce(DoAll(SetArgPointee<1>(channel), Return(SOFTBUS_OK)));
    int32_t ret = TransSetEdcaParamForChannel(channelId, CHANNEL_TYPE_UDP,
        EDCA_SCENE_DATA_CLONE, EDCA_LEVEL_SEMI);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}
} // namespace OHOS
