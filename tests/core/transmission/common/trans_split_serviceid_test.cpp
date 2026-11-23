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

#include <cerrno>
#include <cstdint>

#include <gtest/gtest.h>

#include "trans_split_serviceid.c"

using namespace testing::ext;

namespace OHOS {

class TransSplitServiceIdTest : public testing::Test {
public:
    TransSplitServiceIdTest()
    {}
    ~TransSplitServiceIdTest()
    {}
    static void SetUpTestCase(void)
    {}
    static void TearDownTestCase(void)
    {}
    void SetUp() override
    {}
    void TearDown() override
    {}
};

/*
 * @tc.name: SplitToGetServiceIdTest_001
 * @tc.desc: SplitToGetServiceId parses valid serviceId strings (normal/zero/negative) successfully.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSplitServiceIdTest, SplitToGetServiceIdTest_001, TestSize.Level1)
{
    int64_t serviceId = -1;
    EXPECT_TRUE(SplitToGetServiceId("serviceId_123456789", &serviceId));
    EXPECT_EQ(serviceId, 123456789);
    EXPECT_TRUE(SplitToGetServiceId("serviceId_0", &serviceId));
    EXPECT_EQ(serviceId, 0);
    EXPECT_TRUE(SplitToGetServiceId("serviceId_-100", &serviceId));
    EXPECT_EQ(serviceId, -100);
}

/*
 * @tc.name: SplitToGetServiceIdTest_002
 * @tc.desc: SplitToGetServiceId returns false for null/empty/no-underscore/no-digit/trailing-garbage inputs.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSplitServiceIdTest, SplitToGetServiceIdTest_002, TestSize.Level1)
{
    int64_t serviceId = 0;
    EXPECT_FALSE(SplitToGetServiceId(nullptr, &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId123", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId_", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId_12abc", &serviceId));
}

/*
 * @tc.name: SplitToGetServiceIdTest_003
 * @tc.desc: SplitToGetServiceId returns false when length exceeds STR_SIZE_MAX or strtoll overflows/underflows.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSplitServiceIdTest, SplitToGetServiceIdTest_003, TestSize.Level1)
{
    int64_t serviceId = 0;
    EXPECT_FALSE(SplitToGetServiceId("serviceId_abcdefghijklmnopqrstuvwxy", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId_1234567890123456789012345", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId_9223372036854775808", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId_-9223372036854775809", &serviceId));
    EXPECT_FALSE(SplitToGetServiceId("serviceId_999999999999999999999", &serviceId));
}

/*
 * @tc.name: CheckNameContainServiceIdTest_001
 * @tc.desc: CheckNameContainServiceId returns true for valid prefixed names and false when length exceeds limit.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSplitServiceIdTest, CheckNameContainServiceIdTest_001, TestSize.Level1)
{
    EXPECT_TRUE(CheckNameContainServiceId("serviceId_123"));
    EXPECT_TRUE(CheckNameContainServiceId("serviceId_"));
    EXPECT_TRUE(CheckNameContainServiceId("serviceId_0"));
    EXPECT_TRUE(CheckNameContainServiceId("serviceId_-1"));
    EXPECT_TRUE(CheckNameContainServiceId("serviceId_999999999999999999"));
    EXPECT_FALSE(CheckNameContainServiceId("serviceId_abcdefghijklmnopqrstuvwxy"));
}

/*
 * @tc.name: CheckNameContainServiceIdTest_002
 * @tc.desc: CheckNameContainServiceId returns false for null/no-underscore/wrong-prefix-length/wrong-prefix inputs.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransSplitServiceIdTest, CheckNameContainServiceIdTest_002, TestSize.Level1)
{
    EXPECT_FALSE(CheckNameContainServiceId(nullptr));
    EXPECT_FALSE(CheckNameContainServiceId("serviceId123"));
    EXPECT_FALSE(CheckNameContainServiceId("svcId_1"));
    EXPECT_FALSE(CheckNameContainServiceId("serviceIdx_1"));
    EXPECT_FALSE(CheckNameContainServiceId("serviceIZ_1"));
    EXPECT_FALSE(CheckNameContainServiceId("session_1"));
}
} // namespace OHOS
