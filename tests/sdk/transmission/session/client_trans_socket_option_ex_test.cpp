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

#include <gtest/gtest.h>
#include <securec.h>

#include "client_trans_socket_option.c"
#include "softbus_error_code.h"

using namespace testing::ext;

namespace OHOS {

class TransClientSocketOptionExTest : public testing::Test {
public:
    TransClientSocketOptionExTest() {}
    ~TransClientSocketOptionExTest() {}
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override {}
    void TearDown() override {}
};

/*
 * @tc.name: TransSetEdcaTest001
 * @tc.desc: socket<=0 returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest001, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_SEMI };
    int32_t ret = TransSetEdca(0, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest002
 * @tc.desc: NULL optValue returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest002, TestSize.Level1)
{
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, NULL, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest003
 * @tc.desc: optValueSize < sizeof(EdcaParam) returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest003, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_SEMI };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, 1);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest004
 * @tc.desc: level != OPT_LEVEL_SOFTBUS returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest004, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_SEMI };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_KERNEL, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest005
 * @tc.desc: scene < EDCA_SCENE_DATA_CLONE returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest005, TestSize.Level1)
{
    EdcaParam edca = { .scene = static_cast<EdcaScene>(-1), .level = EDCA_LEVEL_SEMI };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest006
 * @tc.desc: scene >= EDCA_SCENE_BUTT returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest006, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_BUTT, .level = EDCA_LEVEL_SEMI };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest007
 * @tc.desc: level < EDCA_LEVEL_DISABLE returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest007, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = static_cast<EdcaLevel>(-1) };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest008
 * @tc.desc: level >= EDCA_LEVEL_BUTT returns SOFTBUS_INVALID_PARAM.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest008, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_BUTT };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
 * @tc.name: TransSetEdcaTest009
 * @tc.desc: valid params forward to ClientSetEdcaBySocket (returns error since no session).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransClientSocketOptionExTest, TransSetEdcaTest009, TestSize.Level1)
{
    EdcaParam edca = { .scene = EDCA_SCENE_DATA_CLONE, .level = EDCA_LEVEL_SEMI };
    int32_t ret = TransSetEdca(1, OPT_LEVEL_SOFTBUS, OPT_TYPE_EDCA, &edca, sizeof(EdcaParam));
    EXPECT_NE(SOFTBUS_OK, ret);
}
}
