/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "client_trans_proxy_file_manager.h"
#include "client_trans_proxy_manager.c"
#include "client_trans_proxy_manager.h"
#include "client_trans_session_manager.h"
#include "client_trans_socket_manager.h"
#include "client_trans_tcp_direct_message.h"
#include "session.h"
#include "softbus_access_token_test.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "trans_proxy_process_data.c"
#include "trans_proxy_process_data.h"

#define TEST_CHANNEL_ID (-10)
#define TEST_ERR_CODE (-1)
#define TEST_DATA "testdata"
#define TEST_DATA_LENGTH 9
#define TEST_DATA_LENGTH_2 100
#define TEST_FILE_CNT 4
#define TEST_SEQ 188
#define SILCE_NUM_COUNT 3
#define SLICE_SEQ_BEGIN 0
#define SLICE_SEQ_MID 1
#define SLICE_SEQ_END 2

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
char g_sessionKey[32] = "1234567812345678123456781234567";
#define DEFAULT_NEW_BYTES_LEN   (4 * 1024 * 1024)
#define DEFAULT_NEW_MESSAGE_LEN (4 * 1024)

int32_t TransOnSessionOpened(
    const char *sessionName, const ChannelInfo *channel, SessionType flag, SocketAccessInfo *accessInfo)
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

int32_t OnSessionOpened(
    const char *sessionName, const ChannelInfo *channel, SessionType flag, SocketAccessInfo *accessInfo)
{
    (void)sessionName;
    (void)channel;
    (void)flag;
    (void)accessInfo;
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
    TransClientInit();
    int32_t ret = ClientTransProxyInit(&g_clientSessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SetAccessTokenPermission("dsoftbusTransTest");
}

void ClientTransProxyManagerTest::TearDownTestCase(void)
{
    TransClientDeinit();
}

/**
 * @tc.name: ClientTransProxyInitTest
 * @tc.desc: client trans proxy init test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyInitTest, TestSize.Level1)
{
    int32_t ret = ClientTransProxyInit(nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyOnChannelOpenedTest
 * @tc.desc: client trans proxy on channel opened test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnChannelOpenedTest, TestSize.Level1)
{
    int32_t ret = ClientTransProxyOnChannelOpened(g_proxySessionName, nullptr, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ChannelInfo channelInfo = {0};
    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_MEM_ERR, ret);
}

/**
 * @tc.name: ClientTransProxyOnDataReceivedTest
 * @tc.desc: client trans proxy on data received test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnDataReceivedTest, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t ret = ClientTransProxyOnDataReceived(channelId, nullptr, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = ClientTransProxyOnDataReceived(channelId, TEST_DATA, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_INVALID_CHANNEL_ID, ret);
}

/**
 * @tc.name: ClientTransProxyOnDataReceivedTest001
 * @tc.desc: client trans proxy on data received test. test wrong slice head or packet head
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnDataReceivedTest001, TestSize.Level1)
{
    int32_t channelId = 1;
    ChannelInfo channelInfo;
    channelInfo.channelId = channelId;
    channelInfo.sessionKey = g_sessionKey;
    channelInfo.isEncrypt = true;
    channelInfo.isSupportTlv = true;
    int32_t ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, TEST_DATA, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    SliceHead sliceHead;
    sliceHead.priority = PROXY_CHANNEL_PRORITY_BUTT;
    char buf[TEST_DATA_LENGTH_2];
    ret = memcpy_s(buf, TEST_DATA_LENGTH_2, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    PacketHead packetHead;
    ret = memcpy_s(buf + sizeof(SliceHead), TEST_DATA_LENGTH_2, &packetHead, sizeof(PacketHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_INVALID_SLICE_HEAD, ret);

    sliceHead.priority = PROXY_CHANNEL_PRORITY_MESSAGE;
    sliceHead.sliceNum = 0;
    sliceHead.sliceSeq = 0;
    ret = memcpy_s(buf, TEST_DATA_LENGTH_2, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_INVALID_SLICE_HEAD, ret);

    sliceHead.priority = PROXY_CHANNEL_PRORITY_MESSAGE;
    sliceHead.sliceNum = 1;
    ret = memcpy_s(buf, TEST_DATA_LENGTH_2, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    packetHead.magicNumber = 1;
    ret = memcpy_s(buf + sizeof(SliceHead), TEST_DATA_LENGTH_2, &packetHead, sizeof(PacketHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);

    packetHead.magicNumber = MAGIC_NUMBER;
    packetHead.dataLen = 0;
    ret = memcpy_s(buf + sizeof(SliceHead), TEST_DATA_LENGTH_2, &packetHead, sizeof(PacketHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);

    packetHead.dataLen = sizeof(PacketHead) - 1;
    ret = memcpy_s(buf + sizeof(SliceHead), TEST_DATA_LENGTH_2, &packetHead, sizeof(PacketHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);
}

/**
 * @tc.name: ClientTransProxyOnDataReceivedTest002
 * @tc.desc: client trans proxy on data received test. test wrong slice data
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnDataReceivedTest002, TestSize.Level1)
{
    g_proxyMaxByteBufSize = DEFAULT_NEW_BYTES_LEN;
    g_proxyMaxMessageBufSize = DEFAULT_NEW_MESSAGE_LEN;
    int32_t channelId = 1;

    SliceHead sliceHead;
    sliceHead.priority = PROXY_CHANNEL_PRORITY_MESSAGE;
    sliceHead.sliceNum = 1;
    char buf[TEST_DATA_LENGTH_2];
    int32_t ret = memcpy_s(buf, TEST_DATA_LENGTH_2, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    PacketHead packetHead;
    packetHead.magicNumber = MAGIC_NUMBER;
    packetHead.dataLen = TEST_DATA_LENGTH_2 - sizeof(SliceHead) - sizeof(PacketHead);
    ret = memcpy_s(buf + sizeof(SliceHead), TEST_DATA_LENGTH_2, &packetHead, sizeof(PacketHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);

    sliceHead.sliceNum = SILCE_NUM_COUNT;
    sliceHead.sliceSeq = SLICE_SEQ_BEGIN;
    ret = memcpy_s(buf, TEST_DATA_LENGTH_2, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf, TEST_DATA_LENGTH_2, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);

    sliceHead.priority = PROXY_CHANNEL_PRORITY_BYTES;
    int32_t dataLen = sizeof(SliceHead) + sizeof(PacketHead) + 1;
    char buf2[dataLen];
    ret = memcpy_s(buf2, dataLen, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    ret = memcpy_s(buf2 + sizeof(SliceHead), dataLen, &packetHead, sizeof(PacketHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf2, dataLen, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);

    sliceHead.sliceSeq = SLICE_SEQ_MID;
    ret = memcpy_s(buf2, dataLen, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf2, dataLen, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_NO_INVALID, ret);

    sliceHead.sliceSeq = SLICE_SEQ_END;
    ret = memcpy_s(buf2, dataLen, &sliceHead, sizeof(SliceHead));
    EXPECT_EQ(EOK, ret);
    ret = ClientTransProxyOnDataReceived(channelId, buf2, dataLen, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_NO_INVALID, ret);

    ClientTransProxyCloseChannel(channelId);
}

/**
 * @tc.name: TransProxyChannelSendBytesTest
 * @tc.desc: client trans proxy end bytes test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxyChannelSendBytesTest, TestSize.Level1)
{
    int32_t channelId = 1;
    ChannelInfo channelInfo;
    channelInfo.channelId = channelId;
    channelInfo.sessionKey = g_sessionKey;
    channelInfo.isEncrypt = false;
    channelInfo.isSupportTlv = true;
    int32_t ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransProxyChannelSendBytes(channelId, TEST_DATA, TEST_DATA_LENGTH, false);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    ClientTransProxyCloseChannel(channelId);

    channelInfo.isEncrypt = true;
    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransProxyChannelSendBytes(channelId, TEST_DATA, TEST_DATA_LENGTH, false);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);
    ClientTransProxyCloseChannel(channelId);
}

/**
 * @tc.name: TransProxyChannelSendMessageTest
 * @tc.desc: client trans proxy end bytes test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxyChannelSendMessageTest, TestSize.Level1)
{
    int32_t channelId = 1;
    ChannelInfo channelInfo;
    channelInfo.channelId = channelId;
    channelInfo.sessionKey = g_sessionKey;
    channelInfo.isEncrypt = false;
    channelInfo.isSupportTlv = false;
    int32_t ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransProxyChannelSendMessage(channelId, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    ClientTransProxyCloseChannel(channelId);

    channelInfo.isEncrypt = true;
    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransProxyChannelSendMessage(channelId, TEST_DATA, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);
    ClientTransProxyCloseChannel(channelId);
}

/**
 * @tc.name: ClientTransProxyErrorCallBackTest
 * @tc.desc: client trans proxy error callback test, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyErrorCallBackTest, TestSize.Level1)
{
    int32_t channelId = 1;
    ChannelInfo channelInfo;
    channelInfo.channelId = channelId;
    channelInfo.sessionKey = g_sessionKey;
    channelInfo.isEncrypt = false;
    channelInfo.isSupportTlv = true;
    int32_t ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = ClientTransProxyOnDataReceived(channelId, TEST_DATA, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ClientTransProxyCloseChannel(channelId);

    ret = ClientTransProxyInit(&g_sessionCb);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = ClientTransProxyOnDataReceived(channelId, TEST_DATA, TEST_DATA_LENGTH, TRANS_SESSION_BYTES);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/**
 * @tc.name: ClientTransProxyCloseChannelTest
 * @tc.desc: client trans proxy close channel test, use the normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyCloseChannelTest, TestSize.Level1)
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
HWTEST_F(ClientTransProxyManagerTest, TransProxyChannelSendFileTest, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t ret = TransProxyChannelSendFile(channelId, nullptr, g_proxyFileSet, TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransProxyChannelSendFile(channelId, g_testProxyFileName, g_proxyFileSet, MAX_SEND_FILE_NUM + TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransProxyChannelSendFile(channelId, g_testProxyFileName, nullptr, TEST_FILE_CNT);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);

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
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyGetInfoByChannelIdTest, TestSize.Level1)
{
    int32_t channelId = 1;
    ProxyChannelInfoDetail info;
    memset_s(&info, sizeof(ProxyChannelInfoDetail), 0, sizeof(ProxyChannelInfoDetail));
    int32_t ret = ClientTransProxyGetInfoByChannelId(channelId, nullptr);
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
HWTEST_F(ClientTransProxyManagerTest, TransProxyPackAndSendDataTest, TestSize.Level1)
{
    int32_t channelId = 1;
    const char *data = "test";
    uint32_t len = 5;
    ProxyChannelInfoDetail info;
    memset_s(&info, sizeof(ProxyChannelInfoDetail), 0, sizeof(ProxyChannelInfoDetail));
    SessionPktType pktType = TRANS_SESSION_MESSAGE;
    int32_t ret = ClientTransProxyPackAndSendData(channelId, nullptr, len, &info, pktType);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransProxyPackAndSendData(channelId, data, len, nullptr, pktType);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = ClientTransProxyPackAndSendData(channelId,
        static_cast<const void *>(data), len, &info, pktType);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);
}

/**
 * @tc.name: ClientTransProxyGetLinkTypeByChannelId
 * @tc.desc: Should return SOFTBUS_INVALID_PARAM when given linkType or data is null.
 * @tc.desc: Should return SOFTBUS_NOT_FIND when get link type failed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyGetLinkTypeByChannelIdTest, TestSize.Level1)
{
    int32_t channelId = -1;
    int32_t ret = ClientTransProxyGetLinkTypeByChannelId(channelId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    int32_t linkType;
    ret = ClientTransProxyGetLinkTypeByChannelId(channelId, &linkType);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
}

/**
 * @tc.name: TransGetActualDataLen
 * @tc.desc: TransGetActualDataLen test.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransGetActualDataLenTest, TestSize.Level1)
{
    SliceHead head = {};
    uint32_t actualDataLen = 0;
    g_proxyMaxByteBufSize = DEFAULT_NEW_BYTES_LEN;
    g_proxyMaxMessageBufSize = DEFAULT_NEW_MESSAGE_LEN;

    // Test case 1: Valid sliceNum, priority message
    head.sliceNum = 1;
    head.priority = PROXY_CHANNEL_PRORITY_MESSAGE;
    EXPECT_EQ(SOFTBUS_OK, TransGetActualDataLen(&head, &actualDataLen));
    EXPECT_EQ(head.sliceNum * SLICE_LEN, actualDataLen);

    // Test case 2: Valid sliceNum, priority bytes
    head.sliceNum = 10;
    head.priority = PROXY_CHANNEL_PRORITY_BYTES;
    EXPECT_EQ(SOFTBUS_OK, TransGetActualDataLen(&head, &actualDataLen));
    EXPECT_EQ(head.sliceNum * SLICE_LEN, actualDataLen);

    // Test case 3: Invalid sliceNum (exceeds MAX_MALLOC_SIZE / SLICE_LEN)
    head.sliceNum = (MAX_MALLOC_SIZE / SLICE_LEN) + 1;
    head.priority = PROXY_CHANNEL_PRORITY_MESSAGE;
    EXPECT_EQ(SOFTBUS_INVALID_DATA_HEAD, TransGetActualDataLen(&head, &actualDataLen));

    // Test case 4: Invalid sliceNum (actualLen exceeds maxDataLen)
    head.sliceNum = (g_proxyMaxMessageBufSize / SLICE_LEN) + 2;
    head.priority = PROXY_CHANNEL_PRORITY_MESSAGE;
    EXPECT_EQ(SOFTBUS_INVALID_DATA_HEAD, TransGetActualDataLen(&head, &actualDataLen));
}

/**
 * @tc.name: ProxyBuildNeedAckTlvData001
 * @tc.desc: ProxyBuildNeedAckTlvData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ProxyBuildNeedAckTlvData001, TestSize.Level1)
{
    int32_t bufferSize = 0;
    int32_t ret = ProxyBuildNeedAckTlvData(nullptr, true, 1, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: ProxyBuildTlvDataHead001
 * @tc.desc: ProxyBuildTlvDataHead
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ProxyBuildTlvDataHead001, TestSize.Level1)
{
    int32_t bufferSize = 0;
    DataHead data;
    int32_t ret = ProxyBuildTlvDataHead(&data, 1, 0, 32, &bufferSize);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: ClientTransProxyProcSendMsgAck001
 * @tc.desc: ClientTransProxyProcSendMsgAck
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyProcSendMsgAck001, TestSize.Level1)
{
    const char *data = "test";
    int32_t ret = ClientTransProxyProcSendMsgAck(1, nullptr, PROXY_ACK_SIZE, 1, 1);
    EXPECT_EQ(ret, SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_DATA_NULL);

    ret = ClientTransProxyProcSendMsgAck(1, data, 1, 1, 1);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_DATA_LENGTH);

    ret = ClientTransProxyProcSendMsgAck(1, data, PROXY_ACK_SIZE, 1, 1);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);

    ret = ClientTransProxyProcSendMsgAck(1, data, PROXY_ACK_SIZE, 1, 0);
    EXPECT_EQ(ret, SOFTBUS_TRANS_NODE_NOT_FOUND);
}

/**
 * @tc.name: ClientTransProxyBytesNotifySession001
 * @tc.desc: ClientTransProxyBytesNotifySession
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyBytesNotifySession001, TestSize.Level1)
{
    const char *data = "test";
    DataHeadTlvPacketHead dataHead;
    dataHead.dataSeq = 1;
    dataHead.seq = 1;
    dataHead.flags = TRANS_SESSION_ACK;
    int32_t ret = ClientTransProxyBytesNotifySession(1, &dataHead, data, 4);
    EXPECT_EQ(ret, SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND);

    dataHead.flags = TRANS_SESSION_BYTES;
    ret = ClientTransProxyBytesNotifySession(1, &dataHead, data, sizeof(data));
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    dataHead.flags = TRANS_SESSION_MESSAGE;
    ret = ClientTransProxyBytesNotifySession(1, &dataHead, data, sizeof(data));
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/**
 * @tc.name: ClientTransProxyProcData001
 * @tc.desc: ClientTransProxyProcData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyProcData001, TestSize.Level1)
{
    int32_t channelId = 1;
    DataHeadTlvPacketHead dataHead;
    dataHead.dataLen = 2;
    const char *data = "test";
    int32_t ret = ClientTransProxyProcData(channelId, &dataHead, data);
    EXPECT_EQ(ret, SOFTBUS_TRANS_INVALID_DATA_LENGTH);

    dataHead.dataLen = 34;
    ret = ClientTransProxyProcData(channelId, &dataHead, data);
    EXPECT_EQ(ret, SOFTBUS_DECRYPT_ERR);
}

/**
 * @tc.name: ProxyBuildNeedAckTlvData002
 * @tc.desc: ProxyBuildNeedAckTlvData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ProxyBuildNeedAckTlvData002, TestSize.Level1)
{
    DataHead pktHead;
    bool needAck = true;
    int32_t ret = ProxyBuildNeedAckTlvData(&pktHead, needAck, TEST_SEQ, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ProxyBuildTlvDataHead002
 * @tc.desc: ProxyBuildTlvDataHead
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ProxyBuildTlvDataHead002, TestSize.Level1)
{
    DataHead pktHead;
    int32_t flag = 0;
    int32_t ret = ProxyBuildTlvDataHead(&pktHead, TEST_SEQ, flag, TEST_DATA_LENGTH_2, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: TransProxyParseTlv001
 * @tc.desc: TransProxyParseTlv
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxyParseTlv001, TestSize.Level1)
{
    uint32_t newDataHeadSize = 0;
    int32_t ret = TransProxyParseTlv(TEST_DATA_LENGTH, nullptr, nullptr, &newDataHeadSize);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    const char *data = "test";
    ret = TransProxyParseTlv(TEST_DATA_LENGTH, data, nullptr, &newDataHeadSize);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    DataHeadTlvPacketHead head;
    ret = TransProxyParseTlv(TEST_DATA_LENGTH_2, data, &head, &newDataHeadSize);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: ClientTransProxyGetOsTypeByChannelId001
 * @tc.desc: ClientTransProxyGetOsTypeByChannelId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyGetOsTypeByChannelId001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t ret = ClientTransProxyGetOsTypeByChannelId(channelId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyOnChannelOpened001
 * @tc.desc: ClientTransProxyOnChannelOpened
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyOnChannelOpened001, TestSize.Level1)
{
    int32_t channelId = 1;
    ChannelInfo channelInfo;
    channelInfo.channelId = channelId;
    channelInfo.sessionKey = g_sessionKey;
    channelInfo.isEncrypt = false;
    channelInfo.isSupportTlv = false;
    channelInfo.businessType = BUSINESS_TYPE_BYTE;
    int32_t ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    channelInfo.businessType = BUSINESS_TYPE_FILE;
    ret = ClientTransProxyOnChannelOpened(g_proxySessionName, &channelInfo, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxySendBytesAck001
 * @tc.desc: ClientTransProxySendBytesAck
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxySendBytesAck001, TestSize.Level1)
{
    int32_t channelId = 1;
    const char *data = "test";
    DataHeadTlvPacketHead dataHead;
    dataHead.dataSeq = 1;
    dataHead.seq = 1;
    dataHead.flags = TRANS_SESSION_BYTES;
    dataHead.needAck = true;
    int32_t ret = ClientTransProxyBytesNotifySession(channelId, &dataHead, data, sizeof(data));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyBytesNotifySession002
 * @tc.desc: ClientTransProxyBytesNotifySession
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyBytesNotifySession002, TestSize.Level1)
{
    int32_t channelId = 1;
    const char *data = "test";
    uint32_t invalidFlag = 22;
    DataHeadTlvPacketHead dataHead;
    dataHead.dataSeq = 1;
    dataHead.seq = 1;
    dataHead.flags = TRANS_SESSION_ASYNC_MESSAGE;
    int32_t ret = ClientTransProxyBytesNotifySession(channelId, &dataHead, data, sizeof(data));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    dataHead.flags = invalidFlag;
    ret = ClientTransProxyBytesNotifySession(channelId, &dataHead, data, sizeof(data));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyNotifySession001
 * @tc.desc: ClientTransProxyNotifySession
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyNotifySession001, TestSize.Level1)
{
    int32_t channelId = 1;
    SessionPktType flag = TRANS_SESSION_MESSAGE;
    const char *data = "test";
    int32_t ret = ClientTransProxyNotifySession(channelId, flag, TEST_SEQ, data, sizeof(data));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    flag = TRANS_SESSION_ACK;
    ret = ClientTransProxyNotifySession(channelId, flag, TEST_SEQ, data, PROXY_ACK_SIZE);
    EXPECT_EQ(SOFTBUS_TRANS_NODE_NOT_FOUND, ret);

    flag = TRANS_SESSION_ASYNC_MESSAGE;
    ret = ClientTransProxyNotifySession(channelId, flag, TEST_SEQ, data, sizeof(data));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    int32_t invalidFlag = 22;
    flag = static_cast<SessionPktType>(invalidFlag);
    ret = ClientTransProxyNotifySession(channelId, flag, TEST_SEQ, data, sizeof(data));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/**
 * @tc.name: ClientTransProxyProcessSessionData001
 * @tc.desc: ClientTransProxyProcessSessionData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyProcessSessionData001, TestSize.Level1)
{
    int32_t channelId = 1;
    PacketHead dataHead;
    dataHead.dataLen = OVERHEAD_LEN;
    const char *data = "test";
    int32_t ret = ClientTransProxyProcessSessionData(channelId, &dataHead, data);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_DATA_LENGTH, ret);

    dataHead.dataLen = TEST_DATA_LENGTH_2;
    dataHead.seq = 1;
    ret = ClientTransProxyProcessSessionData(channelId, &dataHead, data);
    EXPECT_EQ(SOFTBUS_DECRYPT_ERR, ret);
}

/**
 * @tc.name: ClientTransProxyNoSubPacketTlvProc001
 * @tc.desc: ClientTransProxyNoSubPacketTlvProc
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, ClientTransProxyNoSubPacketTlvProc001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t ret = ClientTransProxyNoSubPacketTlvProc(channelId, nullptr, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    const char *data = "test";
    ret = ClientTransProxyNoSubPacketTlvProc(channelId, data, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_INVALID_DATA_HEAD, ret);

    uint32_t magic = MAGIC_NUMBER;
    int32_t len = 20;
    char *magicData = reinterpret_cast<char *>(SoftBusCalloc(len));
    ASSERT_TRUE(magicData != nullptr);
    (void)memcpy_s(magicData, MAGICNUM_SIZE, &magic, MAGICNUM_SIZE);
    ret = ClientTransProxyNoSubPacketTlvProc(channelId, magicData, 0);
    EXPECT_EQ(SOFTBUS_DATA_NOT_ENOUGH, ret);

    ret = ClientTransProxyNoSubPacketTlvProc(channelId, magicData, TEST_DATA_LENGTH);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_DATA_LENGTH, ret);
    SoftBusFree(magicData);
}

/**
 * @tc.name: TransProxySliceProcessChkPkgIsValid001
 * @tc.desc: TransProxySliceProcessChkPkgIsValid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxySliceProcessChkPkgIsValid001, TestSize.Level1)
{
    // head->sliceNum != processor->sliceNumber || head->sliceSeq != processor->expectedSeq
    SliceProcessor processor;
    processor.sliceNumber = 2;
    processor.expectedSeq = 1;
    processor.dataLen = 20;
    processor.bufLen = 10;

    SliceHead head;
    head.sliceNum = 2;
    head.sliceSeq = 1;
    char data[16] = "test";
    uint32_t len = 20;
    int32_t ret = TransProxySliceProcessChkPkgIsValid(&processor, &head, data, len);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_EXCEED_LENGTH, ret);

    processor.bufLen = 20;
    ret = TransProxySliceProcessChkPkgIsValid(&processor, &head, data, len);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_EXCEED_LENGTH, ret);

    processor.bufLen = 50;
    processor.data = nullptr;
    ret = TransProxySliceProcessChkPkgIsValid(&processor, &head, data, len);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_DATA_NULL, ret);

    if (strcpy_s(processor.data, processor.bufLen, "test") != EOK) {
        return;
    }
    ret = TransProxySliceProcessChkPkgIsValid(&processor, &head, data, len);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_DATA_NULL, ret);
}

/**
 * @tc.name: IsValidCheckoutProcess001
 * @tc.desc: IsValidCheckoutProcess
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, IsValidCheckoutProcess001, TestSize.Level1)
{
    int32_t channelId = 1;
    bool ret = IsValidCheckoutProcess(channelId);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: TransProxyPackTlvBytes
 * @tc.desc: ClientTransProxyPackTlvBytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, TransProxyPackTlvBytes, TestSize.Level1)
{
    int32_t channelId = 1;
    SessionPktType flag = TRANS_SESSION_ACK;
    int32_t ret = ClientTransProxyPackTlvBytes(channelId, nullptr, nullptr, flag, TEST_SEQ);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ProxyDataInfo dataInfo;
    ret = ClientTransProxyPackTlvBytes(channelId, &dataInfo, nullptr, flag, TEST_SEQ);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ProxyChannelInfoDetail info;
    (void)memset_s(&info, sizeof(ProxyChannelInfoDetail), 0, sizeof(ProxyChannelInfoDetail));
    ret = ClientTransProxyPackTlvBytes(channelId, &dataInfo, &info, flag, TEST_SEQ);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND, ret);
}

/**
 * @tc.name: SessionPktTypeToProxyIndex001
 * @tc.desc: SessionPktTypeToProxyIndex
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(ClientTransProxyManagerTest, SessionPktTypeToProxyIndex001, TestSize.Level1)
{
    SessionPktType packetType = TRANS_SESSION_ACK;
    int32_t ret = SessionPktTypeToProxyIndex(packetType);
    EXPECT_EQ(PROXY_CHANNEL_PRORITY_MESSAGE, ret);

    packetType = TRANS_SESSION_BYTES;
    ret = SessionPktTypeToProxyIndex(packetType);
    EXPECT_EQ(PROXY_CHANNEL_PRORITY_BYTES, ret);

    packetType = TRANS_SESSION_FILE_FIRST_FRAME;
    ret = SessionPktTypeToProxyIndex(packetType);
    EXPECT_EQ(PROXY_CHANNEL_PRORITY_FILE, ret);
}
} // namespace OHOS