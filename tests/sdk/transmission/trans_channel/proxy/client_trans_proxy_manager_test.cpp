/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "securec.h"

#include "client_trans_proxy_manager.h"
#include "client_trans_session_manager.h"
#include "client_trans_socket_manager.h"
#include "session.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_access_token_test.h"
#include "client_trans_proxy_file_manager.h"

#define TEST_CHANNEL_ID (-10)
#define TEST_ERR_CODE (-1)
#define TEST_DATA "testdata"
#define TEST_DATA_LENGTH 9
#define TEST_FILE_CNT 4
#define TEST_SEQ 188

using namespace std;
using namespace testing::ext;

namespace OHOS {
const char *g_proxyPkgName = "dms";
const char *g_proxySessionName = "ohos.distributedschedule.dms.test";
const char *g_testProxyFileName[] = {
    "/data/test.txt",
    "/data/ss.txt",
    "/data/test.tar",
    "/data/test.mp3",
};
const char *g_proxyFileSet[] = {
    "/data/data/test.txt",
    "/path/max/length/512/"
    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
    "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
    "111111111111111111111111111111111111111111111111111",
    "ss",
    "/data/ss",
};

int32_t TransOnSessionOpened(const char *sessionName, const ChannelInfo *channel, SessionType flag)
{
    return SOFTBUS_OK;
}

int32_t TransOnSessionClosed(int32_t channelId, int32_t channelType, ShutdownReason reason)
{
    return SOFTBUS_OK;
}

int32_t TransOnSessionOpenFailed(int32_t channelId, int32_t channelType, int32_t errCode)
{
    return SOFTBUS_OK;
}

int32_t TransOnBytesReceived(int32_t channelId, int32_t channelType,
    const void *data, uint32_t len, SessionPktType type)
{
    return SOFTBUS_OK;
}

int32_t TransOnOnStreamRecevied(int32_t channelId, int32_t channelType,
    const StreamData *data, const StreamData *ext, const StreamFrameInfo *param)
{
    return SOFTBUS_OK;
}

int32_t TransOnGetSessionId(int32_t channelId, int32_t channelType, int32_t *sessionId)
{
    return SOFTBUS_OK;
}
int32_t TransOnQosEvent(int32_t channelId, int32_t channelType, int32_t eventId,
    int32_t tvCount, const QosTv *tvList)
{
    return SOFTBUS_OK;
}

static IClientSessionCallBack g_clientSessionCb = {
    .OnSessionOpened = TransOnSessionOpened,
    .OnSessionClosed = TransOnSessionClosed,
    .OnSessionOpenFailed = TransOnSessionOpenFailed,
    .OnDataReceived = TransOnBytesReceived,
    .OnStreamReceived = TransOnOnStreamRecevied,
    .OnQosEvent = TransOnQosEvent,
};

int32_t OnSessionOpened(const char *sessionName, const ChannelInfo *channel, SessionType flag)
{
    (void)sessionName;
    (void)channel;
    (void)flag;
    return SOFTBUS_INVALID_PARAM;
}

int32_t OnSessionClosed(int32_t channelId, int32_t channelType, ShutdownReason reason)
{
    (void)channelId;
    (void)channelType;
    (void)reason;
    return SOFTBUS_INVALID_PARAM;
}

int32_t OnSessionOpenFailed(int32_t channelId, int32_t channelType, int32_t errCode)
{
    (void)channelId;
    (void)channelType;
    (void)errCode;
    return SOFTBUS_INVALID_PARAM;
}

int32_t OnBytesReceived(int32_t channelId, int32_t channelType,
    const void *data, uint32_t len, SessionPktType type)
{
    (void)channelId;
    (void)channelType;
    (void)data;
    (void)len;
    (void)type;
    return SOFTBUS_INVALID_PARAM;
}

static IClientSessionCallBack g_sessionCb = {
    .OnSessionOpened = OnSessionOpened,
    .OnSessionClosed = OnSessionClosed,
    .OnSessionOpenFailed = OnSessionOpenFailed,
    .OnDataReceived = OnBytesReceived,
};

class ClientTransProxyManagerTest : public testing::Test {
public:
    ClientTransProxyManagerTest() {}
    ~ClientTransProxyManagerTest() {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override {}
    void TearDown() override {}
};

void ClientTransProxyManagerTest::SetUpTestCase(void)
{
    int ret = ClientTransProxyInit(&g_clientSessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SetAceessTokenPermission("dsoftbusTransTest");
}
void ClientTransProxyManagerTest::TearDownTestCase(void) {}

/**
 * @tc.name: ClientTransProxyInitTest
 * @tc.desc: client trans proxy init test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyInitTest, TestSize.Level0)
{
    int ret = ClientTransProxyInit(nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyOnChannelOpenedTest
 * @tc.desc: client trans proxy on channel opened test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnChannelOpenedTest, TestSize.Level0)
{
    int ret = ClientTransProxyOnChannelOpened(g_proxySessionName, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ChannelInfo channelInfo = {0};
    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo);
    EXPECT_EQ(SOFTBUS_MEM_ERR, ret);
}

/**
 * @tc.name: ClientTransProxyOnDataReceivedTest
 * @tc.desc: client trans proxy on data received test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnDataReceivedTest, TestSize.Level0)
{
    int32_t channelId = 1;
    int ret = ClientTransProxyOnDataReceived(channelId, nullptr, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = ClientTransProxyOnDataReceived(channelId, TEST_DATA, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_INVALID_CHANNEL_ID, ret);
}

/**
 * @tc.name: ClientTransProxyErrorCallBackTest
 * @tc.desc: client trans proxy error callback test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyErrorCallBackTest, TestSize.Level0)
{
    int ret = ClientTransProxyInit(&g_sessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ChannelInfo channelInfo = {0};
    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo);
    EXPECT_EQ(SOFTBUS_MEM_ERR, ret);

    int32_t channelId = 1;
    ret = ClientTransProxyOnDataReceived(channelId, TEST_DATA, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/**
 * @tc.name: ClientTransProxyCloseChannelTest
 * @tc.desc: client trans proxy close channel test, use the normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyCloseChannelTest, TestSize.Level0)
{
    int32_t ret = ClientTransProxyInit(&g_sessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t channelId = 1;
    ClientTransProxyCloseChannel(TEST_CHANNEL_ID);

    ClientTransProxyCloseChannel(channelId);
}

/**
 * @tc.name: TransProxyChannelSendFileTest
 * @tc.desc: trans proxy channel send file test, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxyChannelSendFileTest, TestSize.Level0)
{
    int32_t channelId = 1;
    int ret = TransProxyChannelSendFile(channelId, nullptr, g_proxyFileSet, TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransProxyChannelSendFile(channelId, g_testProxyFileName, g_proxyFileSet, MAX_SEND_FILE_NUM + TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransProxyChannelSendFile(channelId, g_testProxyFileName, nullptr, TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_SERVER_NOINIT, ret);

    ret = TransProxyChannelSendFile(channelId, g_testProxyFileName, g_proxyFileSet, TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyGetInfoByChannelIdTest
 * @tc.desc: Should return SOFTBUS_INVALID_PARAM when given channelInfo is null.
 * @tc.desc: Should return SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND when given invalid parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyGetInfoByChannelIdTest, TestSize.Level0)
{
    int32_t channelId = 1;
    ProxyChannelInfoDetail info;
    memset_s(&info, sizeof(ProxyChannelInfoDetail), 0, sizeof(ProxyChannelInfoDetail));
    int ret = ClientTransProxyGetInfoByChannelId(channelId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = ClientTransProxyGetInfoByChannelId(channelId, &info);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND, ret);
}

/**
 * @tc.name: TransProxyPackAndSendDataTest
 * @tc.desc: Should return SOFTBUS_INVALID_PARAM when given channelInfo or data is null.
 * @tc.desc: Should return SOFTBUS_TRANS_PROXY_SEND_REQUEST_FAILED when given invalid parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxyPackAndSendDataTest, TestSize.Level0)
{
    int32_t channelId = 1;
    const char *data = "test";
    uint32_t len = 5;
    ProxyChannelInfoDetail info;
    memset_s(&info, sizeof(ProxyChannelInfoDetail), 0, sizeof(ProxyChannelInfoDetail));
    SessionPktType pktType = TRANS_SESSION_MESSAGE;
    int32_t ret = TransProxyPackAndSendData(channelId, nullptr, len, &info, pktType);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransProxyPackAndSendData(channelId, data, len, nullptr, pktType);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransProxyPackAndSendData(channelId,
        static_cast<const void *>(data), len, &info, pktType);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/**
 * @tc.name: ClientTransProxyGetLinkTypeByChannelId
 * @tc.desc: Should return SOFTBUS_INVALID_PARAM when given linkType or data is null.
 * @tc.desc: Should return SOFTBUS_NOT_FIND when get link type failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyGetLinkTypeByChannelIdTest, TestSize.Level0)
{
    int32_t channelId = -1;
    int32_t ret = ClientTransProxyGetLinkTypeByChannelId(channelId, NULL);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    int32_t linkType;
    ret = ClientTransProxyGetLinkTypeByChannelId(channelId, &linkType);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
}
} // namespace OHOS