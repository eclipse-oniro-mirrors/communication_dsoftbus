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
std::function<struct WifiDirectManager *()> g_getMgrHook;

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

struct WifiDirectManager g_nullEdcaMgr = {};
} // namespace

extern "C" struct WifiDirectManager *GetWifiDirectManager(void)
{
    if (g_getMgrHook) {
        return g_getMgrHook();
    }
    return &g_wifiDirectManager;
}

class TransChannelCommonTest : public testing::Test {
public:
    TransChannelCommonTest() { }
    ~TransChannelCommonTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override { g_setEdcaParamHook = nullptr; g_getMgrHook = nullptr; }
    void TearDown() override { g_setEdcaParamHook = nullptr; g_getMgrHook = nullptr; }
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

/*
 * @tc.name: TransCancelEdcaParamTest001
 * @tc.desc: NULL edcaParam returns immediately.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransCancelEdcaParamTest001, TestSize.Level1)
{
    TransCancelEdcaParam(NULL, LANE_HML);
}

/*
 * @tc.name: TransCancelEdcaParamTest002
 * @tc.desc: level == EDCA_LEVEL_DISABLE returns immediately.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransCancelEdcaParamTest002, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_DISABLE };
    TransCancelEdcaParam(&edca, LANE_HML);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, edca.level);
}

/*
 * @tc.name: TransCancelEdcaParamTest003
 * @tc.desc: linkType != LANE_HML returns immediately.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransCancelEdcaParamTest003, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_FULL };
    TransCancelEdcaParam(&edca, LANE_WLAN_5G);
    EXPECT_EQ(EDCA_LEVEL_FULL, edca.level);
}

/*
 * @tc.name: TransCancelEdcaParamTest004
 * @tc.desc: valid params with available manager calls setEdcaParam(DISABLE) and resets level.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransCancelEdcaParamTest004, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_FULL };
    g_setEdcaParamHook = [](int scene, int level) {
        EXPECT_EQ(EDCA_LEVEL_DISABLE, level);
        return SOFTBUS_OK;
    };
    TransCancelEdcaParam(&edca, LANE_HML);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, edca.level);
}

/*
 * @tc.name: TransCancelEdcaParamTest005
 * @tc.desc: valid params with mgr == NULL skips setEdcaParam, still resets level.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransCancelEdcaParamTest005, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_SEMI };
    g_getMgrHook = []() -> struct WifiDirectManager * { return nullptr; };
    TransCancelEdcaParam(&edca, LANE_HML);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, edca.level);
}

/*
 * @tc.name: TransCancelEdcaParamTest006
 * @tc.desc: valid params with mgr->setEdcaParam == NULL skips driver call, still resets level.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransCancelEdcaParamTest006, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_FULL };
    g_getMgrHook = []() -> struct WifiDirectManager * { return &g_nullEdcaMgr; };
    TransCancelEdcaParam(&edca, LANE_HML);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, edca.level);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest001
 * @tc.desc: UDP channel, linkType != HML skips EDCA, UDP opens OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest001, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_WLAN_5G;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest002
 * @tc.desc: UDP channel, HML link with level==DISABLE skips EDCA, UDP opens OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest002, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.level = EDCA_LEVEL_DISABLE;
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest003
 * @tc.desc: UDP HML link, driver OK, edcaSet=true, UDP opens OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest003, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    g_setEdcaParamHook = [](int scene, int level) { return SOFTBUS_OK; };
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest004
 * @tc.desc: UDP HML link, driver OK, edcaSet=true, UDP opens fails, cancel EDCA.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest004, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    bool cancelCalled = false;
    g_setEdcaParamHook = [&cancelCalled](int scene, int level) {
        if (level == EDCA_LEVEL_DISABLE) {
            cancelCalled = true;
        }
        return SOFTBUS_OK;
    };
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    EXPECT_TRUE(cancelCalled);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest005
 * @tc.desc: UDP HML link, driver fails (mgr==NULL), edcaSet=false, UDP fails, skip cancel.
 *           Also covers TransApplyEdcaToDriver mgr==NULL branch.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest005, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    g_getMgrHook = []() -> struct WifiDirectManager * { return nullptr; };
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, appInfo.edcaParam.level);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest006
 * @tc.desc: UDP HML link, driver fails (setEdcaParam==NULL), edcaSet=false, UDP fails, skip cancel.
 *           Also covers TransApplyEdcaToDriver setEdcaParam==NULL branch.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest006, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_FULL;
    g_getMgrHook = []() -> struct WifiDirectManager * { return &g_nullEdcaMgr; };
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, appInfo.edcaParam.level);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest007
 * @tc.desc: TCP_DIRECT HML link, driver OK, edcaSet=true, TCP fails, cancel EDCA.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest007, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    bool cancelCalled = false;
    g_setEdcaParamHook = [&cancelCalled](int scene, int level) {
        if (level == EDCA_LEVEL_DISABLE) {
            cancelCalled = true;
        }
        return SOFTBUS_OK;
    };
    EXPECT_CALL(mock, TransOpenDirectChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_TCP_DIRECT, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    EXPECT_TRUE(cancelCalled);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest008
 * @tc.desc: TCP_DIRECT HML link, driver fails, edcaSet=false, TCP fails, skip cancel.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest008, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_FULL;
    g_setEdcaParamHook = [](int scene, int level) { return SOFTBUS_ERR; };
    EXPECT_CALL(mock, TransOpenDirectChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_TCP_DIRECT, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    EXPECT_EQ(EDCA_LEVEL_DISABLE, appInfo.edcaParam.level);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest009
 * @tc.desc: PROXY HML link, driver OK, edcaSet=true, Proxy fails, returns directly (no EDCA cancel).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest009, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    bool cancelCalled = false;
    g_setEdcaParamHook = [&cancelCalled](int scene, int level) {
        if (level == EDCA_LEVEL_DISABLE) {
            cancelCalled = true;
        }
        return SOFTBUS_OK;
    };
    EXPECT_CALL(mock, TransProxyOpenProxyChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_PROXY, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
    EXPECT_FALSE(cancelCalled);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest010
 * @tc.desc: PROXY HML link, driver OK, edcaSet=true, Proxy opens OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest010, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    g_setEdcaParamHook = [](int scene, int level) { return SOFTBUS_OK; };
    EXPECT_CALL(mock, TransProxyOpenProxyChannel(_, _, _)).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_PROXY, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest011
 * @tc.desc: UDP non-HML link, edcaSet=false, UDP fails, goto EXIT, skip cancel.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest011, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_P2P;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest012
 * @tc.desc: TCP_DIRECT non-HML link, edcaSet=false, TCP fails, goto EXIT, skip cancel.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest012, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_WLAN_2P4G;
    appInfo.edcaParam.level = EDCA_LEVEL_FULL;
    EXPECT_CALL(mock, TransOpenDirectChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_TCP_DIRECT, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest013
 * @tc.desc: UDP HML link, driver OK on first call, mgr==NULL on EXIT cancel call.
 *           Covers EXIT_EDCA_CANCEL mgr==NULL branch with edcaSet=true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest013, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    int mgrCallCount = 0;
    g_getMgrHook = [&]() -> struct WifiDirectManager * {
        mgrCallCount++;
        if (mgrCallCount == 1) {
            return &g_wifiDirectManager;
        }
        return nullptr;
    };
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest014
 * @tc.desc: UDP HML link, driver OK on first call, setEdcaParam==NULL on EXIT cancel call.
 *           Covers EXIT_EDCA_CANCEL setEdcaParam==NULL branch with edcaSet=true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest014, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_HML;
    appInfo.edcaParam.scene = EDCA_SCENE_DATA_CLONE;
    appInfo.edcaParam.level = EDCA_LEVEL_SEMI;
    int mgrCallCount = 0;
    g_getMgrHook = [&]() -> struct WifiDirectManager * {
        mgrCallCount++;
        if (mgrCallCount == 1) {
            return &g_wifiDirectManager;
        }
        return &g_nullEdcaMgr;
    };
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_ERR));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_ERR, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest015
 * @tc.desc: NULL appInfo returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest015, TestSize.Level1)
{
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, NULL, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest016
 * @tc.desc: NULL connOpt returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest016, TestSize.Level1)
{
    AppInfo appInfo;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, NULL, &channelId);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest017
 * @tc.desc: type==CHANNEL_TYPE_BUTT returns SOFTBUS_TRANS_INVALID_CHANNEL_TYPE.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest017, TestSize.Level1)
{
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_BUTT, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_TYPE, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest018
 * @tc.desc: TCP_DIRECT non-HML link, TCP opens OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest018, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_WLAN_5G;
    appInfo.edcaParam.level = EDCA_LEVEL_DISABLE;
    EXPECT_CALL(mock, TransOpenDirectChannel(_, _, _)).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_TCP_DIRECT, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransOpenChannelProcEdcaTest019
 * @tc.desc: UDP non-HML link, UDP opens OK.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransChannelCommonTest, TransOpenChannelProcEdcaTest019, TestSize.Level1)
{
    TransChannelCommonMock mock;
    AppInfo appInfo;
    ConnectOption connOpt;
    int32_t channelId = 0;
    (void)memset_s(&appInfo, sizeof(appInfo), 0, sizeof(appInfo));
    (void)memset_s(&connOpt, sizeof(connOpt), 0, sizeof(connOpt));
    appInfo.linkType = LANE_WLAN_5G;
    appInfo.edcaParam.level = EDCA_LEVEL_DISABLE;
    EXPECT_CALL(mock, TransOpenUdpChannel(_, _, _)).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = TransOpenChannelProc(CHANNEL_TYPE_UDP, &appInfo, &connOpt, &channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);
}
} // namespace OHOS
