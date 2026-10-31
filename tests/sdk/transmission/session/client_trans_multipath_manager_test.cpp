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

HWTEST_F(TransMultipathManagerTest, TransMultipathGetEnabledInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathGetEnabled(-1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    bool enabled = false;
    ret = TransMultipathGetEnabled(-1, &enabled);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_SESSION_ID);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathSetEnabledInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathSetEnabled(-1, true);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_SESSION_ID);
    ret = TransMultipathSetEnabled(-1, false);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_SESSION_ID);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathSetStrategyInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathSetStrategy(-1, MULTIPATH_STRATEGY_PARALLEL);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathSetStrategy(0, MULTIPATH_STRATEGY_PARALLEL);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathIsSessionActiveInvalidParamTest001, TestSize.Level1)
{
    bool ret = TransMultipathIsSessionActive(nullptr, nullptr);
    EXPECT_FALSE(ret);
    int32_t id = 0;
    ret = TransMultipathIsSessionActive("testSession", &id);
    EXPECT_FALSE(ret);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathGetReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t channelId = 0;
    int32_t channelType = 0;
    int32_t routeType = 0;
    int32_t ret = TransMultipathGetReserveChannel(-1, nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetReserveChannel(-1, &channelId, &channelType, &routeType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathGetRouteTypeInvalidParamTest001, TestSize.Level1)
{
    int32_t mainRouteType = 0;
    int32_t reserveRouteType = 0;
    int32_t ret = TransMultipathGetRouteType(-1, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetRouteType(-1, &mainRouteType, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetRouteType(1, nullptr, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetRouteType(1, &mainRouteType, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathClearReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathClearReserveChannel(-1);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathGetChannelRoleInvalidParamTest001, TestSize.Level1)
{
    ChannelUseChooseState useType = CHANNEL_USE_CHOOSE_INIT;
    int32_t ret = TransMultipathGetChannelRole(-1, -1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathGetChannelRole(-1, -1, &useType);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathNeedDelReserveInvalidParamTest001, TestSize.Level1)
{
    bool onlyReserve = false;
    bool ret = TransMultipathNeedDelReserve(nullptr, -1, nullptr);
    EXPECT_FALSE(ret);
    SessionInfo sessionNode = {};
    ret = TransMultipathNeedDelReserve(&sessionNode, -1, &onlyReserve);
    EXPECT_FALSE(ret);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathUpdateClosingStateTest001, TestSize.Level1)
{
    TransMultipathUpdateClosingState(nullptr, true);
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = false;
    TransMultipathUpdateClosingState(&sessionNode, true);
    EXPECT_FALSE(sessionNode.isClosingReserve);
    sessionNode.enableMultipath = true;
    TransMultipathUpdateClosingState(&sessionNode, true);
    EXPECT_TRUE(sessionNode.isClosingReserve);
    TransMultipathUpdateClosingState(&sessionNode, false);
    EXPECT_FALSE(sessionNode.isClosingReserve);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathUpdateReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = TransMultipathUpdateReserveChannel(INVALID_SESSION_ID, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TransMultipathUpdateReserveChannel(1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

HWTEST_F(TransMultipathManagerTest, TransMultipathNeedDelReserveValidTest001, TestSize.Level1)
{
    int32_t routeType = 100;
    bool onlyReserveLinkDown = false;
    SessionInfo sessionNode = {};
    sessionNode.enableMultipath = true;
    sessionNode.routeType = routeType;
    sessionNode.routeTypeReserve = 200;
    bool ret = TransMultipathNeedDelReserve(&sessionNode, routeType, &onlyReserveLinkDown);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(onlyReserveLinkDown, false);
    sessionNode.routeType = 200;
    sessionNode.routeTypeReserve = routeType;
    ret = TransMultipathNeedDelReserve(&sessionNode, routeType, &onlyReserveLinkDown);
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
    const int32_t unregisteredSessionId = 999;
    int32_t mainRouteType = -1;
    int32_t reserveRouteType = -1;
    int32_t ret = TransMultipathGetRouteType(unregisteredSessionId, &mainRouteType, &reserveRouteType);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    // 输出参数不应被修改
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
    ISessionListener listener = { 0 };
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

}