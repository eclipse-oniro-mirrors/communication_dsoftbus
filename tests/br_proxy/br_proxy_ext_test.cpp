/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstring>
#include <gtest/gtest.h>
#include <securec.h>

#include "accesstoken_kit.h"
#include "br_proxy_ext_test_mock.h"
#include "br_proxy.c"
#include "message_handler.h"
#include "nativetoken_kit.h"
#include "softbus_adapter_mem.h"
#include "softbus_common.h"
#include "softbus_conn_interface.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "token_setproc.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
#define CHANNEL_ID 5
#define CHANNEL_ID_ERR 0
#define SESSION_ID 2

class BrProxyExtTest : public testing::Test {
public:
    BrProxyExtTest()
    {}
    ~BrProxyExtTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

static void AddPermission()
{
    uint64_t tokenId;
    const char *perms[1];
    perms[0] = OHOS_PERMISSION_ACCESS_BLUETOOTH;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 1,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "BrExProxyTest",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void BrProxyExtTest::SetUpTestCase(void)
{
    AddPermission();
    NiceMock<BrProxyExtInterfaceMock> brProxyExtMock;
    EXPECT_CALL(brProxyExtMock, CreateSoftBusList).WillOnce(Return(nullptr));
    TransClientInit();
}

void BrProxyExtTest::TearDownTestCase(void)
{
}

BrProxyChannelInfo g_channelInfo = {
    .peerBRMacAddr = "AA:BB:CC:AA:BB:CC",
    .peerBRUuid = "AAAAAAAA-BBBB-CCCC-AAAA-BBBBBBBBBBBB",
    .recvPri = 1,
    .recvPriSet = true,
};

static int32_t onChannelOpened(int32_t sessionId, int32_t channelId, int32_t result)
{
    return SOFTBUS_OK;
}

static void onDataReceived(int32_t channelId, const char *data, uint32_t dataLen)
{
}

static void onChannelStatusChanged(int32_t channelId, int32_t state)
{
}

static IBrProxyListener g_listener = {
    .onChannelOpened = onChannelOpened,
    .onDataReceived = onDataReceived,
    .onChannelStatusChanged = onChannelStatusChanged,
};

/**
 * @tc.name: BrProxyExtTest000
 * @tc.desc: BrProxyExtTest000, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, BrProxyExtTest000, TestSize.Level1)
{
    int32_t ret = ClientDeleteChannelFromList(0, nullptr, nullptr);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ret = ClientUpdateList(nullptr, nullptr, 0);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ret = ClientQueryList(0, nullptr, nullptr, nullptr);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ret = ClientRecordListenerState(0, CHANNEL_STATE, true);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ret = IsChannelValid(CHANNEL_ID_ERR);
    EXPECT_EQ(false, ret);
    ret = ClientTransBrProxyChannelChange(CHANNEL_ID_ERR, 0);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ret = ClientTransBrProxyDataReceived(CHANNEL_ID_ERR, nullptr, 0);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);
    ret = SetListenerState(CHANNEL_ID_ERR, CHANNEL_STATE, true);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_ID, ret);
    ret = SendBrProxyData(CHANNEL_ID_ERR, nullptr, 0);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_ID, ret);
    ret = CloseBrProxy(CHANNEL_ID_ERR);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_ID, ret);
}

/**
 * @tc.name: CheckMacFormatTest001
 * @tc.desc: CheckMacFormatTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, CheckMacFormatTest001, TestSize.Level1)
{
    int32_t validChars = 0;
    int32_t sepCount = 0;
    char macAddr1[BR_MAC_LEN] = "H0:AA:CC:BB:AA:CC";
    int32_t ret = CheckMacFormat(macAddr1, BR_MAC_LEN, &validChars, &sepCount);
    EXPECT_EQ(false, ret);

    char macAddr2[BR_MAC_LEN] = "FF:EE:DD:CC:BB:AA";
    macAddr2[5] = '\0';
    ret = CheckMacFormat(macAddr2, BR_MAC_LEN, &validChars, &sepCount);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsMacValidTest001
 * @tc.desc: IsMacValidTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsMacValidTest001, TestSize.Level1)
{
    int32_t ret = IsMacValid(nullptr);
    EXPECT_EQ(false, ret);

    char macAddr1[] = "H0:AA:CC:BB:DD:BB:AA";
    ret = IsMacValid(macAddr1);
    EXPECT_EQ(false, ret);

    char macAddr2[BR_MAC_LEN] = "H0:FF:CC:AA:BB:CC";
    ret = IsMacValid(macAddr2);
    EXPECT_EQ(false, ret);

    macAddr2[5] = ':';
    ret = IsMacValid(macAddr2);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsUuidValidTest001
 * @tc.desc: IsUuidValidTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsUuidValidTest001, TestSize.Level1)
{
    char uuid1[] = "12345";
    int32_t ret = IsUuidValid(uuid1);
    EXPECT_EQ(false, ret);

    char uuid2[] = "EEEEEEE-AAAAA-1111-BBBB-CCCCCCCCCCCC";
    ret = IsUuidValid(uuid2);
    EXPECT_EQ(false, ret);

    char uuid3[] = "HAAAAAAA-0000-1111-2222-333333333333";
    ret = IsUuidValid(uuid3);
    EXPECT_EQ(false, ret);

    char uuid4[] = "88888888-000000000-0000-888888888888";
    ret = IsUuidValid(uuid4);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: CheckOpenParmTest001
 * @tc.desc: CheckOpenParmTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, CheckOpenParmTest001, TestSize.Level1)
{
    BrProxyChannelInfo info1 = {
        .peerBRMacAddr = "F0:FA",
    };
    int32_t ret = CheckOpenParm(&info1, &g_listener);
    EXPECT_EQ(SOFTBUS_TRANS_BR_PROXY_INVALID_PARAM, ret);

    BrProxyChannelInfo info2 = {
        .peerBRMacAddr = "FF:AA:CC:AA:BB:CC",
        .peerBRUuid = "1134",
    };
    ret = CheckOpenParm(&info2, &g_listener);
    EXPECT_EQ(SOFTBUS_TRANS_BR_PROXY_INVALID_PARAM, ret);
}

/**
 * @tc.name: OpenBrProxyTest001
 * @tc.desc: OpenBrProxyTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, OpenBrProxyTest001, TestSize.Level1)
{
    NiceMock<BrProxyExtInterfaceMock> brProxyExtMock;
    EXPECT_CALL(brProxyExtMock, CreateSoftBusList).WillOnce(Return(nullptr));
    int32_t ret = OpenBrProxy(0, &g_channelInfo, &g_listener);
    EXPECT_EQ(SOFTBUS_CREATE_LIST_ERR, ret);
}

/**
 * @tc.name: OpenBrProxyTest002
 * @tc.desc: OpenBrProxyTest002, use the normal or wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, OpenBrProxyTest002, TestSize.Level1)
{
    SoftBusList *list = (SoftBusList *)SoftBusCalloc(sizeof(SoftBusList));
    ASSERT_TRUE(list != nullptr);
    SoftBusMutexAttr mutexAttr;
    mutexAttr.type = SOFTBUS_MUTEX_RECURSIVE;
    SoftBusMutexInit(&list->lock, &mutexAttr);
    ListInit(&list->list);
    NiceMock<BrProxyExtInterfaceMock> brProxyExtMock;
    EXPECT_CALL(brProxyExtMock, CreateSoftBusList).WillRepeatedly(Return(list));
    EXPECT_CALL(brProxyExtMock, ClientStubInit).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(brProxyExtMock, ClientRegisterBrProxyService).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(brProxyExtMock, ServerIpcOpenBrProxy).WillRepeatedly(Return(SOFTBUS_TRANS_SESSION_OPENING));
    int32_t ret = OpenBrProxy(SESSION_ID, &g_channelInfo, &g_listener);
    EXPECT_EQ(SOFTBUS_TRANS_SESSION_OPENING, ret);

    ClientDeleteChannelFromList(CHANNEL_ID, g_channelInfo.peerBRMacAddr, g_channelInfo.peerBRUuid);
    if (list != nullptr) {
        SoftBusFree(list);
    }
}

static SoftBusList *CreateTestClientList()
{
    SoftBusList *list = (SoftBusList *)SoftBusCalloc(sizeof(SoftBusList));
    if (list == nullptr) {
        return nullptr;
    }
    SoftBusMutexAttr mutexAttr;
    mutexAttr.type = SOFTBUS_MUTEX_RECURSIVE;
    SoftBusMutexInit(&list->lock, &mutexAttr);
    ListInit(&list->list);
    return list;
}

static void DestroyTestClientList(SoftBusList *list)
{
    if (list == nullptr) {
        return;
    }
    ClientBrProxyChannelInfo *nodeInfo = nullptr;
    ClientBrProxyChannelInfo *nodeNext = nullptr;
    LIST_FOR_EACH_ENTRY_SAFE(nodeInfo, nodeNext, &list->list, ClientBrProxyChannelInfo, node) {
        ListDelete(&nodeInfo->node);
        SoftBusFree(nodeInfo);
    }
    g_clientList = nullptr;
    SoftBusFree(list);
}

static int32_t PrepareTestChannel(BrProxyExtInterfaceMock &mock, int32_t channelId)
{
    EXPECT_CALL(mock, ClientStubInit).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, ClientRegisterBrProxyService).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, ServerIpcOpenBrProxy).WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = OpenBrProxy(SESSION_ID, &g_channelInfo, &g_listener);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ClientTransOnBrProxyOpened(channelId, g_channelInfo.peerBRMacAddr, g_channelInfo.peerBRUuid, SOFTBUS_OK);
    EXPECT_EQ(SOFTBUS_OK, ret);
    return ret;
}

/**
 * @tc.name: CloseBrProxyTest001
 * @tc.desc: CloseBrProxy, cover ipc success, ipc invalid channel id and ipc failed branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, CloseBrProxyTest001, TestSize.Level1)
{
    SoftBusList *list = CreateTestClientList();
    ASSERT_TRUE(list != nullptr);
    g_clientList = list;
    NiceMock<BrProxyExtInterfaceMock> mock;
    ASSERT_EQ(SOFTBUS_OK, PrepareTestChannel(mock, CHANNEL_ID));

    EXPECT_CALL(mock, ServerIpcCloseBrProxy).WillOnce(Return(SOFTBUS_OK));
    int32_t ret = CloseBrProxy(CHANNEL_ID);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(false, IsChannelValid(CHANNEL_ID));

    ASSERT_EQ(SOFTBUS_OK, PrepareTestChannel(mock, CHANNEL_ID));
    EXPECT_CALL(mock, ServerIpcCloseBrProxy).WillOnce(Return(SOFTBUS_TRANS_INVALID_CHANNEL_ID));
    ret = CloseBrProxy(CHANNEL_ID);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(false, IsChannelValid(CHANNEL_ID));

    ASSERT_EQ(SOFTBUS_OK, PrepareTestChannel(mock, CHANNEL_ID));
    EXPECT_CALL(mock, ServerIpcCloseBrProxy).WillOnce(Return(SOFTBUS_TRANS_PROXY_INVALID_CHANNEL_ID));
    ret = CloseBrProxy(CHANNEL_ID);
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_INVALID_CHANNEL_ID, ret);
    EXPECT_EQ(true, IsChannelValid(CHANNEL_ID));

    DestroyTestClientList(list);
}

/**
 * @tc.name: SendBrProxyDataTest001
 * @tc.desc: SendBrProxyData, cover data too long, ipc failed and ipc success branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, SendBrProxyDataTest001, TestSize.Level1)
{
    SoftBusList *list = CreateTestClientList();
    ASSERT_TRUE(list != nullptr);
    g_clientList = list;
    NiceMock<BrProxyExtInterfaceMock> mock;
    ASSERT_EQ(SOFTBUS_OK, PrepareTestChannel(mock, CHANNEL_ID));

    char data[] = "sendBrProxyData";
    int32_t ret = SendBrProxyData(CHANNEL_ID, data, BR_PROXY_SEND_MAX_LEN + 1);
    EXPECT_EQ(SOFTBUS_TRANS_BR_PROXY_DATA_TOO_LONG, ret);

    EXPECT_CALL(mock, ServerIpcSendBrProxyData).WillOnce(Return(SOFTBUS_TRANS_PROXY_SENDMSG_ERR));
    ret = SendBrProxyData(CHANNEL_ID, data, strlen(data));
    EXPECT_EQ(SOFTBUS_TRANS_PROXY_SENDMSG_ERR, ret);

    EXPECT_CALL(mock, ServerIpcSendBrProxyData).WillOnce(Return(SOFTBUS_OK));
    ret = SendBrProxyData(CHANNEL_ID, data, strlen(data));
    EXPECT_EQ(SOFTBUS_OK, ret);

    DestroyTestClientList(list);
}

/**
 * @tc.name: SetListenerStateTest001
 * @tc.desc: SetListenerState, cover ipc failed and ipc success branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, SetListenerStateTest001, TestSize.Level1)
{
    SoftBusList *list = CreateTestClientList();
    ASSERT_TRUE(list != nullptr);
    g_clientList = list;
    NiceMock<BrProxyExtInterfaceMock> mock;
    ASSERT_EQ(SOFTBUS_OK, PrepareTestChannel(mock, CHANNEL_ID));

    EXPECT_CALL(mock, ServerIpcSetListenerState).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    int32_t ret = SetListenerState(CHANNEL_ID, DATA_RECEIVE, true);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    EXPECT_CALL(mock, ServerIpcSetListenerState).WillOnce(Return(SOFTBUS_OK));
    ret = SetListenerState(CHANNEL_ID, DATA_RECEIVE, true);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_CALL(mock, ServerIpcSetListenerState).WillOnce(Return(SOFTBUS_OK));
    ret = SetListenerState(CHANNEL_ID, LISTENER_TYPE_MAX, true);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    DestroyTestClientList(list);
}

/**
 * @tc.name: ClientTransOnBrProxyOpenedExtTest001
 * @tc.desc: ClientTransOnBrProxyOpened, cover open failed, update failed and null callback branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, ClientTransOnBrProxyOpenedExtTest001, TestSize.Level1)
{
    SoftBusList *list = CreateTestClientList();
    ASSERT_TRUE(list != nullptr);
    g_clientList = list;
    NiceMock<BrProxyExtInterfaceMock> mock;
    ASSERT_EQ(SOFTBUS_OK, PrepareTestChannel(mock, CHANNEL_ID));

    int32_t ret = ClientTransOnBrProxyOpened(CHANNEL_ID, "11:22:33:44:55:66", g_channelInfo.peerBRUuid, SOFTBUS_OK);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);

    ret = ClientTransOnBrProxyOpened(CHANNEL_ID, g_channelInfo.peerBRMacAddr, g_channelInfo.peerBRUuid,
        SOFTBUS_TRANS_INVALID_CHANNEL_ID);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(false, IsChannelValid(CHANNEL_ID));

    DestroyTestClientList(list);
}

/**
 * @tc.name: BrProxyServiceDeathNotifyExtTest001
 * @tc.desc: BrProxyServiceDeathNotify, cover client list not init branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, BrProxyServiceDeathNotifyExtTest001, TestSize.Level1)
{
    g_clientList = nullptr;
    BrProxyServiceDeathNotify();
}

/**
 * @tc.name: IsProxyChannelEnabledExtTest001
 * @tc.desc: IsProxyChannelEnabled, cover ipc success with enabled state branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsProxyChannelEnabledExtTest001, TestSize.Level1)
{
    NiceMock<BrProxyExtInterfaceMock> mock;
    EXPECT_CALL(mock, ClientStubInit).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, ClientRegisterService).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, ServerIpcIsProxyChannelEnabled).WillOnce(DoAll(SetArgPointee<1>(true), Return(SOFTBUS_OK)));
    bool ret = IsProxyChannelEnabled(0);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: SoftbusErrConvertChannelStateTest001
 * @tc.desc: SoftbusErrConvertChannelStateTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, SoftbusErrConvertChannelStateTest001, TestSize.Level1)
{
    int32_t err1 = SOFTBUS_CONN_BR_UNDERLAY_SOCKET_CLOSED;
    int32_t err2 = 4123;
    int32_t ret = SoftbusErrConvertChannelState(err1);
    EXPECT_EQ(CHANNEL_WAIT_RESUME, ret);

    ret = SoftbusErrConvertChannelState(err2);
    EXPECT_EQ(CHANNEL_EXCEPTION_SOFTWARE_FAILED, ret);
}

/**
 * @tc.name: IsProxyChannelEnabledTest001
 * @tc.desc: IsProxyChannelEnabledTest001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsProxyChannelEnabledTest001, TestSize.Level1)
{
    NiceMock<BrProxyExtInterfaceMock> brProxyExtMock;
    EXPECT_CALL(brProxyExtMock, ClientStubInit).WillOnce(Return(SOFTBUS_NO_INIT)).WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = IsProxyChannelEnabled(0);
    EXPECT_EQ(false, ret);

    EXPECT_CALL(brProxyExtMock, ClientRegisterService).WillOnce(Return(SOFTBUS_TRANS_INVALID_CHANNEL_ID));
    ret = IsProxyChannelEnabled(0);
    EXPECT_EQ(false, ret);

    EXPECT_CALL(brProxyExtMock, ClientRegisterService).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(brProxyExtMock, ServerIpcIsProxyChannelEnabled).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    ret = IsProxyChannelEnabled(0);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: RegisterAccessHookTest001
 * @tc.desc: RegisterAccessHookTest001, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, RegisterAccessHookTest001, TestSize.Level1)
{
    int32_t ret = RegisterAccessHook(nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    PermissonHookCb cb;
    NiceMock<BrProxyExtInterfaceMock> brProxyExtMock;
    EXPECT_CALL(brProxyExtMock, ClientStubInit).WillOnce(Return(SOFTBUS_NO_INIT)).WillRepeatedly(Return(SOFTBUS_OK));
    ret = RegisterAccessHook(&cb);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    EXPECT_CALL(brProxyExtMock, ClientRegisterService).WillOnce(Return(SOFTBUS_TRANS_INVALID_CHANNEL_ID))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(brProxyExtMock, ServerIpcRegisterPushHook).WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    ret = RegisterAccessHook(&cb);
    EXPECT_EQ(SOFTBUS_TRANS_INVALID_CHANNEL_ID, ret);

    ret = RegisterAccessHook(&cb);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    ret = RegisterAccessHook(&cb);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

static int32_t QueryPermissionFail(const char *bundleName, bool *isEmpowered)
{
    (void)bundleName;
    (void)isEmpowered;
    return SOFTBUS_INVALID_PARAM;
}

static int32_t QueryPermissionSucc(const char *bundleName, bool *isEmpowered)
{
    (void)bundleName;
    (void)isEmpowered;
    return SOFTBUS_OK;
}

/**
 * @tc.name: ClientTransBrProxyQueryPermissionTest001
 * @tc.desc: ClientTransBrProxyQueryPermissionTest001, use the wrong or normal parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, ClientTransBrProxyQueryPermissionTest001, TestSize.Level1)
{
    int32_t ret = ClientTransBrProxyQueryPermission(nullptr, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    const char *bundleName = "testbundlename";
    bool isEmpowered = false;
    ret = ClientTransBrProxyQueryPermission(bundleName, &isEmpowered);
    EXPECT_EQ(SOFTBUS_NO_INIT, ret);

    g_pushCb.queryPermission = &QueryPermissionFail;
    ret = ClientTransBrProxyQueryPermission(bundleName, &isEmpowered);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    g_pushCb.queryPermission = &QueryPermissionSucc;
    ret = ClientTransBrProxyQueryPermission(bundleName, &isEmpowered);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/**
 * @tc.name: IsMacValidExtTest001
 * @tc.desc: IsMacValid, cover valid formats and each invalid format branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsMacValidExtTest001, TestSize.Level1)
{
    EXPECT_TRUE(IsMacValid("AABBCCDDEEFF"));
    EXPECT_TRUE(IsMacValid("AA:BB:CC:DD:EE:FF"));
    EXPECT_TRUE(IsMacValid("AA-BB-CC-DD-EE-FF"));
    EXPECT_TRUE(IsMacValid("AA.BB.CC.DD.EE.FF"));

    EXPECT_FALSE(IsMacValid("AA:BB"));
    EXPECT_FALSE(IsMacValid("AABBCCDD:EEFF"));
    EXPECT_FALSE(IsMacValid("AA:BB-CC:DD:EE:FF"));
    EXPECT_FALSE(IsMacValid(":AABBCCDD:EEFF"));
    EXPECT_FALSE(IsMacValid("AABBCCDD:EEFF:"));
    EXPECT_FALSE(IsMacValid("AA::BB:CC:DDEEFF"));
    EXPECT_FALSE(IsMacValid("AA:BB:CC:DD:EE:FG"));
}

/**
 * @tc.name: CheckSeparatorConsistencyExtTest001
 * @tc.desc: CheckSeparatorConsistency, cover mixed separators and consistent separators
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, CheckSeparatorConsistencyExtTest001, TestSize.Level1)
{
    EXPECT_FALSE(CheckSeparatorConsistency("AA:BB-CC:DD:EE:FF", 17));
    EXPECT_TRUE(CheckSeparatorConsistency("AA:BB:CC:DD:EE:FF", 17));
}

/**
 * @tc.name: IsUuidValidExtTest001
 * @tc.desc: IsUuidValid, cover standard, no hyphen and each invalid branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsUuidValidExtTest001, TestSize.Level1)
{
    EXPECT_TRUE(IsUuidValid("0000FEEA-0000-1000-8000-00805F9B34FB"));
    EXPECT_TRUE(IsUuidValid("0000FEEA00001000800000805F9B34FB"));

    EXPECT_FALSE(IsUuidValid("0000FEEA00001000-800000805F9B34F"));
    EXPECT_FALSE(IsUuidValid("0000FEEA-0000-1000-0000F9B34FB000000"));
    EXPECT_FALSE(IsUuidValid("0000FEEA-0000-1000-8000-00805F9B-4FB"));
    EXPECT_FALSE(IsUuidValid("0000FEEA00001000G800000805F9B34F"));
}

/**
 * @tc.name: IsPeerDevAddrValidExtTest001
 * @tc.desc: IsPeerDevAddrValid, cover sha256 and mac valid branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyExtTest, IsPeerDevAddrValidExtTest001, TestSize.Level1)
{
    EXPECT_TRUE(IsPeerDevAddrValid("0123456789abcdef0123456789abcdef"));
    EXPECT_TRUE(IsPeerDevAddrValid("AA:BB:CC:DD:EE:FF"));
    EXPECT_FALSE(IsPeerDevAddrValid("AA:BB:CC"));
}
}