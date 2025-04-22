/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <sys/socket.h>

#include "client_trans_tcp_direct_manager.c"
#include "client_trans_tcp_direct_manager.h"
#include "client_trans_tcp_direct_message.c"
#include "client_trans_tcp_direct_listener.c"
#include "client_trans_tcp_direct_message.h"
#include "client_trans_session_manager.h"
#include "client_trans_socket_manager.h"
#include "client_trans_session_callback.h"
#include "session.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "softbus_app_info.h"
#include "softbus_feature_config.h"
#include "softbus_access_token_test.h"
#include "softbus_conn_interface.h"
#include "softbus_socket.h"
#include "trans_tcp_direct_mock.h"
#include "trans_tcp_process_data.h"
#include "trans_tcp_process_data.c"

#define MAX_LEN 2048
#define TEST_FD 10
#define COUNT 11
#define PKG_LEN 32
#define RECV_BUF "testABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00"
#define BUF_LEN 10
#define COUNT 11
#define SESSIONKEY_LEN 32
#define INVALID_VALUE (-1)

#define TRANS_TEST_SESSION_ID 10
#define TRANS_TEST_CHANNEL_ID 1000
#define TRANS_TEST_FILE_ENCRYPT 10
#define TRANS_TEST_ALGORITHM 1
#define TRANS_TEST_CRC 1
#define TRANS_TEST_FD 1000
#define TRANS_TEST_ADDR_INFO_NUM 2
#define TRANS_TEST_INVALID_SESSION_ID (-1)

using namespace testing;
using namespace testing::ext;

namespace OHOS {
const char *g_pkgName = "dms";
const char *g_sessionName = "ohos.distributedschedule.dms.test";
const char *g_networkid = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00";
const char *g_deviceId = "ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF00ABCDEF0";
const char *g_groupId = "TEST_GROUP_ID";
static const char *g_sessionkey = "clientkey";
static int32_t g_fd = socket(AF_INET, SOCK_STREAM, 0);
static SessionAttribute g_sessionAttr = {
    .dataType = TYPE_BYTES,
};

class TransTcpDirectTest : public testing::Test {
public:
    TransTcpDirectTest()
    {}
    ~TransTcpDirectTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void TransTcpDirectTest::SetUpTestCase(void)
{
    int32_t ret = TransClientInit();
    ASSERT_EQ(ret,  SOFTBUS_OK);
    SetAccessTokenPermission("dsoftbusTransTest");
}

void TransTcpDirectTest::TearDownTestCase(void)
{
}

static int32_t OnSessionOpened(int32_t sessionId, int32_t result)
{
    TRANS_LOGI(TRANS_TEST, "session opened, sessionId=%{public}d", sessionId);
    return SOFTBUS_OK;
}

static void OnSessionClosed(int32_t sessionId)
{
    TRANS_LOGI(TRANS_TEST, "session closed, sessionId=%{public}d", sessionId);
}

static void OnBytesReceived(int32_t sessionId, const void *data, unsigned int len)
{
    TRANS_LOGI(TRANS_TEST, "session bytes received, sessionId=%{public}d", sessionId);
}

static void OnMessageReceived(int32_t sessionId, const void *data, unsigned int len)
{
    TRANS_LOGI(TRANS_TEST, "session msg received, sessionId=%{public}d", sessionId);
}

static ISessionListener g_sessionlistener = {
    .OnSessionOpened = OnSessionOpened,
    .OnSessionClosed = OnSessionClosed,
    .OnBytesReceived = OnBytesReceived,
    .OnMessageReceived = OnMessageReceived,
};

/**
 * @tc.name: CreateSessionServerTest001
 * @tc.desc: extern module active publish, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:I5HQGA
 */
HWTEST_F(TransTcpDirectTest, CreateSessionServerTest001, TestSize.Level1)
{
    int32_t ret;
    ret = CreateSessionServer(nullptr, g_sessionName, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = CreateSessionServer(g_pkgName, nullptr, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = CreateSessionServer(g_pkgName, g_sessionName, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = RemoveSessionServer(g_pkgName, g_sessionName);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: CreateSessionServerTest002
 * @tc.desc: extern module active publish, use the normal parameter.
 * @tc.type: FUNC
 * @tc.require:I5HQGA
 */
HWTEST_F(TransTcpDirectTest, CreateSessionServerTest002, TestSize.Level1)
{
    int32_t ret;
    ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = RemoveSessionServer(g_pkgName, g_sessionName);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: CreateSessionServerTest003
 * @tc.desc: extern module active publish, use the same normal parameter.
 * @tc.type: FUNC
 * @tc.require:I5HQGA
 */
HWTEST_F(TransTcpDirectTest, CreateSessionServerTest003, TestSize.Level1)
{
    int32_t ret;
    ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = RemoveSessionServer(g_pkgName, g_sessionName);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: CreateSessionServerTest004
 * @tc.desc: extern module active publish, create 11 sessionServer, succ 10, failed at 11th.
 * @tc.type: FUNC
 * @tc.require:I5HQGA
 */
HWTEST_F(TransTcpDirectTest, CreateSessionServerTest004, TestSize.Level1)
{
    int32_t ret, i;
    char const *sessionName[MAX_SESSION_SERVER_NUMBER + 1] = {
        "ohos.distributedschedule.dms.test0",
        "ohos.distributedschedule.dms.test1",
        "ohos.distributedschedule.dms.test2",
        "ohos.distributedschedule.dms.test3",
        "ohos.distributedschedule.dms.test4",
        "ohos.distributedschedule.dms.test5",
        "ohos.distributedschedule.dms.test6",
        "ohos.distributedschedule.dms.test7",
        "ohos.distributedschedule.dms.test8",
        "ohos.distributedschedule.dms.test9",
        "ohos.distributedschedule.dms.test10"
    };

    for (i = 0; i < COUNT; i++) {
        ret = CreateSessionServer(g_pkgName, sessionName[i], &g_sessionlistener);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    for (i = COUNT; i < MAX_SESSION_SERVER_NUMBER; i++) {
        ret = CreateSessionServer(g_pkgName, sessionName[i], &g_sessionlistener);
        EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    }
    ret = CreateSessionServer(g_pkgName, sessionName[i], &g_sessionlistener);
    EXPECT_NE(SOFTBUS_OK, ret);

    for (i = 0; i < COUNT; i++) {
        ret = RemoveSessionServer(g_pkgName, sessionName[i]);
        EXPECT_EQ(SOFTBUS_OK, ret);
    }
    for (i = COUNT; i < MAX_SESSION_SERVER_NUMBER; i++) {
        ret = RemoveSessionServer(g_pkgName, sessionName[i]);
        EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    }
}

/**
 * @tc.name: RemoveSessionServerTest001
 * @tc.desc: extern module active publish, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, RemoveSessionServerTest001, TestSize.Level1)
{
    int32_t ret;
    ret = RemoveSessionServer(nullptr, g_sessionName);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = RemoveSessionServer(g_pkgName, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: RemoveSessionServerTest002
 * @tc.desc: extern module active publish, use the same parameter.
 * @tc.type: FUNC
 * @tc.require:I5HQGA
 */
HWTEST_F(TransTcpDirectTest, RemoveSessionServerTest002, TestSize.Level1)
{
    int32_t ret;
    ret = CreateSessionServer(g_pkgName, g_sessionName, &g_sessionlistener);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = RemoveSessionServer(g_pkgName, g_sessionName);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = RemoveSessionServer(g_pkgName, g_sessionName);
    EXPECT_EQ(SOFTBUS_TRANS_CHECK_PID_ERROR, ret);
}

/**
 * @tc.name: OpenSessionTest001
 * @tc.desc: extern module active publish, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:I5HQGA
 */
HWTEST_F(TransTcpDirectTest, OpenSessionTest001, TestSize.Level1)
{
    int32_t ret;
    g_sessionAttr.dataType = TYPE_BYTES;

    ret = OpenSession(nullptr, g_sessionName, g_networkid, g_groupId, &g_sessionAttr);
    EXPECT_GE(SOFTBUS_OK, ret);

    ret = OpenSession(g_sessionName, nullptr, g_networkid, g_groupId, &g_sessionAttr);
    EXPECT_GE(SOFTBUS_OK, ret);

    ret = OpenSession(g_sessionName, g_sessionName, nullptr, g_groupId, &g_sessionAttr);
    EXPECT_GE(SOFTBUS_OK, ret);

    ret = OpenSession(g_sessionName, g_sessionName, g_networkid, nullptr, &g_sessionAttr);
    EXPECT_GE(SOFTBUS_OK, ret);

    ret = OpenSession(g_sessionName, g_sessionName, g_networkid, g_groupId, nullptr);
    EXPECT_GE(SOFTBUS_OK, ret);

    g_sessionAttr.dataType = TYPE_BUTT;
    ret = OpenSession(g_sessionName, g_sessionName, g_networkid, g_groupId, &g_sessionAttr);
    g_sessionAttr.dataType = TYPE_BYTES;
    EXPECT_GE(SOFTBUS_OK, ret);
}

/**
 * @tc.name: SendBytesTest001
 * @tc.desc: extern module active publish, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, SendBytesTest001, TestSize.Level1)
{
    int32_t ret;
    int32_t sessionId = 1;
    const char *data = "testdata";
    uint32_t len = strlen(data);
    uint32_t maxLen;

    ret = SendBytes(-1, data, len);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = SendBytes(sessionId, nullptr, len);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = SendBytes(sessionId, data, 0);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = SoftbusGetConfig(SOFTBUS_INT_MAX_BYTES_LENGTH, (unsigned char *)&maxLen, sizeof(maxLen));
    ASSERT_EQ(SOFTBUS_OK, ret);
    ret = SendMessage(sessionId, data, maxLen + 1);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/**
 * @tc.name: SendMessageTest001
 * @tc.desc: extern module active publish, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, SendMessageTest001, TestSize.Level1)
{
    int32_t ret;
    int32_t sessionId = 1;
    const char *data = "testdata";
    uint32_t len = strlen(data);
    uint32_t maxLen;

    ret = SendMessage(-1, data, len);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = SendMessage(sessionId, nullptr, len);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = SendMessage(sessionId, data, 0);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = SoftbusGetConfig(SOFTBUS_INT_MAX_MESSAGE_LENGTH, (unsigned char *)&maxLen, sizeof(maxLen));
    ASSERT_EQ(SOFTBUS_OK, ret);
    ret = SendMessage(sessionId, data, maxLen + 1);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/**
 * @tc.name: TransClientGetTdcDataBufByChannelTest001
 * @tc.desc: improve branch coverage, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransClientGetTdcDataBufByChannelTest001, TestSize.Level1)
{
    int32_t ret;
    int32_t channelId = 0;
    int32_t fd = TEST_FD;
    size_t len = BUF_LEN;

    ret = TransClientGetTdcDataBufByChannel(channelId, nullptr, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransClientGetTdcDataBufByChannel(channelId, &fd, &len);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);

    ret = TransDataListInit();
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransClientGetTdcDataBufByChannel(channelId, &fd, &len);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);

    ret = TransAddDataBufNode(channelId, fd);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransClientGetTdcDataBufByChannel(channelId, &fd, &len);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransDelDataBufNode(channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);

    TransDataListDeinit();
}

/**
 * @tc.name: TransClientUpdateTdcDataBufWInfoTest001
 * @tc.desc: improve branch coverage, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransClientUpdateTdcDataBufWInfoTest001, TestSize.Level1)
{
    int32_t ret;
    int32_t channelId = 0;
    int32_t fd = TEST_FD;
    const char *recvBuf = RECV_BUF;
    int32_t recvLen = MAX_LEN;

    ret = TransClientUpdateTdcDataBufWInfo(channelId, nullptr, recvLen);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransClientUpdateTdcDataBufWInfo(channelId, const_cast<char *>(recvBuf), recvLen);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    ret = TransDataListInit();
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransClientUpdateTdcDataBufWInfo(channelId, const_cast<char *>(recvBuf), recvLen);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);

    ret = TransAddDataBufNode(channelId, fd);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransClientUpdateTdcDataBufWInfo(channelId, const_cast<char *>(recvBuf), recvLen);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_DATA_LENGTH, ret);

    recvLen = strlen(recvBuf);
    ret = TransClientUpdateTdcDataBufWInfo(channelId, const_cast<char *>(recvBuf), recvLen);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransDelDataBufNode(channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);

    TransDataListDeinit();
}

/**
 * @tc.name: TransTdcRecvDataTest001
 * @tc.desc: improve branch coverage, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcRecvDataTest001, TestSize.Level1)
{
    int32_t ret;
    int32_t channelId = -1;
    int32_t fd = -1;

    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    channelId = 0;
    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    NiceMock<TransTcpDirectInterfaceMock> tcpDirectMock;
    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_EINTR));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_EINTR, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_BAD_FD));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_BAD_FD, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_EAGAIN));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_EAGAIN, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_ADDR_ERR));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_ADDR_ERR, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_RESOURCE_BUSY));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_RESOURCE_BUSY, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_INVALID_VARIABLE));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_INVALID_VARIABLE, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_TOO_MUCH_FILE));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_TOO_MUCH_FILE, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_FULL_FD));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_FULL_FD, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NO_SPACE_LEFT));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NO_SPACE_LEFT, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_PIPE_INTER));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_PIPE_INTER, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NOT_SOCKET));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NOT_SOCKET, ret);
    testing::Mock::VerifyAndClearExpectations(&tcpDirectMock);

    ret = TransDelDataBufNode(channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);

    TransDataListDeinit();
}

/**
 * @tc.name: TransTdcRecvDataTest001_1
 * @tc.desc: improve branch coverage, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcRecvDataTest001_1, TestSize.Level1)
{
    int32_t ret;
    int32_t channelId = -1;
    int32_t fd = -1;

    ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    channelId = 0;
    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    NiceMock<TransTcpDirectInterfaceMock> tcpDirectMock;
    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_OPTION_UNKNOWN));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_OPTION_UNKNOWN, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_ADDR_IN_USE));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_ADDR_IN_USE, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_ADDR_NOT_AVAIL));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_ADDR_NOT_AVAIL, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NET_DOWN));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NET_DOWN, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_NET_REACH));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_NET_REACH, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NET_RESET));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NET_RESET, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_CONN_RESET));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_CONN_RESET, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NO_BUFS));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NO_BUFS, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_IS_CONN));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_IS_CONN, ret);
    testing::Mock::VerifyAndClearExpectations(&tcpDirectMock);

    ret = TransDelDataBufNode(channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);

    TransDataListDeinit();
}

/**
 * @tc.name: TransTdcRecvDataTest001_2
 * @tc.desc: improve branch coverage, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcRecvDataTest001_2, TestSize.Level1)
{
    int32_t ret;
    int32_t channelId = -1;
    int32_t fd = -1;

    ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    channelId = 0;
    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    NiceMock<TransTcpDirectInterfaceMock> tcpDirectMock;
    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NOT_CONN));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NOT_CONN, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_TIME_OUT));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_TIME_OUT, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_REFUSED));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_REFUSED, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_HOST_DOWN));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_HOST_DOWN, ret);

    EXPECT_CALL(tcpDirectMock, GetErrCodeBySocketErr).WillOnce(Return(SOFTBUS_CONN_SOCKET_NO_ROUTE_AVALIABLE));
    ret = TransTdcRecvData(channelId);
    EXPECT_EQ(SOFTBUS_CONN_SOCKET_NO_ROUTE_AVALIABLE, ret);
    testing::Mock::VerifyAndClearExpectations(&tcpDirectMock);

    ret = TransDelDataBufNode(channelId);
    EXPECT_EQ(SOFTBUS_OK, ret);

    TransDataListDeinit();
}

/**
 * @tc.name: TransTdcPackDataTest001
 * @tc.desc: TransTdcPackData, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcPackDataTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *channel = (TcpDirectChannelInfo *)SoftBusMalloc(sizeof(TcpDirectChannelInfo));
    ASSERT_TRUE(channel != nullptr);
    (void)memset_s(channel, sizeof(TcpDirectChannelInfo), 0, sizeof(TcpDirectChannelInfo));
    channel->channelId = 1;
    (void)memcpy_s(channel->detail.sessionKey, SESSIONKEY_LEN, g_sessionkey, strlen(g_sessionkey));
    channel->detail.channelType = CHANNEL_TYPE_TCP_DIRECT;
    channel->detail.fd = g_fd;
    channel->detail.sequence = 1;
    const char *data = "data";
    uint32_t len = BUF_LEN;
    int32_t flags = FLAG_ACK;
    char *ret = TransTdcPackData(channel, data, len, flags, nullptr);
    EXPECT_TRUE(ret == nullptr);
    SoftBusFree(channel);
}

/**
 * @tc.name: TransTdcProcessPostDataTest001
 * @tc.desc: TransTdcProcessPostData, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcessPostDataTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *channel = (TcpDirectChannelInfo *)SoftBusMalloc(sizeof(TcpDirectChannelInfo));
    ASSERT_TRUE(channel != nullptr);
    (void)memset_s(channel, sizeof(TcpDirectChannelInfo), 0, sizeof(TcpDirectChannelInfo));
    channel->channelId = 1;
    (void)memcpy_s(channel->detail.sessionKey, SESSIONKEY_LEN, g_sessionkey, strlen(g_sessionkey));
    channel->detail.channelType = CHANNEL_TYPE_TCP_DIRECT;
    channel->detail.fd = g_fd;
    channel->detail.sequence = 1;
    const char *data = "data";
    uint32_t len = BUF_LEN;
    int32_t flags = FLAG_ACK;
    int32_t ret = TransTdcProcessPostData(channel, data, len, flags);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    SoftBusFree(channel);
}

/**
 * @tc.name: TransTdcDecryptTest001
 * @tc.desc: TransTdcDecrypt, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcDecryptTest001, TestSize.Level1)
{
    char *out = nullptr;
    uint32_t outLen = 0;
    uint32_t inLen = 0;
    int32_t ret = TransTdcDecrypt(nullptr, nullptr, inLen, out, &outLen);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: TransTdcEncryptWithSeqTest001
 * @tc.desc: TransTdcEncryptWithSeq, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcEncryptWithSeqTest001, TestSize.Level1)
{
    const char *in = "data";
    char *out = nullptr;
    uint32_t outLen = 0;
    uint32_t inLen = (uint32_t)strlen(in);
    int32_t seqNum = BUF_LEN;
    EncrptyInfo enInfo = {
        .in = in,
        .inLen = inLen,
        .out = out,
        .outLen = &outLen,
    };
    int32_t ret = TransTdcEncryptWithSeq(nullptr, seqNum, &enInfo);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    ret = TransTdcEncryptWithSeq(g_sessionkey, seqNum, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: TransTdcSetPendingPacketTest001
 * @tc.desc: TransTdcSetPendingPacket, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcSetPendingPacketTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    const char *data = "data";
    uint32_t len = 0;
    int32_t seqNum = 1;
    int32_t type = 1;
    int32_t ret = TransTdcSetPendingPacket(channelId, data, len, 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = PendingInit(type);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
    len = ACK_SIZE;
    channelId = INVALID_VALUE;
    ret = TransTdcSetPendingPacket(channelId, data, len, 0);
    EXPECT_EQ(SOFTBUS_TRANS_NODE_NOT_FOUND, ret);
    channelId = 1;
    ret = TransTdcSetPendingPacket(channelId, data, len, 0);
    EXPECT_EQ(SOFTBUS_TRANS_NODE_NOT_FOUND, ret);
    PendingDeinit(type);
}

/**
 * @tc.name: TransTdcSendAckTest001
 * @tc.desc: TransTdcSendAck, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcSendAckTest001, TestSize.Level1)
{
    int32_t seq = 1;
    int32_t channelId = -1;
    int32_t ret = TransTdcSendAck(channelId, seq);
    EXPECT_EQ(ret, SOFTBUS_TRANS_TDC_GET_INFO_FAILED);
}

/**
 * @tc.name: TransGetDataBufSizeTest001
 * @tc.desc: TransGetDataBufSize, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransGetDataBufSizeTest001, TestSize.Level1)
{
    uint32_t ret = TransGetDataBufSize();
    EXPECT_EQ(ret, MIN_BUF_LEN);

    int32_t res = TransGetTdcDataBufMaxSize();
    EXPECT_EQ(res, SOFTBUS_OK);
}

/**
 * @tc.name: TransDestroyDataBufTest001
 * @tc.desc: TransDestroyDataBuf, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransDestroyDataBufTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t fd = TEST_FD;
    int32_t ret = TransDestroyDataBuf();
    EXPECT_EQ(ret, SOFTBUS_NO_INIT);

    ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransDestroyDataBuf();
    EXPECT_EQ(ret, SOFTBUS_OK);
    TransDataListDeinit();
}

/**
 * @tc.name: TransGetDataBufNodeByIdTest001
 * @tc.desc: TransGetDataBufNodeById, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransGetDataBufNodeByIdTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t fd = TEST_FD;
    DataBuf *data = TransGetDataBufNodeById(channelId);
    EXPECT_TRUE(data == nullptr);

    int32_t ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    data = TransGetDataBufNodeById(channelId);
    EXPECT_TRUE(data != nullptr);

    TransDataListDeinit();
}

/**
 * @tc.name: TransTdcProcessDataByFlagTest001
 * @tc.desc: TransTdcProcessDataByFlag, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcessDataByFlagTest001, TestSize.Level1)
{
    uint32_t flag = FLAG_BYTES;
    int32_t seqNum = 1;
    const char *plain = "plain";
    uint32_t plainLen = 0;
    flag = FLAG_ACK;
    int32_t ret = TransTdcProcessDataByFlag(flag, seqNum, nullptr, plain, plainLen);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: TransTdcProcessDataTest001
 * @tc.desc: TransTdcProcessData, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcessDataTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t fd = TEST_FD;
    ChannelInfo *info = (ChannelInfo *)SoftBusCalloc(sizeof(ChannelInfo));
    ASSERT_TRUE(info != nullptr);
    info->peerSessionName = (char *)g_sessionName;
    info->channelId = 1;
    info->channelType = CHANNEL_TYPE_TCP_DIRECT;
    info->sessionKey = (char *)g_sessionkey;
    info->fd = g_fd;

    int32_t ret = TransTdcProcessData(channelId);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);
    IClientSessionCallBack *cb = GetClientSessionCb();
    ret = TransTdcManagerInit(cb);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransTdcProcessData(channelId);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);

    ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransTdcProcessData(channelId);
    EXPECT_NE(SOFTBUS_OK, ret);

    TransDataListDeinit();
    TransTdcManagerDeinit();
    SoftBusFree(info);
}

/**
 * @tc.name: TransResizeDataBufferTest001
 * @tc.desc: TransResizeDataBuffer, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransResizeDataBufferTest001, TestSize.Level1)
{
    DataBuf *oldBuf = (DataBuf *)SoftBusCalloc(sizeof(DataBuf));
    ASSERT_TRUE(oldBuf != nullptr);
    (void)memset_s(oldBuf, sizeof(DataBuf), 0, sizeof(DataBuf));
    int32_t ret = TransResizeDataBuffer(oldBuf, PKG_LEN);
    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);

    (void)memcpy_s(oldBuf->data, strlen("data"), "data", strlen("data"));
    oldBuf->size = BUF_LEN;
    (void)memcpy_s(oldBuf->w, strlen("oldbulf"), "oldbulf", strlen("oldbulf"));

    ret = TransResizeDataBuffer(oldBuf, PKG_LEN);
    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
    SoftBusFree(oldBuf);
}

 /**
 * @tc.name: TransTdcProcAllDataTest001
 * @tc.desc: TransTdcProcAllData, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcAllDataTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t fd = TEST_FD;
    DataBuf *oldBuf = (DataBuf *)SoftBusCalloc(sizeof(DataBuf));
    ASSERT_TRUE(oldBuf != nullptr);
    (void)memset_s(oldBuf, sizeof(DataBuf), 0, sizeof(DataBuf));

    int32_t ret = TransTdcProcAllData(channelId);
    EXPECT_EQ(ret, SOFTBUS_NO_INIT);

    ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransAddDataBufNode(channelId, fd);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransTdcProcAllData(channelId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    TransDataListDeinit();
    SoftBusFree(oldBuf);
}

/**
 * @tc.name: ClientTdcOnConnectEventTest001
 * @tc.desc: ClientTdcOnConnectEvent, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, ClientTdcOnConnectEventTest001, TestSize.Level1)
{
    int32_t cfd = 0;
    int32_t ret = ClientTdcOnConnectEvent(DIRECT_CHANNEL_SERVER_WIFI, cfd, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: ClientTdcOnDataEventTest001
 * @tc.desc: ClientTdcOnDataEvent, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, ClientTdcOnDataEventTest001, TestSize.Level1)
{
    int32_t events = SOFTBUS_SOCKET_IN;
    int32_t fd = g_fd;
    ChannelInfo *info = (ChannelInfo *)SoftBusCalloc(sizeof(ChannelInfo));
    ASSERT_TRUE(info != nullptr);
    (void)memset_s(info, sizeof(ChannelInfo), 0, sizeof(ChannelInfo));
    info->peerSessionName = (char *)g_sessionName;
    info->channelId = 1;
    info->channelType = CHANNEL_TYPE_TCP_DIRECT;
    info->sessionKey = (char *)g_sessionkey;
    info->fd = g_fd;
    IClientSessionCallBack *cb = GetClientSessionCb();
    int32_t ret = TransTdcManagerInit(cb);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = ClientTdcOnDataEvent(DIRECT_CHANNEL_SERVER_WIFI, events, fd);
    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
}

/**
 * @tc.name: TransGetNewTcpChannelTest001
 * @tc.desc: TransGetNewTcpChannel, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransGetNewTcpChannelTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    TcpDirectChannelInfo *info = TransGetNewTcpChannel(nullptr);
    ASSERT_EQ(info, nullptr);
    ChannelInfo *channelInfo = (ChannelInfo *)SoftBusCalloc(sizeof(ChannelInfo));
    ASSERT_TRUE(channelInfo != nullptr);
    (void)memset_s(channelInfo, sizeof(ChannelInfo), 0, sizeof(ChannelInfo));
    channelInfo->peerSessionName = (char *)g_sessionName;
    channelInfo->channelId = 1;
    channelInfo->channelType = CHANNEL_TYPE_TCP_DIRECT;
    channelInfo->sessionKey = (char *)g_sessionkey;
    channelInfo->fd = g_fd;

    IClientSessionCallBack *cb = GetClientSessionCb();
    int32_t ret = TransTdcManagerInit(cb);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = ClientTransCheckTdcChannelExist(channelId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftBusFree(channelInfo);
}

/**
 * @tc.name: TransTdcProcessDataByFlagTest002
 * @tc.desc: TransTdcProcessDataByFlag, use different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcessDataByFlagTest002, TestSize.Level1)
{
    TcpDirectChannelInfo *channel = (TcpDirectChannelInfo *)SoftBusCalloc(sizeof(TcpDirectChannelInfo));
    ASSERT_TRUE(channel != nullptr);

    int32_t ret = memcpy_s(channel->detail.sessionKey, SESSIONKEY_LEN, g_sessionkey, strlen(g_sessionkey));
    ASSERT_EQ(ret, EOK);
    channel->channelId = TRANS_TEST_CHANNEL_ID;
    channel->detail.channelType = CHANNEL_TYPE_TCP_DIRECT;
    channel->detail.fd = TRANS_TEST_FD;
    channel->detail.sequence = 1;
    int32_t seqNum = 1;
    const char *plain = "plain";

    ret = TransTdcProcessDataByFlag(FLAG_BYTES, seqNum, channel, plain, (uint32_t)strlen(plain));
    EXPECT_NE(ret, SOFTBUS_OK);

    ret = TransTdcProcessDataByFlag(FLAG_ACK, seqNum, channel, plain, (uint32_t)strlen(plain));
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = TransTdcProcessDataByFlag(FLAG_MESSAGE, seqNum, channel, plain, (uint32_t)strlen(plain));
    EXPECT_NE(ret, SOFTBUS_OK);

    ret = TransTdcProcessDataByFlag(FILE_FIRST_FRAME, seqNum, channel, plain, (uint32_t)strlen(plain));
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    SoftBusFree(channel);
}

/**
 * @tc.name: TransTdcProcAllDataTest002
 * @tc.desc: TransTdcProcAllData, use different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcAllDataTest002, TestSize.Level1)
{
    int32_t ret = TransDataListInit();
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransAddDataBufNode(TRANS_TEST_CHANNEL_ID, TRANS_TEST_FD);
    ASSERT_EQ(ret, SOFTBUS_OK);
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_OK);

    TransDataListDeinit();
}

/**
 * @tc.name: TransTdcDecryptTest002
 * @tc.desc: TransTdcDecrypt, use different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcDecryptTest002, TestSize.Level1)
{
    char output[MAX_LEN] = {0};
    uint32_t outLen = MAX_LEN;
    int32_t ret = TransTdcDecrypt(g_sessionkey, RECV_BUF, strlen(RECV_BUF) + 1, output, &outLen);
    EXPECT_FALSE(ret == SOFTBUS_OK);
}

/**
 * @tc.name: ClientTransTdcOnChannelOpenedTest001
 * @tc.desc: ClientTransTdcOnChannelOpened, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, ClientTransTdcOnChannelOpenedTest001, TestSize.Level1)
{
    ChannelInfo *info = (ChannelInfo *)SoftBusCalloc(sizeof(ChannelInfo));
    ASSERT_TRUE(info != nullptr);
    info->peerSessionName = (char *)g_sessionName;
    info->channelId = 1;
    info->channelType = CHANNEL_TYPE_TCP_DIRECT;
    info->sessionKey = (char *)g_sessionkey;
    info->fd = g_fd;
    int32_t ret = ClientTransTdcOnChannelOpened(g_sessionName, info);
    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
    SoftBusFree(info);
}

/**
 * @tc.name: ClientTdcOnDataEventTest002
 * @tc.desc: ClientTdcOnDataEvent, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, ClientTdcOnDataEventTest002, TestSize.Level1)
{
    int32_t events = SOFTBUS_SOCKET_OUT;
    int32_t channelId = 1;
    TcpDirectChannelInfo *info = reinterpret_cast<TcpDirectChannelInfo *>(SoftBusCalloc(sizeof(TcpDirectChannelInfo)));
    ASSERT_NE(info, nullptr);

    g_tcpDirectChannelInfoList = CreateSoftBusList();
    ASSERT_NE(g_tcpDirectChannelInfoList, nullptr);

    info->channelId = channelId;
    info->detail.fd = g_fd;

    (void)SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    ListAdd(&g_tcpDirectChannelInfoList->list, &info->node);
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);

    int32_t ret = ClientTdcOnDataEvent(DIRECT_CHANNEL_SERVER_WIFI, events, g_fd);
    EXPECT_EQ(ret, SOFTBUS_OK);

    events = SOFTBUS_SOCKET_EXCEPTION;
    ret = ClientTdcOnDataEvent(DIRECT_CHANNEL_SERVER_WIFI, events, g_fd);
    EXPECT_EQ(ret, SOFTBUS_OK);
    // info is deleted in the abnormal branch
    DestroySoftBusList(g_tcpDirectChannelInfoList);
    g_tcpDirectChannelInfoList = nullptr;
}

/**
 * @tc.name: TransTdcCreateListenerWithoutAddTriggerTest001
 * @tc.desc: TransTdcCreateListenerWithoutAddTrigger, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcCreateListenerWithoutAddTriggerTest001, TestSize.Level1)
{
    g_isInitedFlag = true;
    int32_t fd = g_fd;

    int32_t ret = TransTdcCreateListenerWithoutAddTrigger(fd);
    EXPECT_EQ(ret, SOFTBUS_OK);

    g_isInitedFlag = false;
    ret = TransTdcCreateListenerWithoutAddTrigger(fd);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: TransTdcCloseFdTest001
 * @tc.desc: TransTdcCloseFd, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcCloseFdTest001, TestSize.Level1)
{
    int32_t fd = -1;
    TransTdcCloseFd(fd);
    fd = 1000000;
    NiceMock<TransTcpDirectInterfaceMock> tcpDirectMock;
    EXPECT_CALL(tcpDirectMock, SoftBusSocketGetError).WillOnce(Return(SOFTBUS_CONN_BAD_FD));
    TransTdcCloseFd(fd);
    testing::Mock::VerifyAndClearExpectations(&tcpDirectMock);
    fd = g_fd;
    TransTdcCloseFd(fd);
    EXPECT_TRUE(fd);
}

/**
 * @tc.name: UnPackTcpDataPacketHeadTest001
 * @tc.desc: UnPackTcpDataPacketHead, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, UnPackTcpDataPacketHeadTest001, TestSize.Level1)
{
    TcpDataPacketHead data;
    data.seq = 1;
    UnPackTcpDataPacketHead(&data);
    EXPECT_TRUE(data.seq);
}

/**
 * @tc.name: CheckCollaborationSessionNameTest001
 * @tc.desc: CheckCollaborationSessionName, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, CheckCollaborationSessionNameTest001, TestSize.Level1)
{
    const char *testSessionName = "ohos.collaborationcenter";
    bool ret = CheckCollaborationSessionName(testSessionName);
    EXPECT_EQ(ret, true);
    const char *testSessionName1 = "nullptr";
    ret = CheckCollaborationSessionName(testSessionName1);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: TransTdcProcessPostDataTest002
 * @tc.desc: TransTdcProcessPostData, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcessPostDataTest002, TestSize.Level1)
{
    int32_t channelId = 1;
    ChannelType channelType = CHANNEL_TYPE_TCP_DIRECT;
    const char *data = "data";
    uint32_t len = BUF_LEN;
    int32_t flags = FLAG_ACK;

    TcpDirectChannelInfo *channel = reinterpret_cast<TcpDirectChannelInfo *>(
        SoftBusCalloc(sizeof(TcpDirectChannelInfo)));
    ASSERT_TRUE(channel != nullptr);
    channel->channelId = channelId;
    channel->detail.channelType = channelType;

    ClientSessionServer *serverNode = reinterpret_cast<ClientSessionServer *>(
        SoftBusCalloc(sizeof(ClientSessionServer)));
    ASSERT_NE(serverNode, nullptr);

    SessionInfo *info = reinterpret_cast<SessionInfo *>(SoftBusCalloc(sizeof(SessionInfo)));
    ASSERT_NE(info, nullptr);
    info->channelId = channelId;
    info->channelType = channelType;
    ListInit(&serverNode->sessionList);
    ListAdd(&serverNode->sessionList, &info->node);

    g_tcpDirectChannelInfoList = CreateSoftBusList();
    ASSERT_NE(g_tcpDirectChannelInfoList, nullptr);
    (void)SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    ListAdd(&g_tcpDirectChannelInfoList->list, &serverNode->node);
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);

    int32_t ret = TransTdcProcessPostData(channel, data, len, flags);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    SoftBusFree(channel);
    SoftBusFree(info);
    SoftBusFree(serverNode);
    DestroySoftBusList(g_tcpDirectChannelInfoList);
    g_tcpDirectChannelInfoList = nullptr;
}

/**
 * @tc.name: TransTdcSendBytesTest001
 * @tc.desc: TransTdcSendBytes, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcSendBytesTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    const char *data = "data";
    uint32_t len = BUF_LEN;
    TcpDirectChannelInfo *info = reinterpret_cast<TcpDirectChannelInfo *>(SoftBusCalloc(sizeof(TcpDirectChannelInfo)));
    ASSERT_NE(info, nullptr);
    g_tcpDirectChannelInfoList = CreateSoftBusList();
    ASSERT_NE(g_tcpDirectChannelInfoList, nullptr);

    info->channelId = channelId;
    info->detail.needRelease = true;
    (void)SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    ListAdd(&g_tcpDirectChannelInfoList->list, &info->node);
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);

    int32_t ret = TransTdcSendBytes(channelId, data, len, false);
    EXPECT_EQ(ret, SOFTBUS_TRANS_TDC_CHANNEL_CLOSED_BY_ANOTHER_THREAD);

    info->detail.needRelease = false;
    ret = TransTdcSendBytes(channelId, data, len, false);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    SoftBusFree(info);
    DestroySoftBusList(g_tcpDirectChannelInfoList);
    g_tcpDirectChannelInfoList = nullptr;
}

/**
 * @tc.name: TransTdcSendMessageTest001
 * @tc.desc: TransTdcSendMessage, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcSendMessageTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    uint32_t len = BUF_LEN;
    const char *data = "data";
    TcpDirectChannelInfo *info = reinterpret_cast<TcpDirectChannelInfo *>(SoftBusCalloc(sizeof(TcpDirectChannelInfo)));
    ASSERT_NE(info, nullptr);
    g_tcpDirectChannelInfoList = CreateSoftBusList();
    ASSERT_NE(g_tcpDirectChannelInfoList, nullptr);
    info->channelId = channelId;
    info->detail.needRelease = true;

    (void)SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    ListAdd(&g_tcpDirectChannelInfoList->list, &info->node);
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);

    int32_t ret = TransTdcSendMessage(channelId, data, len);
    EXPECT_EQ(ret, SOFTBUS_TRANS_TDC_CHANNEL_CLOSED_BY_ANOTHER_THREAD);

    info->detail.needRelease = false;
    ret = TransTdcSendMessage(channelId, data, len);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);
    SoftBusFree(info);
    DestroySoftBusList(g_tcpDirectChannelInfoList);
    g_tcpDirectChannelInfoList = nullptr;
}

/**
 * @tc.name: TransTdcProcessDataTest002
 * @tc.desc: TransTdcProcessData, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcessDataTest002, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t ret = TransTdcProcessData(channelId);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND, ret);

    TcpDirectChannelInfo *info = reinterpret_cast<TcpDirectChannelInfo *>(SoftBusCalloc(sizeof(TcpDirectChannelInfo)));
    ASSERT_NE(info, nullptr);
    g_tcpDirectChannelInfoList = CreateSoftBusList();
    ASSERT_NE(g_tcpDirectChannelInfoList, nullptr);
    info->channelId = channelId;
    (void)SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    ListAdd(&g_tcpDirectChannelInfoList->list, &info->node);
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);

    ret = TransTdcProcessData(channelId);
    EXPECT_EQ(SOFTBUS_LOCK_ERR, ret);

    DataBuf *buf = reinterpret_cast<DataBuf *>(SoftBusCalloc(sizeof(DataBuf)));
    ASSERT_NE(buf, nullptr);
    g_tcpDataList = CreateSoftBusList();
    ASSERT_NE(g_tcpDataList, nullptr);
    ret = TransTdcProcessData(channelId);
    EXPECT_EQ(SOFTBUS_TRANS_NODE_NOT_FOUND, ret);

    TcpDataPacketHead *pktHead = reinterpret_cast<TcpDataPacketHead *>(SoftBusCalloc(sizeof(TcpDataPacketHead)));
    ASSERT_NE(pktHead, nullptr);
    pktHead->dataLen = 0;
    pktHead->seq = 0;
    pktHead->flags = 0;
    buf->data = (char*)pktHead;
    buf->channelId = channelId;
    (void)SoftBusMutexLock(&g_tcpDataList->lock);
    ListAdd(&g_tcpDataList->list, &buf->node);
    (void)SoftBusMutexUnlock(&g_tcpDataList->lock);

    ret = TransTdcProcessData(channelId);
    EXPECT_EQ(SOFTBUS_MALLOC_ERR, ret);

    IClientSessionCallBack *cb = GetClientSessionCb();
    ret = TransTdcManagerInit(cb);
    ASSERT_EQ(ret, SOFTBUS_OK);

    ret = TransTdcProcessData(channelId);
    EXPECT_NE(SOFTBUS_OK, ret);
    // pktHead is deleted in the abnormal branch
    SoftBusFree(info);
    SoftBusFree(buf);
    DestroySoftBusList(g_tcpDirectChannelInfoList);
    DestroySoftBusList(g_tcpDataList);
    g_tcpDirectChannelInfoList = nullptr;
    g_tcpDataList = nullptr;
}

/**
 * @tc.name: TransTdcProcAllDataTest003
 * @tc.desc: TransTdcProcAllData, use different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcAllDataTest003, TestSize.Level1)
{
    int32_t ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);;
    EXPECT_EQ(ret, SOFTBUS_NO_INIT);

    DataBuf *buf = reinterpret_cast<DataBuf *>(SoftBusCalloc(sizeof(DataBuf)));
    ASSERT_NE(buf, nullptr);
    g_tcpDataList = CreateSoftBusList();
    ASSERT_NE(g_tcpDataList, nullptr);

    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);;
    EXPECT_EQ(ret, SOFTBUS_TRANS_NODE_NOT_FOUND);

    char testData[] = "data";
    buf->channelId = TRANS_TEST_CHANNEL_ID;
    buf->data = testData;
    buf->w = testData;
    (void)SoftBusMutexLock(&g_tcpDataList->lock);
    ListAdd(&g_tcpDataList->list, &buf->node);
    (void)SoftBusMutexUnlock(&g_tcpDataList->lock);

    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_OK);

    SoftBusFree(buf);
    DestroySoftBusList(g_tcpDataList);
    g_tcpDataList = nullptr;
}

/**
 * @tc.name: TransTdcProcAllDataTest004
 * @tc.desc: TransTdcProcAllData, use different parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcProcAllDataTest004, TestSize.Level1)
{
    int32_t channelId = TRANS_TEST_CHANNEL_ID;
    DataBuf *buf = reinterpret_cast<DataBuf *>(SoftBusCalloc(sizeof(DataBuf)));
    ASSERT_NE(buf, nullptr);
    g_tcpDataList = CreateSoftBusList();
    ASSERT_NE(g_tcpDataList, nullptr);

    TcpDataPacketHead *pktHead = reinterpret_cast<TcpDataPacketHead *>(SoftBusCalloc(sizeof(TcpDataPacketHead)));
    ASSERT_NE(pktHead, nullptr);
    pktHead->magicNumber = 0x01;
    buf->channelId = channelId;
    buf->data = (char *)pktHead;
    buf->w = buf->data + DC_DATA_HEAD_SIZE - 1;
    (void)SoftBusMutexLock(&g_tcpDataList->lock);
    ListAdd(&g_tcpDataList->list, &buf->node);
    (void)SoftBusMutexUnlock(&g_tcpDataList->lock);

    int32_t ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_DATA_NOT_ENOUGH);
    buf->w = buf->data + DC_DATA_HEAD_SIZE;
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_INVALID_DATA_HEAD);

    pktHead->magicNumber = MAGIC_NUMBER;
    pktHead->dataLen = g_dataBufferMaxLen - DC_DATA_HEAD_SIZE + 1;
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_DATA_LENGTH);

    pktHead->dataLen = OVERHEAD_LEN;
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_DATA_LENGTH);

    pktHead->dataLen = 1;
    buf->size = DC_DATA_HEAD_SIZE;
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_DATA_LENGTH);

    pktHead->dataLen = OVERHEAD_LEN + 1;
    buf->size = DC_DATA_HEAD_SIZE;
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_OK);

    pktHead->dataLen = 0;
    ret = TransTdcProcAllData(TRANS_TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_DATA_NOT_ENOUGH);
    // pktHead is deleted in the abnormal branch
    SoftBusFree(buf);
    DestroySoftBusList(g_tcpDataList);
    g_tcpDataList = nullptr;
}

/**
 * @tc.name: TransAssembleTlvData001
 * @tc.desc: TransAssembleTlvData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransAssembleTlvData001, TestSize.Level1)
{
    int32_t bufferSize = 0;
    int32_t ret = TransAssembleTlvData(nullptr, 1, nullptr, 1, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: TransAssembleTlvData002
 * @tc.desc: TransAssembleTlvData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransAssembleTlvData002, TestSize.Level1)
{
    DataHead pktHead;
    uint8_t *tlvElement = (uint8_t *)SoftBusCalloc(TDC_TLV_ELEMENT * sizeof(TlvElement));
    pktHead.tlvElement = tlvElement;
    pktHead.magicNum = SoftBusHtoLl(MAGIC_NUMBER);
    int32_t bufferSize = 0;
    int32_t ret = TransAssembleTlvData(&pktHead, TLV_TYPE_INNER_SEQ, nullptr, 1, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    uint8_t buffer = 0;
    ret = TransAssembleTlvData(&pktHead, TLV_TYPE_FLAG, &buffer, 1, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = TransAssembleTlvData(&pktHead, TLV_TYPE_DATA_LEN, &buffer, 1, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    SoftBusFree(tlvElement);
}

/**
 * @tc.name: BuildNeedAckTlvData001
 * @tc.desc: BuildNeedAckTlvData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, BuildNeedAckTlvData001, TestSize.Level1)
{
    int32_t bufferSize = 0;
    int32_t ret = BuildNeedAckTlvData(nullptr, true, 1, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: BuildDataHead001
 * @tc.desc: BuildDataHead
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, BuildDataHead001, TestSize.Level1)
{
    int32_t bufferSize = 0;
    DataHead data;
    int32_t ret = BuildDataHead(&data, 1, 0, 32, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: TransTdcNeedSendAck001
 * @tc.desc: TransTdcNeedSendAck
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectTest, TransTdcNeedSendAck001, TestSize.Level1)
{
    int32_t ret = TransTdcNeedSendAck(nullptr, 1, 0, false);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}
}
