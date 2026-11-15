/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#define private public
#define ENHANCED_FLAG

#include <gtest/gtest.h>

#include "auth_interface.h"
#include "device_auth.h"
#include "disc_log.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "session_ipc_adapter.h"
#include "softbus_access_token_test.h"
#include "softbus_adapter_mem.h"
#include "softbus_common.h"
#include "softbus_error_code.h"
#include "softbus_feature_config.h"
#include "softbus_server.h"
#include "softbus_server_frame.h"
#include "softbus_server_ipc_interface_code.h"
#include "softbus_server_stub.cpp"
#include "softbus_server_stub.h"
#include "softbus_server_stub_test_mock.h"
#include "system_ability_definition.h"
#include "trans_session_manager.h"
#include "trans_session_service.h"
#include "trans_type.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {

#define TEST_SESSION_NAME_SIZE_MAX 256
#define TEST_DEVICE_ID_SIZE_MAX    50
#define TEST_GROUP_ID_SIZE_MAX     50
#define TEST_PKG_NAME_SIZE_MAX     65
#define TEST_MAX_LEN               1025

char g_mySessionName[TEST_SESSION_NAME_SIZE_MAX] = "com.test.trans.session";
char g_peerSessionName[TEST_SESSION_NAME_SIZE_MAX] = "com.test.trans.session.sendfile";
char g_peerDeviceId[TEST_DEVICE_ID_SIZE_MAX] = "com.test.trans.session.sendfile";
char g_groupId[TEST_GROUP_ID_SIZE_MAX] = "com.test.trans.session.sendfile";
char g_myPkgName[TEST_PKG_NAME_SIZE_MAX] = "test";

class SoftbusServerStubTest : public testing::Test {
public:
    SoftbusServerStubTest()
    {}
    ~SoftbusServerStubTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void SoftbusServerStubTest::SetUpTestCase(void)
{
    SoftbusConfigInit();
    ConnServerInit();
    AuthInit();
    InitDeviceAuthService();
    BusCenterServerInit();
    TransServerInit();
}

void SoftbusServerStubTest::TearDownTestCase(void)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, UnregChangeListener).WillRepeatedly(Return(SOFTBUS_OK));
    ConnServerDeinit();
    AuthDeinit();
    BusCenterServerDeinit();
    TransServerDeinit();
}

SessionParam *GenerateSessionParam()
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    SessionParam *sessionParam = (SessionParam *)SoftBusCalloc(sizeof(SessionParam));
    EXPECT_NE(nullptr, sessionParam);
    SessionAttribute attr;
    attr.dataType = 1;
    attr.linkTypeNum = 0;
    sessionParam->sessionName = g_mySessionName;
    sessionParam->peerSessionName = g_peerSessionName;
    sessionParam->peerDeviceId = g_peerDeviceId;
    sessionParam->groupId = g_groupId;
    sessionParam->attr = &attr;
    return sessionParam;
}

void DeGenerateSessionParam(SessionParam *sessionParam)
{
    if (sessionParam != nullptr) {
        SoftBusFree(sessionParam);
    }
}

static SessionServer *GenerateSessionServer()
{
    SessionServer *sessionServer = (SessionServer*)SoftBusCalloc(sizeof(SessionServer));
    EXPECT_NE(nullptr, sessionServer);
    int32_t ret = strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), g_mySessionName);
    if (ret != EOK) {
        SoftBusFree(sessionServer);
        return nullptr;
    }
    ret = strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_myPkgName);
    if (ret != EOK) {
        SoftBusFree(sessionServer);
        return nullptr;
    }
    return sessionServer;
}

void DeGenerateSessionServer(SessionServer *sessionServer)
{
    if (sessionServer != nullptr) {
        SoftBusFree(sessionServer);
    }
}

static sptr<IRemoteObject> GenerateRemoteObject(void)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr != nullptr) {
        return samgr->GetSystemAbility(SOFTBUS_SERVER_SA_ID);
    }
    return nullptr;
}

/*
 * @tc.name: SoftbusServerStubTest001
 * @tc.desc: Verify the CheckOpenSessionPermission function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest001, TestSize.Level1)
{
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(SOFTBUS_OK, ret);
    SessionServer *sessionServer = GenerateSessionServer();
    EXPECT_NE(nullptr, sessionServer);
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(SOFTBUS_OK, ret);

    SessionParam *sessionParam001 = GenerateSessionParam();
    ASSERT_NE(nullptr, sessionParam001);
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckTransSecLevel).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckUidAndPid).WillRepeatedly(Return(true));
    ret = softBusServer->CheckOpenSessionPermission(sessionParam001);
    EXPECT_EQ(SOFTBUS_OK, ret);
    DeGenerateSessionParam(sessionParam001);

    SessionParam *sessionParam002 = nullptr;
    ret = softBusServer->CheckOpenSessionPermission(sessionParam002);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    DeGenerateSessionParam(sessionParam002);

    SessionParam *sessionParam003 = GenerateSessionParam();
    ASSERT_NE(nullptr, sessionParam003);
    sessionParam003->peerSessionName = nullptr;
    EXPECT_CALL(softbusServerStubMock, CheckTransSecLevel).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    ret = softBusServer->CheckOpenSessionPermission(sessionParam003);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    DeGenerateSessionParam(sessionParam003);

    DeGenerateSessionServer(sessionServer);
    TransSessionMgrDeinit();
}

/*
 * @tc.name: SoftbusServerStubTest002
 * @tc.desc: Verify the CheckChannelPermission function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest002, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    int32_t channelId = 0;
    int32_t channelType = 0;

    EXPECT_CALL(softbusServerStubMock, TransGetNameByChanId).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    int32_t ret = softBusServer->CheckChannelPermission(channelId, channelType);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    EXPECT_CALL(softbusServerStubMock, TransGetNameByChanId).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->CheckChannelPermission(channelId, channelType);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    ret = softBusServer->CheckChannelPermission(channelId, channelType);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest007
 * @tc.desc: Verify the SoftbusRegisterServiceInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest007, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;
    sptr<IRemoteObject> obj = GenerateRemoteObject();
    EXPECT_NE(nullptr, obj);

    int32_t ret = softBusServer->SoftbusRegisterServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_REMOTE_NULL, ret);

    datas.WriteRemoteObject(obj);
    ret = softBusServer->SoftbusRegisterServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteRemoteObject(obj);
    datas.WriteCString(test);
    ret = softBusServer->SoftbusRegisterServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest008
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest008, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, UnregChangeListener).WillRepeatedly(Return(SOFTBUS_OK));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    uint32_t code = SERVER_OPEN_SESSION;
    MessageParcel datas;
    MessageParcel reply;
    MessageOption option;

    int32_t ret = softBusServer->OnRemoteRequest(code, datas, reply, option);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInterfaceToken(std::u16string(u"test"));
    ret = softBusServer->OnRemoteRequest(code, datas, reply, option);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    InitSoftBusServer();
    datas.WriteInterfaceToken(std::u16string(u"test"));
    ret = softBusServer->OnRemoteRequest(code, datas, reply, option);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest009
 * @tc.desc: Verify the Create and Remove SessionServerInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest009, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    uint64_t timestamp = 1;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->CreateSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteUint64(timestamp);
    ret = softBusServer->CreateSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->CreateSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = softBusServer->RemoveSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test);
    ret = softBusServer->RemoveSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->RemoveSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_OK));
    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->RemoveSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_CHECK_PID_ERROR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest010
 * @tc.desc: Verify the ReadQosInfo function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest010, TestSize.Level1)
{
    unsigned int qosCount = 0;
    bool boolNum = false;
    QosTV qosInfo[7];
    MessageParcel datas;
    SessionParam param;

    bool ret = ReadQosInfo(datas, param);
    EXPECT_EQ(false, ret);

    datas.WriteBool(boolNum);
    ret = ReadQosInfo(datas, param);
    EXPECT_EQ(true, ret);

    boolNum = true;
    datas.WriteBool(boolNum);
    ret = ReadQosInfo(datas, param);
    EXPECT_EQ(false, ret);

    datas.WriteBool(boolNum);
    datas.WriteUint32(qosCount);
    ret = ReadQosInfo(datas, param);
    EXPECT_EQ(true, ret);

    qosCount = 10;
    datas.WriteBool(boolNum);
    datas.WriteUint32(qosCount);
    ret = ReadQosInfo(datas, param);
    EXPECT_EQ(false, ret);

    qosCount = 7;
    datas.WriteBool(boolNum);
    datas.WriteUint32(qosCount);
    ret = ReadQosInfo(datas, param);
    EXPECT_EQ(false, ret);

    datas.WriteBool(boolNum);
    datas.WriteUint32(qosCount);
    datas.WriteBuffer(qosInfo, sizeof(QosTV) * 7);
    ret = ReadQosInfo(datas, param);
    EXPECT_EQ(true, ret);
}

/*
 * @tc.name: SoftbusServerStubTest011
 * @tc.desc: Verify the OpenSessionInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest011, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    bool boolNum = false;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->OpenSessionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteBool(boolNum);
    datas.WriteInt32(0);
    datas.WriteInt32(0);
    datas.WriteInt32(0);
    datas.WriteInt32(0);
    datas.WriteUint16(0);
    datas.WriteBool(boolNum);
    ret = softBusServer->OpenSessionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest012
 * @tc.desc: Verify the OpenAuthSessionInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest012, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    ConnectionAddr addrInfo;
    memset_s(&addrInfo, sizeof(ConnectionAddr), 0, sizeof(ConnectionAddr));
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->OpenAuthSessionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test);
    datas.WriteRawData(&addrInfo, sizeof(ConnectionAddr));
    ret = softBusServer->OpenAuthSessionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest013
 * @tc.desc: Verify the NotifyAuthSuccessInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest013, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    int32_t channelId = 0;
    int32_t channelType = 0;
    MessageParcel datas;
    MessageParcel reply;

    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid).WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = softBusServer->NotifyAuthSuccessInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->NotifyAuthSuccessInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->NotifyAuthSuccessInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest014
 * @tc.desc: Verify the ReleaseResourcesInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest014, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    int32_t channelId = 0;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->ReleaseResourcesInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInt32(channelId);
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid).WillRepeatedly(Return(SOFTBUS_NO_INIT));
    ret = softBusServer->ReleaseResourcesInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    datas.WriteInt32(channelId);
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, TransReleaseUdpResources).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->ReleaseResourcesInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest015
 * @tc.desc: Verify the CloseChannelInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest015, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    int32_t channelId = 0;
    int32_t channelType = CHANNEL_TYPE_UNDEFINED;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);
    datas.WriteInt32(channelId);
    ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteCString(test);
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePidBySession).WillRepeatedly(Return(SOFTBUS_NO_INIT));
    ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteCString(test);
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePidBySession).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    channelType = 0;
    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteCString(test);
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid).WillRepeatedly(Return(SOFTBUS_NO_INIT));
    ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteCString(test);
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->CloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest016
 * @tc.desc: Verify the CloseChannelWithStatisticsInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest016, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t channelId = 1;
    uint64_t laneId = 1;
    uint32_t len = 10;
    int32_t channelType = 0;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->CloseChannelWithStatisticsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->CloseChannelWithStatisticsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->CloseChannelWithStatisticsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READUINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteUint64(laneId);
    ret = softBusServer->CloseChannelWithStatisticsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READUINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteUint64(laneId);
    datas.WriteUint32(len);
    ret = softBusServer->CloseChannelWithStatisticsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteUint64(laneId);
    datas.WriteUint32(len);
    datas.WriteRawData(test, len);
    ret = softBusServer->CloseChannelWithStatisticsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest017
 * @tc.desc: Verify the SendMessageInner function part01
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest017, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t channelId = 0;
    int32_t channelType = 0;
    uint32_t len = 10;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->SendMessageInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->SendMessageInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->SendMessageInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READUINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteUint32(len);
    ret = softBusServer->SendMessageInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteUint32(len);
    datas.WriteRawData(test, len);
    ret = softBusServer->SendMessageInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest018
 * @tc.desc: Verify the SendMessageInner function part02
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest018, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    int32_t channelId = 0;
    int32_t channelType = 0;
    uint32_t len = 10;
    int32_t msgType = 0;
    MessageParcel datas;
    MessageParcel reply;

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteUint32(len);
    datas.WriteRawData(test, len);
    datas.WriteInt32(msgType);
    EXPECT_CALL(softbusServerStubMock, TransGetAppInfoByChanId).WillRepeatedly(Return(SOFTBUS_NOT_FIND));
    int32_t ret = softBusServer->SendMessageInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest019
 * @tc.desc: Verify the EvaluateQosInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest019, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    QosTV qosInfo[7];
    int32_t dataTypeNumber = 0;
    uint32_t qosCount = 10;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(dataTypeNumber);
    ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    dataTypeNumber = 5;
    datas.WriteCString(test);
    datas.WriteInt32(dataTypeNumber);
    ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(dataTypeNumber);
    datas.WriteInt32(qosCount);
    ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    qosCount = 7;
    datas.WriteCString(test);
    datas.WriteInt32(dataTypeNumber);
    datas.WriteInt32(qosCount);
    ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(dataTypeNumber);
    datas.WriteInt32(qosCount);
    datas.WriteBuffer(qosInfo, sizeof(QosTV) * qosCount);
    ret = softBusServer->EvaluateQosInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest020
 * @tc.desc: Verify the Join and Leave LNNInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest020, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    ConnectionAddr addr;
    char test[10] = "test";
    uint32_t addrTypeLen = sizeof(ConnectionAddr);
    MessageParcel datas;
    MessageParcel reply;

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->JoinLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->JoinLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->JoinLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteUint32(addrTypeLen);
    ret = softBusServer->JoinLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteUint32(addrTypeLen);
    datas.WriteRawData(&addr, addrTypeLen);
    ret = softBusServer->JoinLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    ret = softBusServer->LeaveLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->LeaveLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->LeaveLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    ret = softBusServer->LeaveLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest021
 * @tc.desc: Verify the GetAllOnlineNodeInfoInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest021, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    uint32_t infoTypeLen = 0;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->GetAllOnlineNodeInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    ret = softBusServer->GetAllOnlineNodeInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    datas.WriteUint32(infoTypeLen);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetAllOnlineNodeInfo).WillRepeatedly(
        Return(SOFTBUS_NETWORK_GET_ALL_NODE_INFO_ERR)
    );
    EXPECT_CALL(softbusServerStubMock, SoftBusCheckIsNormalApp).WillRepeatedly(
        Return(false)
    );
    ret = softBusServer->GetAllOnlineNodeInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    datas.WriteUint32(infoTypeLen);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetAllOnlineNodeInfo).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->GetAllOnlineNodeInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);
}

/*
 * @tc.name: SoftbusServerStubTest022
 * @tc.desc: Verify the GetLocalDeviceInfoInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest022, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    uint32_t infoTypeLen = 0;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->GetLocalDeviceInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->GetLocalDeviceInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteUint32(infoTypeLen);
    ret = softBusServer->GetLocalDeviceInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    infoTypeLen = sizeof(NodeBasicInfo);
    datas.WriteCString(test);
    datas.WriteUint32(infoTypeLen);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetLocalDeviceInfo).WillRepeatedly(
        Return(SOFTBUS_NETWORK_GET_LOCAL_NODE_INFO_ERR)
    );
    ret = softBusServer->GetLocalDeviceInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    datas.WriteUint32(infoTypeLen);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetLocalDeviceInfo).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->GetLocalDeviceInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest023
 * @tc.desc: Verify the GetNodeKeyInfoInner function part01
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest023, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetNodeKeyInfo).WillRepeatedly(Return(SOFTBUS_OK));
    char test[10] = "test";
    int32_t key = 13;
    uint32_t len = 20;
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(key);
    datas.WriteUint32(len);
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest024
 * @tc.desc: Verify the GetNodeKeyInfoInner function part02
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest024, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    int32_t key = 0;
    uint32_t len = 0;
    MessageParcel datas;
    MessageParcel reply;

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(key);
    int32_t ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(key);
    datas.WriteUint32(len);
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    len = 65;
    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(key);
    datas.WriteUint32(len);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetNodeKeyInfo).WillRepeatedly(Return(SOFTBUS_NETWORK_NODE_KEY_INFO_ERR));
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(key);
    datas.WriteUint32(len);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetNodeKeyInfo).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->GetNodeKeyInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest025
 * @tc.desc: Verify the SetNodeDataChangeFlagInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest025, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    uint16_t changeFlag = 0;
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->SetNodeDataChangeFlagInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->SetNodeDataChangeFlagInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->SetNodeDataChangeFlagInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->SetNodeDataChangeFlagInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteUint16(changeFlag);
    ret = softBusServer->SetNodeDataChangeFlagInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest026
 * @tc.desc: Verify the RegDataLevelChangeCbInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest026, TestSize.Level1)
{
    SetAccessTokenPermission("distributeddata");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->RegDataLevelChangeCbInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->RegDataLevelChangeCbInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->RegDataLevelChangeCbInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: SoftbusServerStubTest027
 * @tc.desc: Verify the UnregDataLevelChangeCbInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest027, TestSize.Level1)
{
    SetAccessTokenPermission("distributeddata");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->UnregDataLevelChangeCbInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->UnregDataLevelChangeCbInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->UnregDataLevelChangeCbInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: SoftbusServerStubTest028
 * @tc.desc: Verify the SetDataLevelInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest028, TestSize.Level1)
{
    SetAccessTokenPermission("distributeddata");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    DataLevel dataLevel;
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->SetDataLevelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->SetDataLevelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteRawData(&dataLevel, sizeof(dataLevel));
    ret = softBusServer->SetDataLevelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest029
 * @tc.desc: Verify the Start and Stop TimeSyncInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest029, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t accuracy = 0;
    int32_t period = 0;
    MessageParcel datas;
    MessageParcel datas2;
    MessageParcel reply;

    int32_t ret = softBusServer->StartTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->StartTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(accuracy);
    ret = softBusServer->StartTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(accuracy);
    datas.WriteInt32(period);
    ret = softBusServer->StartTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    ret = softBusServer->StopTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->StopTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = softBusServer->StopTimeSyncInner(datas2, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->StartTimeSyncInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest030
 * @tc.desc: Verify the QosReportInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest030, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    int32_t channelId = 0;
    int32_t channelType = 0;
    int32_t appType = 0;
    int32_t quality = 0;
    MessageParcel datas;
    MessageParcel reply;

    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, CheckUidAndPid).WillRepeatedly(Return(true));
    int32_t ret = softBusServer->QosReportInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->QosReportInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->QosReportInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteInt32(appType);
    ret = softBusServer->QosReportInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteInt32(appType);
    datas.WriteInt32(quality);
    ret = softBusServer->QosReportInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest031
 * @tc.desc: Verify the StreamStatsInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest031, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    int32_t channelId = 0;
    int32_t channelType = 0;
    StreamSendStats stats;
    MessageParcel datas;
    MessageParcel reply;

    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, CheckUidAndPid).WillRepeatedly(Return(true));
    int32_t ret = softBusServer->StreamStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->StreamStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->StreamStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READRAWDATA_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteRawData(&stats, sizeof(StreamSendStats));
    ret = softBusServer->StreamStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest032
 * @tc.desc: Verify the RippleStatsInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest032, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    int32_t channelId = 0;
    int32_t channelType = 0;
    TrafficStats stats;
    MessageParcel datas;
    MessageParcel reply;

    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, CheckUidAndPid).WillRepeatedly(Return(true));
    int32_t ret = softBusServer->RippleStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->RippleStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    ret = softBusServer->RippleStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READRAWDATA_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(channelType);
    datas.WriteRawData(&stats, sizeof(TrafficStats));
    ret = softBusServer->RippleStatsInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest033
 * @tc.desc: Verify the Grant and Remove Permission Inner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest033, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    int32_t uid = 0;
    int32_t pid = 0;
    MessageParcel datas;
    MessageParcel reply;

    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->GrantPermissionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_OK));
    datas.WriteInt32(uid);
    datas.WriteInt32(pid);
    ret = softBusServer->GrantPermissionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteInt32(uid);
    datas.WriteInt32(pid);
    datas.WriteCString(test);
    ret = softBusServer->GrantPermissionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    ret = softBusServer->RemovePermissionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->RemovePermissionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    ret = softBusServer->RemovePermissionInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest034
 * @tc.desc: Verify the Publish and Stop Publish LNNInner function part01
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest034, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t publishId = 0;
    int32_t mode = 0;
    int32_t medium = -1;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    mode = 85;
    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: SoftbusServerStubTest035
 * @tc.desc: Verify the Publish and Stop Publish LNNInner function part02
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest035, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t publishId = 0;
    int32_t mode = 85;
    int32_t medium = 0;
    int32_t freq = -1;
    MessageParcel datas;
    MessageParcel reply;

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    int32_t ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    freq = 0;
    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteCString(test);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest036
 * @tc.desc: Verify the Publish and Stop Publish LNNInner function part03
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest036, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t publishId = 0;
    int32_t mode = 85;
    int32_t medium = 0;
    int32_t freq = 0;
    uint32_t dataLen = 0;
    bool ranging = true;
    MessageParcel datas;
    MessageParcel reply;

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteCString(test);
    datas.WriteUint32(dataLen);
    int32_t ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteCString(test);
    datas.WriteUint32(dataLen);
    datas.WriteBool(ranging);
    ret = softBusServer->PublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = softBusServer->StopPublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->StopPublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(publishId);
    ret = softBusServer->StopPublishLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest037
 * @tc.desc: Verify the Refresh and Stop Refresh LNNInner function part01
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest037, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t subscribeId = 0;
    int32_t mode = 0;
    int32_t medium = -1;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    mode = 85;
    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    ret = softBusServer->StopRefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest038
 * @tc.desc: Verify the Refresh and Stop Refresh LNNInner function part02
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest038, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t subscribeId = 0;
    int32_t mode = 85;
    int32_t medium = 0;
    int32_t freq = -1;
    bool flag = true;
    MessageParcel datas;
    MessageParcel reply;

    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    int32_t ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    freq = 0;
    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteBool(flag);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    ret = softBusServer->StopRefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest039
 * @tc.desc: Verify the Refresh and Stop Refresh LNNInner function part03
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest039, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t subscribeId = 0;
    int32_t mode = 85;
    int32_t medium = 0;
    int32_t freq = 0;
    bool flag = true;
    uint32_t dataLen = 0;
    int32_t refreshId = 0;
    MessageParcel datas;
    MessageParcel reply;
    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteBool(flag);
    datas.WriteBool(flag);
    int32_t ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteBool(flag);
    datas.WriteBool(flag);
    datas.WriteCString(test);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
    datas.WriteCString(test);
    datas.WriteInt32(subscribeId);
    datas.WriteInt32(mode);
    datas.WriteInt32(medium);
    datas.WriteInt32(freq);
    datas.WriteBool(flag);
    datas.WriteBool(flag);
    datas.WriteCString(test);
    datas.WriteUint32(dataLen);
    ret = softBusServer->RefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    datas.WriteCString(test);
    datas.WriteInt32(refreshId);
    ret = softBusServer->StopRefreshLNNInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest040
 * @tc.desc: Verify the Active and DeActive MetaNode Inner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest040, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    MetaNodeConfigInfo info;
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->ActiveMetaNodeInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->ActiveMetaNodeInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteRawData(&info, sizeof(MetaNodeConfigInfo));
    EXPECT_CALL(softbusServerStubMock, LnnIpcActiveMetaNode).WillRepeatedly(
        Return(SOFTBUS_NETWORK_ACTIVE_META_NODE_ERR)
    );
    ret = softBusServer->ActiveMetaNodeInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteRawData(&info, sizeof(MetaNodeConfigInfo));
    EXPECT_CALL(softbusServerStubMock, LnnIpcActiveMetaNode).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->ActiveMetaNodeInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);

    ret = softBusServer->DeactiveMetaNodeInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    EXPECT_CALL(softbusServerStubMock, LnnIpcDeactiveMetaNode).WillRepeatedly(
        Return(SOFTBUS_NETWORK_DEACTIVE_META_NODE_ERR)
    );
    ret = softBusServer->DeactiveMetaNodeInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NETWORK_DEACTIVE_META_NODE_ERR, ret);

    datas.WriteCString(test);
    EXPECT_CALL(softbusServerStubMock, LnnIpcDeactiveMetaNode).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->DeactiveMetaNodeInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    ret = softBusServer->DeactiveMetaNodeInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest041
 * @tc.desc: Verify the GetAllMetaNodeInfoInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest041, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    int32_t infoNum = 4;
    MessageParcel datas;
    MessageParcel reply;

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->GetAllMetaNodeInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->GetAllMetaNodeInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInt32(infoNum);
    ret = softBusServer->GetAllMetaNodeInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    infoNum = 2;
    datas.WriteInt32(infoNum);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetAllMetaNodeInfo).WillRepeatedly(
        Return(SOFTBUS_NETWORK_GET_META_NODE_INFO_ERR)
    );
    ret = softBusServer->GetAllMetaNodeInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NETWORK_GET_META_NODE_INFO_ERR, ret);

    datas.WriteInt32(infoNum);
    EXPECT_CALL(softbusServerStubMock, LnnIpcGetAllMetaNodeInfo).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->GetAllMetaNodeInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest042
 * @tc.desc: Verify the ShiftLNNGearInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest042, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    GearMode gearMode;
    char test[10] = "test";
    bool flag = true;
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->ShiftLNNGearInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->ShiftLNNGearInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    ret = softBusServer->ShiftLNNGearInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->ShiftLNNGearInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteBool(flag);
    ret = softBusServer->ShiftLNNGearInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteBool(flag);
    datas.WriteRawData(&gearMode, sizeof(GearMode));
    ret = softBusServer->ShiftLNNGearInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);
}

/*
 * @tc.name: SoftbusServerStubTest043
 * @tc.desc: Verify the GetSoftbusSpecObjectInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest043, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->GetSoftbusSpecObjectInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest044
 * @tc.desc: Verify the GetBusCenterExObjInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest044, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->GetBusCenterExObjInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest045
 * @tc.desc: Verify the PrivilegeCloseChannelInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest045, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char networkId[NETWORK_ID_BUF_LEN] = { 0 };
    uint64_t tokenId = 0;
    int32_t pid = 0;
    MessageParcel datas;
    MessageParcel reply;

    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDmsServerPermission)
        .WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->PrivilegeCloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);

    datas.WriteUint64(tokenId);
    datas.WriteInt32(pid);
    datas.WriteCString(networkId);
    ret = softBusServer->PrivilegeCloseChannelInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest046
 * @tc.desc: Verify the RegRangeCbForMsdpInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest046, TestSize.Level1)
{
    SetAccessTokenPermission("msdp_sa");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test1[10] = "test";
    char test2[30] = "ohos.msdp.spatialawareness";
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->RegRangeCbForMsdpInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->RegRangeCbForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test1);
    ret = softBusServer->RegRangeCbForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test2);
    ret = softBusServer->RegRangeCbForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);
}

/*
 * @tc.name: SoftbusServerStubTest047
 * @tc.desc: Verify the UnregRangeCbForMsdpInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest047, TestSize.Level1)
{
    SetAccessTokenPermission("msdp_sa");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test1[10] = "test";
    char test2[30] = "ohos.msdp.spatialawareness";
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->UnregRangeCbForMsdpInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->UnregRangeCbForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test1);
    ret = softBusServer->UnregRangeCbForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test2);
    ret = softBusServer->UnregRangeCbForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);
}

/*
 * @tc.name: SoftbusServerStubTest048
 * @tc.desc: Verify the TriggerRangeForMsdpInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest048, TestSize.Level1)
{
    SetAccessTokenPermission("msdp_sa");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    RangeConfig config;
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->TriggerRangeForMsdpInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->TriggerRangeForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);

    datas.WriteCString(test);
    datas.WriteRawData(&config, sizeof(RangeConfig));
    ret = softBusServer->TriggerRangeForMsdpInner(datas, reply);
    EXPECT_NE(SOFTBUS_NO_INIT, ret);
}

/*
 * @tc.name: SoftbusServerStubTest049
 * @tc.desc: Verify the CheckPkgName function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest049, TestSize.Level1)
{
    int32_t ret = CheckPkgName("mds");
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = CheckPkgName("ohos.distributedschedule.dms");
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest050
 * @tc.desc: Verify the CreateServerInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest050, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);

    MessageParcel data;
    MessageParcel reply;

    int32_t ret = softBusServer->CreateServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    const char *pkgName = "1234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data.WriteCString(pkgName);
    ret = softBusServer->CreateServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    pkgName = "ohos.distributedschedule.dms";
    data.WriteCString(pkgName);
    ret = softBusServer->CreateServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    pkgName = "ohos.distributedschedule.dms";
    data.WriteCString(pkgName);
    const char *name =
        "12345678901234567890123456789012345678901234567890123456789012345678901234567891234567890123456789012"
        "34567890123456789012345678901234567890123456789012345678912345678901234567890123456789012345678901234"
        "56789012345678901234567890123456789123456789012345678901234567890123456789012345678901234567890123456"
        "78901234567891234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data.WriteCString(name);
    ret = softBusServer->CreateServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    name = "test";
    data.WriteCString(pkgName);
    data.WriteCString(name);
    ret = softBusServer->CreateServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest051
 * @tc.desc: Verify the RemoveServerInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest051, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);

    MessageParcel data;
    MessageParcel reply;

    int32_t ret = softBusServer->RemoveServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    const char *pkgName = "1234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data.WriteCString(pkgName);
    ret = softBusServer->RemoveServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    pkgName = "ohos.distributedschedule.dms";
    data.WriteCString(pkgName);
    ret = softBusServer->RemoveServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    pkgName = "ohos.distributedschedule.dms";
    data.WriteCString(pkgName);
    const char *name =
        "12345678901234567890123456789012345678901234567890123456789012345678901234567891234567890123456789012"
        "34567890123456789012345678901234567890123456789012345678912345678901234567890123456789012345678901234"
        "56789012345678901234567890123456789123456789012345678901234567890123456789012345678901234567890123456"
        "78901234567891234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data.WriteCString(name);
    ret = softBusServer->RemoveServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    name = "test";
    data.WriteCString(pkgName);
    data.WriteCString(name);
    ret = softBusServer->RemoveServerInner(data, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest052
 * @tc.desc: Verify the ConnectInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest052, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);

    MessageParcel data;
    MessageParcel reply;

    int32_t ret = softBusServer->ConnectInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    const char *pkgName = "1234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data.WriteCString(pkgName);
    ret = softBusServer->ConnectInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    pkgName = "ohos.distributedschedule.dms";
    data.WriteCString(pkgName);
    ret = softBusServer->ConnectInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    pkgName = "ohos.distributedschedule.dms";
    data.WriteCString(pkgName);
    const char *name =
        "12345678901234567890123456789012345678901234567890123456789012345678901234567891234567890123456789012"
        "34567890123456789012345678901234567890123456789012345678912345678901234567890123456789012345678901234"
        "56789012345678901234567890123456789123456789012345678901234567890123456789012345678901234567890123456"
        "78901234567891234567890123456789012345678901234567890123456789012345678901234567890123456789";
    data.WriteCString(name);
    ret = softBusServer->ConnectInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    name = "test";
    data.WriteCString(pkgName);
    data.WriteCString(name);
    ret = softBusServer->ConnectInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest053
 * @tc.desc: Verify the DisconnectInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest053, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);

    MessageParcel data;
    MessageParcel reply;

    int32_t ret = softBusServer->DisconnectInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
}

/*
 * @tc.name: SoftbusServerStubTest054
 * @tc.desc: Verify the SendInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest054, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);

    MessageParcel data;
    MessageParcel reply;

    int32_t ret = softBusServer->SendInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    int32_t handle = 0;
    data.WriteInt32(handle);
    ret = softBusServer->SendInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    uint32_t len = TEST_MAX_LEN;
    data.WriteInt32(handle);
    data.WriteUint32(len);
    ret = softBusServer->SendInner(data, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: SoftbusServerStubTest055
 * @tc.desc: Verify the GetPeerDeviceIdInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest055, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);

    MessageParcel data;
    MessageParcel reply;

    int32_t ret = softBusServer->GetPeerDeviceIdInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    int32_t handle = 0;
    data.WriteInt32(handle);
    ret = softBusServer->GetPeerDeviceIdInner(data, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    uint32_t len = TEST_DEVICE_ID_SIZE_MAX;
    data.WriteInt32(handle);
    data.WriteUint32(len);
    ret = softBusServer->GetPeerDeviceIdInner(data, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: SoftbusServerStubTest056
 * @tc.desc: Verify the Create and Remove SessionServerInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest056, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;
    datas.WriteCString(test);
    datas.WriteCString(test);
    int32_t ret = softBusServer->CreateSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);

    ret = softBusServer->CreateSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);

    datas.WriteCString(test);
    ret = softBusServer->RemoveSessionServerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest057
 * @tc.desc: GetBundleName error test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest057, TestSize.Level1)
{
    std::string bundleName = "test";
    int32_t ret = GetBundleName(0, bundleName);
    EXPECT_EQ(ret, SOFTBUS_TRANS_GET_BUNDLENAME_FAILED);

    ret = GetAppId(bundleName, bundleName, 0);
    EXPECT_NE(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SoftbusServerStubTest058
 * @tc.desc: CheckNormalAppSessionName error test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest058, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, SoftBusCheckIsNormalApp)
        .WillRepeatedly(Return(true));

    std::string bundleName = "test";
    MessageParcel data;
    ReadSessionAttrs(data, nullptr);
    int32_t ret = CheckNormalAppSessionName(nullptr, 0, bundleName, 0);
    EXPECT_EQ(ret, SOFTBUS_TRANS_GET_BUNDLENAME_FAILED);
}

/*
 * @tc.name: SoftbusServerStubTest059
 * @tc.desc: StopRangeForMsdpInner api test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest059, TestSize.Level1)
{
    SetAccessTokenPermission("distributeddata");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    MessageParcel data;
    MessageParcel reply;
    char test[10] = "test";
    ConnectionAddr addrInfo;
    (void)memset_s(&addrInfo, sizeof(ConnectionAddr), 0, sizeof(ConnectionAddr));
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->StopRangeForMsdpInner(data, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->StopRangeForMsdpInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_TRANS_PROXY_WRITECSTRING_FAILED);

    data.WriteCString(test);
    ret = softBusServer->StopRangeForMsdpInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_TRANS_PROXY_READCSTRING_FAILED);

    data.WriteCString(test);
    data.WriteRawData(&addrInfo, sizeof(ConnectionAddr));
    ret = softBusServer->StopRangeForMsdpInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_TRANS_PROXY_READCSTRING_FAILED);
}

/*
 * @tc.name: SoftbusServerStubTest060
 * @tc.desc: CheckOpenSessionPermission error test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest060, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, UnregChangeListener).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckTransPermission)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckUidAndPid).WillRepeatedly(Return(false));

    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(SOFTBUS_OK, ret);
    SessionServer *sessionServer = GenerateSessionServer();
    EXPECT_NE(nullptr, sessionServer);
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(SOFTBUS_OK, ret);
    SessionParam *sessionParam001 = GenerateSessionParam();
    ASSERT_NE(sessionParam001, nullptr);

    ret = softBusServer->CheckOpenSessionPermission(sessionParam001);
    EXPECT_EQ(ret, SOFTBUS_PERMISSION_DENIED);


    SoftbusReportPermissionFaultEvt(SERVER_OPEN_SESSION);
    ret = softBusServer->CheckOpenSessionPermission(sessionParam001);
    EXPECT_EQ(ret, SOFTBUS_TRANS_CHECK_PID_ERROR);
}

/*
 * @tc.name: SoftbusServerStubTest061
 * @tc.desc: Verify the SyncTrustedRelationShipInner and SetDisplayNameInner function
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest061, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_CALL(softbusServerStubMock, UnregChangeListener).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->SyncTrustedRelationShipInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->SyncTrustedRelationShipInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);
    ret = softBusServer->SetDisplayNameInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    ret = softBusServer->SetDisplayNameInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest062
 * @tc.desc: Verify the SetNodeKeyInfoInner function part01
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest062, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, LnnIpcSetNodeKeyInfo).WillRepeatedly(Return(SOFTBUS_OK));
    char test[10] = "test";
    int32_t key = 13;
    uint32_t len = 20;
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = softBusServer->SetNodeKeyInfoInner(datas, reply);
    ret = softBusServer->SetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    ret = softBusServer->SetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->SetNodeKeyInfoInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteInt32(key);
    datas.WriteUint32(len);
    datas.WriteRawData(test, 10);
    ret = softBusServer->SetNodeKeyInfoInner(datas, reply);
    EXPECT_NE(SOFTBUS_IPC_ERR, ret);
}

/**
 * @tc.name: SoftbusServerStubTest063
 * @tc.desc: StartAccountAuthInner api test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest063, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    MessageParcel data;
    MessageParcel reply;
    int64_t requestId = 0;
    char test[10] = "test";

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->StartAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_PERMISSION_DENIED);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, LnnIpcStartAccountAuth).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->StartAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    ret = softBusServer->StartAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    data.WriteInt64(requestId);
    ret = softBusServer->StartAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    data.WriteInt64(requestId);
    data.WriteCString(test);
    ret = softBusServer->StartAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/**
 * @tc.name: SoftbusServerStubTest064
 * @tc.desc: ProcessAccountAuthInner api test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest064, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    MessageParcel data;
    MessageParcel reply;
    int64_t requestId = 0;
    char test[10] = "test";
    int32_t dataLen = strlen(test) + 1;

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->ProcessAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_PERMISSION_DENIED);
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, LnnIpcProcessAccountAuth).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->ProcessAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    ret = softBusServer->ProcessAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    data.WriteInt64(requestId);
    ret = softBusServer->ProcessAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    data.WriteInt64(requestId);
    data.WriteUint32(dataLen);
    ret = softBusServer->ProcessAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_IPC_ERR);

    data.WriteCString(g_myPkgName);
    data.WriteInt64(requestId);
    data.WriteUint32(dataLen);
    data.WriteRawData(test, dataLen);
    ret = softBusServer->ProcessAccountAuthInner(data, reply);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: InitMemberConstraintSetTest001
 * @tc.desc: Verify InitMemberConstraintSet initializes constraint set with expected codes
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, InitMemberConstraintSetTest001, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    EXPECT_GT(softBusServer->memberConstraintSet_.size(), 0u);

    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_JOIN_LNN) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_START_TIME_SYNC) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_SET_NODE_DATA_CHANGE_FLAG) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_GET_ALL_ONLINE_NODE_INFO) > 0);
}

/*
 * @tc.name: InitMemberConstraintSetTest002
 * @tc.desc: Verify constraint set contains additional expected codes
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, InitMemberConstraintSetTest002, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_SET_DATA_LEVEL) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_SET_NODE_DATA_CHANGE_FLAG) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_SHIFT_LNN_GEAR) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_ACTIVE_META_NODE) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_DEACTIVE_META_NODE) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_GET_ALL_META_NODE_INFO) > 0);
}

/*
 * @tc.name: InitMemberConstraintSetTest003
 * @tc.desc: Verify constraint set contains data level codes
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, InitMemberConstraintSetTest003, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_SET_DATA_LEVEL) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_TRIGGER_RANGE_FOR_MSDP) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_SYNC_TRUSTED_RELATION) > 0);
    EXPECT_TRUE(softBusServer->memberConstraintSet_.count(SERVER_STOP_TIME_SYNC) > 0);
}

/*
 * @tc.name: CheckAccountConstraintTest001
 * @tc.desc: Verify CheckAccountConstraint returns SOFTBUS_OK when constraint is not enabled
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckAccountConstraintTest001, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    int32_t ret = softBusServer->CheckAccountConstraint(SERVER_JOIN_LNN);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = softBusServer->CheckAccountConstraint(SERVER_PUBLISH_LNN);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = softBusServer->CheckAccountConstraint(SERVER_REFRESH_LNN);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CheckAccountConstraintTest002
 * @tc.desc: Verify CheckAccountConstraint returns SOFTBUS_OK for codes not in constraint set
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckAccountConstraintTest002, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    int32_t ret = softBusServer->CheckAccountConstraint(99999);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = softBusServer->CheckAccountConstraint(0);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CheckAccountConstraintTest003
 * @tc.desc: Verify CheckAccountConstraint returns SOFTBUS_OK for all constraint codes when not enabled
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckAccountConstraintTest003, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    for (uint32_t code : softBusServer->memberConstraintSet_) {
        int32_t ret = softBusServer->CheckAccountConstraint(code);
        EXPECT_EQ(ret, SOFTBUS_OK);
    }
}

/*
 * @tc.name: CheckPermissionTest001
 * @tc.desc: Verify CheckPermission returns SOFTBUS_OK for code not in permission map
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckPermissionTest001, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);
    uint32_t unknownCode = 99999;
    int32_t ret = softBusServer->CheckPermission(unknownCode);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CheckPermissionTest002
 * @tc.desc: Verify CheckPermission returns SOFTBUS_OK when permission is nullptr
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckPermissionTest002, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);
    int32_t ret = softBusServer->CheckPermission(SERVER_SESSION_SENDMSG);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CheckPermissionTest003
 * @tc.desc: Verify CheckPermission returns SOFTBUS_OK when permission check passes
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckPermissionTest003, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);
    int32_t ret = softBusServer->CheckPermission(SERVER_JOIN_LNN);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = softBusServer->CheckPermission(SERVER_CREATE_SESSION_SERVER);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CheckAccountConstraintTest004
 * @tc.desc: Verify CheckAccountConstraint returns SOFTBUS_ACCOUNT_CONSTRAINT_ENABLE when constraint enabled
 *           and code is in constraint set
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckAccountConstraintTest004, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> mock;
    EXPECT_CALL(mock, LnnIsOsAccountConstraint).WillRepeatedly(Return(true));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);
    int32_t ret = softBusServer->CheckAccountConstraint(SERVER_JOIN_LNN);
    EXPECT_EQ(ret, SOFTBUS_ACCOUNT_CONSTRAINT_ENABLE);
    ret = softBusServer->CheckAccountConstraint(SERVER_SET_NODE_DATA_CHANGE_FLAG);
    EXPECT_EQ(ret, SOFTBUS_ACCOUNT_CONSTRAINT_ENABLE);
    ret = softBusServer->CheckAccountConstraint(SERVER_SHIFT_LNN_GEAR);
    EXPECT_EQ(ret, SOFTBUS_ACCOUNT_CONSTRAINT_ENABLE);
}

/*
 * @tc.name: CheckAccountConstraintTest005
 * @tc.desc: Verify CheckAccountConstraint returns SOFTBUS_OK when constraint enabled
 *           but code is not in constraint set
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckAccountConstraintTest005, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> mock;
    EXPECT_CALL(mock, LnnIsOsAccountConstraint).WillRepeatedly(Return(true));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);
    int32_t ret = softBusServer->CheckAccountConstraint(SERVER_CREATE_SESSION_SERVER);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = softBusServer->CheckAccountConstraint(99999);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CheckAccountConstraintTest006
 * @tc.desc: Verify CheckAccountConstraint returns SOFTBUS_OK when constraint disabled
 *           with mock returning false
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST_F(SoftbusServerStubTest, CheckAccountConstraintTest006, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> mock;
    EXPECT_CALL(mock, LnnIsOsAccountConstraint).WillRepeatedly(Return(false));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);
    for (uint32_t code : softBusServer->memberConstraintSet_) {
        int32_t ret = softBusServer->CheckAccountConstraint(code);
        EXPECT_EQ(ret, SOFTBUS_OK);
    }
}

/*
 * @tc.name: SoftbusServerStubTestSetWirelessLinkCtrl001
 * @tc.desc: Verify SetWirelessLinkCtrlInner returns SOFTBUS_TRANS_PROXY_READINT_FAILED
 *           when data.ReadInt32(socket) fails (empty parcel).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTestSetWirelessLinkCtrl001, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = softBusServer->SetWirelessLinkCtrlInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);
}

/*
 * @tc.name: SoftbusServerStubTestSetWirelessLinkCtrl002
 * @tc.desc: Verify SetWirelessLinkCtrlInner writes error code to reply and returns it
 *           when TransProcessWirelessLinkCtrl fails (socket=0 triggers SOFTBUS_INVALID_PARAM).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTestSetWirelessLinkCtrl002, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_TRUE(datas.WriteInt32(0));
    int32_t ret = softBusServer->SetWirelessLinkCtrlInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, replyVal);
}

/*
 * @tc.name: SoftbusServerStubTestSetWirelessLinkCtrl003
 * @tc.desc: Verify SetWirelessLinkCtrlInner writes SOFTBUS_OK to reply and returns SOFTBUS_OK
 *           when TransProcessWirelessLinkCtrl succeeds (socket=1).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTestSetWirelessLinkCtrl003, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;
    EXPECT_TRUE(datas.WriteInt32(1));
    int32_t ret = softBusServer->SetWirelessLinkCtrlInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = -1;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
    EXPECT_EQ(SOFTBUS_OK, replyVal);
}

static void WriteOpenSessionParcel(MessageParcel &data, const char *sessionName,
    const char *peerSessionName, const char *peerDeviceId, const char *groupId)
{
    data.WriteCString(sessionName);
    data.WriteCString(peerSessionName);
    data.WriteCString(peerDeviceId);
    data.WriteCString(groupId);
    data.WriteBool(false); // isAsync
    data.WriteInt32(0);    // sessionId
    data.WriteBool(false); // enableMultipath
    data.WriteUint32(0);   // actionId
    data.WriteBool(false); // isLowLatency
    data.WriteUint64(0);   // flowInfo.flowSize
    data.WriteUint32(0);   // flowInfo.sessionType
    data.WriteUint32(0);   // flowInfo.flowQosType
    data.WriteUint32(0);   // cancelEncryptionBit
    data.WriteInt32(0);    // keyType
    data.WriteInt32(0);    // edcaParam.scene
    data.WriteInt32(0);    // edcaParam.level
    data.WriteInt32(0);    // attr.dataType
    data.WriteInt32(0);    // attr.linkTypeNum
    data.WriteInt32(0);    // attr.streamAttr.streamType
    data.WriteBool(false); // isQosLane
}

/*
 * @tc.name: SoftbusServerStubTest065
 * @tc.desc: Verify the OnRemoteRequest permission deny, account constraint and dispatch branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest065, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, UnregChangeListener).WillRepeatedly(Return(SOFTBUS_OK));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageOption option;

    if (!GetServerIsInit()) {
        MessageParcel datas;
        MessageParcel reply;
        EXPECT_TRUE(datas.WriteInterfaceToken(ISoftBusServer::GetDescriptor()));
        int32_t ret = softBusServer->OnRemoteRequest(SERVER_GET_BUS_CENTER_EX_OBJ, datas, reply, option);
        EXPECT_EQ(SOFTBUS_IPC_ERR, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_SERVER_NOT_INIT, replyVal);
        return;
    }

    MessageParcel datas;
    MessageParcel reply;
    EXPECT_TRUE(datas.WriteInterfaceToken(ISoftBusServer::GetDescriptor()));
    int32_t ret = softBusServer->OnRemoteRequest(SERVER_GET_DM_EX_OBJ, datas, reply, option);
    EXPECT_EQ(SOFTBUS_ACCESS_TOKEN_DENIED, ret);

    EXPECT_TRUE(datas.WriteInterfaceToken(ISoftBusServer::GetDescriptor()));
    ret = softBusServer->OnRemoteRequest(SERVER_GET_BUS_CENTER_EX_OBJ, datas, reply, option);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, LnnIsOsAccountConstraint).WillRepeatedly(Return(true));
    EXPECT_TRUE(datas.WriteInterfaceToken(ISoftBusServer::GetDescriptor()));
    ret = softBusServer->OnRemoteRequest(SERVER_JOIN_LNN, datas, reply, option);
    EXPECT_EQ(SOFTBUS_ACCOUNT_CONSTRAINT_ENABLE, ret);

    EXPECT_TRUE(datas.WriteInterfaceToken(ISoftBusServer::GetDescriptor()));
    EXPECT_NO_FATAL_FAILURE(softBusServer->OnRemoteRequest(99999, datas, reply, option));
}

/*
 * @tc.name: SoftbusServerStubTest066
 * @tc.desc: Verify the OpenSessionInner null sessionName and permission denied branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest066, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";

    // sessionName is null, qos info read success, retReply is written into reply raw data
    {
        MessageParcel datas;
        MessageParcel reply;
        datas.WriteBool(false); // isAsync
        datas.WriteBool(false); // isQosLane
        int32_t ret = softBusServer->OpenSessionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        const void *raw = reply.ReadRawData(sizeof(TransSerializer));
        ASSERT_NE(nullptr, raw);
        EXPECT_EQ(SOFTBUS_INVALID_PARAM, reinterpret_cast<const TransSerializer *>(raw)->ret);
    }

    // full parcel, CheckTransPermission denied, retReply is SOFTBUS_PERMISSION_DENIED
    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    {
        MessageParcel datas;
        MessageParcel reply;
        WriteOpenSessionParcel(datas, test, test, test, test);
        int32_t ret = softBusServer->OpenSessionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        const void *raw = reply.ReadRawData(sizeof(TransSerializer));
        ASSERT_NE(nullptr, raw);
        EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, reinterpret_cast<const TransSerializer *>(raw)->ret);
    }
}

/*
 * @tc.name: SoftbusServerStubTest067
 * @tc.desc: Verify the OpenSessionInner serviceId name branch and full success path
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest067, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    char serviceIdName[TEST_SESSION_NAME_SIZE_MAX] = "serviceId_100";

    // sessionName contains serviceId, LnnGetNetworkIdByUuid fails for unknown peerDeviceId
    {
        MessageParcel datas;
        MessageParcel reply;
        WriteOpenSessionParcel(datas, serviceIdName, test, test, test);
        int32_t ret = softBusServer->OpenSessionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        const void *raw = reply.ReadRawData(sizeof(TransSerializer));
        ASSERT_NE(nullptr, raw);
        EXPECT_EQ(SOFTBUS_INVALID_PARAM, reinterpret_cast<const TransSerializer *>(raw)->ret);
    }

    // full success path: register session server, mock permission checks pass
    int32_t ret = TransSessionMgrInit();
    EXPECT_EQ(SOFTBUS_OK, ret);
    SessionServer *sessionServer = (SessionServer *)SoftBusCalloc(sizeof(SessionServer));
    ASSERT_NE(nullptr, sessionServer);
    ASSERT_EQ(EOK, strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), "com.test.trans.session.new"));
    ASSERT_EQ(EOK, strcpy_s(sessionServer->pkgName, sizeof(sessionServer->pkgName), g_myPkgName));
    ret = TransSessionServerAddItem(sessionServer);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(softbusServerStubMock, CheckTransPermission).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(softbusServerStubMock, CheckUidAndPid).WillRepeatedly(Return(true));
    EXPECT_CALL(softbusServerStubMock, CheckTransSecLevel).WillRepeatedly(Return(SOFTBUS_OK));
    {
        MessageParcel datas;
        MessageParcel reply;
        WriteOpenSessionParcel(datas, sessionServer->sessionName, test, test, test);
        EXPECT_NO_FATAL_FAILURE(ret = softBusServer->OpenSessionInner(datas, reply));
        EXPECT_EQ(SOFTBUS_OK, ret);
        EXPECT_NE(nullptr, reply.ReadRawData(sizeof(TransSerializer)));
    }

    SoftBusFree(sessionServer);
    TransSessionMgrDeinit();
}

/*
 * @tc.name: SoftbusServerStubTest068
 * @tc.desc: Verify the GrantPermissionInner reply content and null sessionName branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest068, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t uid = 0;
    int32_t pid = 0;

    // dynamic permission denied, reply records the denied result
    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    {
        MessageParcel datas;
        MessageParcel reply;
        int32_t ret = softBusServer->GrantPermissionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, replyVal);
    }

    // dynamic permission ok, sessionName is null, ret keeps SOFTBUS_OK
    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_OK));
    {
        MessageParcel datas;
        MessageParcel reply;
        datas.WriteInt32(uid);
        datas.WriteInt32(pid);
        int32_t ret = softBusServer->GrantPermissionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_OK, replyVal);
    }

    // full parcel, GrantTransPermission result is written into reply
    {
        MessageParcel datas;
        MessageParcel reply;
        datas.WriteInt32(uid);
        datas.WriteInt32(pid);
        datas.WriteCString(test);
        int32_t ret = softBusServer->GrantPermissionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
    }
}

/*
 * @tc.name: SoftbusServerStubTest069
 * @tc.desc: Verify the RemovePermissionInner reply content and null sessionName branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest069, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";

    // dynamic permission denied, reply records the denied result
    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    {
        MessageParcel datas;
        MessageParcel reply;
        int32_t ret = softBusServer->RemovePermissionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, replyVal);
    }

    // dynamic permission ok, sessionName is null, ret keeps SOFTBUS_OK
    EXPECT_CALL(softbusServerStubMock, SoftBusCheckDynamicPermission).WillRepeatedly(Return(SOFTBUS_OK));
    {
        MessageParcel datas;
        MessageParcel reply;
        int32_t ret = softBusServer->RemovePermissionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_OK, replyVal);
    }

    // full parcel, RemoveTransPermission result is written into reply
    {
        MessageParcel datas;
        MessageParcel reply;
        datas.WriteCString(test);
        int32_t ret = softBusServer->RemovePermissionInner(datas, reply);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(reply.ReadInt32(replyVal));
    }
}

/*
 * @tc.name: SoftbusServerStubTest070
 * @tc.desc: Verify the SetSocketOptEdcaInner read fail, pid check fail and edca param invalid branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest070, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->SetSocketOptEdcaInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(0);
    ret = softBusServer->SetSocketOptEdcaInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid)
        .WillOnce(Return(SOFTBUS_TRANS_CHECK_PID_ERROR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    datas.WriteInt32(0);
    datas.WriteInt32(0);
    ret = softBusServer->SetSocketOptEdcaInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_CHECK_PID_ERROR, ret);

    datas.WriteInt32(0);
    datas.WriteInt32(0);
    ret = softBusServer->SetSocketOptEdcaInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(0);
    datas.WriteInt32(0);
    datas.WriteInt32(EDCA_SCENE_DATA_CLONE);
    ret = softBusServer->SetSocketOptEdcaInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    // invalid scene, TransSetEdcaParamForChannel returns SOFTBUS_INVALID_PARAM
    {
        MessageParcel data;
        MessageParcel replyNew;
        data.WriteInt32(0);
        data.WriteInt32(0);
        data.WriteInt32(EDCA_SCENE_BUTT);
        data.WriteInt32(EDCA_LEVEL_DISABLE);
        ret = softBusServer->SetSocketOptEdcaInner(data, replyNew);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(replyNew.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_INVALID_PARAM, replyVal);
    }

    // invalid level, TransSetEdcaParamForChannel returns SOFTBUS_INVALID_PARAM
    {
        MessageParcel data;
        MessageParcel replyNew;
        data.WriteInt32(0);
        data.WriteInt32(0);
        data.WriteInt32(EDCA_SCENE_DATA_CLONE);
        data.WriteInt32(EDCA_LEVEL_BUTT);
        ret = softBusServer->SetSocketOptEdcaInner(data, replyNew);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(replyNew.ReadInt32(replyVal));
        EXPECT_EQ(SOFTBUS_INVALID_PARAM, replyVal);
    }

    // valid scene and level, channel not exist, real edca lookup result is written into reply
    {
        MessageParcel data;
        MessageParcel replyNew;
        data.WriteInt32(0);
        data.WriteInt32(0);
        data.WriteInt32(EDCA_SCENE_DATA_CLONE);
        data.WriteInt32(EDCA_LEVEL_SEMI);
        ret = softBusServer->SetSocketOptEdcaInner(data, replyNew);
        EXPECT_EQ(SOFTBUS_OK, ret);
        int32_t replyVal = 0;
        EXPECT_TRUE(replyNew.ReadInt32(replyVal));
    }
}

/*
 * @tc.name: SoftbusServerStubTest071
 * @tc.desc: Verify the ProcessPushMsgInner len invalid, raw data read fail and process branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest071, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->ProcessPushMsgInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteUint32(MAX_PUSH_MSG_SIZE + 1);
    ret = softBusServer->ProcessPushMsgInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteUint32(4);
    ret = softBusServer->ProcessPushMsgInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    ret = softBusServer->ProcessPushMsgInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest072
 * @tc.desc: Verify the ProcessInnerEventInner read fail and invalid event type branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest072, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->ProcessInnerEventInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(0);
    ret = softBusServer->ProcessInnerEventInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READUINT_FAILED, ret);

    datas.WriteInt32(0);
    datas.WriteUint32(4);
    ret = softBusServer->ProcessInnerEventInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInt32(9999); // invalid event type
    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    ret = softBusServer->ProcessInnerEventInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_MSG_INVALID_EVENT_TYPE, ret);
}

/*
 * @tc.name: SoftbusServerStubTest073
 * @tc.desc: Verify the SetDisconnectPolicyInner read fail and invalid param branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest073, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->SetDisconnectPolicyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(0);
    ret = softBusServer->SetDisconnectPolicyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(-1); // invalid channelId
    datas.WriteInt32(DISCONNECT_POLICY_DEFAULT);
    ret = softBusServer->SetDisconnectPolicyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, replyVal);

    datas.WriteInt32(0);
    datas.WriteInt32(-1); // invalid policy
    ret = softBusServer->SetDisconnectPolicyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // valid params, reply value equals the returned value of TransProcessDisconnectPolicy
    datas.WriteInt32(0);
    datas.WriteInt32(DISCONNECT_POLICY_DEFAULT);
    ret = softBusServer->SetDisconnectPolicyInner(datas, reply);
    replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
    EXPECT_EQ(ret, replyVal);
}

/*
 * @tc.name: SoftbusServerStubTest074
 * @tc.desc: Verify the SetDisplayNameInner pkgName check, nameData and len check branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest074, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    const char *dmPkgName = "ohos.distributedhardware.devicemanager";
    MessageParcel datas;
    MessageParcel reply;

    // pkgName is not the devicemanager package
    datas.WriteCString(test);
    int32_t ret = softBusServer->SetDisplayNameInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // pkgName is valid but nameData read fail
    datas.WriteCString(dmPkgName);
    ret = softBusServer->SetDisplayNameInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    // len does not match nameData
    datas.WriteCString(dmPkgName);
    datas.WriteCString(test);
    datas.WriteUint32(10); // strlen("test") is 4
    ret = softBusServer->SetDisplayNameInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // full parcel, SetDisplayName result is written into reply
    datas.WriteCString(dmPkgName);
    datas.WriteCString(test);
    datas.WriteUint32(4);
    ret = softBusServer->SetDisplayNameInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest075
 * @tc.desc: Verify the CreateGroupOwnerInner permission, pkgName check and config read branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest075, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    const char *sharePkgName = "ohos.InterConnection.iShare";
    struct GroupOwnerConfig config = { 0 };

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = softBusServer->CreateGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->CreateGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test); // pkgName is not the share package
    ret = softBusServer->CreateGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(sharePkgName); // config read fail
    ret = softBusServer->CreateGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(sharePkgName);
    datas.WriteRawData(&config, sizeof(struct GroupOwnerConfig));
    ret = softBusServer->CreateGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_NE(nullptr, reply.ReadRawData(sizeof(struct GroupOwnerResult)));
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest076
 * @tc.desc: Verify the DestroyGroupOwnerInner permission and pkgName check branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest076, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    const char *sharePkgName = "ohos.InterConnection.iShare";
    MessageParcel datas;
    MessageParcel reply;

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_PERMISSION_DENIED));
    int32_t ret = softBusServer->DestroyGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);

    EXPECT_CALL(softbusServerStubMock, CheckLnnPermission).WillRepeatedly(Return(SOFTBUS_OK));
    ret = softBusServer->DestroyGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test); // pkgName is not the share package
    ret = softBusServer->DestroyGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(sharePkgName);
    ret = softBusServer->DestroyGroupOwnerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: SoftbusServerStubTest077
 * @tc.desc: Verify the SoftbusRegisterBrProxyServiceInner remote object and pkgName check branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest077, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    sptr<IRemoteObject> obj = GenerateRemoteObject();
    ASSERT_NE(nullptr, obj);
    const char *longPkgName =
        "1234567890123456789012345678901234567890123456789012345678901234567890123456789";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->SoftbusRegisterBrProxyServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_REMOTE_NULL, ret);

    datas.WriteRemoteObject(obj);
    ret = softBusServer->SoftbusRegisterBrProxyServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteRemoteObject(obj);
    datas.WriteCString(longPkgName);
    ret = softBusServer->SoftbusRegisterBrProxyServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteRemoteObject(obj);
    datas.WriteCString("test_br_proxy_pkg");
    ret = softBusServer->SoftbusRegisterBrProxyServiceInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest078
 * @tc.desc: Verify the OpenBrProxyInner and CloseBrProxyInner read fail and process branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest078, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t channelId = 1;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->OpenBrProxyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test); // uuid read fail
    ret = softBusServer->OpenBrProxyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READCSTRING_FAILED, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->OpenBrProxyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));

    ret = softBusServer->CloseBrProxyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->CloseBrProxyInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest079
 * @tc.desc: Verify the SendBrProxyDataInner dataLen check and raw data read branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest079, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    int32_t channelId = 1;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->SendBrProxyDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->SendBrProxyDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READUINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteUint32(0); // dataLen == 0
    ret = softBusServer->SendBrProxyDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_BR_PROXY_DATA_TOO_LONG, ret);

    datas.WriteInt32(channelId);
    datas.WriteUint32(BR_PROXY_SEND_MAX_LEN + 1); // dataLen > BR_PROXY_SEND_MAX_LEN
    ret = softBusServer->SendBrProxyDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_BR_PROXY_DATA_TOO_LONG, ret);

    datas.WriteInt32(channelId);
    datas.WriteUint32(4); // raw data read fail
    ret = softBusServer->SendBrProxyDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteInt32(channelId);
    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    ret = softBusServer->SendBrProxyDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest080
 * @tc.desc: Verify the SetBrProxyListenerStateInner and GetBrProxyChannelStateInner branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest080, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    int32_t channelId = 1;
    int32_t type = 0;
    bool cbEnabled = true;
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->SetBrProxyListenerStateInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->SetBrProxyListenerStateInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(type);
    ret = softBusServer->SetBrProxyListenerStateInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    datas.WriteInt32(type);
    datas.WriteBool(cbEnabled);
    ret = softBusServer->SetBrProxyListenerStateInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));

    ret = softBusServer->GetBrProxyChannelStateInner(datas, reply);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_READINT_FAILED, ret);

    datas.WriteInt32(channelId);
    ret = softBusServer->GetBrProxyChannelStateInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    bool isEnable = true;
    EXPECT_TRUE(reply.ReadBool(isEnable));
}

/*
 * @tc.name: SoftbusServerStubTest081
 * @tc.desc: Verify the RegisterPushHookInner branch when no push service client is registered
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest081, TestSize.Level1)
{
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->RegisterPushHookInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_REMOTE_NULL, replyVal);
}

/*
 * @tc.name: SoftbusServerStubTest082
 * @tc.desc: Verify the PostConversationDataInner read fail and CheckBundleName branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest082, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    // len read fail
    int32_t ret = softBusServer->PostConversationDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    // msg raw data read fail
    datas.WriteUint32(4);
    ret = softBusServer->PostConversationDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    // deviceId read fail
    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    ret = softBusServer->PostConversationDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NETWORK_READCSTRING_FAILED, ret);

    // abilityName read fail
    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    datas.WriteCString(test);
    ret = softBusServer->PostConversationDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NETWORK_READCSTRING_FAILED, ret);

    // bundleName is null, CheckBundleName returns SOFTBUS_INVALID_PARAM
    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->PostConversationDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // caller token is native, CheckBundleName returns SOFTBUS_PERMISSION_DENIED
    datas.WriteUint32(4);
    datas.WriteRawData(test, 4);
    datas.WriteCString(test);
    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->PostConversationDataInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest083
 * @tc.desc: Verify the Register and Unregister ConversationListenerInner branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest083, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    // abilityName read fail
    int32_t ret = softBusServer->RegisterConversationListenerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NETWORK_READCSTRING_FAILED, ret);

    // bundleName is null
    datas.WriteCString(test);
    ret = softBusServer->RegisterConversationListenerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // caller token is native, CheckBundleName returns SOFTBUS_PERMISSION_DENIED
    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->RegisterConversationListenerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);

    ret = softBusServer->UnregisterConversationListenerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_NETWORK_READCSTRING_FAILED, ret);

    datas.WriteCString(test);
    ret = softBusServer->UnregisterConversationListenerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteCString(test);
    ret = softBusServer->UnregisterConversationListenerInner(datas, reply);
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, ret);
}

/*
 * @tc.name: SoftbusServerStubTest084
 * @tc.desc: Verify the GetTrustedDevicesInner branch with permission granted
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest084, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    MessageParcel datas;
    MessageParcel reply;

    EXPECT_NO_FATAL_FAILURE(softBusServer->GetTrustedDevicesInner(datas, reply));
}

/*
 * @tc.name: SoftbusServerStubTest085
 * @tc.desc: Verify the StartPerceptionAdvInner and SetPerceptionAdvHighFreqInner branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest085, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    PerceptionAdvParam advParam = { 0 };
    MessageParcel datas;
    MessageParcel reply;

    // pkgName read fail
    int32_t ret = softBusServer->StartPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // type read fail
    datas.WriteCString(test);
    ret = softBusServer->StartPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // type out of range
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_BUTT);
    ret = softBusServer->StartPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // adv param raw data read fail
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    ret = softBusServer->StartPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    // customDataLen is invalid
    advParam.customDataLen = PERCEPTION_CUSTOM_DATA_MAX_LEN + 1;
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    datas.WriteRawData(&advParam, sizeof(PerceptionAdvParam));
    ret = softBusServer->StartPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // full parcel, LnnIpcStartPerceptionAdv result is written into reply
    advParam.customDataLen = 0;
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    datas.WriteRawData(&advParam, sizeof(PerceptionAdvParam));
    ret = softBusServer->StartPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));

    // SetPerceptionAdvHighFreqInner
    ret = softBusServer->SetPerceptionAdvHighFreqInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    ret = softBusServer->SetPerceptionAdvHighFreqInner(datas, reply);
    EXPECT_EQ(SOFTBUS_IPC_ERR, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    datas.WriteRawData(&advParam, sizeof(PerceptionAdvParam));
    ret = softBusServer->SetPerceptionAdvHighFreqInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest086
 * @tc.desc: Verify the StopPerceptionAdvInner and StopPerceptionScanInner branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest086, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->StopPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    ret = softBusServer->StopPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_BUTT);
    ret = softBusServer->StopPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    ret = softBusServer->StopPerceptionAdvInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));

    ret = softBusServer->StopPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    ret = softBusServer->StopPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    ret = softBusServer->StopPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest087
 * @tc.desc: Verify the StartPerceptionScanInner cycle check branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest087, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->StartPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    ret = softBusServer->StartPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // cycle read fail
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    ret = softBusServer->StartPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // cycle out of range
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    datas.WriteInt32(PERCEPTION_CYCLE_BUTT);
    ret = softBusServer->StartPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    // full parcel, LnnIpcStartPerceptionScan result is written into reply
    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    datas.WriteInt32(PERCEPTION_CYCLE_LOW);
    ret = softBusServer->StartPerceptionScanInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
}

/*
 * @tc.name: SoftbusServerStubTest088
 * @tc.desc: Verify the GetPerceptionDeviceListInner header check and reply branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SoftbusServerStubTest088, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    MessageParcel datas;
    MessageParcel reply;

    int32_t ret = softBusServer->GetPerceptionDeviceListInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    ret = softBusServer->GetPerceptionDeviceListInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_BUTT);
    ret = softBusServer->GetPerceptionDeviceListInner(datas, reply);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    datas.WriteCString(test);
    datas.WriteInt32(PERCEPTION_TYPE_COLLABORATIVE_WAKE);
    ret = softBusServer->GetPerceptionDeviceListInner(datas, reply);
    EXPECT_EQ(SOFTBUS_OK, ret);
    int32_t replyVal = 0;
    uint32_t replyCount = 0;
    EXPECT_TRUE(reply.ReadInt32(replyVal));
    EXPECT_TRUE(reply.ReadUint32(replyCount));
}

/*
 * @tc.name: CheckPermissionTest004
 * @tc.desc: Verify CheckPermission returns SOFTBUS_ACCESS_TOKEN_DENIED when the
 *           required permission is not granted to the native token
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, CheckPermissionTest004, TestSize.Level1)
{
    SetAccessTokenPermission("device_manager");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    int32_t ret = softBusServer->CheckPermission(SERVER_GET_DM_EX_OBJ);
    EXPECT_EQ(ret, SOFTBUS_ACCESS_TOKEN_DENIED);

    ret = softBusServer->CheckPermission(SERVER_OPEN_SESSION);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = softBusServer->CheckPermission(SERVER_GET_TRUSTED_DEVICES);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: BasicPermissionVerifyTest001
 * @tc.desc: Verify BasicPermissionVerify returns SOFTBUS_OK when the native token is
 *           a system app with SEC_ACCESS_UDID and DISTRIBUTED_DATASYNC granted
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, BasicPermissionVerifyTest001, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    int32_t ret = softBusServer->BasicPermissionVerify(SERVER_GET_TRUSTED_DEVICES);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SysHapPermissionVerifyTest001
 * @tc.desc: Verify SysHapPermissionVerify returns SOFTBUS_OK when the native token
 *           is a system app token
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubTest, SysHapPermissionVerifyTest001, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    int32_t ret = softBusServer->SysHapPermissionVerify(SERVER_START_PERCEPTION_ADV);
    EXPECT_EQ(ret, SOFTBUS_OK);
}
}