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

#include "client_trans_tcp_direct_manager.c"

using namespace testing::ext;

namespace OHOS {
#define SUPP_CHANNEL_ID 100
#define SUPP_OTHER_CHANNEL_ID 101
#define SUPP_NEG_FD (-1)
#define SUPP_NOT_OPEN_FD 8888
#define SUPP_PEER_PORT 6000
#define SUPP_SHORT_KEY_LEN 16
#define SUPP_PEER_IP "127.0.0.1"
#define SUPP_PEER_DEVICE_ID "testPeerDeviceId012345678901234"
#define SUPP_PKG_NAME "dms"
#define SUPP_EMPTY_PKG_NAME ""
#define SUPP_SESSION_NAME "com.test.trans.tdc.supplement"

static void TestClearSuppTdcChannelList(void);

class TransTcpDirectManagerSupTest : public testing::Test {
public:
    TransTcpDirectManagerSupTest()
    {}
    ~TransTcpDirectManagerSupTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void TransTcpDirectManagerSupTest::SetUpTestCase(void)
{
    int32_t ret = InitBaseListener();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

void TransTcpDirectManagerSupTest::TearDownTestCase(void) {}

void TransTcpDirectManagerSupTest::SetUp(void)
{
    if (g_tcpDirectChannelInfoList == nullptr) {
        g_tcpDirectChannelInfoList = CreateSoftBusList();
    }
    ASSERT_TRUE(g_tcpDirectChannelInfoList != nullptr);
}

void TransTcpDirectManagerSupTest::TearDown(void)
{
    if (g_tcpDirectChannelInfoList != nullptr) {
        TestClearSuppTdcChannelList();
        DestroySoftBusList(g_tcpDirectChannelInfoList);
        g_tcpDirectChannelInfoList = nullptr;
    }
}

static void TestClearSuppTdcChannelList(void)
{
    int32_t ret = SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return;
    }
    TcpDirectChannelInfo *item = nullptr;
    TcpDirectChannelInfo *nextItem = nullptr;
    LIST_FOR_EACH_ENTRY_SAFE(item, nextItem, &g_tcpDirectChannelInfoList->list, TcpDirectChannelInfo, node) {
        ListDelete(&item->node);
        (void)SoftBusMutexDestroy(&item->detail.fdLock);
        SoftBusFree(item);
    }
    g_tcpDirectChannelInfoList->cnt = 0;
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);
}

static TcpDirectChannelInfo *TestAddSuppTdcChannel(
    int32_t channelId, int32_t fd, int32_t fdRefCnt, ProtocolType fdProtocol, const char *pkgName)
{
    TcpDirectChannelInfo *item = reinterpret_cast<TcpDirectChannelInfo *>(SoftBusCalloc(sizeof(TcpDirectChannelInfo)));
    if (item == nullptr) {
        TRANS_LOGE(TRANS_SDK, "calloc failed, channelId=%{public}d", channelId);
        return nullptr;
    }
    item->channelId = channelId;
    item->detail.fd = fd;
    item->detail.fdRefCnt = fdRefCnt;
    item->detail.fdProtocol = fdProtocol;
    item->detail.needRelease = false;
    item->detail.peerPort = SUPP_PEER_PORT;
    int32_t ret = strcpy_s(item->detail.peerIp, IP_LEN, SUPP_PEER_IP);
    if (ret != EOK) {
        TRANS_LOGE(TRANS_SDK, "copy peerIp failed, ret=%{public}d", ret);
        SoftBusFree(item);
        return nullptr;
    }
    ret = strcpy_s(item->detail.peerDeviceId, DEVICE_ID_SIZE_MAX, SUPP_PEER_DEVICE_ID);
    if (ret != EOK) {
        TRANS_LOGE(TRANS_SDK, "copy peerDeviceId failed, ret=%{public}d", ret);
        SoftBusFree(item);
        return nullptr;
    }
    ret = strcpy_s(item->detail.pkgName, PKG_NAME_SIZE_MAX, pkgName);
    if (ret != EOK) {
        TRANS_LOGE(TRANS_SDK, "copy pkgName failed, ret=%{public}d", ret);
        SoftBusFree(item);
        return nullptr;
    }
    ret = SoftBusMutexInit(&item->detail.fdLock, nullptr);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "init fdLock failed, ret=%{public}d", ret);
        SoftBusFree(item);
        return nullptr;
    }
    (void)SoftBusMutexLock(&g_tcpDirectChannelInfoList->lock);
    ListAdd(&g_tcpDirectChannelInfoList->list, &item->node);
    g_tcpDirectChannelInfoList->cnt++;
    (void)SoftBusMutexUnlock(&g_tcpDirectChannelInfoList->lock);
    return item;
}

/*
 * @tc.name: ClientTransTdcOnChannelOpenedTest001
 * @tc.desc: test ClientTransTdcOnChannelOpened with an existing channelId,
 *           returns SOFTBUS_TRANS_TDC_CHANNEL_ALREADY_EXIST
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, ClientTransTdcOnChannelOpenedTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    ChannelInfo channel = { };
    channel.channelId = SUPP_CHANNEL_ID;
    int32_t ret = ClientTransTdcOnChannelOpened(SUPP_SESSION_NAME, &channel, nullptr);
    EXPECT_EQ(ret, SOFTBUS_TRANS_TDC_CHANNEL_ALREADY_EXIST);
}

/*
 * @tc.name: TransTdcGetSessionKeyTest001
 * @tc.desc: test TransTdcGetSessionKey with a key buffer shorter than SESSION_KEY_LENGTH,
 *           returns SOFTBUS_MEM_ERR
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransTdcGetSessionKeyTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    char key[SESSION_KEY_LENGTH] = { 0 };
    int32_t ret = TransTdcGetSessionKey(SUPP_CHANNEL_ID, key, SUPP_SHORT_KEY_LEN);
    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
}

/*
 * @tc.name: TransUpdateFdStateTest001
 * @tc.desc: test TransUpdateFdState with a channel whose needRelease is true and fdRefCnt
 *           drops to zero, the channel is removed from the list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransUpdateFdStateTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 1, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    item->detail.needRelease = true;
    TransUpdateFdState(SUPP_CHANNEL_ID);
    TcpDirectChannelInfo channel = { };
    int32_t ret = TransTdcGetInfoById(SUPP_CHANNEL_ID, &channel);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: TransTdcDeleteChannelInfoTest001
 * @tc.desc: test TransTdcDeleteChannelInfo with a channel whose fdRefCnt is positive,
 *           the channel deletion is deferred and the channel is still findable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransTdcDeleteChannelInfoTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 2, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    TransTdcDeleteChannelInfo(SUPP_CHANNEL_ID);
    TcpDirectChannelInfo channel = { };
    int32_t ret = TransTdcGetInfoById(SUPP_CHANNEL_ID, &channel);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(channel.detail.needRelease, true);
}

/*
 * @tc.name: TransTdcDeleteChannelInfoTest002
 * @tc.desc: test TransTdcDeleteChannelInfo with a channelId not in the list,
 *           the existing channel is not affected
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransTdcDeleteChannelInfoTest002, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    TransTdcDeleteChannelInfo(SUPP_OTHER_CHANNEL_ID);
    TcpDirectChannelInfo channel = { };
    int32_t ret = TransTdcGetInfoById(SUPP_CHANNEL_ID, &channel);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: TransTdcDelChannelInfoTest001
 * @tc.desc: test TransTdcDelChannelInfo with negotiate rejected errCode and zero fdRefCnt,
 *           the channel is removed from the list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransTdcDelChannelInfoTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    TransTdcDelChannelInfo(SUPP_CHANNEL_ID, SOFTBUS_TRANS_NEGOTIATE_REJECTED);
    TcpDirectChannelInfo channel = { };
    int32_t ret = TransTdcGetInfoById(SUPP_CHANNEL_ID, &channel);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: TransTdcDelChannelInfoTest002
 * @tc.desc: test TransTdcDelChannelInfo with a channel whose fdRefCnt is positive,
 *           the channel deletion is deferred and the channel is still findable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransTdcDelChannelInfoTest002, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 2, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    TransTdcDelChannelInfo(SUPP_CHANNEL_ID, SOFTBUS_TRANS_NEGOTIATE_REJECTED);
    TcpDirectChannelInfo channel = { };
    int32_t ret = TransTdcGetInfoById(SUPP_CHANNEL_ID, &channel);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(channel.detail.needRelease, true);
}

/*
 * @tc.name: TransDisableSessionListenerTest001
 * @tc.desc: test TransDisableSessionListener with a channelId not in the list,
 *           returns SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransDisableSessionListenerTest001, TestSize.Level1)
{
    int32_t ret = TransDisableSessionListener(SUPP_OTHER_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND);
}

/*
 * @tc.name: TransDisableSessionListenerTest002
 * @tc.desc: test TransDisableSessionListener with a channel whose fd is negative,
 *           returns SOFTBUS_INVALID_FD
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransDisableSessionListenerTest002, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    int32_t ret = TransDisableSessionListener(SUPP_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_INVALID_FD);
}

/*
 * @tc.name: TransDisableSessionListenerTest003
 * @tc.desc: test TransDisableSessionListener with a channel whose fd is valid,
 *           returns SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransDisableSessionListenerTest003, TestSize.Level1)
{
    TcpDirectChannelInfo *item =
        TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NOT_OPEN_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    int32_t ret = TransDisableSessionListener(SUPP_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: TransStopTimeSyncTest001
 * @tc.desc: test TransStopTimeSync with a channelId not in the list,
 *           returns SOFTBUS_NOT_FIND
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransStopTimeSyncTest001, TestSize.Level1)
{
    int32_t ret = TransStopTimeSync(SUPP_OTHER_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: TransStopTimeSyncTest002
 * @tc.desc: test TransStopTimeSync with a channel whose fdProtocol is not LNN_PROTOCOL_DETTP,
 *           returns SOFTBUS_OK without stopping time sync
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransStopTimeSyncTest002, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    int32_t ret = TransStopTimeSync(SUPP_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: TransStopTimeSyncTest003
 * @tc.desc: test TransStopTimeSync with a DETTP channel and an empty pkgName,
 *           the inner init failure is mapped to SOFTBUS_NETWORK_NOT_INIT
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, TransStopTimeSyncTest003, TestSize.Level1)
{
    TcpDirectChannelInfo *item =
        TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_DETTP, SUPP_EMPTY_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    int32_t ret = TransStopTimeSync(SUPP_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_NETWORK_NOT_INIT);
}

/*
 * @tc.name: OnTimeSyncResultByIpTest001
 * @tc.desc: test OnTimeSyncResultByIp with a channel matching the peer ip and port,
 *           the time sync failure does not affect the channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransTcpDirectManagerSupTest, OnTimeSyncResultByIpTest001, TestSize.Level1)
{
    TcpDirectChannelInfo *item = TestAddSuppTdcChannel(SUPP_CHANNEL_ID, SUPP_NEG_FD, 0, LNN_PROTOCOL_IP, SUPP_PKG_NAME);
    ASSERT_TRUE(item != nullptr);
    TimeSyncResultWithSocket info = { };
    (void)strcpy_s(info.targetSocketInfo.peerIp, sizeof(info.targetSocketInfo.peerIp), SUPP_PEER_IP);
    info.targetSocketInfo.peerPort = SUPP_PEER_PORT;
    OnTimeSyncResultByIp(&info, SOFTBUS_OK);
    TcpDirectChannelInfo channel = { };
    int32_t ret = TransTdcGetInfoById(SUPP_CHANNEL_ID, &channel);
    EXPECT_EQ(ret, SOFTBUS_OK);
}
} // namespace OHOS
