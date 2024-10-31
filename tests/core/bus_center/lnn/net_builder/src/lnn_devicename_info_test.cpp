/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <securec.h>

#include "lnn_connection_mock.h"
#include "lnn_devicename_info.c"
#include "lnn_devicename_info.h"
#include "lnn_net_ledger_mock.h"
#include "lnn_service_mock.h"
#include "lnn_sync_info_mock.h"
#include "softbus_errcode.h"

namespace OHOS {
using namespace testing;
using namespace testing::ext;
NodeInfo *info = {0};
constexpr int64_t ACCOUNT_ID = 10;
constexpr char *DEVICE_NAME1 = nullptr;
constexpr char DEVICE_NAME2[] = "ABCDEFG";
constexpr uint32_t MSG_ERR_LEN0 = 0;
constexpr char NODE_UDID[] = "123456ABCDEF";
constexpr char NETWORKID[NETWORK_ID_BUF_LEN] = "123456ABD";

class LNNDeviceNameInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LNNDeviceNameInfoTest::SetUpTestCase()
{
}

void LNNDeviceNameInfoTest::TearDownTestCase()
{
}

void LNNDeviceNameInfoTest::SetUp()
{
}

void LNNDeviceNameInfoTest::TearDown()
{
}

/*
* @tc.name: LNN_UPDATE_DEVICE_NAME_TEST_001
* @tc.desc: no retry
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_UPDATE_DEVICE_NAME_TEST_001, TestSize.Level1)
{
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    NiceMock<LnnConnectInterfaceMock> connMock;
    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName).WillRepeatedly(
        LnnServicetInterfaceMock::ActionOfLnnGetSettingDeviceName);
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnGetAllOnlineAndMetaNodeInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillRepeatedly(Return(info));
    EXPECT_CALL(ledgerMock, LnnGetDeviceName).WillRepeatedly(Return(DEVICE_NAME1));
    EXPECT_CALL(ledgerMock, LnnGetDeviceName).WillRepeatedly(Return(DEVICE_NAME2));
    EXPECT_CALL(serviceMock, LnnInitGetDeviceName).WillRepeatedly(
        LnnServicetInterfaceMock::ActionOfLnnInitGetDeviceName);
    EXPECT_CALL(serviceMock, RegisterNameMonitor).WillRepeatedly(Return());
    EXPECT_CALL(connMock, DiscDeviceInfoChanged).WillRepeatedly(Return());
    UpdateDeviceName(nullptr);
    LnnDeviceNameHandler HandlerGetDeviceName = LnnServicetInterfaceMock::g_deviceNameHandler;
    HandlerGetDeviceName(DEVICE_NAME_TYPE_DEV_NAME, nullptr);
}

/*
* @tc.name: LNN_UPDATE_DEVICE_NAME_TEST_002
* @tc.desc: looper is null
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_UPDATE_DEVICE_NAME_TEST_002, TestSize.Level1)
{
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(serviceMock, LnnInitGetDeviceName).WillRepeatedly(
        LnnServicetInterfaceMock::ActionOfLnnInitGetDeviceName);
    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName(_, _)).WillRepeatedly(Return(SOFTBUS_ERR));
    UpdateDeviceName(nullptr);
}

/*
* @tc.name: ON_RECEIVE_DEVICE_NAME_TEST_001
* @tc.desc: on receive device name test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, ON_RECEIVE_DEVICE_NAME_TEST_001, TestSize.Level1)
{
    char msg[] = "msg";
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(ledgerMock, LnnConvertDlId).WillOnce(Return(SOFTBUS_ERR)).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnSetDLDeviceInfoName).WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(ledgerMock, LnnGetBasicInfoByUdid).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnNotifyBasicInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, UpdateProfile).WillRepeatedly(Return());
    OnReceiveDeviceName(LNN_INFO_TYPE_COUNT, NETWORKID, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg), MSG_ERR_LEN0);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, nullptr, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, nullptr, strlen(msg));
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
    OnReceiveDeviceName(LNN_INFO_TYPE_DEVICE_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg), strlen(msg) + 1);
}

/*
* @tc.name: LNN_ACCOUNT_STATE_CHANGE_HANDLER_TEST_001
* @tc.desc: lnn account state change handler test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_ACCOUNT_STATE_CHANGE_HANDLER_TEST_001, TestSize.Level1)
{
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillRepeatedly(Return(nullptr));
    LnnEventBasicInfo info1;
    info1.event = LNN_EVENT_TYPE_MAX;
    LnnAccountStateChangeHandler(nullptr);
    LnnAccountStateChangeHandler(&info1);
    info1.event = LNN_EVENT_ACCOUNT_CHANGED;
    LnnAccountStateChangeHandler(&info1);
    LnnAccountStateChangeHandler(&info1);
}

/*
* @tc.name: ON_RECEIVE_DEVICE_NICK_NAME_TEST_001
* @tc.desc: on receive device nick name test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, ON_RECEIVE_DEVICE_NICK_NAME_TEST_001, TestSize.Level1)
{
    NodeInfo nodeInfo;
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr)).WillRepeatedly(Return(&nodeInfo));
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById)
        .WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    char msg1[] = "{\"KEY_NICK_NAME\":\"nickName\"}";
    char msg2[] = "{\"KEY_ACCOUNT\":10}";
    char msg3[] = "{\"KEY_ACCOUNT\":10, \"KEY_NICK_NAME\":\"nickName\"}";
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, nullptr, strlen(msg1) + 1);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_COUNT, NETWORKID, reinterpret_cast<uint8_t *>(msg1), strlen(msg1) + 1);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg1), MSG_ERR_LEN0);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg1), strlen(msg1) + 1);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg2), strlen(msg2) + 1);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg3), strlen(msg3) + 1);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg3), strlen(msg3) + 1);
    OnReceiveDeviceNickName(LNN_INFO_TYPE_NICK_NAME, NETWORKID, reinterpret_cast<uint8_t *>(msg3), strlen(msg3) + 1);
}

/*
* @tc.name: LNN_HANDLER_GET_DEVICE_NAME_TEST_001
* @tc.desc: lnn handler get device name test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_HANDLER_GET_DEVICE_NAME_TEST_001, TestSize.Level1)
{
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    NiceMock<LnnConnectInterfaceMock> connMock;
    EXPECT_CALL(connMock, DiscDeviceInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName)
        .WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnNotifyLocalNetworkIdChanged).WillRepeatedly(Return());
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnGetAllOnlineNodeInfo).WillRepeatedly(Return(SOFTBUS_ERR));
    LnnHandlerGetDeviceName(DEVICE_NAME_TYPE_DEV_NAME, "deviceName");
    LnnHandlerGetDeviceName(DEVICE_NAME_TYPE_DEV_NAME, "deviceName");
    LnnHandlerGetDeviceName(DEVICE_NAME_TYPE_DEV_NAME, "deviceName");
    LnnHandlerGetDeviceName(DEVICE_NAME_TYPE_DEV_NAME, "deviceName");
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr));
    LnnHandlerGetDeviceName(DEVICE_NAME_TYPE_NICK_NAME, "deviceName");
}

/*
* @tc.name: LNN_SYNC_DEVICE_NAME_TEST_001
* @tc.desc: lnn sync device name test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_SYNC_DEVICE_NAME_TEST_001, TestSize.Level1)
{
    NodeInfo nodeInfo;
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr))
        .WillRepeatedly(Return(&nodeInfo));
    EXPECT_CALL(ledgerMock, LnnGetDeviceName).WillOnce(Return(DEVICE_NAME1))
        .WillRepeatedly(Return(DEVICE_NAME2));
    NiceMock<LnnSyncInfoInterfaceMock> lnnSyncInfoMock;
    EXPECT_CALL(lnnSyncInfoMock, LnnSendSyncInfoMsg).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = LnnSyncDeviceName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_ERR);
    ret = LnnSyncDeviceName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_ERR);
    ret = LnnSyncDeviceName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_ERR);
    ret = LnnSyncDeviceName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_OK);
}

/*
* @tc.name: LNN_SYNC_DEVICE_NICK_NAME_TEST_001
* @tc.desc: lnn sync device nick name test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_SYNC_DEVICE_NICK_NAME_TEST_001, TestSize.Level1)
{
    NodeInfo nodeInfo;
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr))
        .WillRepeatedly(Return(&nodeInfo));
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(serviceMock, GetCurrentAccount).WillRepeatedly(Return(0));
    NiceMock<LnnSyncInfoInterfaceMock> lnnSyncInfoMock;
    EXPECT_CALL(lnnSyncInfoMock, LnnSendSyncInfoMsg).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = LnnSyncDeviceNickName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_ERR);
    ret = LnnSyncDeviceNickName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_ERR);
    ret = LnnSyncDeviceNickName(NETWORKID);
    EXPECT_TRUE(ret == SOFTBUS_OK);
}

/*
* @tc.name: NICK_NAME_MSG_PROC_TEST_001
* @tc.desc: nick name msg proc test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, NICK_NAME_MSG_PROC_TEST_001, TestSize.Level1)
{
    NodeInfo nodeInfo = { .accountId = ACCOUNT_ID + 1, };
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr))
        .WillRepeatedly(Return(&nodeInfo));
    EXPECT_CALL(ledgerMock, LnnSetDLDeviceNickName).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnGetBasicInfoByUdid).WillRepeatedly(Return(SOFTBUS_ERR));

    NodeInfo peerNodeInfo1 = { .deviceInfo.nickName = "nickName", };
    NodeInfo peerNodeInfo2 = {
        .deviceInfo.nickName = "diffNickName",
        .deviceInfo.unifiedName = "unifiedName",
        .deviceInfo.unifiedDefaultName = "unifiedDefaultName",
        .deviceInfo.deviceName = "deviceName",
    };
    NodeInfo peerNodeInfo3 = {
        .deviceInfo.nickName = "diffNickName",
        .deviceInfo.unifiedName = "",
        .deviceInfo.unifiedDefaultName = "unifiedDefaultName",
        .deviceInfo.deviceName = "deviceName",
    };
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById)
        .WillOnce(Return(SOFTBUS_ERR))
        .WillOnce(DoAll(SetArgPointee<2>(peerNodeInfo1), Return(SOFTBUS_OK)))
        .WillOnce(DoAll(SetArgPointee<2>(peerNodeInfo2), Return(SOFTBUS_OK)))
        .WillRepeatedly(DoAll(SetArgPointee<2>(peerNodeInfo3), Return(SOFTBUS_OK)));
    const char *displayName = "";
    EXPECT_CALL(serviceMock, LnnNotifyBasicInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, LnnGetDeviceDisplayName)
        .WillRepeatedly(DoAll(SetArgPointee<2>(*(const_cast<char *>(displayName))), Return(SOFTBUS_OK)));
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
    NickNameMsgProc(NETWORKID, ACCOUNT_ID, "nickName");
}

/*
* @tc.name: NOTIFY_DEVICE_DISPLAY_NAME_CHANGE_TEST_001
* @tc.desc: notify device display name change test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, NOTIFY_DEVICE_DISPLAY_NAME_CHANGE_TEST_001, TestSize.Level1)
{
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(ledgerMock, LnnGetBasicInfoByUdid).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnNotifyBasicInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, UpdateProfile).WillRepeatedly(Return());
    NotifyDeviceDisplayNameChange(NETWORKID, NODE_UDID);
    NotifyDeviceDisplayNameChange(NETWORKID, NODE_UDID);
    NotifyDeviceDisplayNameChange(NETWORKID, NODE_UDID);
}

/*
* @tc.name: IS_DEVICE_NEED_SYNC_NICK_NAME_TEST_001
* @tc.desc: Is Device Need Sync Nick Name test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, IS_DEVICE_NEED_SYNC_NICK_NAME_TEST_001, TestSize.Level1)
{
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById)
        .WillOnce(Return(SOFTBUS_OK))
        .WillRepeatedly(Return(SOFTBUS_ERR));
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(serviceMock, IsFeatureSupport).WillRepeatedly(Return(false));
    bool ret = IsDeviceNeedSyncNickName(NETWORKID);
    EXPECT_FALSE(ret);
    ret = IsDeviceNeedSyncNickName(NETWORKID);
    EXPECT_FALSE(ret);
}

/*
* @tc.name: NOTIFY_NICK_NAME_CHANGE_TEST_001
* @tc.desc: NotifyNickNameChange test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, NOTIFY_NICK_NAME_CHANGE_TEST_001, TestSize.Level1)
{
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnGetAllOnlineNodeInfo)
        .WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(LnnNetLedgertInterfaceMock::ActionOfLnnGetAllOnline);
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById).WillOnce(Return(SOFTBUS_ERR));
    NotifyNickNameChange();
    NotifyNickNameChange();
    EXPECT_CALL(ledgerMock, LnnGetRemoteNodeInfoById).WillRepeatedly(Return(SOFTBUS_OK));
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(serviceMock, IsFeatureSupport).WillRepeatedly(Return(true));
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr));
    NotifyNickNameChange();
}

/*
* @tc.name: HANDLER_GET_DEVICE_NICK_NAME_TEST_001
* @tc.desc: HandlerGetDeviceNickName test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, HANDLER_GET_DEVICE_NICK_NAME_TEST_001, TestSize.Level1)
{
    NodeInfo localNodeInfo1 = { .deviceInfo.unifiedName = "", };
    NodeInfo localNodeInfo2 = { .deviceInfo.unifiedName = "nickName", };
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(nullptr));
    HandlerGetDeviceNickName(nullptr);
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(&localNodeInfo1));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDeviceName).WillOnce(Return(SOFTBUS_ERR));
    HandlerGetDeviceNickName(nullptr);
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(&localNodeInfo1));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDeviceName).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName).WillOnce(Return(SOFTBUS_ERR));
    HandlerGetDeviceNickName(nullptr);
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(&localNodeInfo1));
    const char *tmp = "";
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(tmp))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName).WillOnce(Return(SOFTBUS_ERR));
    HandlerGetDeviceNickName(nullptr);
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillOnce(Return(&localNodeInfo1));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(tmp))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName)
        .WillOnce(DoAll(SetArgPointee<2>(*(const_cast<char *>(tmp))), Return(SOFTBUS_OK)));
    EXPECT_CALL(ledgerMock, LnnGetAllOnlineNodeInfo).WillRepeatedly(Return(SOFTBUS_ERR));
    HandlerGetDeviceNickName(nullptr);
    const char *unifiedDefault = "unifiedDefault";
    const char *nickName = "nickName";
    EXPECT_CALL(ledgerMock, LnnGetLocalNodeInfo).WillRepeatedly(Return(&localNodeInfo2));
    EXPECT_CALL(serviceMock, LnnSetLocalUnifiedName).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(unifiedDefault))), Return(SOFTBUS_OK)));
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo)
        .WillOnce(Return(SOFTBUS_ERR))
        .WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName)
        .WillOnce(DoAll(SetArgPointee<2>(*(const_cast<char *>(nickName))), Return(SOFTBUS_OK)));
    HandlerGetDeviceNickName(nullptr);
    EXPECT_CALL(serviceMock, LnnSetLocalUnifiedName).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(unifiedDefault))), Return(SOFTBUS_OK)));
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo)
        .WillOnce(Return(SOFTBUS_OK))
        .WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName)
        .WillOnce(DoAll(SetArgPointee<2>(*(const_cast<char *>(nickName))), Return(SOFTBUS_OK)));
    HandlerGetDeviceNickName(nullptr);
}

/*
* @tc.name: UPDATA_LOCAL_FROM_SETTING_TEST_001
* @tc.desc: UpdataLocalFromSetting test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, UPDATA_LOCAL_FROM_SETTING_TEST_001, TestSize.Level1)
{
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    const char *deviceName = "deviceName";
    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo)
        .WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDeviceName).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, RegisterNameMonitor).WillRepeatedly(Return());
    NiceMock<LnnConnectInterfaceMock> connMock;
    EXPECT_CALL(connMock, DiscDeviceInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, LnnNotifyLocalNetworkIdChanged).WillRepeatedly(Return());
    UpdataLocalFromSetting(nullptr);

    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo)
        .WillOnce(Return(SOFTBUS_OK));
    const char *tmp = "";
    EXPECT_CALL(serviceMock, LnnGetUnifiedDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(tmp))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(tmp))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName)
        .WillOnce(DoAll(SetArgPointee<2>(*(const_cast<char *>(tmp))), Return(SOFTBUS_OK)));
    UpdataLocalFromSetting(nullptr);
}

/*
* @tc.name: UPDATA_LOCAL_FROM_SETTING_TEST_002
* @tc.desc: UpdataLocalFromSetting test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, UPDATA_LOCAL_FROM_SETTING_TEST_002, TestSize.Level1)
{
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    const char *deviceName = "deviceName";
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    const char *nickName = "nickName";
    EXPECT_CALL(serviceMock, RegisterNameMonitor).WillRepeatedly(Return());
    NiceMock<LnnConnectInterfaceMock> connMock;
    EXPECT_CALL(connMock, DiscDeviceInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, LnnNotifyLocalNetworkIdChanged).WillRepeatedly(Return());

    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName)
        .WillRepeatedly(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo)
        .WillOnce(Return(SOFTBUS_OK))
        .WillRepeatedly(Return(SOFTBUS_ERR));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDeviceName)
        .WillRepeatedly(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillRepeatedly(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName)
        .WillRepeatedly(DoAll(SetArgPointee<2>(*(const_cast<char *>(nickName))), Return(SOFTBUS_OK)));
    UpdataLocalFromSetting(nullptr);
}

/*
* @tc.name: UPDATA_LOCAL_FROM_SETTING_TEST_003
* @tc.desc: UpdataLocalFromSetting test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, UPDATA_LOCAL_FROM_SETTING_TEST_003, TestSize.Level1)
{
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    const char *deviceName = "deviceName";
    NiceMock<LnnNetLedgertInterfaceMock> ledgerMock;
    const char *nickName = "nickName";

    EXPECT_CALL(serviceMock, LnnGetSettingDeviceName)
        .WillOnce(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)))
        .WillRepeatedly(Return(SOFTBUS_ERR));
    EXPECT_CALL(ledgerMock, LnnSetLocalStrInfo)
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDeviceName)
        .WillRepeatedly(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetUnifiedDefaultDeviceName)
        .WillRepeatedly(DoAll(SetArgPointee<0>(*(const_cast<char *>(deviceName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, LnnGetSettingNickName)
        .WillRepeatedly(DoAll(SetArgPointee<2>(*(const_cast<char *>(nickName))), Return(SOFTBUS_OK)));
    EXPECT_CALL(serviceMock, RegisterNameMonitor).WillRepeatedly(Return());
    NiceMock<LnnConnectInterfaceMock> connMock;
    EXPECT_CALL(connMock, DiscDeviceInfoChanged).WillRepeatedly(Return());
    EXPECT_CALL(serviceMock, LnnNotifyLocalNetworkIdChanged).WillRepeatedly(Return());
    UpdataLocalFromSetting(nullptr);
    UpdataLocalFromSetting(nullptr);
}

/*
* @tc.name: LNN_INIT_DEVICE_NAME_TEST_001
* @tc.desc: UpdataLocalFromSetting test
* @tc.type: FUNC
* @tc.require:
*/
HWTEST_F(LNNDeviceNameInfoTest, LNN_INIT_DEVICE_NAME_TEST_001, TestSize.Level1)
{
    NiceMock<LnnSyncInfoInterfaceMock> lnnSyncInfoMock;
    EXPECT_CALL(lnnSyncInfoMock, LnnRegSyncInfoHandler).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    int32_t ret = LnnInitDevicename();
    EXPECT_EQ(ret, SOFTBUS_ERR);
    NiceMock<LnnServicetInterfaceMock> serviceMock;
    EXPECT_CALL(serviceMock, LnnRegisterEventHandler).WillOnce(Return(SOFTBUS_ERR))
        .WillRepeatedly(Return(SOFTBUS_OK));
    ret = LnnInitDevicename();
    EXPECT_EQ(ret, SOFTBUS_ERR);
    ret = LnnInitDevicename();
    EXPECT_EQ(ret, SOFTBUS_OK);
}
} // namespace OHOS
