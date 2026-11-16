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

#include "perception_advertiser.c"

namespace OHOS {
using namespace testing::ext;

class PerceptionAdvTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void PerceptionAdvTest::SetUpTestCase() { }

void PerceptionAdvTest::TearDownTestCase() { }

void PerceptionAdvTest::SetUp()
{
    (void)memset_s(&g_advCtx.currentParam, sizeof(g_advCtx.currentParam), 0, sizeof(g_advCtx.currentParam));
}

void PerceptionAdvTest::TearDown() { }

/*
 * @tc.name: CACHE_PARAM_NORMAL_Test_001
 * @tc.desc: Verify CacheParam caches custom data when customDataLen is
 *           within PERCEPTION_CUSTOM_DATA_MAX_LEN
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(PerceptionAdvTest, CACHE_PARAM_NORMAL_Test_001, TestSize.Level1)
{
    PerceptionAdvParam param;
    (void)memset_s(&param, sizeof(param), 0, sizeof(param));
    param.customData[0] = 0x11;
    param.customData[1] = 0x22;
    param.customData[2] = 0x33;
    param.customDataLen = 3;

    int32_t ret = CacheParam(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param);

    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_advCtx.currentParam.customDataLen, param.customDataLen);
    EXPECT_EQ(g_advCtx.currentParam.customData[0], 0x11);
    EXPECT_EQ(g_advCtx.currentParam.customData[1], 0x22);
    EXPECT_EQ(g_advCtx.currentParam.customData[2], 0x33);
}

/*
 * @tc.name: CACHE_PARAM_TOO_LONG_Test_001
 * @tc.desc: Verify CacheParam enters the failure branch when customDataLen
 *           exceeds PERCEPTION_CUSTOM_DATA_MAX_LEN, logs customData hex and
 *           length, and resets customDataLen to zero
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(PerceptionAdvTest, CACHE_PARAM_TOO_LONG_Test_001, TestSize.Level1)
{
    PerceptionAdvParam param;
    (void)memset_s(&param, sizeof(param), 0, sizeof(param));
    for (uint32_t i = 0; i < PERCEPTION_CUSTOM_DATA_MAX_LEN; i++) {
        param.customData[i] = static_cast<uint8_t>(i + 1);
    }
    param.customDataLen = PERCEPTION_CUSTOM_DATA_MAX_LEN + 3; /* over limit, memcpy_s fail */

    int32_t ret = CacheParam(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param);

    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
    EXPECT_EQ(g_advCtx.currentParam.customDataLen, 0);
    EXPECT_EQ(g_advCtx.currentParam.customData[0], 0);
    EXPECT_EQ(g_advCtx.currentParam.customData[PERCEPTION_CUSTOM_DATA_MAX_LEN - 1], 0);
}

/*
 * @tc.name: CACHE_PARAM_ZERO_LEN_Test_001
 * @tc.desc: Verify CacheParam skips copy and keeps customDataLen unchanged
 *           when customDataLen is zero
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(PerceptionAdvTest, CACHE_PARAM_ZERO_LEN_Test_001, TestSize.Level1)
{
    g_advCtx.currentParam.customDataLen = PERCEPTION_CUSTOM_DATA_MAX_LEN;
    PerceptionAdvParam param;
    (void)memset_s(&param, sizeof(param), 0, sizeof(param));
    param.customDataLen = 0;

    int32_t ret = CacheParam(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param);

    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_advCtx.currentParam.customDataLen, 0);
}

/*
 * @tc.name: DO_START_BROADCAST_CACHE_FAIL_Test_001
 * @tc.desc: Verify DoStartBroadcast returns SOFTBUS_MEM_ERR and skips the
 *           broadcast flow when CacheParam fails with an over-length customData
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(PerceptionAdvTest, DO_START_BROADCAST_CACHE_FAIL_Test_001, TestSize.Level1)
{
    PerceptionAdvParam param;
    (void)memset_s(&param, sizeof(param), 0, sizeof(param));
    param.customDataLen = PERCEPTION_CUSTOM_DATA_MAX_LEN + 1; /* over limit, CacheParam fail */

    int32_t ret = DoStartBroadcast(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param, PERCEPTION_ADV_FREQ_LOW);

    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
}

/*
 * @tc.name: UPDATE_ADV_DATA_LOCKED_CACHE_FAIL_Test_001
 * @tc.desc: Verify UpdateAdvDataLocked returns SOFTBUS_MEM_ERR and skips the
 *           broadcast data update when CacheParam fails with an over-length customData
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(PerceptionAdvTest, UPDATE_ADV_DATA_LOCKED_CACHE_FAIL_Test_001, TestSize.Level1)
{
    PerceptionAdvParam param;
    (void)memset_s(&param, sizeof(param), 0, sizeof(param));
    param.customDataLen = PERCEPTION_CUSTOM_DATA_MAX_LEN + 1; /* over limit, CacheParam fail */

    int32_t ret = UpdateAdvDataLocked(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &param);

    EXPECT_EQ(ret, SOFTBUS_MEM_ERR);
}
} // namespace OHOS
