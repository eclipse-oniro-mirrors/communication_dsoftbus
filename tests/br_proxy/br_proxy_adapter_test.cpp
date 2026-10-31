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

#include <cstdint>
#include <string>
#include <unistd.h>

#include "softbus_error_code.h"
#include "gtest/gtest.h"

extern "C" {
int32_t StartAbility(const char *bundleName, const char *abilityName, int32_t appIndex, int32_t userId);
bool GetRunningProcessInformation(const std::string bundleName, int32_t userId, pid_t uid, pid_t *pid);
int32_t ProxyChannelMgrGetAbilityName(char *abilityName, int32_t userId, uint32_t abilityNameLen,
    std::string bundleName, int32_t *appIndex);
int32_t Unrestricted(const char *bundleName, pid_t pid, pid_t uid, bool isThaw);
}

using namespace testing;
using namespace testing::ext;

namespace OHOS {
// BrProxyAdapterTest compiles br_proxy_adapter_virtual.cpp directly to cover the stub contract used
// when device_standby is unsupported (or on non-standard systems): every int32_t interface returns
// SOFTBUS_FUNC_NOT_SUPPORT and GetRunningProcessInformation returns false. The signatures here are
// kept aligned with br_proxy_adapter.cpp (real impl) and br_proxy_common.cpp (dlsym caller).
class BrProxyAdapterTest : public testing::Test {
public:
    BrProxyAdapterTest() { }
    ~BrProxyAdapterTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override { }
    void TearDown() override { }
};

void BrProxyAdapterTest::SetUpTestCase(void) { }

void BrProxyAdapterTest::TearDownTestCase(void) { }

/**
 * @tc.name: StartAbility001
 * @tc.desc: when device_standby is unsupported the stub returns SOFTBUS_FUNC_NOT_SUPPORT
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyAdapterTest, StartAbility001, TestSize.Level1)
{
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT, StartAbility("com.test.bundle", "MainAbility", 0, 100));
}

/**
 * @tc.name: GetRunningProcessInformation001
 * @tc.desc: when device_standby is unsupported the stub returns false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyAdapterTest, GetRunningProcessInformation001, TestSize.Level1)
{
    pid_t pid = -1;
    EXPECT_FALSE(GetRunningProcessInformation("com.test.bundle", 100, 10000, &pid));
}

/**
 * @tc.name: ProxyChannelMgrGetAbilityName001
 * @tc.desc: when device_standby is unsupported the stub returns SOFTBUS_FUNC_NOT_SUPPORT
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyAdapterTest, ProxyChannelMgrGetAbilityName001, TestSize.Level1)
{
    char abilityName[256] = { 0 };
    int32_t appIndex = -1;
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT,
        ProxyChannelMgrGetAbilityName(abilityName, 100, sizeof(abilityName), "com.test.bundle", &appIndex));
}

/**
 * @tc.name: Unrestricted001
 * @tc.desc: when device_standby is unsupported Unrestricted returns SOFTBUS_FUNC_NOT_SUPPORT regardless of isThaw
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(BrProxyAdapterTest, Unrestricted001, TestSize.Level1)
{
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT, Unrestricted("com.test.bundle", 1234, 10000, true));
    EXPECT_EQ(SOFTBUS_FUNC_NOT_SUPPORT, Unrestricted("com.test.bundle", 1234, 10000, false));
}
} // namespace OHOS
