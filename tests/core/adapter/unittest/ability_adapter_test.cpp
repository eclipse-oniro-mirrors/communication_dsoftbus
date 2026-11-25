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

#include "parameter_mock.h"
#include "ability_adapter.cpp"

using namespace testing::ext;
using testing::_;
using testing::NiceMock;
using testing::Return;

namespace OHOS {
class AbilityAdapterTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }

    void SetUp() override { }

    void TearDown() override { }
};

/*
 * @tc.name: IsMultiScreenDevice001
 * @tc.desc: model is "SPN-w29", prefix matches SPN, expect true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityAdapterTest, IsMultiScreenDevice001, TestSize.Level1)
{
    ParameterMock parameterMock;

    EXPECT_CALL(parameterMock, GetProductModel).WillOnce(ParameterMock::ActionOfGetProductModelSPN);
    EXPECT_EQ(true, IsMultiScreenDevice());
}

/*
 * @tc.name: IsMultiScreenDevice002
 * @tc.desc: model is "SZY-100", prefix matches SZY, expect true
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityAdapterTest, IsMultiScreenDevice002, TestSize.Level1)
{
    ParameterMock parameterMock;

    EXPECT_CALL(parameterMock, GetProductModel).WillOnce(ParameterMock::ActionOfGetProductModelSZY);
    EXPECT_EQ(true, IsMultiScreenDevice());
}

/*
 * @tc.name: IsMultiScreenDevice003
 * @tc.desc: model is "ABC123", no prefix match, expect false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityAdapterTest, IsMultiScreenDevice003, TestSize.Level1)
{
    ParameterMock parameterMock;

    EXPECT_CALL(parameterMock, GetProductModel).WillOnce(ParameterMock::ActionOfGetProductModelOther);
    EXPECT_EQ(false, IsMultiScreenDevice());
}

/*
 * @tc.name: IsMultiScreenDevice004
 * @tc.desc: model is empty string, expect false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityAdapterTest, IsMultiScreenDevice004, TestSize.Level1)
{
    ParameterMock parameterMock;

    EXPECT_CALL(parameterMock, GetProductModel).WillOnce(ParameterMock::ActionOfGetProductModelFail);
    EXPECT_EQ(false, IsMultiScreenDevice());
}

/*
 * @tc.name: IsMultiScreenDevice005
 * @tc.desc: GetProductModel returns nullptr, expect false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityAdapterTest, IsMultiScreenDevice005, TestSize.Level1)
{
    ParameterMock parameterMock;

    EXPECT_CALL(parameterMock, GetProductModel).WillOnce(ParameterMock::ActionOfGetProductModelNull);
    EXPECT_EQ(false, IsMultiScreenDevice());
}

/*
 * @tc.name: StartAbility001
 * @tc.desc: invalid params return SOFTBUS_INVALID_PARAM,
 *           valid params return SOFTBUS_PERMISSION_DENIED (no system services in test env)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityAdapterTest, StartAbility001, TestSize.Level1)
{
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartAbility(nullptr, "MainAbility", 100));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartAbility("com.test.bundle", nullptr, 100));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartAbility("com.test.bundle", "MainAbility", -1));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartAbility(nullptr, nullptr, -1));
    EXPECT_EQ(SOFTBUS_PERMISSION_DENIED, StartAbility("com.test.bundle", "MainAbility", 100));
}
} // namespace OHOS
