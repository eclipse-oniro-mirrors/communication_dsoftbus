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

/* Defined in softbus_server_stub_test.cpp, linked from the same test target. */
extern char g_mySessionName[TEST_SESSION_NAME_SIZE_MAX];
extern char g_peerSessionName[TEST_SESSION_NAME_SIZE_MAX];
extern char g_peerDeviceId[TEST_DEVICE_ID_SIZE_MAX];
extern char g_groupId[TEST_GROUP_ID_SIZE_MAX];
extern char g_myPkgName[TEST_PKG_NAME_SIZE_MAX];

class SoftbusServerStubExtTest : public testing::Test {
public:
    SoftbusServerStubExtTest()
    {}
    ~SoftbusServerStubExtTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void SoftbusServerStubExtTest::SetUpTestCase(void)
{
    SoftbusConfigInit();
    ConnServerInit();
    AuthInit();
    InitDeviceAuthService();
    BusCenterServerInit();
    TransServerInit();
}

void SoftbusServerStubExtTest::TearDownTestCase(void)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, UnregChangeListener).WillRepeatedly(Return(SOFTBUS_OK));
    ConnServerDeinit();
    AuthDeinit();
    BusCenterServerDeinit();
    TransServerDeinit();
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest065, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest066, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest067, TestSize.Level1)
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
    const char *newSessionName = "com.test.trans.session.new";
    ASSERT_EQ(EOK, strcpy_s(sessionServer->sessionName, sizeof(sessionServer->sessionName), newSessionName));
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest068, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest069, TestSize.Level1)
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
 * @tc.desc: Verify the SetSocketOptEdcaInner read fail and pid check fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest070, TestSize.Level1)
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
}

/*
 * @tc.name: SoftbusServerStubTest089
 * @tc.desc: Verify the SetSocketOptEdcaInner edca param invalid and channel not exist branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest089, TestSize.Level1)
{
    NiceMock<SoftbusServerStubTestInterfaceMock> softbusServerStubMock;
    EXPECT_CALL(softbusServerStubMock, TransGetAndComparePid).WillRepeatedly(Return(SOFTBUS_OK));
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    int32_t ret;

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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest071, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest072, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest073, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest074, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest075, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest076, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest077, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest078, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest079, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest080, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest081, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest082, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest083, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest084, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest085, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(nullptr, softBusServer);
    char test[10] = "test";
    PerceptionAdvParam advParam = {};
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest086, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest087, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SoftbusServerStubTest088, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, CheckPermissionTest004, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, BasicPermissionVerifyTest001, TestSize.Level1)
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
HWTEST_F(SoftbusServerStubExtTest, SysHapPermissionVerifyTest001, TestSize.Level1)
{
    SetAccessTokenPermission("SoftBusServerStubTest");
    sptr<OHOS::SoftBusServerStub> softBusServer = new OHOS::SoftBusServer(SOFTBUS_SERVER_SA_ID, true);
    ASSERT_NE(softBusServer, nullptr);

    int32_t ret = softBusServer->SysHapPermissionVerify(SERVER_START_PERCEPTION_ADV);
    EXPECT_EQ(ret, SOFTBUS_OK);
}
} // namespace OHOS
