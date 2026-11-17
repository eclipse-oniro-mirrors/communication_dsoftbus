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

// proxy_observer UT: core/connection/proxy/br/proxy_observer.cpp.
// The test compiles proxy_observer.cpp directly (the real implementation) and
// provides link-time mock implementations of the Bluetooth C++ APIs
// (BluetoothHost, HandsFreeAudioGateway, BluetoothRemoteDevice) and the dsoftbus
// utility functions (SoftBusGenerateStrHash, ConvertBytesToHexString) via
// proxy_observer_mock.cpp. This allows full branch coverage of every function
// in the translation unit.

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "mock/proxy_observer_mock.h"
#include "proxy_observer.h"

#include "bluetooth_def.h"
#include "conn_log.h"
#include "softbus_common.h"
#include "softbus_error_code.h"

using namespace testing::ext;
using testing::Return;
using testing::Invoke;

namespace {
constexpr const char *TEST_MAC_A = "11:22:33:44:55:66";
constexpr const char *TEST_MAC_B = "AA:BB:CC:DD:EE:FF";
constexpr const char *TEST_MAC_C = "00:11:22:33:44:55";
constexpr const char *HFP_HF_UUID = "0000111E-0000-1000-8000-00805F9B34FB";

// Captured listener invocation state.
std::string g_listenerAddr;
int32_t g_listenerState = 0;
int32_t g_listenerCallCount = 0;

void ResetListenerState()
{
    g_listenerAddr.clear();
    g_listenerState = 0;
    g_listenerCallCount = 0;
}

void TestProxyListener(const char *addr, int32_t state)
{
    g_listenerCallCount++;
    if (addr != nullptr) {
        g_listenerAddr = addr;
    }
    g_listenerState = state;
}

// Build a BluetoothRemoteDevice for passing to observer callbacks.
OHOS::Bluetooth::BluetoothRemoteDevice MakeRemoteDevice(const std::string &addr)
{
    return OHOS::Bluetooth::BluetoothRemoteDevice(addr, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
}

// Build a vector of mock paired devices.
std::vector<MockBtDevice> MakeMockDeviceList()
{
    std::vector<MockBtDevice> devices;
    MockBtDevice devA;
    devA.address = TEST_MAC_A;
    devA.pairState = OHOS::Bluetooth::PAIR_PAIRED;
    devA.isAclConnected = true;
    devA.uuids = {HFP_HF_UUID};
    devices.push_back(devA);

    MockBtDevice devB;
    devB.address = TEST_MAC_B;
    devB.pairState = OHOS::Bluetooth::PAIR_PAIRED;
    devB.isAclConnected = false;
    devB.uuids = {}; // no HFP support
    devices.push_back(devB);

    MockBtDevice devC;
    devC.address = TEST_MAC_C;
    devC.pairState = OHOS::Bluetooth::PAIR_PAIRING; // not paired yet
    devC.isAclConnected = false;
    devC.uuids = {HFP_HF_UUID};
    devices.push_back(devC);
    return devices;
}
} // namespace

class ProxyObserverTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        ProxyObserverBtMock::Reset();
        ResetListenerState();
        utilMock_ = std::make_unique<ProxyObserverUtilMock>();
        EXPECT_CALL(*utilMock_, SoftBusGenerateStrHash)
            .WillRepeatedly(ProxyObserverUtilMock::ActionOfSoftBusGenerateStrHash);
        EXPECT_CALL(*utilMock_, ConvertBytesToHexString)
            .WillRepeatedly(ProxyObserverUtilMock::ActionOfConvertBytesToHexString);
        ProxyObserverBtMock::SetMockDevices(MakeMockDeviceList());
    }

    void TearDown() override
    {
        utilMock_.reset();
        ProxyObserverBtMock::Reset();
        ResetListenerState();
    }

protected:
    std::unique_ptr<ProxyObserverUtilMock> utilMock_;
};

// ===========================================================================
// RegisterHfpListener
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest001
 * @tc.desc: RegisterHfpListener with null listener returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest001, TestSize.Level1)
{
    int32_t ret = RegisterHfpListener(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    // observers should not have been registered
    EXPECT_EQ(ProxyObserverBtMock::GetHfpObserver(), nullptr);
    EXPECT_EQ(ProxyObserverBtMock::GetRemoteDeviceObserver(), nullptr);
}

/*
 * @tc.name: ProxyObserverTest002
 * @tc.desc: RegisterHfpListener with valid listener returns SOFTBUS_OK and registers both observers
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest002, TestSize.Level1)
{
    int32_t ret = RegisterHfpListener(TestProxyListener);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_NE(ProxyObserverBtMock::GetHfpObserver(), nullptr);
    EXPECT_NE(ProxyObserverBtMock::GetRemoteDeviceObserver(), nullptr);
}

// ===========================================================================
// ProxyObserver::OnConnectionStateChanged (via captured HFP observer)
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest003
 * @tc.desc: OnConnectionStateChanged with CONNECTED state invokes listener with SOFTBUS_HFP_CONNECTED
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest003, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetHfpObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnConnectionStateChanged(device,
        static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::CONNECTED), 0);
    EXPECT_EQ(g_listenerCallCount, 1);
    EXPECT_EQ(g_listenerState, SOFTBUS_HFP_CONNECTED);
    EXPECT_EQ(g_listenerAddr, TEST_MAC_A);
}

/*
 * @tc.name: ProxyObserverTest004
 * @tc.desc: OnConnectionStateChanged with CONNECTING state does not invoke listener
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest004, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetHfpObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnConnectionStateChanged(device,
        static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::CONNECTING), 0);
    EXPECT_EQ(g_listenerCallCount, 0);
}

/*
 * @tc.name: ProxyObserverTest005
 * @tc.desc: OnConnectionStateChanged with DISCONNECTED state does not invoke listener
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest005, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetHfpObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnConnectionStateChanged(device,
        static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::DISCONNECTED), 0);
    EXPECT_EQ(g_listenerCallCount, 0);
}

/*
 * @tc.name: ProxyObserverTest006
 * @tc.desc: OnConnectionStateChanged with DISCONNECTING state does not invoke listener
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest006, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetHfpObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnConnectionStateChanged(device,
        static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::DISCONNECTING), 0);
    EXPECT_EQ(g_listenerCallCount, 0);
}

// ===========================================================================
// ProxyPairStatusObserver::OnPairStatusChanged (via captured remote device observer)
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest007
 * @tc.desc: OnPairStatusChanged with PAIR_NONE invokes listener with SOFTBUS_DEVICE_UNPAIRED
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest007, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetRemoteDeviceObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnPairStatusChanged(device, OHOS::Bluetooth::PAIR_NONE, 0);
    EXPECT_EQ(g_listenerCallCount, 1);
    EXPECT_EQ(g_listenerState, SOFTBUS_DEVICE_UNPAIRED);
    EXPECT_EQ(g_listenerAddr, TEST_MAC_A);
}

/*
 * @tc.name: ProxyObserverTest008
 * @tc.desc: OnPairStatusChanged with PAIR_PAIRING does not invoke listener
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest008, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetRemoteDeviceObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnPairStatusChanged(device, OHOS::Bluetooth::PAIR_PAIRING, 0);
    EXPECT_EQ(g_listenerCallCount, 0);
}

/*
 * @tc.name: ProxyObserverTest009
 * @tc.desc: OnPairStatusChanged with PAIR_PAIRED does not invoke listener
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest009, TestSize.Level1)
{
    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer = ProxyObserverBtMock::GetRemoteDeviceObserver();
    ASSERT_NE(observer, nullptr);

    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer->OnPairStatusChanged(device, OHOS::Bluetooth::PAIR_PAIRED, 0);
    EXPECT_EQ(g_listenerCallCount, 0);
}

// ===========================================================================
// IsPairedDevice — null / failure / empty branches
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest010
 * @tc.desc: IsPairedDevice with null addr returns false
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest010, TestSize.Level1)
{
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(nullptr, true, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ProxyObserverTest011
 * @tc.desc: IsPairedDevice returns false when GetPairedDevices fails
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest011, TestSize.Level1)
{
    ProxyObserverBtMock::SetGetPairedDevicesResult(-1);
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ProxyObserverTest012
 * @tc.desc: IsPairedDevice returns false when the paired device list is empty
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest012, TestSize.Level1)
{
    ProxyObserverBtMock::SetMockDevices({});
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

// ===========================================================================
// IsPairedDevice — isRealMac=true matching branches
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest013
 * @tc.desc: IsPairedDevice with isRealMac=true, matching paired device, sets isSupportHfp & isAclConnected
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest013, TestSize.Level1)
{
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(isSupportHfp);   // device A has HFP_HF uuid
    EXPECT_TRUE(isAclConnected); // device A acl connected
}

/*
 * @tc.name: ProxyObserverTest014
 * @tc.desc: IsPairedDevice with isRealMac=true, matching paired device without HFP uuid
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest014, TestSize.Level1)
{
    bool isSupportHfp = true;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_B, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(isSupportHfp);  // device B has no HFP uuid
    EXPECT_FALSE(isAclConnected); // device B acl not connected
}

/*
 * @tc.name: ProxyObserverTest015
 * @tc.desc: IsPairedDevice with isRealMac=true, device present but not PAIRED → false
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest015, TestSize.Level1)
{
    // device C is in PAIR_PAIRING state
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_C, true, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ProxyObserverTest016
 * @tc.desc: IsPairedDevice with isRealMac=true, address not in list → false
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest016, TestSize.Level1)
{
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice("99:88:77:66:55:44", true, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ProxyObserverTest017
 * @tc.desc: IsPairedDevice with isSupportHfp=nullptr, isAclConnected=nullptr still returns true
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest017, TestSize.Level1)
{
    bool ret = IsPairedDevice(TEST_MAC_A, true, nullptr, nullptr);
    EXPECT_TRUE(ret);
}

// ===========================================================================
// IsPairedDevice — isRealMac=false (hash mac) matching branches
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest018
 * @tc.desc: IsPairedDevice with isRealMac=false, hash matching a paired device → true
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest018, TestSize.Level1)
{
    std::string hashMac = ProxyObserverUtilMock::ComputeHashMac(TEST_MAC_A);
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(hashMac.c_str(), false, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(isSupportHfp);
    EXPECT_TRUE(isAclConnected);
}

/*
 * @tc.name: ProxyObserverTest019
 * @tc.desc: IsPairedDevice with isRealMac=false, hash not matching any device → false
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest019, TestSize.Level1)
{
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice("deadbeefdeadbeefdeadbeefdeadbeef", false, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ProxyObserverTest020
 * @tc.desc: IsPairedDevice with isRealMac=false, hash matches device that is not PAIRED → false
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest020, TestSize.Level1)
{
    std::string hashMac = ProxyObserverUtilMock::ComputeHashMac(TEST_MAC_C); // device C is PAIR_PAIRING
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(hashMac.c_str(), false, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

// ===========================================================================
// IsPairedDevice — case-insensitive matching
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest021
 * @tc.desc: IsPairedDevice matches address case-insensitively (lowercase query)
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest021, TestSize.Level1)
{
    // TEST_MAC_B is "AA:BB:CC:DD:EE:FF" — query with lowercase
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice("aa:bb:cc:dd:ee:ff", true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
}

// ===========================================================================
// IsPairedDevice — multiple devices, verify loop iterates correctly
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest022
 * @tc.desc: IsPairedDevice iterates past non-matching devices to find the matching one
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest022, TestSize.Level1)
{
    // Device B is second in the list; device A (first) does not match the queried address.
    bool isSupportHfp = true;
    bool isAclConnected = true;
    bool ret = IsPairedDevice(TEST_MAC_B, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(isSupportHfp);  // device B has no HFP
    EXPECT_FALSE(isAclConnected); // device B acl not connected
}

// ===========================================================================
// GetRealMac — null parameter branches
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest023
 * @tc.desc: GetRealMac with null realAddr returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest023, TestSize.Level1)
{
    int32_t ret = GetRealMac(nullptr, BT_MAC_LEN, "somhash");
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: ProxyObserverTest024
 * @tc.desc: GetRealMac with null hashAddr returns SOFTBUS_INVALID_PARAM
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest024, TestSize.Level1)
{
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

// ===========================================================================
// GetRealMac — failure / not-found branches
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest025
 * @tc.desc: GetRealMac returns SOFTBUS_CONN_PROXY_INTERNAL_ERR when GetPairedDevices fails
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest025, TestSize.Level1)
{
    ProxyObserverBtMock::SetGetPairedDevicesResult(-1);
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, "somehash");
    EXPECT_EQ(ret, SOFTBUS_CONN_PROXY_INTERNAL_ERR);
}

/*
 * @tc.name: ProxyObserverTest026
 * @tc.desc: GetRealMac returns SOFTBUS_NOT_FIND when no device matches the hash
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest026, TestSize.Level1)
{
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, "nonexistenthashstring1234567890123456");
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: ProxyObserverTest027
 * @tc.desc: GetRealMac returns SOFTBUS_NOT_FIND when matching device is not PAIRED
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest027, TestSize.Level1)
{
    // device C is in PAIR_PAIRING state
    std::string hashMac = ProxyObserverUtilMock::ComputeHashMac(TEST_MAC_C);
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, hashMac.c_str());
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

/*
 * @tc.name: ProxyObserverTest028
 * @tc.desc: GetRealMac returns SOFTBUS_NOT_FIND when device list is empty
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest028, TestSize.Level1)
{
    ProxyObserverBtMock::SetMockDevices({});
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, "somehash");
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

// ===========================================================================
// GetRealMac — success branch
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest029
 * @tc.desc: GetRealMac succeeds and copies the real MAC address when hash matches a paired device
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest029, TestSize.Level1)
{
    std::string hashMac = ProxyObserverUtilMock::ComputeHashMac(TEST_MAC_A);
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, hashMac.c_str());
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_STREQ(realAddr, TEST_MAC_A);
}

/*
 * @tc.name: ProxyObserverTest030
 * @tc.desc: GetRealMac succeeds for the second device in the list (loop iteration)
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest030, TestSize.Level1)
{
    std::string hashMac = ProxyObserverUtilMock::ComputeHashMac(TEST_MAC_B);
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, hashMac.c_str());
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_STREQ(realAddr, TEST_MAC_B);
}

// ===========================================================================
// ConvertRealMacToHashMac failure paths (tested indirectly via IsPairedDevice/GetRealMac)
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest032
 * @tc.desc: ConvertRealMacToHashMac returns empty string when SoftBusGenerateStrHash fails;
 *          IsPairedDevice with isRealMac=false then cannot match → false
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest032, TestSize.Level1)
{
    EXPECT_CALL(*utilMock_, SoftBusGenerateStrHash).WillRepeatedly(Return(SOFTBUS_ERR));
    // With hash generation failing, ConvertRealMacToHashMac returns "".
    // StrCmpIgnoreCase("", hashMac) != 0, so no device matches.
    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice("somehashvalue", false, &isSupportHfp, &isAclConnected);
    EXPECT_FALSE(ret);
}

/*
 * @tc.name: ProxyObserverTest033
 * @tc.desc: ConvertRealMacToHashMac returns empty string when ConvertBytesToHexString fails;
 *          GetRealMac cannot match → SOFTBUS_NOT_FIND
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest033, TestSize.Level1)
{
    EXPECT_CALL(*utilMock_, ConvertBytesToHexString).WillRepeatedly(Return(SOFTBUS_ERR));
    char realAddr[BT_MAC_LEN] = {0};
    int32_t ret = GetRealMac(realAddr, BT_MAC_LEN, "somehash");
    EXPECT_EQ(ret, SOFTBUS_NOT_FIND);
}

// ===========================================================================
// IsSupportHfp (tested indirectly via IsPairedDevice) — UUID matching
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest034
 * @tc.desc: IsSupportHfp returns true when device uuid list contains HFP_HF uuid (case-insensitive)
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest034, TestSize.Level1)
{
    // Build a device list with a device that has the HFP uuid in different case.
    std::vector<MockBtDevice> devices;
    MockBtDevice dev;
    dev.address = TEST_MAC_A;
    dev.pairState = OHOS::Bluetooth::PAIR_PAIRED;
    dev.isAclConnected = true;
    dev.uuids = {"0000111e-0000-1000-8000-00805f9b34fb"}; // lowercase HFP_HF uuid
    devices.push_back(dev);
    ProxyObserverBtMock::SetMockDevices(devices);

    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(isSupportHfp);
}

/*
 * @tc.name: ProxyObserverTest035
 * @tc.desc: IsSupportHfp returns false when device uuid list does not contain HFP_HF uuid
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest035, TestSize.Level1)
{
    std::vector<MockBtDevice> devices;
    MockBtDevice dev;
    dev.address = TEST_MAC_A;
    dev.pairState = OHOS::Bluetooth::PAIR_PAIRED;
    dev.isAclConnected = true;
    dev.uuids = {"0000110B-0000-1000-8000-00805F9B34FB"}; // A2DP_SINK, not HFP
    devices.push_back(dev);
    ProxyObserverBtMock::SetMockDevices(devices);

    bool isSupportHfp = true;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(isSupportHfp);
}

/*
 * @tc.name: ProxyObserverTest036
 * @tc.desc: IsSupportHfp returns false when device has multiple uuids but none is HFP_HF
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest036, TestSize.Level1)
{
    std::vector<MockBtDevice> devices;
    MockBtDevice dev;
    dev.address = TEST_MAC_A;
    dev.pairState = OHOS::Bluetooth::PAIR_PAIRED;
    dev.isAclConnected = true;
    dev.uuids = {
        "0000110A-0000-1000-8000-00805F9B34FB", // A2DP_SRC
        "0000110E-0000-1000-8000-00805F9B34FB", // AVRCP_CT
        "00001101-0000-1000-8000-00805F9B34FB", // SPP
    };
    devices.push_back(dev);
    ProxyObserverBtMock::SetMockDevices(devices);

    bool isSupportHfp = true;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(isSupportHfp);
}

/*
 * @tc.name: ProxyObserverTest037
 * @tc.desc: IsSupportHfp returns true when device has multiple uuids including HFP_HF
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest037, TestSize.Level1)
{
    std::vector<MockBtDevice> devices;
    MockBtDevice dev;
    dev.address = TEST_MAC_A;
    dev.pairState = OHOS::Bluetooth::PAIR_PAIRED;
    dev.isAclConnected = true;
    dev.uuids = {
        "0000110A-0000-1000-8000-00805F9B34FB", // A2DP_SRC
        OHOS::Bluetooth::BLUETOOTH_UUID_HFP_HF,   // HFP_HF
        "00001101-0000-1000-8000-00805F9B34FB", // SPP
    };
    devices.push_back(dev);
    ProxyObserverBtMock::SetMockDevices(devices);

    bool isSupportHfp = false;
    bool isAclConnected = false;
    bool ret = IsPairedDevice(TEST_MAC_A, true, &isSupportHfp, &isAclConnected);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(isSupportHfp);
}

// ===========================================================================
// Multiple RegisterHfpListener calls — observer replacement
// ===========================================================================

/*
 * @tc.name: ProxyObserverTest038
 * @tc.desc: Calling RegisterHfpListener twice replaces the captured observer
 * @tc.type: FUNC
 */
HWTEST_F(ProxyObserverTest, ProxyObserverTest038, TestSize.Level1)
{
    void (*firstListener)(const char *, int32_t) = [](const char *addr, int32_t state) {
        g_listenerCallCount++;
        g_listenerState = state + 100; // distinguish from second listener
    };
    ASSERT_EQ(RegisterHfpListener(firstListener), SOFTBUS_OK);
    auto observer1 = ProxyObserverBtMock::GetHfpObserver();
    ASSERT_NE(observer1, nullptr);

    ASSERT_EQ(RegisterHfpListener(TestProxyListener), SOFTBUS_OK);
    auto observer2 = ProxyObserverBtMock::GetHfpObserver();
    ASSERT_NE(observer2, nullptr);

    // The second call should have replaced the observer.
    auto device = MakeRemoteDevice(TEST_MAC_A);
    observer2->OnConnectionStateChanged(device,
        static_cast<int32_t>(OHOS::Bluetooth::BTConnectState::CONNECTED), 0);
    EXPECT_EQ(g_listenerState, SOFTBUS_HFP_CONNECTED); // set by TestProxyListener
}
