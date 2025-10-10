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

#include <gmock/gmock.h>
#include <securec.h>

#include "message_handler.h"
#include "session.h"
#include "softbus_adapter_crypto.h"
#include "softbus_conn_manager.h"
#include "softbus_error_code.h"
#include "softbus_feature_config.h"
#include "softbus_json_utils.h"
#include "softbus_protocol_def.h"
#include "softbus_proxychannel_manager.h"
#include "softbus_proxychannel_message.h"
#include "softbus_proxychannel_transceiver.c"
#include "softbus_utils.h"
#include "trans_channel_common.h"

using namespace testing;
using namespace testing::ext;

class SoftbusTransProxyTransceiverInterface {
public:

    virtual int32_t TransProxyParseMessage(char *data, int32_t len, ProxyMessage *msg, AuthHandle *auth) = 0;

    virtual int32_t SoftBusGenerateStrHash(const unsigned char *str, uint32_t len, unsigned char *hash) = 0;
};

class SoftbusTransProxyTransceiverMock : public SoftbusTransProxyTransceiverInterface {
public:
    static SoftbusTransProxyTransceiverMock &GetMockObj(void)
    {
        return *gmock_;
    }
    SoftbusTransProxyTransceiverMock();
    ~SoftbusTransProxyTransceiverMock();

    MOCK_METHOD(int32_t, TransProxyParseMessage,
        (char *data, int32_t len, ProxyMessage *msg, AuthHandle *auth), (override));
    
    MOCK_METHOD(int32_t, SoftBusGenerateStrHash,
        (const unsigned char *str, uint32_t len, unsigned char *hash), (override));

private:
    static SoftbusTransProxyTransceiverMock *gmock_;
};

SoftbusTransProxyTransceiverMock *SoftbusTransProxyTransceiverMock::gmock_;

SoftbusTransProxyTransceiverMock::SoftbusTransProxyTransceiverMock()
{
    gmock_ = this;
}

SoftbusTransProxyTransceiverMock::~SoftbusTransProxyTransceiverMock()
{
    gmock_ = nullptr;
}

int32_t TransProxyParseMessage(char *data, int32_t len, ProxyMessage *msg, AuthHandle *auth)
{
    std::cout << "TransProxyParseMessage mock calling enter" << std::endl;
    return SoftbusTransProxyTransceiverMock::GetMockObj().TransProxyParseMessage(data, len, msg, auth);
}

int32_t SoftBusGenerateStrHash(const unsigned char *str, uint32_t len, unsigned char *hash)
{
    std::cout << "SoftBusGenerateStrHash mock calling enter" << std::endl;
    return SoftbusTransProxyTransceiverMock::GetMockObj().SoftBusGenerateStrHash(str, len, hash);
}

namespace OHOS {

#define TEST_STRING_IDENTITY "11"
#define TEST_VALID_CHANNEL_ID 1
#define TEST_DATA_LEN 128
#define TEST_DATALEN 10

class SoftbusProxyTransceiverTest : public testing::Test {
public:
    SoftbusProxyTransceiverTest()
    {}
    ~SoftbusProxyTransceiverTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void SoftbusProxyTransceiverTest::SetUpTestCase(void)
{
    SoftbusConfigInit();
    ASSERT_EQ(SOFTBUS_OK, LooperInit());
    ASSERT_EQ(SOFTBUS_OK, SoftBusTimerInit());
    ASSERT_EQ(SOFTBUS_OK, TransProxyLoopInit());

    IServerChannelCallBack callBack;
    ASSERT_NE(SOFTBUS_OK, TransProxyManagerInit(&callBack));
}

void SoftbusProxyTransceiverTest::TearDownTestCase(void)
{
    TransProxyManagerDeinit();
}

/*
 * @tc.name: TransProxyOpenConnChannelTest001
 * @tc.desc: test proxy open new conn channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOpenConnChannelTest001, TestSize.Level1)
{
    AppInfo appInfo;
    appInfo.appType = APP_TYPE_NORMAL;
    ConnectOption connInfo;
    int32_t channelId = -1;

    int32_t ret = TransProxyOpenConnChannel(nullptr, &connInfo, &channelId);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransProxyOpenConnChannel(&appInfo, nullptr, &channelId);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);

    appInfo.appType = APP_TYPE_AUTH;
    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransProxyOpenConnChannelTest002
 * @tc.desc: test proxy open exist channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOpenConnChannelTest002, TestSize.Level1)
{
    ConnectionInfo tcpInfo;
    tcpInfo.type = CONNECT_TCP;
    ConnectionInfo brInfo;
    brInfo.type = CONNECT_BR;
    ConnectionInfo bleInfo;
    bleInfo.type = CONNECT_BLE;
    bool isServer = false;
    TransCreateConnByConnId(1, isServer);
    TransCreateConnByConnId(2, isServer);
    TransCreateConnByConnId(3, isServer);
    TransCreateConnByConnId(4, isServer);

    AppInfo appInfo;
    appInfo.appType = APP_TYPE_AUTH;
    int32_t channelId = -1;
    ConnectOption connInfo;
    connInfo.type = CONNECT_TCP;
    int32_t ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);
    connInfo.type = CONNECT_BR;
    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);
    connInfo.type = CONNECT_BLE;
    ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_NE(SOFTBUS_OK, ret);
    sleep(1);
}

/*
 * @tc.name: TransProxyOpenConnChannelTest003
 * @tc.desc: test proxy open exist channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOpenConnChannelTest003, TestSize.Level1)
{
    AppInfo appInfo;
    appInfo.appType = APP_TYPE_AUTH;

    int32_t channelId = 2058;
    ConnectOption connInfo;
    connInfo.type = CONNECT_TCP;

    int32_t ret = TransProxyOpenConnChannel(&appInfo, &connInfo, &channelId);
    EXPECT_EQ(ret, SOFTBUS_TRANS_PROXY_CONN_REPEAT);
}

/*
 * @tc.name: TransProxyCloseConnChannelTest001
 * @tc.desc: test proxy close conn channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyCloseConnChannelTest001, TestSize.Level1)
{
    ConnectionInfo tcpInfo;
    tcpInfo.type = CONNECT_TCP;
    bool isServer = false;
    bool isD2d = false;
    TransCreateConnByConnId(1, isServer);

    int32_t ret = TransProxyCloseConnChannel(1, isServer, isD2d);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransProxyCloseConnChannel(1, isServer, isD2d);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransProxyCloseConnChannel(1, isServer, isD2d);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransProxyCloseConnChannelTest002
 * @tc.desc: test proxy close conn channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyCloseConnChannelTest002, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ProxyConnInfo *connChan = reinterpret_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    EXPECT_NE(nullptr, connChan);
    connChan->isServerSide = false;
    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption.type = CONNECT_BR;
    connChan->connInfo = connectOption;
    int32_t ret = TransAddConnItem(connChan);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ConnectionInfo tcpInfo;
    tcpInfo.type = CONNECT_TCP;
    bool isServer = false;
    bool isD2d = false;
    TransCreateConnByConnId(TEST_VALID_CHANNEL_ID, isServer);

    ret = TransProxyCloseConnChannel(TEST_VALID_CHANNEL_ID, isServer, isD2d);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SoftBusFree(connChan);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransProxyCloseConnChannelResetTest001
 * @tc.desc: test proxy dec connInfo ref count
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyCloseConnChannelResetTest001, TestSize.Level1)
{
    ConnectionInfo tcpInfo;
    tcpInfo.type = CONNECT_TCP;
    bool isServer = false;
    TransCreateConnByConnId(2, isServer);

    int32_t ret = TransProxyCloseConnChannelReset(2, false, isServer, false, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransProxyCloseConnChannelReset(2, false, isServer, false, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransProxyCloseConnChannelReset(2, true, isServer, true, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransProxyGetConnInfoByConnIdTest001
 * @tc.desc: test proxy get conn info by conn id
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyGetConnInfoByConnIdTest001, TestSize.Level1)
{
    ConnectOption connOptInfo;

    int32_t ret = TransProxyGetConnInfoByConnId(3, nullptr);
    EXPECT_NE(SOFTBUS_OK, ret);
    ret = TransProxyGetConnInfoByConnId(100, &connOptInfo);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = TransProxyGetConnInfoByConnId(3, &connOptInfo);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransProxyTransSendMsgTest001
 * @tc.desc: test proxy send message
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyTransSendMsgTest001, TestSize.Level1)
{
    uint32_t connectionId = 1;
    uint8_t *buf = nullptr;
    uint32_t len = 1;
    int32_t priority = 0;
    int32_t pid = 0;
    int32_t ret = TransProxyTransSendMsg(connectionId, buf, len, priority, pid);
    EXPECT_NE(SOFTBUS_OK, ret);

    ret = TransProxyTransSendMsg(connectionId, buf, len, priority, pid);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/*
 * @tc.name: CompareConnectOption001
 * @tc.desc: test CompareConnectOption
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, CompareConnectOption001, TestSize.Level1)
{
    ConnectOption connInfo;
    connInfo.type = CONNECT_TCP;
    connInfo.socketOption.protocol = LNN_PROTOCOL_IP;
    ConnectOption itemConnInfo;
    bool ret = false;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(false, ret);

    itemConnInfo.socketOption.protocol = LNN_PROTOCOL_IP;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(true, ret);

    connInfo.socketOption.port = 1000;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(false, ret);

    itemConnInfo.socketOption.port = 1000;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(true, ret);

    connInfo.type = CONNECT_BR;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(true, ret);

    connInfo.type = CONNECT_BLE;
    connInfo.bleOption.protocol = BLE_GATT;
    itemConnInfo.bleOption.protocol = BLE_COC;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(false, ret);

    itemConnInfo.bleOption.protocol = BLE_GATT;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(true, ret);

    connInfo.bleOption.psm = 1;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(false, ret);

    itemConnInfo.bleOption.psm = 1;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(true, ret);

    connInfo.type = CONNECT_BLE_DIRECT;
    connInfo.bleDirectOption.protoType = BLE_COC;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(false, ret);

    itemConnInfo.bleDirectOption.protoType = BLE_COC;
    ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(true, ret);
}

/*
 * @tc.name: CompareConnectOptionTest002
 * @tc.desc: test CompareConnectOption
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, CompareConnectOptionTest002, TestSize.Level1)
{
    ConnectOption connInfo;
    connInfo.type = CONNECT_BR;
    (void)strcpy_s(connInfo.brOption.brMac, BT_MAC_LEN, "hofdhfhwqofqho");

    ConnectOption itemConnInfo;
    (void)strcpy_s(itemConnInfo.brOption.brMac, BT_MAC_LEN, "ngfjrpgjrhp");

    bool ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: CompareConnectOptionTest003
 * @tc.desc: test CompareConnectOption
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, CompareConnectOptionTest003, TestSize.Level1)
{
    ConnectOption connInfo = {
        .type = CONNECT_SLE_DIRECT,
        .sleDirectOption.protoType = SLE_SSAP
    };
    (void)strcpy_s(connInfo.sleDirectOption.networkId, NETWORK_ID_BUF_LEN, "1284456419466");

    ConnectOption itemConnInfo;
    itemConnInfo.sleDirectOption.protoType = SLE_SSAP;
    (void)strcpy_s(itemConnInfo.sleDirectOption.networkId, NETWORK_ID_BUF_LEN, "1284456419466");

    bool ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: CompareConnectOptionTest004
 * @tc.desc: test CompareConnectOption
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, CompareConnectOptionTest004, TestSize.Level1)
{
    ConnectOption connInfo = {
        .type = CONNECT_SLE_DIRECT
    };
    (void)strcpy_s(connInfo.sleDirectOption.networkId, NETWORK_ID_BUF_LEN, "1284456419466");

    ConnectOption itemConnInfo;
    (void)strcpy_s(itemConnInfo.sleDirectOption.networkId, NETWORK_ID_BUF_LEN, "148754644566");

    bool ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: CompareConnectOptionTest005
 * @tc.desc: test CompareConnectOption
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, CompareConnectOptionTest005, TestSize.Level1)
{
    ConnectOption itemConnInfo;
    ConnectOption connInfo = {
        .type = CONNECT_TYPE_MAX
    };

    bool ret = CompareConnectOption(&itemConnInfo, &connInfo);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: TransProxyConnExistProc001
 * @tc.desc: test TransProxyConnExistProc
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyConnExistProc001, TestSize.Level1)
{
    ConnectOption connInfo;
    (void)memset_s(&connInfo, sizeof(ConnectOption), 1, sizeof(ConnectOption));
    int32_t chanNewId = 1;
    bool isServer = false;
    TransCreateConnByConnId(1, isServer);
    int32_t ret = TransProxyConnExistProc(isServer, chanNewId, &connInfo);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_CONN_ADD_REF_FAILED, ret);
}

/*
 * @tc.name: TransProxyConnectDevice001
 * @tc.desc: test TransProxyConnectDevice
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyConnectDevice001, TestSize.Level1)
{
    ConnectOption connInfo;
    connInfo.type = CONNECT_BLE_DIRECT;
    uint32_t reqId = ConnGetNewRequestId(MODULE_PROXY_CHANNEL);
    int32_t ret = TransProxyConnectDevice(&connInfo, reqId);
    EXPECT_NE(SOFTBUS_OK, ret);

    connInfo.type = CONNECT_TYPE_MAX;
    ret = TransProxyConnectDevice(&connInfo, reqId);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransProxySendBadKeyMessagel001
 * @tc.desc: test TransProxySendBadKeyMessage
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxySendBadKeyMessagel001, TestSize.Level1)
{
    int32_t channelId = -1;
    ConnectOption connInfo;
    ListenerModule moduleId = PROXY;
    connInfo.type = CONNECT_TCP;
    int32_t ret = TransProxyOpenNewConnChannel(moduleId, &connInfo, channelId);
    EXPECT_NE(SOFTBUS_OK, ret);
}

/*
 * @tc.name: TransProxyTransInitl001
 * @tc.desc: test TransProxyTransInit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyTransInit001, TestSize.Level1)
{
    int32_t ret = TransProxyTransInit();
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransDelConnByReqId001
 * @tc.desc: test TransDelConnByReqId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransDelConnByReqId001, TestSize.Level1)
{
    uint32_t reqId = ConnGetNewRequestId(MODULE_PROXY_CHANNEL);
    int32_t ret = TransDelConnByReqId(reqId);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    g_proxyConnectionList = CreateSoftBusList();
    ret = TransDelConnByReqId(reqId);
    EXPECT_EQ(SOFTBUS_OK, ret);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransDelConnByReqId002
 * @tc.desc: test TransDelConnByReqId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransDelConnByReqId002, TestSize.Level1)
{
    uint32_t reqId = ConnGetNewRequestId(MODULE_PROXY_CHANNEL);
    int32_t ret = TransDelConnByReqId(reqId);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ProxyConnInfo *removeNode = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, removeNode);
    removeNode->requestId = reqId;
    removeNode->state = PROXY_CHANNEL_STATUS_PYH_CONNECTING;

    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ListAdd(&(g_proxyConnectionList->list), &(removeNode->node));
    g_proxyConnectionList->cnt++;
    ret = TransDelConnByReqId(reqId);
    EXPECT_EQ(SOFTBUS_OK, ret);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransDelConnByConnId001
 * @tc.desc: test TransDelConnByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransDelConnByConnId001, TestSize.Level1)
{
    TransDelConnByConnId(0);
    EXPECT_EQ(nullptr, g_proxyConnectionList);
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    TransDelConnByConnId(0);
    uint32_t connId = 1;
    ProxyConnInfo *removeNode = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, removeNode);
    removeNode->connId = connId;
    ListAdd(&(g_proxyConnectionList->list), &(removeNode->node));
    g_proxyConnectionList->cnt++;
    TransDelConnByConnId(connId);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransDecConnRefByConnId001
 * @tc.desc: test TransDecConnRefByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransDecConnRefByConnId001, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    uint32_t connId1 = 1;
    ProxyConnInfo *removeNode1 = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, removeNode1);
    removeNode1->connId = connId1;
    removeNode1->isServerSide = true;
    removeNode1->ref = 1;
    ListAdd(&(g_proxyConnectionList->list), &(removeNode1->node));
    g_proxyConnectionList->cnt++;
    uint32_t connId2 = 2;
    ProxyConnInfo *removeNode2 = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, removeNode2);
    removeNode1->connId = connId1;
    removeNode1->isServerSide = false;
    removeNode1->ref = 2;
    ListAdd(&(g_proxyConnectionList->list), &(removeNode2->node));
    g_proxyConnectionList->cnt++;
    int32_t ret = TransDecConnRefByConnId(connId1, true, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransDecConnRefByConnId(connId2, false, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SoftBusFree(removeNode1);
    SoftBusFree(removeNode2);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransDecConnRefByConnId002
 * @tc.desc: test TransDecConnRefByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransDecConnRefByConnId002, TestSize.Level1)
{
    ProxyConnInfo *chan = reinterpret_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    EXPECT_NE(nullptr, chan);
    chan->connId = 123; // test value
    chan->isServerSide = true;
    chan->ref = 1;

    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption.type = CONNECT_BR;
    chan->connInfo = connectOption;
    int32_t ret = TransAddConnItem(chan);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransDecConnRefByConnId(123, true, false); // test value
    EXPECT_EQ(SOFTBUS_OK, ret);

    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransDecConnRefByConnId003
 * @tc.desc: test TransDecConnRefByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransDecConnRefByConnId003, TestSize.Level1)
{
    ProxyConnInfo *chan = reinterpret_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    EXPECT_NE(nullptr, chan);
    chan->connId = 123; // test value
    chan->isServerSide = true;
    chan->ref = 5; // test value

    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption.type = CONNECT_BR;
    chan->connInfo = connectOption;
    int32_t ret = TransAddConnItem(chan);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = TransAddConnItem(chan);
    EXPECT_EQ(SOFTBUS_TRANS_NOT_MATCH, ret);
    ret = TransDecConnRefByConnId(123, true, false); // test value
    EXPECT_EQ(SOFTBUS_TRANS_NOT_MATCH, ret);

    SoftBusFree(chan);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransProxyPostOpenCloseMsgToLoop001
 * @tc.desc: test TransDecConnRefByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyPostOpenCloseMsgToLoop001, TestSize.Level1)
{
    int32_t ret = TransAddConnItem(nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ProxyConnInfo *chan = reinterpret_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    EXPECT_NE(nullptr, chan);
    chan->connId = 10; // test value
    chan->requestId = 10; // test value
    chan->state = PROXY_CHANNEL_STATUS_PYH_CONNECTING;
    chan->isServerSide = true;
    chan->ref = 1;

    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption.type = CONNECT_BR;
    chan->connInfo = connectOption;
    ret = TransAddConnItem(chan);
    EXPECT_EQ(SOFTBUS_OK, ret);

    uint32_t state = 1;
    ret = TransSetConnStateByReqId(chan->requestId, chan->connId, state);
    EXPECT_EQ(SOFTBUS_OK, ret);

    SoftBusFree(chan);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: CheckIsProxyAuthChannel001
 * @tc.desc: test TransDecConnRefByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, CheckIsProxyAuthChannel001, TestSize.Level1)
{
    int32_t ret = CheckIsProxyAuthChannel(nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    ret = CheckIsProxyAuthChannel(&connectOption);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
}

/*
 * @tc.name: TransAddConnRefByConnId001
 * @tc.desc: test TransAddConnRefByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransAddConnRefByConnId001, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    uint32_t connId = 1;
    ProxyConnInfo removeNode;
    removeNode.connId = connId;
    removeNode.isServerSide = true;
    ListAdd(&(g_proxyConnectionList->list), &(removeNode.node));
    g_proxyConnectionList->cnt++;
    int32_t ret = TransAddConnRefByConnId(connId, true);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = TransAddConnRefByConnId(connId, false);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_CONN_ADD_REF_FAILED, ret);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransProxyOnConnectedAndDisConnect001
 * @tc.desc: test TransProxyOnConnectedAndDisConnect
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnConnectedAndDisConnect001, TestSize.Level1)
{
    uint32_t connId = 1;
    ConnectionInfo connectionInfo;
    TransProxyOnConnected(connId, &connectionInfo);
    EXPECT_EQ(nullptr, g_proxyConnectionList);
}

/*
 * @tc.name: TransAddConnItem001
 * @tc.desc: test TransAddConnItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransAddConnItem001, TestSize.Level1)
{
    ProxyConnInfo chan;
    int32_t ret =  TransAddConnItem(&chan);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ProxyConnInfo *connChan1 = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, connChan1);
    connChan1->isServerSide = false;
    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption.type = CONNECT_BR;
    connChan1->connInfo = connectOption;
    ret = TransAddConnItem(connChan1);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SoftBusFree(connChan1);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransAddConnItem002
 * @tc.desc: test TransAddConnItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransAddConnItem002, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ProxyConnInfo *connChanSrc = reinterpret_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    connChanSrc->isServerSide = 1;
    connChanSrc->state = PROXY_CHANNEL_STATUS_PYH_CONNECTING;
    int32_t ret = TransAddConnItem(connChanSrc);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ProxyConnInfo *connChanDst = reinterpret_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    EXPECT_NE(nullptr, connChanDst);
    connChanDst->isServerSide = false;
    ConnectOption connectOption;
    (void)memset_s(&connectOption, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption.type = CONNECT_BR;
    connChanDst->connInfo = connectOption;
    ret = TransAddConnItem(connChanDst);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SoftBusFree(connChanDst);
    SoftBusFree(connChanSrc);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransConnInfoToConnOpt001
 * @tc.desc: test TransConnInfoToConnOpt
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransConnInfoToConnOpt001, TestSize.Level1)
{
    ConnectionInfo connInfo;
    ConnectOption connOption;

    connInfo.type = CONNECT_BR;
    TransConnInfoToConnOpt(&connInfo, &connOption);
    connInfo.type = CONNECT_BLE;
    TransConnInfoToConnOpt(&connInfo, &connOption);
    connInfo.type = CONNECT_P2P;
    TransConnInfoToConnOpt(&connInfo, &connOption);
    EXPECT_EQ(nullptr, g_proxyConnectionList);
}

/*
 * @tc.name: TransConnInfoToConnOpt002
 * @tc.desc: test TransConnInfoToConnOpt
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransConnInfoToConnOpt002, TestSize.Level1)
{
    ConnectionInfo connInfo = {
        .type = CONNECT_SLE
    };
    (void)strcpy_s(connInfo.sleInfo.address, BT_MAC_LEN, "Fjhfwofwho");
    (void)strcpy_s(connInfo.sleInfo.networkId, NETWORK_ID_BUF_LEN, "165489675616");
    ConnectOption connOption;

    EXPECT_NO_FATAL_FAILURE(TransConnInfoToConnOpt(&connInfo, &connOption));
}

/*
 * @tc.name: TransCreateConnByConnId001
 * @tc.desc: test TransCreateConnByConnId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransCreateConnByConnId001, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    uint32_t connId = 1;
    ProxyConnInfo proxyConnInfo;
    proxyConnInfo.connId = connId;
    proxyConnInfo.isServerSide = true;
    ListAdd(&(g_proxyConnectionList->list), &(proxyConnInfo.node));
    TransCreateConnByConnId(connId, true);
    EXPECT_NE(g_proxyConnectionList, nullptr);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransGetConn001
 * @tc.desc: test TransGetConn
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransGetConn001, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    ProxyConnInfo *connChan1 = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, connChan1);
    connChan1->isServerSide = false;
    ConnectOption connectOption1;
    (void)memset_s(&connectOption1, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption1.type = CONNECT_BR;
    connChan1->connInfo = connectOption1;
    ListAdd(&(g_proxyConnectionList->list), &(connChan1->node));

    ProxyConnInfo *connChan2 = (ProxyConnInfo *)SoftBusCalloc(sizeof(ProxyConnInfo));
    EXPECT_NE(nullptr, connChan2);
    connChan2->isServerSide = false;
    ConnectOption connectOption2;
    (void)memset_s(&connectOption2, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    connectOption2.type = CONNECT_BR;

    int32_t ret = TransGetConn(&connectOption2, connChan2, false);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SoftBusFree(connChan1);
    SoftBusFree(connChan2);
    EXPECT_NE(g_proxyConnectionList, nullptr);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransGetConn001
 * @tc.desc: test TransGetConn.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransGetConn002, TestSize.Level1)
{
    ConnectOption connInfo;
    memset_s(&connInfo, sizeof(ConnectOption), 0, sizeof(ConnectOption));
    ProxyConnInfo proxyConn;
    memset_s(&proxyConn, sizeof(ProxyConnInfo), 0, sizeof(ProxyConnInfo));
    bool isServer = false;
    int32_t ret = TransGetConn(&connInfo, &proxyConn, isServer);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(g_proxyConnectionList, nullptr);
    ret = TransGetConn(nullptr, &proxyConn, isServer);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    ret = TransGetConn(&connInfo, nullptr, isServer);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = TransGetConn(&connInfo, &proxyConn, isServer);
    EXPECT_EQ(SOFTBUS_TRANS_NOT_MATCH, ret);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransProxySendBadKeyMessage001
 * @tc.desc: test TransProxySendBadKeyMessage
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxySendBadKeyMessage001, TestSize.Level1)
{
    ProxyMessage msg;
    memset_s(&msg, sizeof(ProxyMessage), 0, sizeof(ProxyMessage));
    const char *identity = TEST_STRING_IDENTITY;
    msg.data = TransProxyPackIdentity(identity);
    msg.dataLen = 9;
    AuthHandle authHandle = { .authId = AUTH_INVALID_ID };
    int32_t ret = TransProxySendBadKeyMessage(&msg, &authHandle);
    EXPECT_EQ(SOFTBUS_CONN_MANAGER_TYPE_NOT_SUPPORT, ret);
}

/*
 * @tc.name: TransSetConnStateByReqId001
 * @tc.desc: test TransSetConnStateByReqId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransSetConnStateByReqId001, TestSize.Level1)
{
    uint32_t requestId = 1;
    uint32_t connId = 1;
    uint32_t state = PROXY_CHANNEL_STATUS_PYH_CONNECTING;
    ProxyConnInfo proxyConnInfo;
    proxyConnInfo.requestId = requestId;
    proxyConnInfo.state = state;
    TransSetConnStateByReqId(requestId, connId, state);
    g_proxyConnectionList = CreateSoftBusList();
    EXPECT_NE(nullptr, g_proxyConnectionList);
    TransSetConnStateByReqId(requestId, connId, state);
    EXPECT_NE(nullptr, g_proxyConnectionList);
    TransSetConnStateByReqId(requestId, connId, state);
    DestroySoftBusList(g_proxyConnectionList);
    g_proxyConnectionList = nullptr;
}

/*
 * @tc.name: TransOnConnectSucceedAndFailed001
 * @tc.desc: test TransOnConnectSucceedAndFailed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransOnConnectSucceedAndFailed001, TestSize.Level1)
{
    uint32_t requestId = 1;
    uint32_t connectionId = 1;
    ConnectionInfo connInfo;
    TransOnConnectSucceed(requestId, connectionId, &connInfo);
    uint32_t reason = 0;
    TransOnConnectFailed(requestId, reason);
    EXPECT_EQ(nullptr, g_proxyConnectionList);
}

/*
 * @tc.name: TransProxyCreateLoopMsg001
 * @tc.desc: test TransProxyCreateLoopMsg
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyCreateLoopMsg001, TestSize.Level1)
{
    const char *chan = "testchan";
    SoftBusMessage *ret = TransProxyCreateLoopMsg(LOOP_RESETPEER_MSG, 0,
        0, const_cast<char *>(chan));
    EXPECT_NE(nullptr, ret);
}

/*
 * @tc.name: TransProxyPostAuthNegoMsgToLooperDelay001
 * @tc.desc: test TransProxyPostAuthNegoMsgToLooperDelay
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyPostAuthNegoMsgToLooperDelay001, TestSize.Level1)
{
    int32_t authRequestId = 1;
    int32_t channelId = 1;
    uint32_t delayTime = 0;
    TransProxyPostAuthNegoMsgToLooperDelay(authRequestId, channelId, delayTime);
    EXPECT_NE(nullptr, g_transLoopHandler.looper);
}

/*
 * @tc.name: TransProxyLoopMsgHandler001
 * @tc.desc: test TransProxyLoopMsgHandler
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyLoopMsgHandler001, TestSize.Level1)
{
    TransProxyLoopMsgHandler(nullptr);
    SoftBusMessage *msg = (SoftBusMessage *)SoftBusCalloc(sizeof(SoftBusMessage));
    EXPECT_NE(nullptr, msg);
    ProxyChannelInfo chan;
    int32_t channelId = 1;
    msg->obj = reinterpret_cast<void *>(&channelId);
    msg->what = LOOP_HANDSHAKE_MSG;
    TransProxyLoopMsgHandler(msg);
    msg->obj = reinterpret_cast<void *>(&chan);
    msg->what = LOOP_DISCONNECT_MSG;
    TransProxyLoopMsgHandler(msg);
    msg->what = LOOP_OPENFAIL_MSG;
    TransProxyLoopMsgHandler(msg);
    msg->what = LOOP_OPENCLOSE_MSG;
    TransProxyLoopMsgHandler(msg);
    msg->what = LOOP_KEEPALIVE_MSG;
    TransProxyLoopMsgHandler(msg);
    msg->what = LOOP_RESETPEER_MSG;
    TransProxyLoopMsgHandler(msg);
    msg->what = LOOP_AUTHSTATECHECK_MSG;
    TransProxyLoopMsgHandler(msg);
}

/*
 * @tc.name: TransProxyPostResetPeerMsgToLoopTest001
 * @tc.desc: TransProxyPostResetPeerMsgToLoop test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyPostResetPeerMsgToLoopTest001, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(TransProxyPostResetPeerMsgToLoop(nullptr));
}

/*
 * @tc.name: TransProxyPostDisConnectMsgToLoopTest001
 * @tc.desc: TransProxyPostDisConnectMsgToLoop test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyPostDisConnectMsgToLoopTest001, TestSize.Level1)
{
    uint32_t connId = 1032;
    EXPECT_NO_FATAL_FAILURE(TransProxyPostDisConnectMsgToLoop(connId, true, nullptr));
}

/*
 * @tc.name: TransProxyOnDisConnectTest001
 * @tc.desc: TransProxyOnDisConnect test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDisConnectTest001, TestSize.Level1)
{
    uint32_t connId = 1032;
    ConnectionInfo *connInfo = static_cast<ConnectionInfo *>(SoftBusCalloc(sizeof(ConnectionInfo)));

    EXPECT_NO_FATAL_FAILURE(TransProxyOnDisConnect(connId, connInfo));
    SoftBusFree(connInfo);
}

/*
 * @tc.name: TransReportStartConnectEventTest001
 * @tc.desc: TransReportStartConnectEvent test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransReportStartConnectEventTest001, TestSize.Level1)
{
    AppInfo appInfo = {
        .appType = APP_TYPE_AUTH,
    };
    (void)strcpy_s(appInfo.myData.sessionName, SESSION_NAME_SIZE_MAX, "com.wanna.yeel");

    int32_t channelId = 1024;
    ConnectOption connInfo {
        .type = CONNECT_SLE_DIRECT
    };

    EXPECT_NO_FATAL_FAILURE(TransReportStartConnectEvent(&appInfo, &connInfo, channelId));
}

/*
 * @tc.name: TransProxyOnDataReceivedTest001
 * @tc.desc: TransProxyOnDataReceived test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceivedTest001, TestSize.Level1)
{
    uint32_t connectionId = 304513;
    int64_t seq = 121;
    
    EXPECT_NO_FATAL_FAILURE(
        TransProxyOnDataReceived(connectionId, MODULE_BLUETOOTH_MANAGER, seq, nullptr, TEST_DATA_LEN));
}

/*
 * @tc.name: TransProxyOnDataReceivedTest002
 * @tc.desc: TransProxyOnDataReceived test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceivedTest002, TestSize.Level1)
{
    uint32_t connectionId = 304513;
    int64_t seq = 121;
    char data[TEST_DATA_LEN];
    
    EXPECT_NO_FATAL_FAILURE(
        TransProxyOnDataReceived(connectionId, MODULE_BLUETOOTH_MANAGER, seq, data, TEST_DATA_LEN));
}

/*
 * @tc.name: TransProxyOnDataReceivedTest003
 * @tc.desc: TransProxyOnDataReceived test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceivedTest003, TestSize.Level1)
{
    uint32_t connectionId = 304513;
    int64_t seq = 121;
    char data[TEST_DATA_LEN];
    
    ProxyMessage msg;
    msg.msgHead.type = PROXYCHANNEL_MSG_TYPE_HANDSHAKE;
    SoftbusTransProxyTransceiverMock transceiverMockObj;
    EXPECT_CALL(transceiverMockObj, TransProxyParseMessage)
        .WillRepeatedly(DoAll(SetArgPointee<2>(msg), Return(SOFTBUS_AUTH_NOT_FOUND)));

    EXPECT_NO_FATAL_FAILURE(
        TransProxyOnDataReceived(connectionId, MODULE_PROXY_CHANNEL, seq, data, TEST_DATA_LEN));
}

/*
 * @tc.name: TransProxyOnDataReceivedTest004
 * @tc.desc: TransProxyOnDataReceived test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceivedTest004, TestSize.Level1)
{
    uint32_t connectionId = 304513;
    int64_t seq = 121;
    char data[TEST_DATA_LEN];
    
    ProxyMessage msg;
    msg.msgHead.type = PROXYCHANNEL_MSG_TYPE_HANDSHAKE;
    SoftbusTransProxyTransceiverMock transceiverMockObj;
    EXPECT_CALL(transceiverMockObj, TransProxyParseMessage)
        .WillRepeatedly(DoAll(SetArgPointee<2>(msg), Return(SOFTBUS_DECRYPT_ERR)));

    EXPECT_NO_FATAL_FAILURE(
        TransProxyOnDataReceived(connectionId, MODULE_PROXY_CHANNEL, seq, data, TEST_DATA_LEN));
}

/*
 * @tc.name: TransProxyOnDataReceivedTest005
 * @tc.desc: TransProxyOnDataReceived test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceivedTest005, TestSize.Level1)
{
    uint32_t connectionId = 304513;
    int64_t seq = 121;
    char data[TEST_DATA_LEN];
    
    SoftbusTransProxyTransceiverMock transceiverMockObj;
    EXPECT_CALL(transceiverMockObj, TransProxyParseMessage).WillRepeatedly(Return(SOFTBUS_TRANS_RECV_DATA_OVER_LEN));

    EXPECT_NO_FATAL_FAILURE(
        TransProxyOnDataReceived(connectionId, MODULE_PROXY_CHANNEL, seq, data, TEST_DATA_LEN));
}

/*
 * @tc.name: TransProxyOnDataReceivedTest006
 * @tc.desc: TransProxyOnDataReceived test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceivedTest006, TestSize.Level1)
{
    uint32_t connectionId = 304513;
    int64_t seq = 121;
    char data[TEST_DATA_LEN];
    
    SoftbusTransProxyTransceiverMock transceiverMockObj;
    EXPECT_CALL(transceiverMockObj, TransProxyParseMessage).WillRepeatedly(Return(SOFTBUS_OK));

    EXPECT_NO_FATAL_FAILURE(
        TransProxyOnDataReceived(connectionId, MODULE_PROXY_CHANNEL, seq, data, TEST_DATA_LEN));
}

/*
 * @tc.name: TransProxyUdpateNewPeerUdidHashTest001
 * @tc.desc: TransProxyUdpateNewPeerUdidHash test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyUdpateNewPeerUdidHashTest001, TestSize.Level1)
{
    const char *deviceId = "hnfwoeiuyfownfoplqwrfh23092";
    ConnectOption connOpt;
    
    SoftbusTransProxyTransceiverMock transceiverMockObj;
    EXPECT_CALL(transceiverMockObj, SoftBusGenerateStrHash).WillOnce(Return(SOFTBUS_ENCRYPT_ERR));

    int32_t ret = TransProxyUdpateNewPeerUdidHash(deviceId, &connOpt);
    EXPECT_EQ(ret, SOFTBUS_ENCRYPT_ERR);
}

/*
 * @tc.name: TransProxyUdpateNewPeerUdidHashTest002
 * @tc.desc: TransProxyUdpateNewPeerUdidHash test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyUdpateNewPeerUdidHashTest002, TestSize.Level1)
{
    ConnectOption connOpt;
    const char *deviceId = "hnfwoeiuyfownfoplqwrfh23092";
    char udidHash[UDID_HASH_LEN] = { 0 };
    (void)strcpy_s(udidHash, UDID_HASH_LEN, "nhfwwoafhweo");

    SoftbusTransProxyTransceiverMock transceiverMockObj;
    EXPECT_CALL(transceiverMockObj, SoftBusGenerateStrHash)
        .WillRepeatedly(DoAll(SetArgPointee<2>(*udidHash), Return(SOFTBUS_OK)));

    int32_t ret = TransProxyUdpateNewPeerUdidHash(deviceId, &connOpt);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: TransProxyResetAndCloseConnTest001
 * @tc.desc: TransProxyResetAndCloseConn test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyResetAndCloseConnTest001, TestSize.Level1)
{
    g_proxyConnectionList = CreateSoftBusList();
    ASSERT_TRUE(g_proxyConnectionList != nullptr);
    g_proxyConnectionList->cnt = 1;

    ProxyConnInfo *proxyConnInfo = static_cast<ProxyConnInfo *>(SoftBusCalloc(sizeof(ProxyConnInfo)));
    ASSERT_TRUE(proxyConnInfo != nullptr);
    proxyConnInfo->connId = 1234;
    proxyConnInfo->isServerSide = true;
    proxyConnInfo->ref = 1;

    ListAdd(&(g_proxyConnectionList->list), &(proxyConnInfo->node));

    ProxyChannelInfo chan = {
        .connId = 1234,
        .isServer = true,
        .deviceTypeIsWinpc = true,
        .myId = 4380,
        .peerId = 2599,
        .appInfo.appType = APP_TYPE_AUTH
    };

    int32_t ret = TransProxyResetAndCloseConn(&chan);
    EXPECT_EQ(ret, SOFTBUS_OK);

    chan.isServer = false;
    ret = TransProxyResetAndCloseConn(&chan);
    EXPECT_EQ(ret, SOFTBUS_OK);

    chan.deviceTypeIsWinpc = false;
    ret = TransProxyResetAndCloseConn(&chan);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: TransProxyOnDataReceived001
 * @tc.desc: test TransProxyOnDataReceived
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusProxyTransceiverTest, TransProxyOnDataReceived001, TestSize.Level1)
{
    ProxyMessageShortHead msgHead = { 0 };
    char data[TEST_DATALEN];
    int32_t len = TEST_DATALEN;
    msgHead.type = (PROXYCHANNEL_MSG_TYPE_D2D & FOUR_BIT_MASK) | (VERSION << VERSION_SHIFT);
    msgHead.myId = 1;
    msgHead.peerId = 1;
    (void)memcpy_s(data, TEST_DATALEN, &msgHead, sizeof(ProxyMessageShortHead));
    uint32_t connectionId = 1;
    ConnModule moduleId = MODULE_CONNECTION;
    int64_t seq = 1;
    EXPECT_NO_FATAL_FAILURE(TransProxyOnDataReceived(connectionId, moduleId, seq, data, len));

    msgHead.type = (PROXYCHANNEL_MSG_TYPE_NORMAL & FOUR_BIT_MASK) | (VERSION << VERSION_SHIFT);
    EXPECT_NO_FATAL_FAILURE(TransProxyOnDataReceived(connectionId, moduleId, seq, data, len));
}
} // namespace OHOS
