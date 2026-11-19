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
#include <securec.h>

#include "lnn_distributed_user_info.c"
#include "lnn_distributed_user_info.h"
#include "lnn_log.h"
#include "softbus_error_code.h"

namespace OHOS {
using namespace testing::ext;

constexpr char UDID_TEST[] = "udid_test";
constexpr char UDID_NOT_ONLINE[] = "udid_not_online";
constexpr uint64_t OLD_TIME_STAMP = 5000;
constexpr uint64_t STALE_TIME_STAMP = 4000;
constexpr uint64_t NEW_TIME_STAMP = 6000;
constexpr int32_t USER_ID = 1;
constexpr int64_t ACCOUNT_ID = 100;
constexpr int32_t USER_ID_SECOND = 2;

class LnnDistributedUserInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void LnnDistributedUserInfoTest::SetUpTestCase() { }

void LnnDistributedUserInfoTest::TearDownTestCase() { }

void LnnDistributedUserInfoTest::SetUp()
{
    LNN_LOGI(LNN_TEST, "LnnDistributedUserInfoTest start");
    int32_t ret = LnnInitDistributedUserLedger();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

void LnnDistributedUserInfoTest::TearDown()
{
    LNN_LOGI(LNN_TEST, "LnnDistributedUserInfoTest end");
    LnnRemoveUserInfoNode(UDID_TEST);
    LnnDeinitDistributedUserLedger();
}

/*
 * @tc.name: LNN_INIT_DISTRIBUTED_USER_LEDGER_TEST_001
 * @tc.desc: Verify LnnInitDistributedUserLedger succeeds on first
 *           and repeated init
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_INIT_DISTRIBUTED_USER_LEDGER_TEST_001, TestSize.Level1)
{
    int32_t ret = LnnInitDistributedUserLedger();
    EXPECT_EQ(ret, SOFTBUS_OK);
    LnnDeinitDistributedUserLedger();
    ret = LnnInitDistributedUserLedger();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: LNN_DEINIT_DISTRIBUTED_USER_LEDGER_TEST_001
 * @tc.desc: Verify LnnDeinitDistributedUserLedger cleans empty ledger
 *           and handles deinit without init
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_DEINIT_DISTRIBUTED_USER_LEDGER_TEST_001, TestSize.Level1)
{
    UserInfo info = {};
    int32_t ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    LnnRemoveUserInfoNode(UDID_TEST);
    LnnDeinitDistributedUserLedger();
    LnnDeinitDistributedUserLedger();
    ret = LnnInitDistributedUserLedger();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: LNN_REMOVE_USER_INFO_NODE_TEST_001
 * @tc.desc: Verify LnnRemoveUserInfoNode removes user node and handles
 *           null or unknown udid
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_REMOVE_USER_INFO_NODE_TEST_001, TestSize.Level1)
{
    UserInfo info = {};
    int32_t ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    LnnRemoveUserInfoNode(nullptr);
    LnnRemoveUserInfoNode(UDID_NOT_ONLINE);
    LnnRemoveUserInfoNode(UDID_TEST);
}

/*
 * @tc.name: LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_001
 * @tc.desc: Verify LnnUpdateDistributedUserInfo rejects invalid params
 *           and inserts new user
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_001, TestSize.Level1)
{
    UserInfo info = {};
    int32_t ret = LnnUpdateDistributedUserInfo(nullptr, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = LnnUpdateDistributedUserInfo(&info, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_002
 * @tc.desc: Verify LnnUpdateDistributedUserInfo ignores stale user info
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_002, TestSize.Level1)
{
    UserInfo info = {};
    info.updateTimestamp = OLD_TIME_STAMP;
    int32_t ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    info.updateTimestamp = STALE_TIME_STAMP;
    ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    UserInfo findInfo = {};
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, 0, &findInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(findInfo.updateTimestamp, OLD_TIME_STAMP);
}

/*
 * @tc.name: LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_003
 * @tc.desc: Verify LnnUpdateDistributedUserInfo updates existing user info
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_003, TestSize.Level1)
{
    UserInfo info = {};
    info.updateTimestamp = OLD_TIME_STAMP;
    int32_t ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    info.updateTimestamp = NEW_TIME_STAMP;
    ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    UserInfo findInfo = {};
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, 0, &findInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(findInfo.updateTimestamp, NEW_TIME_STAMP);
}

/*
 * @tc.name: LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_004
 * @tc.desc: Verify LnnUpdateDistributedUserInfo adds user and rejects long udid
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_004, TestSize.Level1)
{
    UserInfo info = {};
    int32_t ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    info.userId = USER_ID_SECOND;
    ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    char longUdid[UDID_BUF_LEN + 8] = { 0 };
    (void)memset_s(longUdid, sizeof(longUdid), 'a', sizeof(longUdid) - 1);
    ret = LnnUpdateDistributedUserInfo(&info, longUdid);
    EXPECT_EQ(ret, SOFTBUS_STRCPY_ERR);
}

/*
 * @tc.name: LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_005
 * @tc.desc: Verify update and find handle map node without user list
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_UPDATE_DISTRIBUTED_USER_INFO_TEST_005, TestSize.Level1)
{
    UserInfo info = {};
    DistributedUserInfo nullListInfo = {};
    (void)strcpy_s(nullListInfo.udid, UDID_BUF_LEN, UDID_TEST);
    int32_t ret = LnnMapSet(&g_distributedUserMap, UDID_TEST, &nullListInfo,
        sizeof(DistributedUserInfo));
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, 0, &info);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: LNN_FIND_USER_BY_USERID_AND_UDID_TEST_001
 * @tc.desc: Verify LnnFindUserByUserIdAndUdid rejects invalid params
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_FIND_USER_BY_USERID_AND_UDID_TEST_001, TestSize.Level1)
{
    UserInfo info = {};
    int32_t ret = LnnFindUserByUserIdAndUdid(nullptr, 0, &info);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, 0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, 0, &info);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: LNN_FIND_USER_BY_USERID_AND_UDID_TEST_002
 * @tc.desc: Verify LnnFindUserByUserIdAndUdid returns user info by userId
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(LnnDistributedUserInfoTest, LNN_FIND_USER_BY_USERID_AND_UDID_TEST_002, TestSize.Level1)
{
    UserInfo info = {};
    info.userId = USER_ID;
    info.accountId = ACCOUNT_ID;
    int32_t ret = LnnUpdateDistributedUserInfo(&info, UDID_TEST);
    EXPECT_EQ(ret, SOFTBUS_OK);
    UserInfo findInfo = {};
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, USER_ID_SECOND, &findInfo);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
    ret = LnnFindUserByUserIdAndUdid(UDID_TEST, USER_ID, &findInfo);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(findInfo.accountId, ACCOUNT_ID);
}
} // namespace OHOS
