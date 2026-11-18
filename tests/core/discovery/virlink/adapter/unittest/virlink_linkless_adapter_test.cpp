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

#include <cstdint>
#include <cstdio>
#include <ctime>
#include <gtest/gtest.h>
#include <securec.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>

#include "disc_log.h"
#include "disc_virLink_adapter.h"
#include "virlink_linkless_adapter_mock.h"
#include "softbus_error_code.h"

using namespace testing::ext;

namespace OHOS {
#define VIRLINK_TEST_MAC_STR_LEN 128
#define VIRLINK_TEST_IP_STR_LEN 128
#define VIRLINK_TEST_UUID_LEN 128

struct VirlinkTestConn {
    char remoteMac[VIRLINK_TEST_MAC_STR_LEN];
    char remoteIp[VIRLINK_TEST_IP_STR_LEN];
    char remoteNetworkId[VIRLINK_TEST_UUID_LEN];
    char localIp[VIRLINK_TEST_IP_STR_LEN];
    bool isSource;
};

struct VirlinkTestConn g_virlinkTestConn;

static void VirlinkOnDeviceOnlineMock(const char *remoteMac, const char *remoteIp,
    const char *remoteNetworkId, bool isSource)
{
    if (strcpy_s(g_virlinkTestConn.remoteMac, sizeof(g_virlinkTestConn.remoteMac), remoteMac) != EOK) {
        return;
    }

    if (strcpy_s(g_virlinkTestConn.remoteIp, sizeof(g_virlinkTestConn.remoteIp), remoteIp) != EOK) {
        return;
    }

    if (strcpy_s(g_virlinkTestConn.remoteNetworkId, sizeof(g_virlinkTestConn.remoteNetworkId),
        remoteNetworkId) != EOK) {
        return;
    }

    g_virlinkTestConn.isSource = isSource;
}

static void VirlinkOnDeviceOfflineMock(const char *remoteMac, const char *remoteIp,
    const char *remoteNetworkId, const char *localIp)
{
    if (strcpy_s(g_virlinkTestConn.remoteMac, sizeof(g_virlinkTestConn.remoteMac), remoteMac) != EOK) {
        return;
    }

    if (strcpy_s(g_virlinkTestConn.remoteIp, sizeof(g_virlinkTestConn.remoteIp), remoteIp) != EOK) {
        return;
    }

    if (strcpy_s(g_virlinkTestConn.remoteNetworkId, sizeof(g_virlinkTestConn.remoteNetworkId),
        remoteNetworkId) != EOK) {
        return;
    }

    if (strcpy_s(g_virlinkTestConn.localIp, sizeof(g_virlinkTestConn.localIp), localIp) != EOK) {
        return;
    }
}

#define VIRLINK_TEST_BUF_SIZE 2048
static char g_virlinkTestNetworkId[NETWORK_ID_BUF_LEN];
static uint8_t g_virlinkTestRecvBuf[VIRLINK_TEST_BUF_SIZE];
static uint32_t g_virlinkTestRecvBufLen = 0;

void DiscVirlinkLinklessRecvCbMock(const char *networkId, const uint8_t *data, uint32_t dataLen)
{
    if (strcpy_s(g_virlinkTestNetworkId, sizeof(g_virlinkTestNetworkId), networkId) != EOK) {
        return;
    }

    if (memcpy_s(g_virlinkTestRecvBuf, sizeof(g_virlinkTestRecvBuf), data, dataLen) != EOK) {
        return;
    }
    g_virlinkTestRecvBufLen = dataLen;
}

class VirlinkLinklessAdapterTest : public testing::Test {
public:
    VirlinkLinklessAdapterTest() { }
    ~VirlinkLinklessAdapterTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override { }
    void TearDown() override { }
};

void VirlinkLinklessAdapterTest::SetUpTestCase(void)
{
    struct DiscVirlinkConnStatusListener l = {
        VirlinkOnDeviceOnlineMock,
        VirlinkOnDeviceOfflineMock,
    };
    DiscVirlinkLinklessRegisterListener(&l);

    DiscVirlinkLinklessRegisterRecvCallback(DiscVirlinkLinklessRecvCbMock);
}

void VirlinkLinklessAdapterTest::TearDownTestCase(void) { }

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest001
 * @tc.desc: Test send function
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest001, TestSize.Level1)
{
    std::string networkId = "123";
    uint8_t data[VIRLINK_TEST_BUF_SIZE] = { 1, 2, 3 };
    int32_t ret = DiscVirlinkLinklessVirtualSend(networkId.c_str(), data, 1);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest002
 * @tc.desc: Test recv function
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest002, TestSize.Level1)
{
    uint8_t data[VIRLINK_TEST_BUF_SIZE] = { 1, 2, 3 };
    VirlinkTestRecv(data, 1);
    EXPECT_EQ(g_virlinkTestRecvBuf[0], data[0]);
    EXPECT_EQ(g_virlinkTestRecvBufLen, 1);

    VirlinkTestAuthClose();
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest003
 * @tc.desc: Test online function
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest003, TestSize.Level1)
{
    std::string remoteMac = "12:34:56:78:90:ab";
    std::string remoteIp = "172.32.1.1";
    std::string remoteUuid = "uuu";
    bool isSource = true;
    VirlinkTestDeviceOnline(remoteMac.c_str(), remoteIp.c_str(), remoteUuid.c_str(), isSource);
    EXPECT_EQ(g_virlinkTestConn.remoteMac, remoteMac);
    EXPECT_EQ(g_virlinkTestConn.remoteIp, remoteIp);
    EXPECT_EQ(g_virlinkTestConn.isSource, isSource);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest004
 * @tc.desc: Test offline function
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest004, TestSize.Level1)
{
    (void)memset_s(&g_virlinkTestConn, sizeof(g_virlinkTestConn), 0, sizeof(g_virlinkTestConn));
    std::string remoteMac = "12:34:56:78:90:ab";
    std::string remoteIp = "172.32.1.1";
    std::string remoteUuid = "uuu";
    std::string localIp = "172.32.1.2";
    VirlinkTestDeviceOffline(remoteMac.c_str(), remoteIp.c_str(), remoteUuid.c_str(), localIp.c_str());
    EXPECT_EQ(g_virlinkTestConn.remoteMac, remoteMac);
    EXPECT_EQ(g_virlinkTestConn.remoteIp, remoteIp);
    EXPECT_EQ(g_virlinkTestConn.localIp, localIp);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest005
 * @tc.desc: Test send function when no valid wifidirect authHandle found,
 *           VirLinkLinklessGetWifiDirectAuthByNetworkId returns SOFTBUS_INVALID_PARAM,
 *           covering the "get authHandle fail" branch in DiscVirlinkLinklessVirtualSend
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest005, TestSize.Level1)
{
    std::string networkId = "123";
    uint8_t data[VIRLINK_TEST_BUF_SIZE] = { 1, 2, 3 };
    VirlinkTestSetAuthIdValid(false);
    int32_t ret = DiscVirlinkLinklessVirtualSend(networkId.c_str(), data, 1);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    VirlinkTestSetAuthIdValid(true);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest006
 * @tc.desc: Test send function with invalid params,
 *           covering the "invalid param" branch in DiscVirlinkLinklessVirtualSend
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest006, TestSize.Level1)
{
    std::string networkId = "123";
    uint8_t data[VIRLINK_TEST_BUF_SIZE] = { 1, 2, 3 };
    int32_t ret = DiscVirlinkLinklessVirtualSend(nullptr, data, 1);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    ret = DiscVirlinkLinklessVirtualSend(networkId.c_str(), nullptr, 1);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    ret = DiscVirlinkLinklessVirtualSend(networkId.c_str(), data, 0);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest007
 * @tc.desc: Test recv function when LnnGetNetworkIdByUdid fails,
 *           covering the "LnnGetNetworkIdByUdid fail" branch in OnVirlinkLinklessMsgRecv
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest007, TestSize.Level1)
{
    uint8_t data[VIRLINK_TEST_BUF_SIZE] = { 1, 2, 3 };
    g_virlinkTestRecvBufLen = 0;
    VirlinkTestSetNetworkIdByUdidValid(false);
    VirlinkTestRecv(data, 1);
    EXPECT_EQ(g_virlinkTestRecvBufLen, 0);
    VirlinkTestSetNetworkIdByUdidValid(true);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest008
 * @tc.desc: Test online function with invalid params,
 *           covering the "invalid param" branch in VirlinkLinklessOnDeviceOnline
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest008, TestSize.Level1)
{
    (void)memset_s(&g_virlinkTestConn, sizeof(g_virlinkTestConn), 0, sizeof(g_virlinkTestConn));
    std::string remoteMac = "12:34:56:78:90:ab";
    std::string remoteIp = "172.32.1.1";
    std::string remoteUuid = "uuu";
    bool isSource = true;
    VirlinkTestDeviceOnline(nullptr, remoteIp.c_str(), remoteUuid.c_str(), isSource);
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');

    VirlinkTestDeviceOnline(remoteMac.c_str(), nullptr, remoteUuid.c_str(), isSource);
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');

    VirlinkTestDeviceOnline(remoteMac.c_str(), remoteIp.c_str(), nullptr, isSource);
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest009
 * @tc.desc: Test online function when LnnGetNetworkIdByUuid fails,
 *           covering the "LnnGetNetworkIdByUuid fail" branch in VirlinkLinklessOnDeviceOnline
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest009, TestSize.Level1)
{
    (void)memset_s(&g_virlinkTestConn, sizeof(g_virlinkTestConn), 0, sizeof(g_virlinkTestConn));
    std::string remoteMac = "12:34:56:78:90:ab";
    std::string remoteIp = "172.32.1.1";
    std::string remoteUuid = "uuu";
    bool isSource = true;
    VirlinkTestSetNetworkIdByUuidValid(false);
    VirlinkTestDeviceOnline(remoteMac.c_str(), remoteIp.c_str(), remoteUuid.c_str(), isSource);
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');
    VirlinkTestSetNetworkIdByUuidValid(true);
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest010
 * @tc.desc: Test offline function with invalid params,
 *           covering the "invalid param" branch in VirlinkLinklessOnDeviceOffline
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest010, TestSize.Level1)
{
    (void)memset_s(&g_virlinkTestConn, sizeof(g_virlinkTestConn), 0, sizeof(g_virlinkTestConn));
    std::string remoteMac = "12:34:56:78:90:ab";
    std::string remoteIp = "172.32.1.1";
    std::string remoteUuid = "uuu";
    std::string localIp = "172.32.1.2";
    VirlinkTestDeviceOffline(nullptr, remoteIp.c_str(), remoteUuid.c_str(), localIp.c_str());
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');

    VirlinkTestDeviceOffline(remoteMac.c_str(), nullptr, remoteUuid.c_str(), localIp.c_str());
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');

    VirlinkTestDeviceOffline(remoteMac.c_str(), remoteIp.c_str(), nullptr, localIp.c_str());
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');

    VirlinkTestDeviceOffline(remoteMac.c_str(), remoteIp.c_str(), remoteUuid.c_str(), nullptr);
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');
}

/*
 * @tc.name: DiscVirlinkLinklessAdapterTest011
 * @tc.desc: Test offline function when LnnGetNetworkIdByUuid fails,
 *           covering the "LnnGetNetworkIdByUuid fail" branch in VirlinkLinklessOnDeviceOffline
 * @tc.type: FUNC
 * @tc.require: The DiscVirlinkLinklessAdapter operates normally
 */
HWTEST_F(VirlinkLinklessAdapterTest, DiscVirlinkLinklessAdapterTest011, TestSize.Level1)
{
    (void)memset_s(&g_virlinkTestConn, sizeof(g_virlinkTestConn), 0, sizeof(g_virlinkTestConn));
    std::string remoteMac = "12:34:56:78:90:ab";
    std::string remoteIp = "172.32.1.1";
    std::string remoteUuid = "uuu";
    std::string localIp = "172.32.1.2";
    VirlinkTestSetNetworkIdByUuidValid(false);
    VirlinkTestDeviceOffline(remoteMac.c_str(), remoteIp.c_str(), remoteUuid.c_str(), localIp.c_str());
    EXPECT_EQ(g_virlinkTestConn.remoteMac[0], '\0');
    VirlinkTestSetNetworkIdByUuidValid(true);
}

} // namespace OHOS
