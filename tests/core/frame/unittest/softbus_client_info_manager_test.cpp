/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "iservice_registry.h"
#include "softbus_client_info_manager.h"
#include "softbus_error_code.h"
#include "system_ability_definition.h"

#define TEST_PID 100

using namespace testing::ext;

namespace OHOS {
class SoftbusClientInfoManagerTest : public testing::Test {
public:
    SoftbusClientInfoManagerTest()
    {}
    ~SoftbusClientInfoManagerTest()
    {}
    static void SetUpTestCase()
    {}
    static void TearDownTestCase()
    {}
    void SetUp() override
    {}
    void TearDown() override
    {}
};

class TestDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &object) override
    {
        (void)object;
    }
};

static sptr<IRemoteObject> GenerateRemoteObject(void)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr != nullptr) {
        return samgr->GetSystemAbility(SOFTBUS_SERVER_SA_ID);
    }
    return nullptr;
}

/*
 * @tc.name: SoftbusClientInfoManagerTest001
 * @tc.desc: SoftbusAddService function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest001, TestSize.Level1)
{
    int32_t pidTest = TEST_PID;
    std::string pkgName = "testPkgName";
    std::string permissionName = "";
    const sptr<IRemoteObject> object = nullptr;
    const sptr<IRemoteObject::DeathRecipient> abilityDeath = nullptr;
    int32_t ret = SoftbusClientInfoManager::GetInstance().SoftbusAddService(
        pkgName, object, abilityDeath, pidTest, permissionName);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    permissionName = "testPermissionName";
    ret = SoftbusClientInfoManager::GetInstance().SoftbusAddService(
        pkgName, object, abilityDeath, pidTest, permissionName);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    ret = SoftbusClientInfoManager::GetInstance().SoftbusAddServiceInner(pkgName,
        nullptr, pidTest);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest002
 * @tc.desc: SoftbusRemoveServiceInner function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest002, TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveServiceInner(pkgName);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    const char *sessionName = "ohos.distributedschedule.dms.test";
    ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveServiceInner(sessionName);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest003
 * @tc.desc: SoftbusRemoveService function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest003, TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveService(
        nullptr, pkgName, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest004
 * @tc.desc: GetSoftbusInnerObject function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest004, TestSize.Level1)
{
    std::string pkgName;
    int32_t ret = SoftbusClientInfoManager::GetInstance().GetSoftbusInnerObject(
        pkgName, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    ISessionListenerInner object;
    const char *sessionName = "ohos.distributedschedule.dms.test";
    ret = SoftbusClientInfoManager::GetInstance().GetSoftbusInnerObject(
        sessionName, &object);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest005
 * @tc.desc: GetSoftbusClientProxy function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest005, TestSize.Level1)
{
    const char *sessionName = "ohos.distributedschedule.dms.test";
    sptr<IRemoteObject> clientObject =
        SoftbusClientInfoManager::GetInstance().GetSoftbusClientProxy(sessionName);
    EXPECT_EQ(clientObject, nullptr);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest006
 * @tc.desc: SoftbusAddService and SoftbusRemoveService success path test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest006, TestSize.Level1)
{
    sptr<IRemoteObject> object = GenerateRemoteObject();
    ASSERT_NE(object, nullptr);
    sptr<IRemoteObject::DeathRecipient> deathRecipient = new TestDeathRecipient();
    ASSERT_NE(deathRecipient, nullptr);
    std::string pkgName = "softbus.client.info.test006";
    std::string permissionName = "ohos.permission.DISTRIBUTED_DATASYNC";

    int32_t ret = SoftbusClientInfoManager::GetInstance().SoftbusAddService(
        pkgName, object, deathRecipient, TEST_PID, permissionName);
    EXPECT_EQ(ret, SOFTBUS_OK);

    sptr<IRemoteObject> clientObject = SoftbusClientInfoManager::GetInstance().GetSoftbusClientProxy(pkgName);
    EXPECT_EQ(clientObject, object);
    clientObject = SoftbusClientInfoManager::GetInstance().GetSoftbusClientProxy(pkgName, TEST_PID);
    EXPECT_EQ(clientObject, object);
    clientObject = SoftbusClientInfoManager::GetInstance().GetSoftbusClientProxy(pkgName, TEST_PID + 1);
    EXPECT_EQ(clientObject, nullptr);

    bool isExist = SoftbusClientInfoManager::GetInstance().SoftbusClientIsExist(pkgName, TEST_PID);
    EXPECT_EQ(isExist, true);
    isExist = SoftbusClientInfoManager::GetInstance().SoftbusClientIsExist(pkgName, TEST_PID + 1);
    EXPECT_EQ(isExist, false);

    std::multimap<std::string, sptr<IRemoteObject>> clientMap;
    SoftbusClientInfoManager::GetInstance().GetSoftbusClientProxyMap(clientMap);
    EXPECT_GE(clientMap.count(pkgName), 1);

    std::string removedPkgName;
    int32_t removedPid = 0;
    ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveService(object, removedPkgName, &removedPid);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(removedPkgName, pkgName);
    EXPECT_EQ(removedPid, TEST_PID);
    clientObject = SoftbusClientInfoManager::GetInstance().GetSoftbusClientProxy(pkgName);
    EXPECT_EQ(clientObject, nullptr);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest007
 * @tc.desc: SoftbusRemoveServiceWithPid success path test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest007, TestSize.Level1)
{
    sptr<IRemoteObject> object = GenerateRemoteObject();
    ASSERT_NE(object, nullptr);
    sptr<IRemoteObject::DeathRecipient> deathRecipient = new TestDeathRecipient();
    ASSERT_NE(deathRecipient, nullptr);
    std::string pkgName = "softbus.client.info.test007";
    std::string permissionName = "ohos.permission.DISTRIBUTED_DATASYNC";

    int32_t ret = SoftbusClientInfoManager::GetInstance().SoftbusAddService(
        pkgName, object, deathRecipient, TEST_PID, permissionName);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveServiceWithPid(pkgName, TEST_PID + 1);
    EXPECT_EQ(ret, SOFTBUS_OK);
    bool isExist = SoftbusClientInfoManager::GetInstance().SoftbusClientIsExist(pkgName, TEST_PID);
    EXPECT_EQ(isExist, true);

    ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveServiceWithPid(pkgName, TEST_PID);
    EXPECT_EQ(ret, SOFTBUS_OK);
    isExist = SoftbusClientInfoManager::GetInstance().SoftbusClientIsExist(pkgName, TEST_PID);
    EXPECT_EQ(isExist, false);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest008
 * @tc.desc: SoftbusAddServiceInner and GetSoftbusInnerObject success path test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest008, TestSize.Level1)
{
    std::string pkgName = "softbus.client.info.test008";
    ISessionListenerInner listener;
    (void)memset_s(&listener, sizeof(ISessionListenerInner), 0, sizeof(ISessionListenerInner));

    int32_t ret = SoftbusClientInfoManager::GetInstance().SoftbusAddServiceInner(pkgName, &listener, TEST_PID);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ISessionListenerInner getListener;
    (void)memset_s(&getListener, sizeof(ISessionListenerInner), 0, sizeof(ISessionListenerInner));
    ret = SoftbusClientInfoManager::GetInstance().GetSoftbusInnerObject(pkgName, &getListener);
    EXPECT_EQ(ret, SOFTBUS_OK);

    ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveServiceInner(pkgName);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = SoftbusClientInfoManager::GetInstance().GetSoftbusInnerObject(pkgName, &getListener);
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: SoftbusClientInfoManagerTest009
 * @tc.desc: SoftbusAddServiceInnerForEnhance function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusClientInfoManagerTest, SoftbusClientInfoManagerTest009, TestSize.Level1)
{
    const char *pkgName = "softbus.client.info.test009";
    ISessionListenerInner listener;
    (void)memset_s(&listener, sizeof(ISessionListenerInner), 0, sizeof(ISessionListenerInner));

    int32_t ret = SoftbusAddServiceInnerForEnhance(pkgName, &listener, TEST_PID);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = SoftbusAddServiceInnerForEnhance(pkgName, nullptr, TEST_PID);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    ret = SoftbusClientInfoManager::GetInstance().SoftbusRemoveServiceInner(pkgName);
    EXPECT_EQ(ret, SOFTBUS_OK);
}
}
