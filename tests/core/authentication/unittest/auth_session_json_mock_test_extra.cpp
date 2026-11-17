/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cinttypes>
#include <gtest/gtest.h>
#include <securec.h>
#include <sys/time.h>

#include "auth_session_json.c"
#include "auth_session_json_deps_mock.h"
#include "softbus_error_code.h"

namespace OHOS {
using namespace testing;
using namespace testing::ext;
constexpr int32_t KEY_VALUE_LEN = 13;
constexpr uint8_t KEY_VALUE[SESSION_KEY_LENGTH] = "123456keytest";
constexpr int64_t TEST_AUTH_ID = 1;
constexpr int32_t TEST_BASIC_USER_ID = 100;

class AuthSessionJsonMockTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AuthSessionJsonMockTest::SetUpTestCase() { }

void AuthSessionJsonMockTest::TearDownTestCase() { }

void AuthSessionJsonMockTest::SetUp() { }

void AuthSessionJsonMockTest::TearDown() { }
/*
 * @tc.name: GET_UDID_SHORT_HASH_TEST_001
 * @tc.desc: test GetUdidShortHash for param check and all connInfo type branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, GET_UDID_SHORT_HASH_TEST_001, TestSize.Level1)
{
    char udidHash[SHA_256_HEX_HASH_LEN] = { 0 };
    EXPECT_FALSE(GetUdidShortHash(nullptr, udidHash, SHA_256_HEX_HASH_LEN));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    EXPECT_FALSE(GetUdidShortHash(&info, nullptr, SHA_256_HEX_HASH_LEN));
    EXPECT_FALSE(GetUdidShortHash(&info, udidHash, 0));

    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    EXPECT_CALL(mocker, SoftBusGenerateStrHash).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertBytesToHexString).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_EQ(strcpy_s(info.udid, UDID_BUF_LEN, "testUdid"), EOK);
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.connInfo.type = AUTH_LINK_TYPE_BR;
    EXPECT_CALL(mocker, LnnGetUdidByBrMac).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_FALSE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));
    EXPECT_CALL(mocker, LnnGetUdidByBrMac).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    info.connInfo.type = AUTH_LINK_TYPE_WIFI;
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));
    info.connInfo.type = AUTH_LINK_TYPE_SESSION_KEY;
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));
    info.connInfo.type = AUTH_LINK_TYPE_USB;
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    info.connInfo.type = AUTH_LINK_TYPE_BLE;
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    info.connInfo.type = AUTH_LINK_TYPE_SLE;
    EXPECT_FALSE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    info.connInfo.type = AUTH_LINK_TYPE_P2P;
    info.isServer = true;
    EXPECT_FALSE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));
    info.connInfo.type = AUTH_LINK_TYPE_ENHANCED_P2P;
    info.isServer = false;
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    info.connInfo.type = AUTH_LINK_TYPE_SESSION;
    EXPECT_TRUE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));

    info.connInfo.type = (AuthLinkType)-1;
    EXPECT_FALSE(GetUdidShortHash(&info, udidHash, SHA_256_HEX_HASH_LEN));
}

/*
 * @tc.name: GET_FAST_AUTH_KEY_TEST_001
 * @tc.desc: test GetFastAuthKey for enhanced p2p branch and normal branch
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, GET_FAST_AUTH_KEY_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    char udidHash[SHA_256_HEX_HASH_LEN] = { 0 };
    AuthSessionInfo info;
    AuthDeviceKeyInfo deviceKey;
    (void)memset_s(&deviceKey, sizeof(AuthDeviceKeyInfo), 0, sizeof(AuthDeviceKeyInfo));
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.connInfo.type = AUTH_LINK_TYPE_ENHANCED_P2P;
    EXPECT_CALL(mocker, AuthFindLatestNormalizeKey).WillRepeatedly(Return(SOFTBUS_AUTH_NOT_FOUND));
    EXPECT_CALL(mocker, AuthFindDeviceKey).WillRepeatedly(Return(SOFTBUS_AUTH_NOT_FOUND));
    EXPECT_EQ(GetFastAuthKey(udidHash, &info, &deviceKey), SOFTBUS_AUTH_NOT_FOUND);

    info.connInfo.type = AUTH_LINK_TYPE_WIFI;
    EXPECT_EQ(GetFastAuthKey(udidHash, &info, &deviceKey), SOFTBUS_AUTH_NOT_FOUND);
    EXPECT_FALSE(info.isSupportFastAuth);
    EXPECT_CALL(mocker, AuthFindDeviceKey).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_EQ(GetFastAuthKey(udidHash, &info, &deviceKey), SOFTBUS_OK);
}

/*
 * @tc.name: IS_CMD_MATCH_BY_DEVICE_ID_TEST_001
 * @tc.desc: test IsCmdMatchByDeviceId for cmd missing, mismatch and match branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, IS_CMD_MATCH_BY_DEVICE_ID_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.connInfo.type = AUTH_LINK_TYPE_BR;

    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    EXPECT_EQ(IsCmdMatchByDeviceId(&obj, &info), SOFTBUS_NOT_FIND);

    static const char cmdRet[] = "retAuthInfo";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(cmdRet, cmdRet + sizeof(cmdRet)), Return(true)));
    EXPECT_EQ(IsCmdMatchByDeviceId(&obj, &info), SOFTBUS_OK);

    info.connInfo.type = AUTH_LINK_TYPE_WIFI;
    info.isConnectServer = true;
    EXPECT_EQ(IsCmdMatchByDeviceId(&obj, &info), SOFTBUS_CMP_FAIL);

    static const char cmdGet[] = "getAuthInfo";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(cmdGet, cmdGet + sizeof(cmdGet)), Return(true)));
    EXPECT_CALL(mocker, GetFd).WillRepeatedly(Return(1));
    EXPECT_CALL(mocker, SoftBusSocketGetPeerName).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_EQ(IsCmdMatchByDeviceId(&obj, &info), SOFTBUS_OK);
}

/*
 * @tc.name: UN_PACK_VERSION_BY_DEVICE_ID_TEST_001
 * @tc.desc: test UnPackVersionByDeviceId for udid present and absent branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UN_PACK_VERSION_BY_DEVICE_ID_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    static const char peerUdid[] = "peerUdid";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(peerUdid, peerUdid + sizeof(peerUdid)), Return(true)));
    UnPackVersionByDeviceId(&obj, &info);
    EXPECT_EQ(info.version, SOFTBUS_OLD_V2);

    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    EXPECT_EQ(strcpy_s(info.uuid, UUID_BUF_LEN, "peerUuid"), EOK);
    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    UnPackVersionByDeviceId(&obj, &info);
    EXPECT_EQ(info.version, SOFTBUS_OLD_V1);
    EXPECT_STREQ(info.udid, "peerUuid");
}

/*
 * @tc.name: SET_COMPRESS_FLAG_TEST_001
 * @tc.desc: test SetCompressFlag for true, false and null node branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, SET_COMPRESS_FLAG_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    NodeInfo node;
    (void)memset_s(&node, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    bool flag = false;

    EXPECT_CALL(mocker, LnnGetLocalNodeInfo).WillRepeatedly(Return(nullptr));
    EXPECT_NO_FATAL_FAILURE(SetCompressFlag(TRUE_STRING_TAG, &flag));
    EXPECT_FALSE(flag);

    EXPECT_CALL(mocker, LnnGetLocalNodeInfo).WillRepeatedly(Return(&node));
    EXPECT_CALL(mocker, IsFeatureSupport).WillRepeatedly(Return(true));
    SetCompressFlag(TRUE_STRING_TAG, &flag);
    EXPECT_TRUE(flag);
    SetCompressFlag(FALSE_STRING_TAG, &flag);
    EXPECT_FALSE(flag);
}

/*
 * @tc.name: UN_PACK_AUTH_PRELINK_NODE_TEST_001
 * @tc.desc: test UnPackAuthPreLinkNode when prelink node flag is absent
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UN_PACK_AUTH_PRELINK_NODE_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.requestId = 1;
    EXPECT_CALL(mocker, JSON_GetBoolFromOject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(UnPackAuthPreLinkNode(&obj, &info));
}

/*
 * @tc.name: PACK_AUTH_PRELINK_NODE_TEST_001
 * @tc.desc: test PackAuthPreLinkNode when prelink node not found
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_AUTH_PRELINK_NODE_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    EXPECT_CALL(mocker, FindAuthPreLinkNodeById).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.requestId = 1;
    EXPECT_NO_FATAL_FAILURE(PackAuthPreLinkNode(&obj, &info));
}

/*
 * @tc.name: PACK_USER_ID_TEST_001
 * @tc.desc: test PackUserId for normal and negative userId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_USER_ID_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    PackUserId(&json, TEST_BASIC_USER_ID, &info);
    EXPECT_EQ(info.localUserId, TEST_BASIC_USER_ID);
    PackUserId(&json, -1, &info);
    EXPECT_EQ(info.localUserId, 0);
}

/*
 * @tc.name: PACK_COMPRESS_INFO_TEST_001
 * @tc.desc: test PackCompressInfo for null node and feature support branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_COMPRESS_INFO_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    EXPECT_NO_FATAL_FAILURE(PackCompressInfo(&json, nullptr));

    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, IsFeatureSupport).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_NO_FATAL_FAILURE(PackCompressInfo(&json, &info));
    EXPECT_NO_FATAL_FAILURE(PackCompressInfo(&json, &info));
}

/*
 * @tc.name: PACK_NET_CAPACITY_BY_VERSION_TEST_001
 * @tc.desc: test PackNetCapacityByVersion for old and new versions
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_NET_CAPACITY_BY_VERSION_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    info.netCapacity = 0x1FF;
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    EXPECT_EQ(PackNetCapacityByVersion(&json, &info, SOFTBUS_OLD_V1), SOFTBUS_OK);
    EXPECT_EQ(PackNetCapacityByVersion(&json, &info, SOFTBUS_OLD_V2), SOFTBUS_OK);
    EXPECT_EQ(PackNetCapacityByVersion(&json, &info, SOFTBUS_NEW_V1), SOFTBUS_OK);
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(false));
    EXPECT_EQ(PackNetCapacityByVersion(&json, &info, SOFTBUS_NEW_V1), SOFTBUS_AUTH_PACK_DEVINFO_FAIL);
}

/*
 * @tc.name: GET_BT_DISC_TYPE_STRING_TEST_001
 * @tc.desc: test GetBtDiscTypeString for ble, br and buffer insufficient branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, GET_BT_DISC_TYPE_STRING_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    char buf[BT_DISC_TYPE_MAX_LEN] = { 0 };
    EXPECT_CALL(mocker, LnnHasDiscoveryType).WillRepeatedly(Return(false));
    EXPECT_EQ(GetBtDiscTypeString(&info, buf, BT_DISC_TYPE_MAX_LEN), SOFTBUS_OK);

    EXPECT_CALL(mocker, LnnHasDiscoveryType)
        .WillOnce(Return(true)).WillOnce(Return(false));
    (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
    EXPECT_EQ(GetBtDiscTypeString(&info, buf, BT_DISC_TYPE_MAX_LEN), SOFTBUS_OK);
    EXPECT_EQ(strlen(buf), 1);

    EXPECT_CALL(mocker, LnnHasDiscoveryType)
        .WillOnce(Return(true)).WillOnce(Return(true));
    (void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
    EXPECT_EQ(GetBtDiscTypeString(&info, buf, BT_DISC_TYPE_MAX_LEN), SOFTBUS_OK);
    EXPECT_EQ(strlen(buf), 3);

    EXPECT_CALL(mocker, LnnHasDiscoveryType)
        .WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_EQ(GetBtDiscTypeString(&info, buf, 1), SOFTBUS_INVALID_NUM);
}

/*
 * @tc.name: SET_DISC_TYPE_ADD_DISCOVERY_TYPE_TEST_001
 * @tc.desc: test SetDiscType and AddDiscoveryType branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, SET_DISC_TYPE_ADD_DISCOVERY_TYPE_TEST_001, TestSize.Level1)
{
    uint32_t discType = 0;
    EXPECT_EQ(SetDiscType(&discType, DEFAULT_BT_DISC_TYPE_STR), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(SetDiscType(&discType, "1"), SOFTBUS_OK);

    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    EXPECT_NO_FATAL_FAILURE(AddDiscoveryType(&json, nullptr));

    static const char networkId[] = "networkId";
    EXPECT_CALL(mocker, LnnGetNetworkIdByUuid)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(DoAll(SetArrayArgument<1>(networkId, networkId + sizeof(networkId)), Return(SOFTBUS_OK)));
    EXPECT_NO_FATAL_FAILURE(AddDiscoveryType(&json, "remoteUuid"));

    EXPECT_CALL(mocker, LnnGetRemoteNumInfo).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(AddDiscoveryType(&json, "remoteUuid"));

    EXPECT_CALL(mocker, LnnGetRemoteNumInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnHasDiscoveryType).WillRepeatedly(Return(false));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_NO_FATAL_FAILURE(AddDiscoveryType(&json, "remoteUuid"));
}

/*
 * @tc.name: PACK_OS_DEVICE_VERSION_COMM_P2P_TEST_001
 * @tc.desc: test PackOsInfo, PackDeviceVersion, PackCommP2pInfo, PackCommonFastAuth, PackServiceFindCap
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_OS_DEVICE_VERSION_COMM_P2P_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, LnnGetP2pRole).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, LnnGetP2pMac).WillRepeatedly(Return(info.connectInfo.macAddr));
    EXPECT_CALL(mocker, LnnGetStaFrequency).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, GetExtData).WillOnce(Return(SOFTBUS_INVALID_PARAM)).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(PackOsInfo(&json, &info));
    EXPECT_NO_FATAL_FAILURE(PackDeviceVersion(&json, &info));
    EXPECT_NO_FATAL_FAILURE(PackCommP2pInfo(&json, &info));
    EXPECT_NO_FATAL_FAILURE(PackCommonFastAuth(&json, &info));
    EXPECT_NO_FATAL_FAILURE(PackServiceFindCap(&json, &info));
}

/*
 * @tc.name: PACK_DEVICE_INFO_MAC_BASE_TEST_001
 * @tc.desc: test PackDeviceInfoMac and PackDeviceBaseInfo for success and add-fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_DEVICE_INFO_MAC_BASE_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, LnnGetBtMac).WillRepeatedly(Return(info.connectInfo.macAddr));
    EXPECT_CALL(mocker, LnnGetP2pMac).WillRepeatedly(Return(info.p2pInfo.p2pMac));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt64ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddBoolToObject).WillRepeatedly(Return(true));
    EXPECT_EQ(PackDeviceInfoMac(&json, &info, false), SOFTBUS_OK);
    EXPECT_EQ(PackDeviceBaseInfo(&json, &info, false), SOFTBUS_OK);

    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(false));
    EXPECT_EQ(PackDeviceInfoMac(&json, &info, false), SOFTBUS_AUTH_REG_DATA_FAIL);
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(false));
    EXPECT_CALL(mocker, JSON_AddInt64ToObject).WillRepeatedly(Return(false));
    EXPECT_CALL(mocker, JSON_AddBoolToObject).WillRepeatedly(Return(false));
    EXPECT_EQ(PackDeviceBaseInfo(&json, &info, false), SOFTBUS_AUTH_PACK_DEVINFO_FAIL);
}

/*
 * @tc.name: PACK_WIFIDIRECT_INFO_TEST_001
 * @tc.desc: test PackWifiDirectInfo for meta auth, uuid branch and ptk get fail
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_WIFIDIRECT_INFO_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    AuthConnInfo connInfo;
    (void)memset_s(&connInfo, sizeof(AuthConnInfo), 0, sizeof(AuthConnInfo));
    EXPECT_CALL(mocker, AuthMetaGetConnIdByInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnGetMetaPtk)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnGetLocalPtkByUuid)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, SoftBusBase64Encode).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertBytesToHexString).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));

    EXPECT_NO_FATAL_FAILURE(PackWifiDirectInfo(&connInfo, &json, &info, nullptr, true));
    EXPECT_NO_FATAL_FAILURE(PackWifiDirectInfo(&connInfo, &json, &info, nullptr, true));
    EXPECT_NO_FATAL_FAILURE(PackWifiDirectInfo(&connInfo, &json, &info, nullptr, false));
    EXPECT_NO_FATAL_FAILURE(PackWifiDirectInfo(&connInfo, &json, &info, nullptr, false));

    EXPECT_CALL(mocker, SoftBusBase64Encode).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(PackWifiDirectInfo(&connInfo, &json, &info, "remoteUuid", false));
}

/*
 * @tc.name: UPDATE_BROADCAST_CIPHER_KEY_TEST_001
 * @tc.desc: test UpdateBroadcastCipherKey for success and update fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UPDATE_BROADCAST_CIPHER_KEY_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, LnnUpdateLocalBroadcastCipherKey)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_EQ(UpdateBroadcastCipherKey(&info), SOFTBUS_AUTH_CIPHER_KEY_PROC_ERR);
    EXPECT_EQ(UpdateBroadcastCipherKey(&info), SOFTBUS_OK);
}

/*
 * @tc.name: PARSE_COMMON_JSON_INFO_TEST_001
 * @tc.desc: test ParseDeviceName, ParseCommonJsonInfo and ParseCommonJsonOptInfo default values
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PARSE_COMMON_JSON_INFO_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    static const char deviceName[] = "myDevice";
    EXPECT_CALL(mocker, JSON_GetStringFromObject(_, _, _, _)).WillRepeatedly(Return(false));
    EXPECT_CALL(mocker, JSON_GetStringFromObject(_, StrEq(DEVICE_NAME), _, _))
        .WillRepeatedly(DoAll(SetArrayArgument<2>(deviceName, deviceName + sizeof(deviceName)), Return(true)));
    ParseDeviceName(&json, &info);
    EXPECT_STREQ(info.deviceInfo.deviceName, "myDevice");

    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    ParseCommonJsonInfo(&json, &info, false);
    EXPECT_EQ(info.staticNetCap, DEFAULT_STATIC_NET_CAP);
    EXPECT_EQ(info.masterWeight, DEFAULT_NODE_WEIGHT);
    EXPECT_EQ(info.wifiBuffSize, DEFAULT_BR_BUFF_SIZE);
    EXPECT_STREQ(info.connectInfo.macAddr, "");

    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    info.authCapacity = 1;
    ParseCommonJsonOptInfo(&json, &info);
    EXPECT_EQ(info.deviceInfo.osType, OH_OS_TYPE);
}

/*
 * @tc.name: UNPACK_DEVICE_INFO_BT_V1_TEST_001
 * @tc.desc: test UnpackDeviceInfoBtV1 for field missing and all-fields-present branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_DEVICE_INFO_BT_V1_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    EXPECT_EQ(UnpackDeviceInfoBtV1(&json, &info), SOFTBUS_AUTH_UNPACK_DEVINFO_FAIL);

    static const char testStr[] = "testValue";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(testStr, testStr + sizeof(testStr)), Return(true)));
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_EQ(UnpackDeviceInfoBtV1(&json, &info), SOFTBUS_OK);
    EXPECT_STREQ(info.deviceInfo.deviceName, "testValue");
    EXPECT_STREQ(info.uuid, "testValue");
    EXPECT_STREQ(info.networkId, "testValue");
}

/*
 * @tc.name: IS_INVALID_DEVICE_INFO_TEST_001
 * @tc.desc: test IsInvalidDeviceInfo for param, udid mismatch, version change branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, IS_INVALID_DEVICE_INFO_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    NodeInfo nodeInfo;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    (void)memset_s(&nodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_FALSE(IsInvalidDeviceInfo(nullptr, &nodeInfo, &info));
    EXPECT_FALSE(IsInvalidDeviceInfo(&json, nullptr, &info));
    EXPECT_FALSE(IsInvalidDeviceInfo(&json, &nodeInfo, nullptr));

    EXPECT_EQ(strcpy_s(info.udid, UDID_BUF_LEN, "peerUdid"), EOK);
    EXPECT_TRUE(IsInvalidDeviceInfo(&json, &nodeInfo, &info));

    EXPECT_EQ(strcpy_s(nodeInfo.deviceInfo.deviceUdid, UDID_BUF_LEN, "peerUdid"), EOK);
    EXPECT_CALL(mocker, JSON_GetInt32FromOject).WillRepeatedly(Return(false));
    EXPECT_FALSE(IsInvalidDeviceInfo(&json, &nodeInfo, &info));

    info.authVersion = AUTH_VERSION_V2;
    EXPECT_CALL(mocker, JSON_GetInt32FromOject)
        .WillRepeatedly(DoAll(SetArgPointee<2>((int32_t)AUTH_VERSION_V2), Return(true)));
    EXPECT_CALL(mocker, SoftBusGenerateStrHash).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_FALSE(IsInvalidDeviceInfo(&json, &nodeInfo, &info));

    static const char versionStr[] = "1";
    (void)memcpy_s(info.udidShortHash, sizeof(info.udidShortHash), versionStr, sizeof(versionStr));
    EXPECT_CALL(mocker, JSON_GetInt32FromOject)
        .WillOnce(DoAll(SetArgPointee<2>((int32_t)AUTH_VERSION_V1), Return(true)));
    EXPECT_TRUE(IsInvalidDeviceInfo(&json, &nodeInfo, &info));
}

/*
 * @tc.name: UNPACK_USERID_CHECKSUM_TEST_001
 * @tc.desc: test UnpackUserIdCheckSum for field missing, convert fail and success branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_USERID_CHECKSUM_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo nodeInfo;
    (void)memset_s(&nodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(UnpackUserIdCheckSum(&json, &nodeInfo));

    static const char checkSum[] = "12345678";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(checkSum, checkSum + sizeof(checkSum)), Return(true)));
    EXPECT_CALL(mocker, ConvertHexStringToBytes).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UnpackUserIdCheckSum(&json, &nodeInfo));
    EXPECT_CALL(mocker, ConvertHexStringToBytes).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(UnpackUserIdCheckSum(&json, &nodeInfo));
}

/*
 * @tc.name: UPDATE_PEER_DEVICE_NAME_TEST_001
 * @tc.desc: test UpdatePeerDeviceName for local null, unified name, nickname and same account branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UPDATE_PEER_DEVICE_NAME_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    NodeInfo peerNodeInfo;
    NodeInfo localInfo;
    (void)memset_s(&peerNodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    (void)memset_s(&localInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, LnnGetLocalNodeInfo).WillRepeatedly(Return(nullptr));
    EXPECT_NO_FATAL_FAILURE(UpdatePeerDeviceName(&peerNodeInfo));

    EXPECT_CALL(mocker, LnnGetLocalNodeInfo).WillRepeatedly(Return(&localInfo));
    EXPECT_EQ(strcpy_s(peerNodeInfo.deviceInfo.unifiedName, DEVICE_NAME_BUF_LEN, "unified"), EOK);
    EXPECT_EQ(strcpy_s(peerNodeInfo.deviceInfo.unifiedDefaultName, DEVICE_NAME_BUF_LEN, "default"), EOK);
    EXPECT_NO_FATAL_FAILURE(UpdatePeerDeviceName(&peerNodeInfo));
    EXPECT_STREQ(peerNodeInfo.deviceInfo.deviceName, "unified");

    (void)memset_s(&peerNodeInfo.deviceInfo, sizeof(peerNodeInfo.deviceInfo), 0, sizeof(peerNodeInfo.deviceInfo));
    EXPECT_EQ(strcpy_s(peerNodeInfo.deviceInfo.unifiedDefaultName, DEVICE_NAME_BUF_LEN, "default"), EOK);
    EXPECT_NO_FATAL_FAILURE(UpdatePeerDeviceName(&peerNodeInfo));
    EXPECT_STREQ(peerNodeInfo.deviceInfo.deviceName, "default");

    (void)memset_s(&peerNodeInfo.deviceInfo, sizeof(peerNodeInfo.deviceInfo), 0, sizeof(peerNodeInfo.deviceInfo));
    EXPECT_EQ(strcpy_s(peerNodeInfo.deviceInfo.unifiedDefaultName, DEVICE_NAME_BUF_LEN, "default"), EOK);
    EXPECT_EQ(strcpy_s(peerNodeInfo.deviceInfo.nickName, DEVICE_NAME_BUF_LEN, "nick"), EOK);
    peerNodeInfo.accountId = 100;
    localInfo.accountId = 200;
    EXPECT_CALL(mocker, LnnGetDeviceDisplayName).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(UpdatePeerDeviceName(&peerNodeInfo));
}

/*
 * @tc.name: UPDATE_LOCAL_NET_BR_MAC_TEST_001
 * @tc.desc: test UpdateLocalNetBrMac for get fail, valid mac, bt state and mac addr fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UPDATE_LOCAL_NET_BR_MAC_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    static char btMac[BT_MAC_LEN] = "11:22:33:44:55:66";
    static char invalidMac[BT_MAC_LEN] = "00:00:00:00:00:00";

    EXPECT_CALL(mocker, LnnGetLocalNodeInfoSafe).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());

    EXPECT_CALL(mocker, LnnGetLocalNodeInfoSafe).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnGetBtMac).WillOnce(Return(btMac)).WillRepeatedly(Return(invalidMac));
    EXPECT_CALL(mocker, SoftBusGetBtState).WillRepeatedly(Return(BLE_ENABLE));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());

    EXPECT_CALL(mocker, SoftBusGetBtState).WillRepeatedly(Return(BLE_DISABLE));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());

    EXPECT_CALL(mocker, SoftBusGetBtState).WillRepeatedly(Return(BLE_ENABLE));
    EXPECT_CALL(mocker, SoftBusGetBtMacAddr).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());

    EXPECT_CALL(mocker, SoftBusGetBtMacAddr).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertBtMacToStr)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(DoAll(SetArrayArgument<0>(btMac, btMac + strlen(btMac) + 1), Return(SOFTBUS_OK)));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());

    EXPECT_CALL(mocker, LnnSetLocalStrInfo).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());
    EXPECT_CALL(mocker, LnnSetLocalStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(UpdateLocalNetBrMac());
}

/*
 * @tc.name: PACK_CRED_TYPE_INFO_TEST_001
 * @tc.desc: test PackCredTypeInfo for success branch
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_CRED_TYPE_INFO_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    EXPECT_CALL(mocker, AddNumberToJsonObject).WillRepeatedly(Return(true));
    cJSON *json = cJSON_CreateObject();
    ASSERT_NE(json, nullptr);
    EXPECT_TRUE(PackCredTypeInfo(json, TEST_BASIC_USER_ID, ACCOUNT_SHARED, TEST_BASIC_USER_ID + 1));
    cJSON_Delete(json);
}

/*
 * @tc.name: PACK_CRED_NEGO_ALL_USERIDS_TEST_001
 * @tc.desc: test PackCredNegoAllUserIds for get fail, single user and multi user branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_CRED_NEGO_ALL_USERIDS_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    EXPECT_CALL(mocker, GetAllForegroundAccountIds).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_TRUE(PackCredNegoAllUserIds(&json));

    int32_t *userIds = static_cast<int32_t *>(SoftBusCalloc(sizeof(int32_t) * DEFAULT_MAIN_USERID_NUMS));
    ASSERT_NE(userIds, nullptr);
    userIds[0] = TEST_BASIC_USER_ID;
    EXPECT_CALL(mocker, GetAllForegroundAccountIds)
        .WillOnce(DoAll(SetArgPointee<0>(userIds),
            SetArgPointee<1>(static_cast<uint32_t>(DEFAULT_MAIN_USERID_NUMS)),
        Return(SOFTBUS_OK)));
    EXPECT_TRUE(PackCredNegoAllUserIds(&json));

    int32_t *multiUserIds = static_cast<int32_t *>(SoftBusCalloc(sizeof(int32_t) * 2));
    ASSERT_NE(multiUserIds, nullptr);
    multiUserIds[0] = TEST_BASIC_USER_ID;
    multiUserIds[1] = TEST_BASIC_USER_ID + 1;
    EXPECT_CALL(mocker, GetAllForegroundAccountIds)
        .WillOnce(DoAll(SetArgPointee<0>(multiUserIds), SetArgPointee<1>(static_cast<uint32_t>(2)),
            Return(SOFTBUS_OK)));
    EXPECT_CALL(mocker, cJSON_CreateArray).WillRepeatedly(Return(nullptr));
    EXPECT_FALSE(PackCredNegoAllUserIds(&json));
}

/*
 * @tc.name: GET_AND_SET_LOCAL_UNIFIED_NAME_TEST_001
 * @tc.desc: test GetAndSetLocalUnifiedName for get fail, empty name and set success branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, GET_AND_SET_LOCAL_UNIFIED_NAME_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    static const char unified[] = "unifiedName";

    EXPECT_CALL(mocker, LnnGetUnifiedDeviceName).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_NO_FATAL_FAILURE(GetAndSetLocalUnifiedName(&json));

    EXPECT_CALL(mocker, LnnGetUnifiedDeviceName).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(GetAndSetLocalUnifiedName(&json));

    EXPECT_CALL(mocker, LnnGetUnifiedDeviceName)
        .WillRepeatedly(DoAll(SetArrayArgument<0>(unified, unified + sizeof(unified)), Return(SOFTBUS_OK)));
    EXPECT_CALL(mocker, LnnSetLocalStrInfo)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(GetAndSetLocalUnifiedName(&json));
    EXPECT_NO_FATAL_FAILURE(GetAndSetLocalUnifiedName(&json));
}

/*
 * @tc.name: UNPACK_NORMALIZED_KEY_TEST_001
 * @tc.desc: test UnpackNormalizedKey for not support and no normalized data branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_NORMALIZED_KEY_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;

    UnpackNormalizedKey(&obj, &info, NORMALIZED_NOT_SUPPORT, authSeq);
    EXPECT_EQ(info.normalizedType, NORMALIZED_NOT_SUPPORT);

    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    UnpackNormalizedKey(&obj, &info, true, authSeq);
    EXPECT_EQ(info.normalizedType, NORMALIZED_KEY_ERROR);

    static const char encKey[] = "abcd";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(encKey, encKey + sizeof(encKey)), Return(true)));
    info.isServer = false;
    SessionKey sessionKey;
    (void)memset_s(&sessionKey, sizeof(SessionKey), 0, sizeof(SessionKey));
    info.normalizedKey = &sessionKey;
    EXPECT_CALL(mocker, LnnDecryptAesGcm).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UnpackNormalizedKey(&obj, &info, true, authSeq));
    EXPECT_EQ(info.normalizedType, NORMALIZED_SUPPORT);
    info.normalizedKey = nullptr;
}

/*
 * @tc.name: PACK_NORMALIZED_KEY_TEST_001
 * @tc.desc: test PackNormalizedKey for force auth, key error and nego state branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_NORMALIZED_KEY_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;

    info.isNeedFastAuth = false;
    info.isServer = false;
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKey(&obj, &info, authSeq));

    info.isServer = true;
    info.normalizedType = NORMALIZED_KEY_ERROR;
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKey(&obj, &info, authSeq));

    info.normalizedType = NORMALIZED_SUPPORT;
    info.localState = AUTH_STATE_WAIT;
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKey(&obj, &info, authSeq));

    info.localState = AUTH_STATE_START;
    EXPECT_CALL(mocker, SoftBusGenerateStrHash).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKey(&obj, &info, authSeq));
}

/*
 * @tc.name: UNPACK_EXTERNAL_AUTH_INFO_TEST_001
 * @tc.desc: test UnpackExternalAuthInfo for lower version, field missing and full fields branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_EXTERNAL_AUTH_INFO_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;

    info.authVersion = AUTH_VERSION_V1;
    EXPECT_NO_FATAL_FAILURE(UnpackExternalAuthInfo(&obj, &info, authSeq));
    EXPECT_EQ(info.userId, 0);

    info.authVersion = AUTH_VERSION_V2;
    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(UnpackExternalAuthInfo(&obj, &info, authSeq));

    static const char hashStr[] = "hashValue";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(hashStr, hashStr + sizeof(hashStr)), Return(true)));
    EXPECT_CALL(mocker, LnnGetLocalByteInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertHexStringToBytes).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UnpackExternalAuthInfo(&obj, &info, authSeq));
}

/*
 * @tc.name: UNPACK_WIFI_TEST_001
 * @tc.desc: test UnpackWiFi for bus version check fail and normal branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_WIFI_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, JSON_GetInt32FromOject).WillRepeatedly(Return(false));
    EXPECT_EQ(UnpackWiFi(&json, &info, SOFTBUS_NEW_V1, false, WLAN_IF), SOFTBUS_AUTH_UNPACK_DEVINFO_FAIL);

    EXPECT_CALL(mocker, JSON_GetInt32FromOject)
        .WillOnce(DoAll(SetArgPointee<2>((int32_t)BUS_V2), Return(true)))
        .WillOnce(DoAll(SetArgPointee<2>((int32_t)BUS_V1), Return(true)))
        .WillRepeatedly(Return(false));
    EXPECT_EQ(UnpackWiFi(&json, &info, SOFTBUS_NEW_V1, false, WLAN_IF), SOFTBUS_OK);
}

/*
 * @tc.name: UNPACK_BT_TEST_001
 * @tc.desc: test UnpackBt with default optional values
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_BT_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, JSON_GetInt64FromOject).WillRepeatedly(Return(false));
    EXPECT_EQ(UnpackBt(&json, &info, SOFTBUS_NEW_V1, false), SOFTBUS_OK);
    EXPECT_EQ(info.supportedProtocols, LNN_PROTOCOL_BR | LNN_PROTOCOL_BLE);
    EXPECT_EQ(info.bleConnCloseDelayTime, BLE_CONNECTION_CLOSE_DELAY);
    EXPECT_EQ(info.bleMacRefreshSwitch, BLE_MAC_AUTO_REFRESH_SWITCH);
}

/*
 * @tc.name: UNPACK_DEVICE_ID_JSON_TEST_001
 * @tc.desc: test UnpackDeviceIdJson for param check, parse fail, cmd mismatch and full unpack path
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_DEVICE_ID_JSON_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;

    EXPECT_EQ(UnpackDeviceIdJson(nullptr, 0, &info, authSeq), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(UnpackDeviceIdJson("{}", 2, nullptr, authSeq), SOFTBUS_INVALID_PARAM);

    EXPECT_CALL(mocker, JSON_Parse).WillRepeatedly(Return(nullptr));
    EXPECT_EQ(UnpackDeviceIdJson("{}", 2, &info, authSeq), SOFTBUS_INVALID_PARAM);

    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    EXPECT_CALL(mocker, JSON_Parse).WillRepeatedly(Return(&obj));
    static const char cmdRet[] = "retAuthInfo";
    info.connInfo.type = AUTH_LINK_TYPE_BR;
    ON_CALL(mocker, JSON_GetStringFromObject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, char *value, uint32_t size) -> bool {
            if (strcmp(key, CMD_TAG) == 0) {
                return strcpy_s(value, size, cmdRet) == EOK;
            }
            if (strcmp(key, DATA_TAG) == 0 || strcmp(key, DEVICE_ID_TAG) == 0) {
                return strcpy_s(value, size, "fieldValue") == EOK;
            }
            return false;
        }));
    EXPECT_EQ(UnpackDeviceIdJson("{}", 2, &info, authSeq), SOFTBUS_OK);
}

/*
 * @tc.name: UNPACK_DEVICE_ID_JSON_TEST_003
 * @tc.desc: test UnpackDeviceIdJson for full unpack path and device id missing branch
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_DEVICE_ID_JSON_TEST_003, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    static const char cmdRet[] = "retAuthInfo";
    static const char uuid[] = "uuidValue";
    static const char udid[] = "udidValue";
    info.connInfo.type = AUTH_LINK_TYPE_BR;

    ON_CALL(mocker, JSON_Parse).WillByDefault(Return(&obj));
    ON_CALL(mocker, JSON_GetStringFromObject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, char *value, uint32_t size) -> bool {
            if (strcmp(key, CMD_TAG) == 0) {
                return strcpy_s(value, size, cmdRet) == EOK;
            }
            if (strcmp(key, DATA_TAG) == 0) {
                return strcpy_s(value, size, uuid) == EOK;
            }
            if (strcmp(key, DEVICE_ID_TAG) == 0) {
                return strcpy_s(value, size, udid) == EOK;
            }
            return false;
        }));
    EXPECT_EQ(UnpackDeviceIdJson("{}", 2, &info, authSeq), SOFTBUS_OK);
    EXPECT_STREQ(info.uuid, uuid);
    EXPECT_STREQ(info.udid, udid);

    static const char cmdGet[] = "getAuthInfo";
    ON_CALL(mocker, JSON_GetStringFromObject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, char *value, uint32_t size) -> bool {
            if (strcmp(key, CMD_TAG) == 0) {
                return strcpy_s(value, size, cmdGet) == EOK;
            }
            return false;
        }));
    info.isConnectServer = true;
    info.connInfo.type = AUTH_LINK_TYPE_WIFI;
    EXPECT_EQ(UnpackDeviceIdJson("{}", 2, &info, authSeq), SOFTBUS_AUTH_UNPACK_DEV_ID_FAIL);
}

/*
 * @tc.name: UNPACK_DEVICE_ID_JSON_TEST_002
 * @tc.desc: test UnpackDeviceIdJson with compress flag and prelink node branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_DEVICE_ID_JSON_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;
    info.connInfo.type = AUTH_LINK_TYPE_BR;

    static const char cmdRet[] = "retAuthInfo";
    static const char uuid[] = "uuidValue";
    static const char udid[] = "udidValue";
    ON_CALL(mocker, JSON_Parse).WillByDefault(Return(&obj));
    ON_CALL(mocker, JSON_GetStringFromObject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, char *value, uint32_t size) -> bool {
            if (strcmp(key, CMD_TAG) == 0) {
                return strcpy_s(value, size, cmdRet) == EOK;
            }
            if (strcmp(key, DATA_TAG) == 0) {
                return strcpy_s(value, size, uuid) == EOK;
            }
            if (strcmp(key, DEVICE_ID_TAG) == 0) {
                return strcpy_s(value, size, udid) == EOK;
            }
            return false;
        }));
    NodeInfo nodeInfo;
    (void)memset_s(&nodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    ON_CALL(mocker, LnnGetLocalNodeInfo).WillByDefault(Return(&nodeInfo));
    ON_CALL(mocker, JSON_GetBoolFromOject).WillByDefault(Return(true));
    ON_CALL(mocker, IsFeatureSupport).WillByDefault(Return(false));
    EXPECT_EQ(UnpackDeviceIdJson("{}", 2, &info, authSeq), SOFTBUS_OK);
    EXPECT_EQ(info.version, SOFTBUS_OLD_V2);
}

/*
 * @tc.name: UNPACK_DEVICE_INFO_MESSAGE_TEST_001
 * @tc.desc: test UnpackDeviceInfoMessage for param check, parse fail, wifi/bt branches and trust verify
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_DEVICE_INFO_MESSAGE_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    NodeInfo nodeInfo;
    DevInfoData devInfo;
    (void)memset_s(&nodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    (void)memset_s(&devInfo, sizeof(DevInfoData), 0, sizeof(DevInfoData));
    static const char msg[] = "{}";
    devInfo.msg = msg;
    devInfo.len = strlen(msg);

    EXPECT_EQ(UnpackDeviceInfoMessage(nullptr, &nodeInfo, false, nullptr), SOFTBUS_INVALID_PARAM);
    EXPECT_EQ(UnpackDeviceInfoMessage(&devInfo, nullptr, false, nullptr), SOFTBUS_INVALID_PARAM);

    EXPECT_CALL(mocker, JSON_Parse).WillRepeatedly(Return(nullptr));
    EXPECT_EQ(UnpackDeviceInfoMessage(&devInfo, &nodeInfo, false, nullptr), SOFTBUS_PARSE_JSON_ERR);

    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    EXPECT_CALL(mocker, JSON_Parse).WillRepeatedly(Return(&obj));
    devInfo.linkType = AUTH_LINK_TYPE_WIFI;
    devInfo.version = SOFTBUS_NEW_V1;
    EXPECT_CALL(mocker, JSON_GetInt32FromOject).WillRepeatedly(Return(false));
    EXPECT_CALL(mocker, SoftBusGetSysTimeMs).WillRepeatedly(Return(1));
    NodeInfo localNode;
    (void)memset_s(&localNode, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, LnnGetLocalNodeInfo).WillRepeatedly(Return(&localNode));
    EXPECT_CALL(mocker, LnnGetLocalNumInfo).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_CALL(mocker, IsSupportUDIDAbatement).WillRepeatedly(Return(false));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    EXPECT_EQ(strcpy_s(info.udid, UDID_BUF_LEN, "testValue"), EOK);
    EXPECT_EQ(UnpackDeviceInfoMessage(&devInfo, &nodeInfo, false, &info), SOFTBUS_AUTH_UNPACK_DEVINFO_FAIL);

    static const char testStr[] = "testValue";
    ON_CALL(mocker, JSON_GetStringFromObject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, char *value, uint32_t size) -> bool {
            (void)key;
            return strcpy_s(value, size, testStr) == EOK;
        }));
    (void)memset_s(&devInfo, sizeof(DevInfoData), 0, sizeof(DevInfoData));
    devInfo.msg = msg;
    devInfo.len = strlen(msg);
    devInfo.linkType = AUTH_LINK_TYPE_BR;
    devInfo.version = SOFTBUS_OLD_V1;
    EXPECT_EQ(UnpackDeviceInfoMessage(&devInfo, &nodeInfo, false, &info), SOFTBUS_OK);
    EXPECT_STREQ(nodeInfo.networkId, testStr);
}

static void SetupPackDeviceIdJsonDefaultMocks(NiceMock<AuthSessionJsonDepsInterfaceMock> &mocker)
{
    static const char field[] = "fieldValue";
    ON_CALL(mocker, LnnGetLocalStrInfo).WillByDefault(Invoke(
        [](InfoKey, char *val, uint32_t size) -> int32_t {
            return strcpy_s(val, size, field) == EOK ? SOFTBUS_OK : SOFTBUS_INVALID_PARAM;
        }));
    ON_CALL(mocker, LnnGetLocalStrInfoByIfnameIdx).WillByDefault(Invoke(
        [](InfoKey, char *val, uint32_t size, int32_t) -> int32_t {
            return strcpy_s(val, size, field) == EOK ? SOFTBUS_OK : SOFTBUS_INVALID_PARAM;
        }));
    ON_CALL(mocker, LnnGetLocalByteInfo).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(mocker, SoftBusGenerateStrHash).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(mocker, ConvertBytesToHexString).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(mocker, ConvertBytesToUpperCaseHexString).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(mocker, IsPotentialTrustedDevice).WillByDefault(Return(true));
    ON_CALL(mocker, AuthFindDeviceKey).WillByDefault(Return(SOFTBUS_OK));
    ON_CALL(mocker, LnnEncryptAesGcm).WillByDefault(Invoke(
        [](AesGcmInputParam *, int32_t, uint8_t **out, uint32_t *outLen) -> int32_t {
            *out = static_cast<uint8_t *>(SoftBusCalloc(1));
            *outLen = 1;
            return *out != nullptr ? SOFTBUS_OK : SOFTBUS_MEM_ERR;
        }));
    ON_CALL(mocker, IsSupportFeatureByCapaBit).WillByDefault(Return(false));
    ON_CALL(mocker, IsSupportUDIDAbatement).WillByDefault(Return(false));
    ON_CALL(mocker, LnnGetLocalNodeInfo)
        .WillByDefault(Return(static_cast<NodeInfo *>(SoftBusCalloc(sizeof(NodeInfo)))));
    ON_CALL(mocker, JudgeDeviceTypeAndGetOsAccountIds).WillByDefault(Return(TEST_BASIC_USER_ID));
    ON_CALL(mocker, JSON_AddStringToObject).WillByDefault(Return(true));
    ON_CALL(mocker, JSON_AddInt32ToObject).WillByDefault(Return(true));
    ON_CALL(mocker, JSON_AddBoolToObject).WillByDefault(Return(true));
    ON_CALL(mocker, GetAllForegroundAccountIds).WillByDefault(Return(SOFTBUS_INVALID_PARAM));
    ON_CALL(mocker, AddNumberToJsonObject).WillByDefault(Return(true));
    ON_CALL(mocker, cJSON_PrintUnformatted).WillByDefault(Invoke([](const cJSON *) -> char* {
        static char msg[] = "{}";
        return msg;
    }));
    static JsonObj *jsonObj = nullptr;
    if (jsonObj == nullptr) {
        jsonObj = static_cast<JsonObj *>(SoftBusCalloc(sizeof(JsonObj)));
    }
    ON_CALL(mocker, JSON_CreateObject).WillByDefault(Return(jsonObj));
    ON_CALL(mocker, JSON_PrintUnformatted).WillByDefault(Invoke([](const JsonObj *) -> char* {
        static char msg[] = "{}";
        return msg;
    }));
}

/*
 * @tc.name: PACK_DEVICE_ID_JSON_TEST_002
 * @tc.desc: test PackDeviceIdJson full path covering PackFastAuth and PackExternalAuthInfo
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_DEVICE_ID_JSON_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    SetupPackDeviceIdJsonDefaultMocks(mocker);
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;
    info.connInfo.type = AUTH_LINK_TYPE_WIFI;
    info.isServer = false;
    info.isNeedFastAuth = true;
    info.isSupportFastAuth = false;
    info.localState = AUTH_STATE_START;
    info.version = SOFTBUS_NEW_V1;

    char *ret = PackDeviceIdJson(&info, authSeq);
    EXPECT_NE(ret, nullptr);
    if (ret != nullptr) {
        cJSON_free(ret);
    }
}

/*
 * @tc.name: UNPACK_FAST_AUTH_TEST_001
 * @tc.desc: test UnpackFastAuth for field missing, udid hash fail, untrusted and key fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_FAST_AUTH_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.connInfo.type = AUTH_LINK_TYPE_WIFI;

    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(UnpackFastAuth(&obj, &info));
    EXPECT_FALSE(info.isSupportFastAuth);

    static const char fastAuth[] = "aabb";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(fastAuth, fastAuth + sizeof(fastAuth)), Return(true)));
    EXPECT_EQ(strcpy_s(info.udid, UDID_BUF_LEN, "peerUdid"), EOK);
    EXPECT_CALL(mocker, SoftBusGenerateStrHash).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UnpackFastAuth(&obj, &info));
    EXPECT_FALSE(info.isSupportFastAuth);

    EXPECT_CALL(mocker, SoftBusGenerateStrHash).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertBytesToHexString).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_NO_FATAL_FAILURE(UnpackFastAuth(&obj, &info));
    EXPECT_FALSE(info.isSupportFastAuth);

    EXPECT_CALL(mocker, ConvertBytesToHexString).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, IsPotentialTrustedDevice).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(UnpackFastAuth(&obj, &info));
    EXPECT_FALSE(info.isSupportFastAuth);

    EXPECT_CALL(mocker, IsPotentialTrustedDevice).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, AuthFindDeviceKey).WillRepeatedly(Return(SOFTBUS_AUTH_NOT_FOUND));
    EXPECT_NO_FATAL_FAILURE(UnpackFastAuth(&obj, &info));
    EXPECT_FALSE(info.isSupportFastAuth);
}

/*
 * @tc.name: UNPACK_WIFI_DIRECT_INFO_TEST_002
 * @tc.desc: test UnpackWifiDirectInfo for field missing and normal branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_WIFI_DIRECT_INFO_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo nodeInfo;
    (void)memset_s(&nodeInfo, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, JSON_GetInt32FromOject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(UnpackWifiDirectInfo(&json, &nodeInfo, false));

    static const char staticCap[] = "aabb";
    ON_CALL(mocker, JSON_GetInt32FromOject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, int32_t *value) -> bool {
            (void)key;
            *value = 1;
            return true;
        }));
    ON_CALL(mocker, JSON_GetStringFromObject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, char *value, uint32_t size) -> bool {
            if (strcmp(key, STATIC_CAP) == 0 || strcmp(key, PTK) == 0) {
                return strcpy_s(value, size, staticCap) == EOK;
            }
            return false;
        }));
    EXPECT_CALL(mocker, ConvertHexStringToBytes).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, SoftBusBase64Decode).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(UnpackWifiDirectInfo(&json, &nodeInfo, true));
    EXPECT_NO_FATAL_FAILURE(UnpackWifiDirectInfo(&json, &nodeInfo, false));
}

/*
 * @tc.name: PACK_DEVICE_INFO_BT_V1_TEST_002
 * @tc.desc: test PackDeviceInfoBtV1 for success and mac/base info fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_DEVICE_INFO_BT_V1_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, LnnGetBtMac).WillRepeatedly(Return(info.connectInfo.macAddr));
    EXPECT_CALL(mocker, LnnGetP2pMac).WillRepeatedly(Return(info.p2pInfo.p2pMac));
    EXPECT_CALL(mocker, LnnGetLocalStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt64ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddBoolToObject).WillRepeatedly(Return(true));
    EXPECT_EQ(PackDeviceInfoBtV1(&json, &info, false), SOFTBUS_OK);

    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(false));
    EXPECT_EQ(PackDeviceInfoBtV1(&json, &info, false), SOFTBUS_AUTH_PACK_DEVINFO_FAIL);
}

/*
 * @tc.name: PACK_BT_TEST_002
 * @tc.desc: test PackBt for code add fail, config fail and PackCommon fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_BT_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, SoftbusGetConfig).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_CALL(mocker, LnnGetBtMac).WillRepeatedly(Return(info.connectInfo.macAddr));
    EXPECT_CALL(mocker, LnnGetP2pMac).WillRepeatedly(Return(info.p2pInfo.p2pMac));
    EXPECT_CALL(mocker, LnnGetP2pRole).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, LnnGetStaFrequency).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, GetExtData).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_CALL(mocker, LnnGetSupportedProtocols).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, ConvertBytesToHexString).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertBytesToUpperCaseHexString).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnUpdateLocalBroadcastCipherKey).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, IsFeatureSupport).WillRepeatedly(Return(false));
    EXPECT_CALL(mocker, PackCipherKeySyncMsg).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, LnnGetAuthPort).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, LnnGetSessionPort).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, LnnGetProxyPort).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, SoftBusBase64Encode).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt64ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddInt16ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddBoolToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, LnnGetLocalStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_EQ(PackBt(&json, &info, SOFTBUS_NEW_V1, false, nullptr), SOFTBUS_OK);

    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(false));
    EXPECT_EQ(PackBt(&json, &info, SOFTBUS_NEW_V1, false, nullptr), SOFTBUS_AUTH_PACK_DEVINFO_FAIL);
}

/*
 * @tc.name: SET_EXCHANGE_ID_TYPE_AND_VALUE_TEST_002
 * @tc.desc: test SetExchangeIdTypeAndValue for udid exchange, server and client network id branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, SET_EXCHANGE_ID_TYPE_AND_VALUE_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));

    ON_CALL(mocker, JSON_GetInt32FromOject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, int32_t *value) -> bool {
            if (strcmp(key, EXCHANGE_ID_TYPE) == 0) {
                *value = EXCHANGE_UDID;
                return true;
            }
            return false;
        }));
    EXPECT_EQ(strcpy_s(info.udid, UDID_BUF_LEN, "peerUdid"), EOK);
    EXPECT_EQ(SetExchangeIdTypeAndValue(&obj, &info), SOFTBUS_OK);
    EXPECT_EQ(info.idType, EXCHANGE_UDID);

    ON_CALL(mocker, JSON_GetInt32FromOject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, int32_t *value) -> bool {
            if (strcmp(key, EXCHANGE_ID_TYPE) == 0) {
                *value = EXCHANGE_NETWORKID;
                return true;
            }
            return false;
        }));
    info.isServer = true;
    static const char peerUdid[] = "peerUdidValue";
    EXPECT_CALL(mocker, GetPeerUdidByNetworkId)
        .WillRepeatedly(DoAll(SetArrayArgument<1>(peerUdid, peerUdid + sizeof(peerUdid)), Return(SOFTBUS_OK)));
    EXPECT_CALL(mocker, GetIsExchangeUdidByNetworkId).WillRepeatedly(DoAll(SetArgPointee<1>(true), Return(SOFTBUS_OK)));
    EXPECT_EQ(SetExchangeIdTypeAndValue(&obj, &info), SOFTBUS_OK);
    EXPECT_EQ(info.idType, EXCHANGE_UDID);

    info.isServer = false;
    info.idType = EXCHANGE_NETWORKID;
    EXPECT_EQ(SetExchangeIdTypeAndValue(&obj, &info), SOFTBUS_OK);

    ON_CALL(mocker, JSON_GetInt32FromOject).WillByDefault(Invoke(
        [](const JsonObj *, const char *key, int32_t *value) -> bool {
            if (strcmp(key, EXCHANGE_ID_TYPE) == 0) {
                *value = EXCHANGE_FAIL;
                return true;
            }
            return false;
        }));
    info.idType = EXCHANGE_NETWORKID;
    EXPECT_EQ(SetExchangeIdTypeAndValue(&obj, &info), SOFTBUS_OK);
    EXPECT_EQ(info.idType, EXCHANGE_FAIL);
}

/*
 * @tc.name: PACK_NORMALIZED_KEY_INNER_TEST_001
 * @tc.desc: test PackNormalizedKeyInner for existing key, deviceKeyId and find-key branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_NORMALIZED_KEY_INNER_TEST_001, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    int64_t authSeq = 1;

    EXPECT_EQ(strcpy_s(info.udid, UDID_BUF_LEN, "peerUdid"), EOK);
    EXPECT_CALL(mocker, SoftBusGenerateStrHash).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, ConvertBytesToHexString).WillRepeatedly(Return(SOFTBUS_OK));

    SessionKey sessionKey;
    (void)memset_s(&sessionKey, sizeof(SessionKey), 0, sizeof(SessionKey));
    info.normalizedKey = &sessionKey;
    EXPECT_CALL(mocker, LnnEncryptAesGcm).WillRepeatedly(Invoke(
        [](AesGcmInputParam *, int32_t, uint8_t **out, uint32_t *outLen) -> int32_t {
            *out = static_cast<uint8_t *>(SoftBusCalloc(1));
            *outLen = 1;
            return *out != nullptr ? SOFTBUS_OK : SOFTBUS_MEM_ERR;
        }));
    EXPECT_CALL(mocker, ConvertBytesToUpperCaseHexString).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKeyInner(&obj, &info, authSeq));

    info.normalizedKey = nullptr;
    info.deviceKeyId.hasDeviceKeyId = true;
    EXPECT_CALL(mocker, GetSessionKeyProfile).WillOnce(Return(false)).WillRepeatedly(Return(true));
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKeyInner(&obj, &info, authSeq));
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKeyInner(&obj, &info, authSeq));
    EXPECT_NE(info.normalizedKey, nullptr);
    SoftBusFree(info.normalizedKey);
    info.normalizedKey = nullptr;

    info.deviceKeyId.hasDeviceKeyId = false;
    EXPECT_CALL(mocker, AuthFindLatestNormalizeKey).WillOnce(Return(SOFTBUS_AUTH_NOT_FOUND))
        .WillRepeatedly(DoAll([](const char *, AuthDeviceKeyInfo *key, bool) {
            key->keyLen = SESSION_KEY_LENGTH;
        }, Return(SOFTBUS_OK)));
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKeyInner(&obj, &info, authSeq));
    EXPECT_NO_FATAL_FAILURE(PackNormalizedKeyInner(&obj, &info, authSeq));
    if (info.normalizedKey != nullptr) {
        SoftBusFree(info.normalizedKey);
        info.normalizedKey = nullptr;
    }
}

/*
 * @tc.name: GET_ENHANCED_P2P_AUTH_KEY_TEST_002
 * @tc.desc: test GetEnhancedP2pAuthKey for ble key reuse and wifi session key branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, GET_ENHANCED_P2P_AUTH_KEY_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    char udidHash[SHA_256_HEX_HASH_LEN] = { 0 };
    AuthSessionInfo info;
    AuthDeviceKeyInfo deviceKey;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    (void)memset_s(&deviceKey, sizeof(AuthDeviceKeyInfo), 0, sizeof(AuthDeviceKeyInfo));
    info.connInfo.type = AUTH_LINK_TYPE_ENHANCED_P2P;

    EXPECT_CALL(mocker, AuthFindLatestNormalizeKey).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_EQ(GetEnhancedP2pAuthKey(udidHash, &info, &deviceKey), SOFTBUS_OK);

    EXPECT_CALL(mocker, AuthFindLatestNormalizeKey).WillRepeatedly(Return(SOFTBUS_AUTH_NOT_FOUND));
    EXPECT_CALL(mocker, AuthFindDeviceKey).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_EQ(GetEnhancedP2pAuthKey(udidHash, &info, &deviceKey), SOFTBUS_OK);

    EXPECT_CALL(mocker, AuthFindDeviceKey).WillRepeatedly(Return(SOFTBUS_AUTH_NOT_FOUND));
    AuthHandle authHandle = { .authId = TEST_AUTH_ID, .type = AUTH_LINK_TYPE_WIFI };
    EXPECT_CALL(mocker, AuthGetLatestIdByUuid)
        .WillRepeatedly(DoAll(SetArgPointee<3>(authHandle), Return()));
    AuthManager auth;
    (void)memset_s(&auth, sizeof(AuthManager), 0, sizeof(AuthManager));
    EXPECT_CALL(mocker, GetAuthManagerByAuthId).WillRepeatedly(Return(&auth));
    EXPECT_CALL(mocker, DelDupAuthManager).WillRepeatedly(Return());
    SessionKey sessionKey;
    (void)memset_s(&sessionKey, sizeof(SessionKey), 0, sizeof(SessionKey));
    sessionKey.len = SESSION_KEY_LENGTH;
    EXPECT_EQ(memcpy_s(sessionKey.value, SESSION_KEY_LENGTH, KEY_VALUE, KEY_VALUE_LEN), EOK);
    EXPECT_CALL(mocker, GetLatestSessionKey)
        .WillOnce(DoAll(SetArgPointee<3>(sessionKey), Return(SOFTBUS_OK)));
    EXPECT_NE(GetEnhancedP2pAuthKey(udidHash, &info, &deviceKey), SOFTBUS_OK);
    EXPECT_EQ(deviceKey.keyLen, sessionKey.len);
}

/*
 * @tc.name: UN_PACK_AUTH_PRELINK_NODE_TEST_002
 * @tc.desc: test UnPackAuthPreLinkNode with prelink node flag present
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UN_PACK_AUTH_PRELINK_NODE_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.requestId = 2;
    info.connId = 1;
    EXPECT_CALL(mocker, JSON_GetBoolFromOject)
        .WillRepeatedly(DoAll(SetArgPointee<2>(true), Return(true)));
    EXPECT_CALL(mocker, GetFd).WillRepeatedly(Return(1));
    EXPECT_NO_FATAL_FAILURE(UnPackAuthPreLinkNode(&obj, &info));
}

/*
 * @tc.name: PACK_AUTH_PRELINK_NODE_TEST_002
 * @tc.desc: test PackAuthPreLinkNode when prelink node found and add flag success
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_AUTH_PRELINK_NODE_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    EXPECT_CALL(mocker, FindAuthPreLinkNodeById).WillRepeatedly(Return(SOFTBUS_OK));
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    info.requestId = 1;
    EXPECT_CALL(mocker, JSON_AddBoolToObject).WillRepeatedly(Return(true));
    EXPECT_NO_FATAL_FAILURE(PackAuthPreLinkNode(&obj, &info));
}

/*
 * @tc.name: UNPACK_WIFI_SINGLE_PASS_INFO_TEST_002
 * @tc.desc: test UnpackWifiSinglePassInfo for non-wifi, field missing, peer name and ip hash branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, UNPACK_WIFI_SINGLE_PASS_INFO_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));

    info.connInfo.type = AUTH_LINK_TYPE_BR;
    EXPECT_TRUE(UnpackWifiSinglePassInfo(&obj, &info));

    info.connInfo.type = AUTH_LINK_TYPE_WIFI;
    EXPECT_CALL(mocker, JSON_GetStringFromObject).WillRepeatedly(Return(false));
    EXPECT_TRUE(UnpackWifiSinglePassInfo(&obj, &info));

    static const char devIpHash[] = "aabb";
    EXPECT_CALL(mocker, JSON_GetStringFromObject)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(devIpHash, devIpHash + sizeof(devIpHash)), Return(true)));
    EXPECT_CALL(mocker, GetFd).WillRepeatedly(Return(1));
    EXPECT_CALL(mocker, SoftBusSocketGetPeerName).WillOnce(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_TRUE(UnpackWifiSinglePassInfo(&obj, &info));

    SoftBusSockAddr addr;
    (void)memset_s(&addr, sizeof(SoftBusSockAddr), 0, sizeof(SoftBusSockAddr));
    addr.saFamily = SOFTBUS_AF_INET;
    EXPECT_CALL(mocker, SoftBusSocketGetPeerName)
        .WillRepeatedly(DoAll(SetArgPointee<1>(addr), Return(SOFTBUS_OK)));
    EXPECT_CALL(mocker, SoftBusInetNtoP).WillOnce(Return(nullptr));
    EXPECT_TRUE(UnpackWifiSinglePassInfo(&obj, &info));

    static const char ipStr[] = "192.168.1.1";
    EXPECT_CALL(mocker, SoftBusInetNtoP)
        .WillRepeatedly(DoAll(SetArrayArgument<2>(ipStr, ipStr + sizeof(ipStr)), Return(ipStr)));
    EXPECT_CALL(mocker, SoftBusGenerateStrHash)
        .WillOnce(Return(SOFTBUS_INVALID_PARAM))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_TRUE(UnpackWifiSinglePassInfo(&obj, &info));
    EXPECT_CALL(mocker, ConvertBytesToUpperCaseHexString).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_TRUE(UnpackWifiSinglePassInfo(&obj, &info));
}

/*
 * @tc.name: PACK_CRED_NEGO_ALL_USERIDS_TEST_002
 * @tc.desc: test PackCredNegoAllUserIds for add json fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_CRED_NEGO_ALL_USERIDS_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    int32_t *userIds = static_cast<int32_t *>(SoftBusCalloc(sizeof(int32_t) * 2));
    ASSERT_NE(userIds, nullptr);
    userIds[0] = TEST_BASIC_USER_ID;
    userIds[1] = TEST_BASIC_USER_ID + 1;
    EXPECT_CALL(mocker, GetAllForegroundAccountIds)
        .WillRepeatedly(
            DoAll(SetArgPointee<0>(userIds), SetArgPointee<1>(static_cast<uint32_t>(2)), Return(SOFTBUS_OK)));
    EXPECT_CALL(mocker, cJSON_CreateArray).WillRepeatedly(Return(nullptr));
    EXPECT_FALSE(PackCredNegoAllUserIds(&json));
}

/*
 * @tc.name: VERIFY_SESSION_INFO_ID_TYPE_TEST_002
 * @tc.desc: test VerifySessionInfoIdType for network id and udid add fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, VERIFY_SESSION_INFO_ID_TYPE_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj obj;
    (void)memset_s(&obj, sizeof(JsonObj), 0, sizeof(JsonObj));
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    char networkId[NETWORK_ID_BUF_LEN] = "networkId";
    char udid[UDID_BUF_LEN] = "udid";

    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    info.idType = EXCHANGE_NETWORKID;
    EXPECT_TRUE(VerifySessionInfoIdType(&info, &obj, networkId, udid));
    info.idType = EXCHANGE_UDID;
    EXPECT_TRUE(VerifySessionInfoIdType(&info, &obj, networkId, udid));

    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(false));
    info.idType = EXCHANGE_NETWORKID;
    EXPECT_FALSE(VerifySessionInfoIdType(&info, &obj, networkId, udid));
    info.idType = EXCHANGE_UDID;
    EXPECT_FALSE(VerifySessionInfoIdType(&info, &obj, networkId, udid));
}

/*
 * @tc.name: PACK_WIFI_TEST_002
 * @tc.desc: test PackWiFi for code add fail, base64 encode fail and PackCommon fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_WIFI_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));

    EXPECT_CALL(mocker, LnnGetAuthPort).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, LnnGetSessionPort).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, LnnGetProxyPort).WillRepeatedly(Return(0));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, SoftBusBase64Encode).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    EXPECT_EQ(PackWiFi(&json, &info, SOFTBUS_NEW_V1, false, WLAN_IF), SOFTBUS_ENCRYPT_ERR);

    EXPECT_CALL(mocker, SoftBusBase64Encode).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(false));
    EXPECT_EQ(PackWiFi(&json, &info, SOFTBUS_NEW_V1, false, WLAN_IF), SOFTBUS_AUTH_PACK_DEVINFO_FAIL);
}

/*
 * @tc.name: IS_NEED_NORMALIZED_PROCESS_TEST_002
 * @tc.desc: test IsNeedNormalizedProcess for client, lower version, external user ids and trust branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, IS_NEED_NORMALIZED_PROCESS_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    AuthSessionInfo info;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));

    info.isConnectServer = false;
    EXPECT_TRUE(IsNeedNormalizedProcess(&info));

    info.isConnectServer = true;
    info.authVersion = AUTH_VERSION_V1;
    EXPECT_TRUE(IsNeedNormalizedProcess(&info));

    info.authVersion = AUTH_VERSION_V2;
    info.credNegoState = CRED_NEGO_STATE_COMPATIBLE;
    EXPECT_CALL(mocker, IsTrustedDeviceFromAccess).WillRepeatedly(Return(true));
    EXPECT_TRUE(IsNeedNormalizedProcess(&info));

    EXPECT_CALL(mocker, IsTrustedDeviceFromAccess).WillRepeatedly(Return(false));
    EXPECT_FALSE(IsNeedNormalizedProcess(&info));
    EXPECT_EQ(info.normalizedType, NORMALIZED_KEY_ERROR);
}

/*
 * @tc.name: PARSE_FAST_AUTH_VALUE_TEST_002
 * @tc.desc: test ParseFastAuthValue for content mismatch branch
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PARSE_FAST_AUTH_VALUE_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    AuthSessionInfo info;
    AuthDeviceKeyInfo deviceKey;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    (void)memset_s(&deviceKey, sizeof(AuthDeviceKeyInfo), 0, sizeof(AuthDeviceKeyInfo));
    const char *fastAuth = "aabbccdd";
    uint8_t *data = static_cast<uint8_t *>(SoftBusCalloc(ENCRYPTED_FAST_AUTH_MAX_LEN));
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(memcpy_s(data, ENCRYPTED_FAST_AUTH_MAX_LEN, KEY_VALUE, KEY_VALUE_LEN), EOK);

    EXPECT_CALL(mocker, ConvertHexStringToBytes).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnDecryptAesGcm)
        .WillRepeatedly(
            DoAll(SetArgPointee<1>(data), SetArgPointee<2>(static_cast<uint32_t>(KEY_VALUE_LEN)), Return(SOFTBUS_OK)));
    EXPECT_NO_FATAL_FAILURE(ParseFastAuthValue(&info, fastAuth, &deviceKey));
    EXPECT_FALSE(info.isSupportFastAuth);
}

/*
 * @tc.name: TRY_GET_DM_SESSION_KEY_TEST_002
 * @tc.desc: test TryGetDmSessionKeyForUnpack for get profile success and parse fail branches
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, TRY_GET_DM_SESSION_KEY_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    AuthSessionInfo info;
    AuthDeviceKeyInfo deviceKey;
    (void)memset_s(&info, sizeof(AuthSessionInfo), 0, sizeof(AuthSessionInfo));
    (void)memset_s(&deviceKey, sizeof(AuthDeviceKeyInfo), 0, sizeof(AuthDeviceKeyInfo));
    info.deviceKeyId.hasDeviceKeyId = true;
    info.deviceKeyId.localDeviceKeyId = 1;
    const char *encKey = "aabb";

    EXPECT_CALL(mocker, GetSessionKeyProfile).WillRepeatedly(Return(true));
    EXPECT_CALL(mocker, ConvertHexStringToBytes).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mocker, LnnDecryptAesGcm).WillRepeatedly(Return(SOFTBUS_DECRYPT_ERR));
    int32_t ret = TryGetDmSessionKeyForUnpack(&info, const_cast<char *>(encKey), &deviceKey, 1);
    EXPECT_EQ(ret, SOFTBUS_AUTH_NORMALIZED_KEY_PROC_ERR);
}

/*
 * @tc.name: PACK_USER_ID_TEST_002
 * @tc.desc: test PackUserId for add fail branch
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_USER_ID_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, JSON_AddInt32ToObject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(PackUserId(&json, TEST_BASIC_USER_ID, &info));
    EXPECT_EQ(info.localUserId, TEST_BASIC_USER_ID);
}

/*
 * @tc.name: PACK_SERVICE_FIND_CAP_TEST_002
 * @tc.desc: test PackServiceFindCap for add fail branch
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(AuthSessionJsonMockTest, PACK_SERVICE_FIND_CAP_TEST_002, TestSize.Level1)
{
    NiceMock<AuthSessionJsonDepsInterfaceMock> mocker;
    JsonObj json;
    (void)memset_s(&json, sizeof(JsonObj), 0, sizeof(JsonObj));
    NodeInfo info;
    (void)memset_s(&info, sizeof(NodeInfo), 0, sizeof(NodeInfo));
    EXPECT_CALL(mocker, JSON_AddStringToObject).WillRepeatedly(Return(false));
    EXPECT_NO_FATAL_FAILURE(PackServiceFindCap(&json, &info));
}
} // namespace OHOS
