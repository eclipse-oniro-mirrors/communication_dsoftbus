/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "securec.h"
#include <gtest/gtest.h>

#include "client_trans_auth_manager.h"
#include "client_trans_channel_manager.h"
#include "client_trans_proxy_manager.h"
#include "client_trans_session_callback.h"
#include "client_trans_tcp_direct_callback.h"
#include "client_trans_udp_manager.h"
#include "softbus_def.h"
#include "softbus_error_code.h"

#define TEST_DATA_LENGTH 1024
#define TEST_CHANNEL_ID 1
#define TEST_DATA "test"
#define TEST_FILE_PATH "/data/test.txt"

using namespace std;
using namespace testing::ext;

namespace OHOS {
class ClientTransChannelManagerTest : public testing::Test {
public:
    ClientTransChannelManagerTest() { }
    ~ClientTransChannelManagerTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void) override { }
    void TearDown(void) override { }
};

int32_t OnSessionOpened(
    const char *sessionName, const ChannelInfo *channel, SessionType flag, SocketAccessInfo *accessInfo)
{
    (void)sessionName;
    (void)channel;
    (void)flag;
    (void)accessInfo;
    return SOFTBUS_OK;
}

int32_t OnSessionClosed(int32_t channelId, int32_t channelType, ShutdownReason reason)
{
    (void)channelId;
    (void)channelType;
    (void)reason;
    return SOFTBUS_OK;
}

int32_t OnSessionOpenFailed(int32_t channelId, int32_t channelType, int32_t errCode)
{
    (void)channelId;
    (void)channelType;
    (void)errCode;
    return SOFTBUS_OK;
}

static int32_t OnDataReceived(
    int32_t channelId, int32_t channelType, const void *data, uint32_t len, SessionPktType type)
{
    (void)channelId;
    (void)channelType;
    (void)data;
    (void)len;
    (void)type;
    return SOFTBUS_OK;
}

static int32_t OnStreamReceived(
    int32_t channelId, int32_t channelType, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param)
{
    (void)channelId;
    (void)channelType;
    (void)data;
    (void)ext;
    (void)param;
    return SOFTBUS_OK;
}

static int32_t OnQosEvent(int32_t channelId, int32_t channelType, int32_t eventId, int32_t tvCount, const QosTv *tvList)
{
    (void)channelId;
    (void)channelType;
    (void)eventId;
    (void)tvCount;
    (void)tvList;
    return SOFTBUS_OK;
}

static IClientSessionCallBack g_clientSessionCb = {
    .OnSessionOpened = OnSessionOpened,
    .OnSessionClosed = OnSessionClosed,
    .OnSessionOpenFailed = OnSessionOpenFailed,
    .OnDataReceived = OnDataReceived,
    .OnStreamReceived = OnStreamReceived,
    .OnQosEvent = OnQosEvent,
};

void ClientTransChannelManagerTest::SetUpTestCase(void)
{
    int32_t ret = ClientTransAuthInit(&g_clientSessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransProxyInit(&g_clientSessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransTdcSetCallBack(&g_clientSessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransUdpMgrInit(&g_clientSessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

void ClientTransChannelManagerTest::TearDownTestCase(void) { }

/*
 * @tc.name: ClientTransCloseChannelProxyTest001
 * @tc.desc: test ClientTransCloseChannel with proxy channel type returns SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransCloseChannelProxyTest001, TestSize.Level1)
{
    int32_t ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_CHANNEL_ID + 1, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: ClientTransCloseChannelTcpDirectTest001
 * @tc.desc: test ClientTransCloseChannel with tcp direct channel type returns SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransCloseChannelTcpDirectTest001, TestSize.Level1)
{
    int32_t ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_CHANNEL_ID + 1, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: ClientTransCloseChannelAuthTest001
 * @tc.desc: test ClientTransCloseChannel with auth channel type returns SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransCloseChannelAuthTest001, TestSize.Level1)
{
    int32_t ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_CHANNEL_ID + 1, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: ClientTransCloseChannelInvalidTypeTest001
 * @tc.desc: test ClientTransCloseChannel with invalid channel types returns error
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransCloseChannelInvalidTypeTest001, TestSize.Level1)
{
    int32_t ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_BUTT, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_TYPE, ret);
    ret = ClientTransCloseChannel(TEST_CHANNEL_ID, CHANNEL_TYPE_UNDEFINED, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_TYPE, ret);
}

/*
 * @tc.name: ClientTransCloseChannelInvalidIdTest001
 * @tc.desc: test ClientTransCloseChannel with invalid channel id returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransCloseChannelInvalidIdTest001, TestSize.Level1)
{
    int32_t ret = ClientTransCloseChannel(INVALID_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransCloseChannel(INVALID_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransCloseChannel(INVALID_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: ClientTransChannelSendBytesInvalidParamTest001
 * @tc.desc: test ClientTransChannelSendBytes with null data and zero len returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendBytesInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, nullptr, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: ClientTransChannelSendBytesAuthTest001
 * @tc.desc: test ClientTransChannelSendBytes with auth channel type returns SOFTBUS_TRANS_SESSION_SERVER_NOINIT
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendBytesAuthTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_SERVER_NOINIT, ret);
}

/*
 * @tc.name: ClientTransChannelSendBytesProxyTest001
 * @tc.desc: test ClientTransChannelSendBytes with proxy channel type returns SOFTBUS_TRANS_SESSION_SERVER_NOINIT
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendBytesProxyTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_SERVER_NOINIT, ret);
}

/*
 * @tc.name: ClientTransChannelSendBytesTcpDirectTest001
 * @tc.desc: test ClientTransChannelSendBytes with tcp direct channel type returns SOFTBUS_TRANS_SESSION_SERVER_NOINIT
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendBytesTcpDirectTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_SERVER_NOINIT, ret);
}

/*
 * @tc.name: ClientTransChannelSendBytesInvalidTypeTest001
 * @tc.desc: test ClientTransChannelSendBytes with invalid channel type returns SOFTBUS_TRANS_SESSION_SERVER_NOINIT
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendBytesInvalidTypeTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_BUTT, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_SERVER_NOINIT, ret);
}

/*
 * @tc.name: ClientTransChannelAsyncSendBytesInvalidParamTest001
 * @tc.desc: test ClientTransChannelAsyncSendBytes with null data and zero len returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendBytesInvalidParamTest001, TestSize.Level1)
{
    uint32_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendBytes(
        TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, nullptr, TEST_DATA_LENGTH, dataSeq);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelAsyncSendBytes(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, 0, dataSeq);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: ClientTransChannelAsyncSendBytesAuthTest001
 * @tc.desc: test ClientTransChannelAsyncSendBytes with auth channel type returns SOFTBUS_TRANS_INVALID_CHANNEL_TYPE
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendBytesAuthTest001, TestSize.Level1)
{
    uint32_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendBytes(
        TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, TEST_DATA_LENGTH, dataSeq);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_TYPE, ret);
}

/*
 * @tc.name: ClientTransChannelAsyncSendBytesProxyTest001
 * @tc.desc: test ClientTransChannelAsyncSendBytes with proxy channel type
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendBytesProxyTest001, TestSize.Level1)
{
    uint32_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendBytes(
        TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_DATA, TEST_DATA_LENGTH, dataSeq);
#ifdef DSOFTBUS_FEATURE_PROXY_CHANNEL
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND, ret);
#else
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT, ret);
#endif
}

/*
 * @tc.name: ClientTransChannelAsyncSendBytesTcpDirectTest001
 * @tc.desc: test ClientTransChannelAsyncSendBytes with tcp direct channel type
 *           returns SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendBytesTcpDirectTest001, TestSize.Level1)
{
    uint32_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendBytes(
        TEST_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_DATA, TEST_DATA_LENGTH, dataSeq);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);
}

/*
 * @tc.name: ClientTransChannelSendMessageInvalidParamTest001
 * @tc.desc: test ClientTransChannelSendMessage with null data returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendMessageInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, nullptr, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, nullptr, 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: ClientTransChannelSendMessageAuthTest001
 * @tc.desc: test ClientTransChannelSendMessage with auth channel type returns SOFTBUS_PERMISSION_DENIED
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendMessageAuthTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: ClientTransChannelSendMessageProxyTest001
 * @tc.desc: test ClientTransChannelSendMessage with proxy channel type
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendMessageProxyTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_DATA, TEST_DATA_LENGTH);
#ifdef DSOFTBUS_FEATURE_PROXY_CHANNEL
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND, ret);
#else
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT, ret);
#endif
}

/*
 * @tc.name: ClientTransChannelSendMessageTcpDirectTest001
 * @tc.desc: test ClientTransChannelSendMessage with tcp direct channel type
 *           returns SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendMessageTcpDirectTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);
}

/*
 * @tc.name: ClientTransChannelSendMessageInvalidTypeTest001
 * @tc.desc: test ClientTransChannelSendMessage with invalid channel type returns error
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendMessageInvalidTypeTest001, TestSize.Level1)
{
    int32_t ret = ClientTransChannelSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_BUTT, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_CHANNEL_TYPE_INVALID, ret);
}

/*
 * @tc.name: ClientTransChannelSendStreamInvalidParamTest001
 * @tc.desc: test ClientTransChannelSendStream with null data, ext or param returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendStreamInvalidParamTest001, TestSize.Level1)
{
    const StreamData data = {};
    const StreamData ext = {};
    const StreamFrameInfo param = {};
    int32_t ret = ClientTransChannelSendStream(TEST_CHANNEL_ID, CHANNEL_TYPE_UDP, nullptr, &ext, &param);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelSendStream(TEST_CHANNEL_ID, CHANNEL_TYPE_UDP, &data, nullptr, &param);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelSendStream(TEST_CHANNEL_ID, CHANNEL_TYPE_UDP, &data, &ext, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: ClientTransChannelSendStreamUdpTest001
 * @tc.desc: test ClientTransChannelSendStream with UDP channel type returns SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendStreamUdpTest001, TestSize.Level1)
{
    const StreamData data = {};
    const StreamData ext = {};
    const StreamFrameInfo param = {};
    int32_t ret = ClientTransChannelSendStream(TEST_CHANNEL_ID, CHANNEL_TYPE_UDP, &data, &ext, &param);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);
}

/*
 * @tc.name: ClientTransChannelSendStreamInvalidTypeTest001
 * @tc.desc: test ClientTransChannelSendStream with invalid channel type returns error
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendStreamInvalidTypeTest001, TestSize.Level1)
{
    const StreamData data = {};
    const StreamData ext = {};
    const StreamFrameInfo param = {};
    int32_t ret = ClientTransChannelSendStream(TEST_CHANNEL_ID, CHANNEL_TYPE_BUTT, &data, &ext, &param);
    EXPECT_EQ(SOFTBUS_TRANS_CHANNEL_TYPE_INVALID, ret);
}

#ifdef DSOFTBUS_FEATURE_TRANS_FILE
/*
 * @tc.name: ClientTransChannelSendFileUdpTest001
 * @tc.desc: test ClientTransChannelSendFile with UDP channel type returns SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendFileUdpTest001, TestSize.Level1)
{
    const char *sFileList[] = { TEST_FILE_PATH };
    const char *dFileList[] = { TEST_FILE_PATH };
    int32_t ret = ClientTransChannelSendFile(TEST_CHANNEL_ID, CHANNEL_TYPE_UDP, sFileList, dFileList, 1);
    EXPECT_EQ(SOFTBUS_TRANS_UDP_GET_CHANNEL_FAILED, ret);
}

/*
 * @tc.name: ClientTransChannelSendFileProxyTest001
 * @tc.desc: test ClientTransChannelSendFile with proxy channel type returns SOFTBUS_TRANS_SESSION_SERVER_NOINIT
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendFileProxyTest001, TestSize.Level1)
{
    const char *sFileList[] = { TEST_FILE_PATH };
    const char *dFileList[] = { TEST_FILE_PATH };
    int32_t ret = ClientTransChannelSendFile(TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, sFileList, dFileList, 1);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_SERVER_NOINIT, ret);
}

/*
 * @tc.name: ClientTransChannelSendFileInvalidTypeTest001
 * @tc.desc: test ClientTransChannelSendFile with invalid channel type returns error
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelSendFileInvalidTypeTest001, TestSize.Level1)
{
    const char *sFileList[] = { TEST_FILE_PATH };
    const char *dFileList[] = { TEST_FILE_PATH };
    int32_t ret = ClientTransChannelSendFile(TEST_CHANNEL_ID, CHANNEL_TYPE_BUTT, sFileList, dFileList, 1);
    EXPECT_EQ(SOFTBUS_TRANS_CHANNEL_TYPE_INVALID, ret);
}
#endif

/*
 * @tc.name: ClientTransChannelAsyncSendMessageInvalidParamTest001
 * @tc.desc: test ClientTransChannelAsyncSendMessage with null data, zero len and zero seq
 *           returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendMessageInvalidParamTest001, TestSize.Level1)
{
    uint16_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendMessage(
        TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, nullptr, TEST_DATA_LENGTH, dataSeq);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelAsyncSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, 0, dataSeq);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransChannelAsyncSendMessage(TEST_CHANNEL_ID, CHANNEL_TYPE_AUTH, TEST_DATA, TEST_DATA_LENGTH, 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: ClientTransChannelAsyncSendMessageProxyTest001
 * @tc.desc: test ClientTransChannelAsyncSendMessage with proxy channel type
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendMessageProxyTest001, TestSize.Level1)
{
    uint16_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendMessage(
        TEST_CHANNEL_ID, CHANNEL_TYPE_PROXY, TEST_DATA, TEST_DATA_LENGTH, dataSeq);
#ifdef DSOFTBUS_FEATURE_PROXY_CHANNEL
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND, ret);
#else
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT, ret);
#endif
}

/*
 * @tc.name: ClientTransChannelAsyncSendMessageTcpDirectTest001
 * @tc.desc: test ClientTransChannelAsyncSendMessage with tcp direct channel type
 *           returns SOFTBUS_TRANS_INVALID_CHANNEL_TYPE
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransChannelAsyncSendMessageTcpDirectTest001, TestSize.Level1)
{
    uint16_t dataSeq = 1;
    int32_t ret = ClientTransChannelAsyncSendMessage(
        TEST_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, TEST_DATA, TEST_DATA_LENGTH, dataSeq);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_TYPE, ret);
}

/*
 * @tc.name: ClientTransCloseReserveChannelInvalidParamTest001
 * @tc.desc: test ClientTransCloseReserveChannel with invalid channelId returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(ClientTransChannelManagerTest, ClientTransCloseReserveChannelInvalidParamTest001, TestSize.Level1)
{
    int32_t ret = ClientTransCloseReserveChannel(
        INVALID_CHANNEL_ID, CHANNEL_TYPE_AUTH, INVALID_CHANNEL_ID, true);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransCloseReserveChannel(
        INVALID_CHANNEL_ID, CHANNEL_TYPE_TCP_DIRECT, INVALID_CHANNEL_ID, false);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransCloseReserveChannel(-1, CHANNEL_TYPE_UDP, 0, true);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}
} // namespace OHOS
