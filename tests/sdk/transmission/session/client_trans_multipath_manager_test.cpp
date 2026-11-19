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
#include "client_trans_multipath_manager.h"
#include "client_trans_session_manager.h"
#include "session.h"
#include "softbus_adapter_mem.h"
#include "softbus_app_info.h"
#include "softbus_error_code.h"
#include "softbus_trans_def.h"
#include "softbus_def.h"
#include "trans_log.h"

using namespace testing::ext;

namespace {
const char *MULTIPATH_PKG_NAME = "dms_multipath";
const char *MULTIPATH_SESSION_NAME = "ohos.distributedschedule.dms.test.multipath";
const char *MULTIPATH_PEER_NAME = "ohos.distributedschedule.dms.test.multipath.peer";
const char *MULTIPATH_DEVICE_ID = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF0";
const char *MULTIPATH_GROUP_ID = "TEST_GROUP_ID_MULTIPATH";
const int32_t TEST_CHANNEL_ID = 1000;
const int32_t TEST_PRIMARY_CH_ID = 100;
const int32_t TEST_RESERVE_CH_ID = 200;
const int32_t TEST_INVALID_CH_ID = 999;
const int32_t TEST_UNREGISTERED_SESSION_ID = 999;
static SessionAttribute MULTIPATH_SESSION_ATTR = {
    .dataType = TYPE_BYTES,
};

static void TestGenerateMultipathCommParam(SessionParam *sessionParam)
{
    sessionParam->sessionName = const_cast<char *>(MULTIPATH_SESSION_NAME);
    sessionParam->peerSessionName = const_cast<char *>(MULTIPATH_PEER_NAME);
    sessionParam->peerDeviceId = const_cast<char *>(MULTIPATH_DEVICE_ID);
    sessionParam->groupId = const_cast<char *>(MULTIPATH_GROUP_ID);
    sessionParam->attr = &MULTIPATH_SESSION_ATTR;
}

static SessionInfo *TestGenerateMultipathSession(const SessionParam *param)
{
    SessionInfo *session = reinterpret_cast<SessionInfo *>(SoftBusCalloc(sizeof(SessionInfo)));
    if (session == nullptr) {
        return nullptr;
    }
    if (strcpy_s(session->info.peerSessionName, SESSION_NAME_SIZE_MAX, param->peerSessionName) != EOK ||
        strcpy_s(session->info.peerDeviceId, DEVICE_ID_SIZE_MAX, param->peerDeviceId) != EOK ||
        strcpy_s(session->info.groupId, GROUP_ID_SIZE_MAX, param->groupId) != EOK) {
        SoftBusFree(session);
        return nullptr;
    }
    session->sessionId = INVALID_SESSION_ID;
    session->channelId = 0;
    session->channelType = CHANNEL_TYPE_BUTT;
    session->isServer = false;
    session->enableStatus = ENABLE_STATUS_INIT;
    session->routeType = ROUTE_TYPE_ALL;
    session->info.flag = TYPE_BYTES;
    session->isEncrypt = true;
    session->algorithm = 0;
    session->fileEncrypt = 0;
    session->crc = 0;
    session->lifecycle.sessionState = SESSION_STATE_INIT;
    return session;
}
} // namespace

namespace OHOS {
class TransMultipathManagerTest : public testing::Test {
public:
    TransMultipathManagerTest() {}
    ~TransMultipathManagerTest() {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override {}
    void TearDown(void) override {}
};

void TransMultipathManagerTest::SetUpTestCase(void)
{
    (void)TransClientInit();
}

void TransMultipathManagerTest::TearDownTestCase(void)
{
    TransClientDeinit();
}

/*
 * @tc.name: TransMultipathGetEnabledInvalidParamTest001
 * @tc.desc: test TransMultipathGetEnabled with null output and invalid sessionId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetEnabledInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathGetEnabled(-1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    bool enabled = false;
    ret = TransMultipathGetEnabled(-1, &enabled);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_SESSION_ID);
}

/*
 * @tc.name: TransMultipathSetEnabledInvalidParamTest001
 * @tc.desc: test TransMultipathSetEnabled with invalid sessionId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSetEnabledInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathSetEnabled(-1, true);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_SESSION_ID);
    ret = TransMultipathSetEnabled(0, true);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    ret = TransMultipathSetEnabled(-1, false);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_SESSION_ID);
}

/*
 * @tc.name: TransMultipathSetStrategyInvalidParamTest001
 * @tc.desc: test TransMultipathSetStrategy with invalid sessionId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSetStrategyInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathSetStrategy(-1, MULTIPATH_STRATEGY_PARALLEL);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathSetStrategy(0, MULTIPATH_STRATEGY_PARALLEL);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathSetStrategy(1, MULTIPATH_STRATEGY_MAX);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
}

/*
 * @tc.name: TransMultipathIsSessionActiveInvalidParamTest001
 * @tc.desc: test TransMultipathIsSessionActive with null params
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathIsSessionActiveInvalidParamTest001, TestSize.Level1)
{
    bool ret = TransMultipathIsSessionActive(nullptr, nullptr);
    EXPECT_FALSE(ret);
    int32_t id = 0;
    ret = TransMultipathIsSessionActive("testSession", &id);
    EXPECT_FALSE(ret);
    ret = TransMultipathIsSessionActive(nullptr, &id);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: TransMultipathGetReserveChannelInvalidParamTest001
 * @tc.desc: test TransMultipathGetReserveChannel with null params and invalid sessionId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathGetReserveChannel(-1, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    int32_t channelId = 0;
    int32_t channelType = 0;
    int32_t routeType = 0;
    ret = TransMultipathGetReserveChannel(-1, &channelId, &channelType, &routeType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathGetRouteTypeInvalidParamTest001
 * @tc.desc: test TransMultipathGetRouteType with null params and invalid sessionId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetRouteTypeInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathGetRouteType(-1, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    int32_t mainRouteType = 0;
    int32_t reserveRouteType = 0;
    ret = TransMultipathGetRouteType(-1, &mainRouteType, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetRouteType(1, nullptr, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetRouteType(1, &mainRouteType, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathClearReserveChannelInvalidParamTest001
 * @tc.desc: test TransMultipathClearReserveChannel with invalid sessionId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathClearReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathClearReserveChannel(-1);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathClearReserveChannel(0);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    ret = TransMultipathClearReserveChannel(TEST_UNREGISTERED_SESSION_ID);
    EXPECT_NE(ret, SOFTBUS_OK);
}

/*
 * @tc.name: TransMultipathGetChannelRoleInvalidParamTest001
 * @tc.desc: test TransMultipathGetChannelRole with null output and invalid params
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetChannelRoleInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathGetChannelRole(-1, -1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ChannelUseChooseState useType = CHANNEL_USE_CHOOSE_INIT;
    ret = TransMultipathGetChannelRole(-1, -1, &useType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetChannelRole(1, 1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathNeedDelReserveInvalidParamTest001
 * @tc.desc: test TransMultipathNeedDelReserve with null node and invalid routeType
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathNeedDelReserveInvalidParamTest001, TestSize.Level1)
{
    bool ret = TransMultipathNeedDelReserve(nullptr, -1, nullptr);
    EXPECT_FALSE(ret);
    SessionInfo sessionNode = {};
    ret = TransMultipathNeedDelReserve(&sessionNode, -1, nullptr);
    EXPECT_FALSE(ret);
    bool onlyReserve = false;
    ret = TransMultipathNeedDelReserve(&sessionNode, -1, &onlyReserve);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: TransMultipathUpdateClosingStateNoEffectTest001
 * @tc.desc: test TransMultipathUpdateClosingState with null node and non-multipath session
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathUpdateClosingStateNoEffectTest001, TestSize.Level1)
{
    TransMultipathUpdateClosingState(nullptr, true);
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = false;
    TransMultipathUpdateClosingState(&sessionNode, true);
    EXPECT_FALSE(sessionNode.isClosingReserve);
    TransMultipathUpdateClosingState(&sessionNode, false);
    EXPECT_FALSE(sessionNode.isClosingReserve);
}

/*
 * @tc.name: TransMultipathUpdateClosingStateEnableTest001
 * @tc.desc: test TransMultipathUpdateClosingState sets isClosingReserve to true for multipath session
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathUpdateClosingStateEnableTest001, TestSize.Level1)
{
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = true;
    EXPECT_FALSE(sessionNode.isClosingReserve);
    TransMultipathUpdateClosingState(&sessionNode, true);
    EXPECT_TRUE(sessionNode.isClosingReserve);
}

/*
 * @tc.name: TransMultipathUpdateClosingStateDisableTest001
 * @tc.desc: test TransMultipathUpdateClosingState sets isClosingReserve to false for multipath session
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathUpdateClosingStateDisableTest001, TestSize.Level1)
{
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = true;
    sessionNode.isClosingReserve = true;
    EXPECT_TRUE(sessionNode.isClosingReserve);
    TransMultipathUpdateClosingState(&sessionNode, false);
    EXPECT_FALSE(sessionNode.isClosingReserve);
}

/*
 * @tc.name: TransMultipathUpdateReserveChannelInvalidParamTest001
 * @tc.desc: test TransMultipathUpdateReserveChannel with invalid sessionId and null channel
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathUpdateReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathUpdateReserveChannel(INVALID_SESSION_ID, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathUpdateReserveChannel(1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathUpdateReserveChannel(-1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathNeedDelReserveMainMatchTest001
 * @tc.desc: test TransMultipathNeedDelReserve when main routeType matches and reserve exists
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathNeedDelReserveMainMatchTest001, TestSize.Level1)
{
    const int32_t routeType = 100;
    bool onlyReserveLinkDown = false;
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = true;
    sessionNode.routeType = routeType;
    sessionNode.routeTypeReserve = 200;
    bool ret = TransMultipathNeedDelReserve(&sessionNode, routeType, &onlyReserveLinkDown);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(onlyReserveLinkDown, false);
}

/*
 * @tc.name: TransMultipathNeedDelReserveReserveMatchTest001
 * @tc.desc: test TransMultipathNeedDelReserve when reserve routeType matches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathNeedDelReserveReserveMatchTest001, TestSize.Level1)
{
    const int32_t routeType = 100;
    bool onlyReserveLinkDown = false;
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = true;
    sessionNode.routeType = 200;
    sessionNode.routeTypeReserve = routeType;
    bool ret = TransMultipathNeedDelReserve(&sessionNode, routeType, &onlyReserveLinkDown);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(onlyReserveLinkDown, true);
}

/*
 * @tc.name: TransMultipathGetRouteTypeGetSessionByIdFailTest001
 * @tc.desc: test TransMultipathGetRouteType with an unregistered sessionId,
 *           GetSessionById fails and returns SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetRouteTypeGetSessionByIdFailTest001, TestSize.Level1)
{
    int32_t mainRouteType = -1;
    int32_t reserveRouteType = -1;
    int32_t ret = TransMultipathGetRouteType(TEST_UNREGISTERED_SESSION_ID, &mainRouteType, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    // output params should not be modified
    EXPECT_EQ(mainRouteType, -1);
    EXPECT_EQ(reserveRouteType, -1);
}

/*
 * @tc.name: TransMultipathGetRouteTypeSuccessTest001
 * @tc.desc: test TransMultipathGetRouteType with a registered session whose routeType=WIFI_STA
 *           and routeTypeReserve=WIFI_P2P, returns SOFTBUS_OK and outputs the route types
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetRouteTypeSuccessTest001, TestSize.Level1)
{
    ISessionListener listener = {};
    uint64_t timestamp = 0;
    int32_t ret = ClientAddSessionServer(SEC_TYPE_PLAINTEXT, MULTIPATH_PKG_NAME, MULTIPATH_SESSION_NAME,
        &listener, &timestamp);
    EXPECT_EQ(ret, SOFTBUS_OK);

    SessionParam *sessionParam = reinterpret_cast<SessionParam *>(SoftBusCalloc(sizeof(SessionParam)));
    ASSERT_TRUE(sessionParam != nullptr);
    TestGenerateMultipathCommParam(sessionParam);
    SessionInfo *session = TestGenerateMultipathSession(sessionParam);
    ASSERT_TRUE(session != nullptr);
    session->channelId = TEST_CHANNEL_ID;
    session->channelType = CHANNEL_TYPE_PROXY;
    session->role = SESSION_ROLE_CLIENT;
    session->routeType = WIFI_STA;
    session->routeTypeReserve = WIFI_P2P;
    session->lifecycle.sessionState = SESSION_STATE_INIT;
    ret = ClientAddNewSession(MULTIPATH_SESSION_NAME, session);
    EXPECT_EQ(ret, SOFTBUS_OK);

    int32_t mainRouteType = -1;
    int32_t reserveRouteType = -1;
    ret = TransMultipathGetRouteType(session->sessionId, &mainRouteType, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(mainRouteType, WIFI_STA);
    EXPECT_EQ(reserveRouteType, WIFI_P2P);

    ret = ClientDeleteSession(session->sessionId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = ClientDeleteSessionServer(SEC_TYPE_PLAINTEXT, MULTIPATH_SESSION_NAME);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftBusFree(sessionParam);
}

/*
 * @tc.name: TransMultipathIsWiredRouteTest001
 * @tc.desc: test TransMultipathIsWiredRoute returns true for WIFI_USB, false for others
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathIsWiredRouteTest001, TestSize.Level1)
{
    EXPECT_TRUE(TransMultipathIsWiredRoute(WIFI_USB));
    EXPECT_FALSE(TransMultipathIsWiredRoute(WIFI_P2P));
    EXPECT_FALSE(TransMultipathIsWiredRoute(WIFI_STA));
    EXPECT_FALSE(TransMultipathIsWiredRoute(ROUTE_TYPE_ALL));
    EXPECT_FALSE(TransMultipathIsWiredRoute(INVALID_ROUTE_TYPE));
}

/*
 * @tc.name: TransMultipathSelectCtrlChannelNonMultipathTest001
 * @tc.desc: test TransMultipathSelectCtrlChannelOnNode returns main channel when multipath disabled
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSelectCtrlChannelNonMultipathTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = false;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    int32_t channelId = -1;
    int32_t channelType = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &channelId, &channelType), SOFTBUS_OK);
    EXPECT_EQ(channelId, TEST_PRIMARY_CH_ID);
    EXPECT_EQ(channelType, CHANNEL_TYPE_TCP_DIRECT);
}

/*
 * @tc.name: TransMultipathSelectCtrlChannelUsbPrimaryBothUpTest001
 * @tc.desc: test TransMultipathSelectCtrlChannelOnNode returns primary when USB in primary and both up
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSelectCtrlChannelUsbPrimaryBothUpTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.channelTypeReserve = CHANNEL_TYPE_TCP_DIRECT;
    node.routeTypeReserve = WIFI_P2P;
    node.ctrlWiredStatus = CTRL_CH_STATUS_UP;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    int32_t channelId = -1;
    int32_t channelType = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &channelId, &channelType), SOFTBUS_OK);
    EXPECT_EQ(channelId, TEST_PRIMARY_CH_ID);
}

/*
 * @tc.name: TransMultipathSelectCtrlChannelUsbReserveBothUpTest001
 * @tc.desc: test TransMultipathSelectCtrlChannelOnNode returns reserve when USB in reserve and both up
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSelectCtrlChannelUsbReserveBothUpTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    node.routeType = WIFI_P2P;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.channelTypeReserve = CHANNEL_TYPE_TCP_DIRECT;
    node.routeTypeReserve = WIFI_USB;
    node.ctrlWiredStatus = CTRL_CH_STATUS_UP;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    int32_t channelId = -1;
    int32_t channelType = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &channelId, &channelType), SOFTBUS_OK);
    EXPECT_EQ(channelId, TEST_RESERVE_CH_ID);
}

/*
 * @tc.name: TransMultipathSelectCtrlChannelWiredDownTest001
 * @tc.desc: test TransMultipathSelectCtrlChannelOnNode returns wireless reserve when wired is down
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSelectCtrlChannelWiredDownTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.channelTypeReserve = CHANNEL_TYPE_TCP_DIRECT;
    node.routeTypeReserve = WIFI_P2P;
    node.ctrlWiredStatus = CTRL_CH_STATUS_DOWN;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    int32_t channelId = -1;
    int32_t channelType = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &channelId, &channelType), SOFTBUS_OK);
    EXPECT_EQ(channelId, TEST_RESERVE_CH_ID);
}

/*
 * @tc.name: TransMultipathSelectCtrlChannelAllDownTest001
 * @tc.desc: test TransMultipathSelectCtrlChannelOnNode returns error when all paths down
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSelectCtrlChannelAllDownTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.channelTypeReserve = CHANNEL_TYPE_TCP_DIRECT;
    node.routeTypeReserve = WIFI_P2P;
    node.ctrlWiredStatus = CTRL_CH_STATUS_DOWN;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_DOWN;
    int32_t channelId = -1;
    int32_t channelType = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &channelId, &channelType),
        SOFTBUS_TRANS_INVALID_CHANNEL_TYPE);
}

/*
 * @tc.name: TransMultipathSelectCtrlChannelNullParamTest001
 * @tc.desc: test TransMultipathSelectCtrlChannelOnNode returns SOFTBUS_INVALID_PARAM for null inputs
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathSelectCtrlChannelNullParamTest001, TestSize.Level1)
{
    SessionInfo node = {};
    int32_t channelId = -1;
    int32_t channelType = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(nullptr, &channelId, &channelType), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, nullptr, &channelType), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &channelId, nullptr), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathMarkDownOnNodeTest001
 * @tc.desc: test TransMultipathMarkDownOnNode marks correct control status and handles invalid params
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathMarkDownOnNodeTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.routeTypeReserve = WIFI_P2P;
    node.ctrlWiredStatus = CTRL_CH_STATUS_UP;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    EXPECT_EQ(TransMultipathMarkDownOnNode(&node, TEST_PRIMARY_CH_ID), SOFTBUS_OK);
    EXPECT_EQ(node.ctrlWiredStatus, CTRL_CH_STATUS_DOWN);
    EXPECT_EQ(node.ctrlWirelessStatus, CTRL_CH_STATUS_UP);
    EXPECT_EQ(TransMultipathMarkDownOnNode(&node, TEST_RESERVE_CH_ID), SOFTBUS_OK);
    EXPECT_EQ(node.ctrlWirelessStatus, CTRL_CH_STATUS_DOWN);
    EXPECT_EQ(TransMultipathMarkDownOnNode(&node, TEST_INVALID_CH_ID), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(TransMultipathMarkDownOnNode(nullptr, TEST_PRIMARY_CH_ID), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathClearChannelSlotOnNodeTest001
 * @tc.desc: test TransMultipathClearChannelSlotOnNode clears correct slot and handles invalid params
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathClearChannelSlotOnNodeTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    EXPECT_EQ(TransMultipathClearChannelSlotOnNode(&node, TEST_PRIMARY_CH_ID), SOFTBUS_OK);
    EXPECT_EQ(node.channelId, INVALID_CHANNEL_ID);
    EXPECT_EQ(node.channelIdReserve, TEST_RESERVE_CH_ID);
    EXPECT_EQ(TransMultipathClearChannelSlotOnNode(&node, TEST_RESERVE_CH_ID), SOFTBUS_OK);
    EXPECT_EQ(node.channelIdReserve, INVALID_CHANNEL_ID);
    EXPECT_EQ(TransMultipathClearChannelSlotOnNode(&node, TEST_INVALID_CH_ID), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathGetOtherCtrlChannelOnNodeTest001
 * @tc.desc: test TransMultipathGetOtherCtrlChannelOnNode returns surviving channel and handles invalid params
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathGetOtherCtrlChannelOnNodeTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.channelTypeReserve = CHANNEL_TYPE_TCP_DIRECT;
    node.routeTypeReserve = WIFI_P2P;
    int32_t id = -1;
    int32_t type = -1;
    node.ctrlWiredStatus = CTRL_CH_STATUS_DOWN;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_PRIMARY_CH_ID, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_RESERVE_CH_ID);
    node.ctrlWiredStatus = CTRL_CH_STATUS_UP;
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_RESERVE_CH_ID, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_PRIMARY_CH_ID);
    node.ctrlWiredStatus = CTRL_CH_STATUS_DOWN;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_DOWN;
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_PRIMARY_CH_ID, &id, &type),
        SOFTBUS_TRANS_INVALID_CHANNEL_TYPE);
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(nullptr, TEST_PRIMARY_CH_ID, &id, &type),
        SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: TransMultipathMarkUpOnNodeTest001
 * @tc.desc: test TransMultipathMarkUpOnNode sets correct control status and handles null param
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, TransMultipathMarkUpOnNodeTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.ctrlWiredStatus = CTRL_CH_STATUS_DOWN;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_DOWN;
    EXPECT_EQ(TransMultipathMarkUpOnNode(&node, WIFI_USB), SOFTBUS_OK);
    EXPECT_EQ(node.ctrlWiredStatus, CTRL_CH_STATUS_UP);
    EXPECT_EQ(node.ctrlWirelessStatus, CTRL_CH_STATUS_DOWN);
    EXPECT_EQ(TransMultipathMarkUpOnNode(&node, WIFI_P2P), SOFTBUS_OK);
    EXPECT_EQ(node.ctrlWirelessStatus, CTRL_CH_STATUS_UP);
    EXPECT_EQ(TransMultipathMarkUpOnNode(nullptr, WIFI_USB), SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: CtrlFallbackDecisionTest001
 * @tc.desc: test control-plane fallback: select primary, mark down, fallback to reserve, mark up, recover
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, CtrlFallbackDecisionTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.channelTypeReserve = CHANNEL_TYPE_TCP_DIRECT;
    node.routeTypeReserve = WIFI_P2P;
    node.ctrlWiredStatus = CTRL_CH_STATUS_UP;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    int32_t id = -1;
    int32_t type = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_PRIMARY_CH_ID);
    EXPECT_EQ(TransMultipathMarkDownOnNode(&node, TEST_PRIMARY_CH_ID), SOFTBUS_OK);
    EXPECT_EQ(node.ctrlWiredStatus, CTRL_CH_STATUS_DOWN);
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_PRIMARY_CH_ID, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_RESERVE_CH_ID);
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_RESERVE_CH_ID);
    EXPECT_EQ(TransMultipathMarkUpOnNode(&node, WIFI_USB), SOFTBUS_OK);
    EXPECT_EQ(node.ctrlWiredStatus, CTRL_CH_STATUS_UP);
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_PRIMARY_CH_ID);
}

/*
 * @tc.name: CtrlBothDeadDecisionTest001
 * @tc.desc: test control-plane when both channels marked down: get other and select both fail
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, CtrlBothDeadDecisionTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = true;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.routeType = WIFI_USB;
    node.channelIdReserve = TEST_RESERVE_CH_ID;
    node.routeTypeReserve = WIFI_P2P;
    node.ctrlWiredStatus = CTRL_CH_STATUS_UP;
    node.ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    EXPECT_EQ(TransMultipathMarkDownOnNode(&node, TEST_PRIMARY_CH_ID), SOFTBUS_OK);
    EXPECT_EQ(TransMultipathMarkDownOnNode(&node, TEST_RESERVE_CH_ID), SOFTBUS_OK);
    int32_t id = -1;
    int32_t type = -1;
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_PRIMARY_CH_ID, &id, &type),
        SOFTBUS_TRANS_INVALID_CHANNEL_TYPE);
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &id, &type),
        SOFTBUS_TRANS_INVALID_CHANNEL_TYPE);
}

/*
 * @tc.name: CtrlNonMultipathUnchangedTest001
 * @tc.desc: test control-plane select returns main and get other fails when multipath disabled
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(TransMultipathManagerTest, CtrlNonMultipathUnchangedTest001, TestSize.Level1)
{
    SessionInfo node = {};
    node.enableMultipath = false;
    node.channelId = TEST_PRIMARY_CH_ID;
    node.channelType = CHANNEL_TYPE_TCP_DIRECT;
    int32_t id = -1;
    int32_t type = -1;
    EXPECT_EQ(TransMultipathSelectCtrlChannelOnNode(&node, &id, &type), SOFTBUS_OK);
    EXPECT_EQ(id, TEST_PRIMARY_CH_ID);
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_PRIMARY_CH_ID, &id, &type),
        SOFTBUS_TRANS_INVALID_CHANNEL_TYPE);
    EXPECT_EQ(TransMultipathGetOtherCtrlChannelOnNode(&node, TEST_INVALID_CH_ID, &id, &type),
        SOFTBUS_INVALID_PARAM);
}
} // namespace OHOS
