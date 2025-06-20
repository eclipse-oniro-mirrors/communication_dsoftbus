/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <securec.h>
#include <thread>

#include "lnn_feature_capability.h"
#include "lnn_lane_net_capability_mock.h"
#include "lnn_lane_common.h"
#include "lnn_lane_deps_mock.h"
#include "lnn_lane_interface.h"
#include "lnn_lane_link_deps_mock.h"
#include "lnn_lane_link_p2p.h"
#include "lnn_lane_listener.h"
#include "lnn_select_rule.h"

namespace OHOS {
using namespace testing::ext;
using namespace testing;

constexpr char NODE_NETWORK_ID[] = "123456789";
constexpr int32_t SYNCFAIL = 0;
constexpr int32_t SYNCSUCC = 1;
constexpr int32_t ASYNCFAIL = 2;
constexpr int32_t ASYNCSUCC = 3;
constexpr uint64_t DEFAULT_LINK_LATENCY = 30000;
constexpr int32_t SOFTBUS_LNN_PTK_NOT_MATCH  = (SOFTBUS_ERRNO(SHORT_DISTANCE_MAPPING_MODULE_CODE) + 4);
constexpr int32_t LANE_CAP_VALUE = 63;
constexpr uint32_t LANE_LINK_TYPE_NUM = 5;
constexpr int32_t DEFAULT_CAP_VALUE = 65535;
constexpr int32_t CAP_VALUE = 49151;

static SoftBusCond g_cond = {0};
static SoftBusMutex g_lock = {0};
static bool g_isRawHmlResuse = true;
static bool g_isNeedCondWait = true;
int32_t g_laneLinkResult = SOFTBUS_INVALID_PARAM;
int32_t g_connFailReason = ERROR_WIFI_DIRECT_WAIT_REUSE_RESPONSE_TIMEOUT;
uint32_t g_laneReqId = 1000;

class LNNLaneLinkExtTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LNNLaneLinkExtTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "LNNLaneLinkExtTest start";
    (void)SoftBusMutexInit(&g_lock, nullptr);
    (void)SoftBusCondInit(&g_cond);
    LnnInitLnnLooper();
}

void LNNLaneLinkExtTest::TearDownTestCase()
{
    LnnDeinitLnnLooper();
    (void)SoftBusCondDestroy(&g_cond);
    (void)SoftBusMutexDestroy(&g_lock);
    GTEST_LOG_(INFO) << "LNNLaneLinkExtTest end";
}

void LNNLaneLinkExtTest::SetUp()
{
}

void LNNLaneLinkExtTest::TearDown()
{
}

static void CondSignal(void)
{
    if (SoftBusMutexLock(&g_lock) != SOFTBUS_OK) {
        GTEST_LOG_(ERROR) << "mutex lock fail";
        return;
    }
    if (SoftBusCondSignal(&g_cond) != SOFTBUS_OK) {
        GTEST_LOG_(ERROR) << "cond signal fail";
        (void)SoftBusMutexUnlock(&g_lock);
        return;
    }
    g_isNeedCondWait = false;
    (void)SoftBusMutexUnlock(&g_lock);
}

static void CondWait(void)
{
    if (SoftBusMutexLock(&g_lock) != SOFTBUS_OK) {
        GTEST_LOG_(ERROR) << "mutex lock fail";
        return;
    }
    if (!g_isNeedCondWait) {
        GTEST_LOG_(ERROR) << "has cond signal, no need cond wait";
        (void)SoftBusMutexUnlock(&g_lock);
        return;
    }
    if (SoftBusCondWait(&g_cond, &g_lock, nullptr) != SOFTBUS_OK) {
        GTEST_LOG_(ERROR) << "cond wait fail";
        (void)SoftBusMutexUnlock(&g_lock);
        return;
    }
    (void)SoftBusMutexUnlock(&g_lock);
}

static void SetIsNeedCondWait(bool isNeedWait)
{
    if (SoftBusMutexLock(&g_lock) != SOFTBUS_OK) {
        GTEST_LOG_(ERROR) << "mutex lock fail";
        return;
    }
    g_isNeedCondWait = isNeedWait;
    (void)SoftBusMutexUnlock(&g_lock);
}

static void OnLaneLinkSuccess(uint32_t reqId, LaneLinkType linkType, const LaneLinkInfo *linkInfo)
{
    (void)reqId;
    (void)linkType;
    (void)linkInfo;
    g_laneLinkResult = SOFTBUS_OK;
    CondSignal();
    return;
}

static void OnLaneLinkFail(uint32_t reqId, int32_t reason, LaneLinkType linkType)
{
    (void)reqId;
    (void)reason;
    (void)linkType;
    g_laneLinkResult = reason;
    CondSignal();
    return;
}

static LaneLinkCb g_linkCb = {
    .onLaneLinkSuccess = OnLaneLinkSuccess,
    .onLaneLinkFail = OnLaneLinkFail,
};

static bool IsNegotiateChannelNeeded(const char *remoteNetworkId, enum WifiDirectLinkType linkType)
{
    return false;
}

static bool IsNegotiateChannelNeededTrue(const char *remoteNetworkId, enum WifiDirectLinkType linkType)
{
    return true;
}

static uint32_t GetRequestId(void)
{
    return 1;
}

static int32_t ConnectDevice(struct WifiDirectConnectInfo *info, struct WifiDirectConnectCallback *callback)
{
    if (info->pid == SYNCFAIL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (info->pid == SYNCSUCC) {
        return SOFTBUS_OK;
    }
    if (info->pid == ASYNCFAIL) {
        callback->onConnectFailure(info->requestId, g_connFailReason);
        return SOFTBUS_OK;
    }
    struct WifiDirectLink link = {
        .linkId = 1,
        .linkType = WIFI_DIRECT_LINK_TYPE_P2P,
    };
    callback->onConnectSuccess(info->requestId, &link);
    return SOFTBUS_OK;
}

static int32_t ConnectDeviceRawHml(struct WifiDirectConnectInfo *info, struct WifiDirectConnectCallback *callback)
{
    if (info->pid == ASYNCFAIL) {
        callback->onConnectFailure(info->requestId, ERROR_WIFI_DIRECT_WAIT_REUSE_RESPONSE_TIMEOUT);
        return SOFTBUS_OK;
    }
    struct WifiDirectLink link = {
        .linkId = 1,
        .linkType = WIFI_DIRECT_LINK_TYPE_HML,
        .isReuse = g_isRawHmlResuse,
    };
    callback->onConnectSuccess(info->requestId, &link);
    return SOFTBUS_OK;
}

static int32_t DisconnectDevice(struct WifiDirectDisconnectInfo *info, struct WifiDirectDisconnectCallback *callback)
{
    (void)info;
    (void)callback;
    return SOFTBUS_OK;
}

static int32_t DisconnectDevice2(struct WifiDirectDisconnectInfo *info, struct WifiDirectDisconnectCallback *callback)
{
    (void)info;
    (void)callback;
    return SOFTBUS_INVALID_PARAM;
}

static int32_t CancelConnectDevice(const struct WifiDirectConnectInfo *info)
{
    GTEST_LOG_(INFO) << "CancelConnectDevice enter";
    (void)info;
    return SOFTBUS_OK;
}

static bool SupportHmlTwo(void)
{
    return true;
}

static struct WifiDirectManager g_manager = {
    .isNegotiateChannelNeeded= IsNegotiateChannelNeeded,
    .getRequestId = GetRequestId,
    .connectDevice = ConnectDevice,
    .cancelConnectDevice = CancelConnectDevice,
    .disconnectDevice = DisconnectDevice,
    .supportHmlTwo = SupportHmlTwo,
};

/*
* @tc.name: LNN_LANE_LINK_CONNDEVICE_TEST_001
* @tc.desc: test wifi directConnect fail, reason = SOFTBUS_LNN_PTK_NOT_MATCH
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_CONNDEVICE_TEST_001, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_HML;
    request.pid = ASYNCFAIL;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    request.actionAddr = 1;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    g_connFailReason = SOFTBUS_LNN_PTK_NOT_MATCH;

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, SetRemoteDynamicNetCap).WillRepeatedly(Return());
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND)).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneLinkMock, GetTransReqInfoByLaneReqId).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneLinkMock, LnnSyncPtk).WillRepeatedly(Return(SOFTBUS_OK));
    g_manager.connectDevice = ConnectDevice;
    g_manager.disconnectDevice = DisconnectDevice;
    g_manager.isNegotiateChannelNeeded = IsNegotiateChannelNeeded;
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&g_manager));

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}

/*
* @tc.name: LNN_LANE_LINK_CONNDEVICE_TEST_002
* @tc.desc: test wifiDirectConnect fail with resuse, reason = SOFTBUS_CONN_SOURCE_REUSE_LINK_FAILED
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_CONNDEVICE_TEST_002, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_HML;
    request.pid = ASYNCFAIL;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    request.actionAddr = 1;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    uint32_t requestId = 1;
    AuthConnInfo connInfo = {.type = AUTH_LINK_TYPE_P2P};
    g_connFailReason = SOFTBUS_CONN_SOURCE_REUSE_LINK_FAILED;

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, SetRemoteDynamicNetCap).WillRepeatedly(Return());
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneLinkMock, GetTransReqInfoByLaneReqId).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneLinkMock, FindLaneResourceByLinkType).WillRepeatedly(Return(SOFTBUS_OK));
    g_manager.connectDevice = ConnectDevice;
    g_manager.disconnectDevice = DisconnectDevice;
    g_manager.isNegotiateChannelNeeded = IsNegotiateChannelNeeded;
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&g_manager));
    EXPECT_CALL(laneMock, AuthGetConnInfoByType)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(connInfo), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, AuthGenRequestId).WillRepeatedly(Return(requestId));
    EXPECT_CALL(laneMock, AuthOpenConn(_, requestId, NotNull(), _)).WillRepeatedly(laneMock.ActionOfConnOpened);

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}

/*
* @tc.name: LNN_LANE_LINK_CONNDEVICE_TEST_003
* @tc.desc: test wifiDirectConnect fail, reason = SOFTBUS_CONN_HV3_WAIT_CONNECTION_TIMEOUT
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_CONNDEVICE_TEST_003, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_HML_RAW;
    request.pid = ASYNCFAIL;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    request.actionAddr = 1;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    g_connFailReason = SOFTBUS_CONN_HV2_WAIT_CONNECT_RESPONSE_TIMEOUT;

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, SetRemoteDynamicNetCap).WillRepeatedly(Return());
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneLinkMock, GetTransReqInfoByLaneReqId).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneLinkMock, FindLaneResourceByLinkType).WillRepeatedly(Return(SOFTBUS_OK));
    g_manager.connectDevice = ConnectDevice;
    g_manager.disconnectDevice = DisconnectDevice;
    g_manager.isNegotiateChannelNeeded = IsNegotiateChannelNeeded;
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&g_manager));
    EXPECT_CALL(laneMock, LnnRequestCheckOnlineStatus).WillRepeatedly(Return(SOFTBUS_OK));

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}

/*
* @tc.name: LNN_LANE_LINK_CONNDEVICE_TEST_004
* @tc.desc: test wifiDirectConnect fail, reason = SOFTBUS_CONN_P2P_STA_SAME_MAC
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_CONNDEVICE_TEST_004, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_P2P;
    request.pid = ASYNCFAIL;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    uint64_t local = 1 << BIT_SUPPORT_NEGO_P2P_BY_CHANNEL_CAPABILITY;
    uint64_t remote = 1 << BIT_SUPPORT_NEGO_P2P_BY_CHANNEL_CAPABILITY;
    uint32_t requestId = 1;
    g_connFailReason = SOFTBUS_CONN_P2P_STA_SAME_MAC;

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND)).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, LnnGetLocalNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(local), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, LnnGetRemoteNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(remote), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, AuthDeviceCheckConnInfo).WillRepeatedly(Return(false));
    EXPECT_CALL(laneMock, CheckActiveConnection).WillRepeatedly(Return(false));
    EXPECT_CALL(laneLinkMock, TransProxyPipelineGenRequestId).WillRepeatedly(Return(requestId));
    EXPECT_CALL(laneLinkMock, TransProxyPipelineOpenChannel(requestId, _, _, NotNull()))
        .WillRepeatedly(laneLinkMock.ActionOfChannelOpened);
    EXPECT_CALL(laneLinkMock, GetConflictTypeWithErrcode).WillRepeatedly(Return(CONFLICT_BUTT));
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&g_manager));

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_EQ(SOFTBUS_CONN_P2P_STA_SAME_MAC, g_laneLinkResult);
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}

/*
* @tc.name: LNN_LANE_LINK_CONNDEVICE_TEST_005
* @tc.desc: test wifiDirectConnect fail, reason = SOFTBUS_CONN_RETRYABLE_FAIL_WITH_CURRENT_GUIDE
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_CONNDEVICE_TEST_005, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_P2P;
    request.pid = ASYNCFAIL;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    uint64_t local = 1 << BIT_SUPPORT_NEGO_P2P_BY_CHANNEL_CAPABILITY;
    uint64_t remote = 1 << BIT_SUPPORT_NEGO_P2P_BY_CHANNEL_CAPABILITY;
    uint32_t requestId = 1;
    g_connFailReason = SOFTBUS_CONN_RETRYABLE_FAIL_WITH_CURRENT_GUIDE;

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND)).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, LnnGetLocalNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(local), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, LnnGetRemoteNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(remote), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, AuthDeviceCheckConnInfo).WillRepeatedly(Return(false));
    EXPECT_CALL(laneMock, CheckActiveConnection).WillRepeatedly(Return(false));
    EXPECT_CALL(laneLinkMock, TransProxyPipelineGenRequestId).WillRepeatedly(Return(requestId));
    EXPECT_CALL(laneLinkMock, TransProxyPipelineOpenChannel(requestId, _, _, NotNull()))
        .WillRepeatedly(laneLinkMock.ActionOfChannelOpened);
    EXPECT_CALL(laneLinkMock, GetConflictTypeWithErrcode).WillRepeatedly(Return(CONFLICT_BUTT));
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&g_manager));

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_EQ(SOFTBUS_CONN_RETRYABLE_FAIL_WITH_CURRENT_GUIDE, g_laneLinkResult);
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}

/*
* @tc.name: LNN_LANE_LINK_DISCONN_TEST_001
* @tc.desc: test lane link disconn error with auth opened fail
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_DISCONN_TEST_001, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_HML;
    request.pid = ASYNCSUCC;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    request.actionAddr = 1;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    uint32_t requestId = 1;
    g_isRawHmlResuse = false;
    AuthConnInfo connInfo = {.type = AUTH_LINK_TYPE_P2P};

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, SetRemoteDynamicNetCap).WillRepeatedly(Return());
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND)).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneLinkMock, GetTransReqInfoByLaneReqId).WillRepeatedly(Return(SOFTBUS_OK));
    struct WifiDirectManager manager1 = g_manager;
    manager1.connectDevice = ConnectDeviceRawHml;
    manager1.disconnectDevice = DisconnectDevice2;
    manager1.isNegotiateChannelNeeded = IsNegotiateChannelNeededTrue;
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&manager1));
    EXPECT_CALL(laneMock, AuthGetHmlConnInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(connInfo), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneMock, AuthGenRequestId).WillRepeatedly(Return(requestId));
    EXPECT_CALL(laneMock, AuthOpenConn(_, requestId, NotNull(), _)).WillRepeatedly(laneMock.ActionOfConnOpenFailed);
    EXPECT_CALL(laneMock, AuthCloseConn).WillRepeatedly(Return());

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}

/*
* @tc.name: SELECT_EXPECT_LANES_BY_QOS_TEST_001
* @tc.desc: test SelectExpectLanesByQos parameter reuseBestEffort is true
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, SELECT_EXPECT_LANES_BY_QOS_TEST_001, TestSize.Level1)
{
    NiceMock<LaneDepsInterfaceMock> mock;
    LanePreferredLinkList recommendList = {};
    LaneSelectParam request = {};
    request.qosRequire.reuseBestEffort = true;
    EXPECT_CALL(mock, LnnGetOnlineStateById).WillRepeatedly(Return(true));
    EXPECT_CALL(mock, LnnGetRemoteNodeInfoById).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = SelectExpectLanesByQos(NODE_NETWORK_ID, &request, &recommendList);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
* @tc.name: SELECT_AUTH_LANE_TEST_001
* @tc.desc: SelectAuthLane test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, SELECT_AUTH_LANE_TEST_001, TestSize.Level1)
{
    LanePreferredLinkList recommendList = {};
    LanePreferredLinkList request = {};
    request.linkTypeNum = 1;
    request.linkType[0] = LANE_BR;
    NiceMock<LaneDepsInterfaceMock> mock;
    EXPECT_CALL(mock, LnnGetLocalNumU32Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(LANE_CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(mock, LnnGetRemoteNumU32Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(LANE_CAP_VALUE), Return(SOFTBUS_OK)));
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, CheckStaticNetCap).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(capMock, CheckDynamicNetCap).WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = SelectAuthLane(NODE_NETWORK_ID, &request, &recommendList);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
* @tc.name: SELECT_LANE_TEST_001
* @tc.desc: SelectLane test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, SELECT_LANE_TEST_001, TestSize.Level1)
{
    LanePreferredLinkList linkList = {};
    uint32_t listNum = 0;
    LaneSelectParam selectParam = {};
    selectParam.transType = LANE_T_FILE;
    selectParam.expectedBw = 0;
    selectParam.list.linkTypeNum = LANE_LINK_TYPE_NUM;
    selectParam.list.linkType[0] = LANE_HML;
    selectParam.list.linkType[1] = LANE_P2P_REUSE;
    selectParam.list.linkType[2] = LANE_BLE_DIRECT;
    selectParam.list.linkType[3] = LANE_COC;
    selectParam.list.linkType[4] = LANE_COC_DIRECT;

    NiceMock<LaneDepsInterfaceMock> linkMock;
    EXPECT_CALL(linkMock, LnnGetOnlineStateById).WillRepeatedly(Return(true));
    EXPECT_CALL(linkMock, LnnGetLocalNumU32Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(DEFAULT_CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(linkMock, LnnGetRemoteNumU32Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(DEFAULT_CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(linkMock, LnnGetLocalNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(DEFAULT_CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(linkMock, LnnGetRemoteNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(DEFAULT_CAP_VALUE), Return(SOFTBUS_OK)));
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, CheckStaticNetCap).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(capMock, CheckDynamicNetCap).WillRepeatedly(Return(SOFTBUS_OK));

    int32_t ret = SelectLane(NODE_NETWORK_ID, &selectParam, &linkList, &listNum);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(listNum, LANE_LINK_TYPE_NUM);
}

/*
* @tc.name: SELECT_LANE_TEST_002
* @tc.desc: SelectLane test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, SELECT_LANE_TEST_002, TestSize.Level1)
{
    LanePreferredLinkList linkList = {};
    uint32_t listNum = 0;
    LaneSelectParam selectParam = {};
    selectParam.transType = LANE_T_BYTE;
    selectParam.expectedBw = 0;
    selectParam.list.linkTypeNum = 1;
    selectParam.list.linkType[0] = LANE_P2P;

    NiceMock<LaneDepsInterfaceMock> linkMock;
    EXPECT_CALL(linkMock, LnnGetOnlineStateById).WillRepeatedly(Return(true));
    EXPECT_CALL(linkMock, LnnGetLocalNumU32Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(linkMock, LnnGetRemoteNumU32Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(linkMock, LnnGetLocalNumU64Info)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillOnce(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(CAP_VALUE), Return(SOFTBUS_OK)));
    EXPECT_CALL(linkMock, LnnGetRemoteNumU64Info)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(CAP_VALUE), Return(SOFTBUS_OK)));
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, CheckStaticNetCap).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(capMock, CheckDynamicNetCap).WillRepeatedly(Return(SOFTBUS_OK));

    int32_t ret = SelectLane(NODE_NETWORK_ID, &selectParam, &linkList, &listNum);
    EXPECT_EQ(ret, SOFTBUS_LANE_NO_AVAILABLE_LINK);
}

/*
* @tc.name: LNN_LANE_LINK_RATE_PRE_TEST_001
* @tc.desc: test build raw hml link for rate preference
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNLaneLinkExtTest, LNN_LANE_LINK_RATE_PRE_TEST_001, TestSize.Level1)
{
    LinkRequest request = {};
    EXPECT_EQ(strcpy_s(request.peerNetworkId, NETWORK_ID_BUF_LEN, NODE_NETWORK_ID), EOK);
    request.linkType = LANE_HML_RAW;
    request.pid = ASYNCSUCC;
    request.triggerLinkTime = SoftBusGetSysTimeMs();
    request.availableLinkTime = DEFAULT_LINK_LATENCY;
    request.actionAddr = 1;
    uint32_t laneReqId = g_laneReqId++;
    int32_t value = 3;
    g_isRawHmlResuse = false;
    TransReqInfo reqInfo = {.allocInfo.qosRequire.ratePreference = true};

    NiceMock<LaneDepsInterfaceMock> laneMock;
    NiceMock<LaneLinkDepsInterfaceMock> laneLinkMock;
    EXPECT_CALL(laneMock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_NOT_FIND)).WillOnce(Return(SOFTBUS_NOT_FIND))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(laneMock, LnnGetRemoteNumInfo)
        .WillRepeatedly(DoAll(SetArgPointee<LANE_MOCK_PARAM3>(value), Return(SOFTBUS_OK)));
    EXPECT_CALL(laneLinkMock, GetTransReqInfoByLaneReqId).WillOnce(Return(SOFTBUS_OK))
        .WillOnce(DoAll(SetArgPointee<LANE_MOCK_PARAM2>(reqInfo), Return(SOFTBUS_OK)))
        .WillRepeatedly(Return(SOFTBUS_OK));
    g_manager.connectDevice = ConnectDeviceRawHml;
    EXPECT_CALL(laneMock, GetWifiDirectManager).WillRepeatedly(Return(&g_manager));
    EXPECT_CALL(laneLinkMock, CheckLaneLinkExistByType).WillRepeatedly(Return(true));
    NiceMock<LaneNetCapInterfaceMock> capMock;
    EXPECT_CALL(capMock, SetRemoteDynamicNetCap).WillRepeatedly(Return());

    SetIsNeedCondWait(true);
    int32_t ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));

    EXPECT_CALL(laneLinkMock, GetTransReqInfoByLaneReqId).WillOnce(Return(SOFTBUS_OK))
        .WillOnce(Return(SOFTBUS_NOT_FIND)).WillRepeatedly(Return(SOFTBUS_OK));
    SetIsNeedCondWait(true);
    ret = LnnConnectP2p(&request, laneReqId, &g_linkCb);
    EXPECT_EQ(SOFTBUS_OK, ret);
    CondWait();
    EXPECT_NO_FATAL_FAILURE(LnnDisconnectP2p(NODE_NETWORK_ID, laneReqId));
    EXPECT_NO_FATAL_FAILURE(LnnDestroyP2p());
}
} // namespace OHOS