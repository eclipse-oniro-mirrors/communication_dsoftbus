/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define private   public
#define protected public
#include "dfx/duration_statistic.h"
#include "dfx/wifi_direct_dfx.h"
#undef protected
#undef private

#include <functional>
#include <gtest/gtest.h>
#include <iostream>
#include <securec.h>
#include <string>
#include "data/link_manager.h"
#include "net_conn_client.h"
#include "wifi_device.h"
#include "wifi_direct_anonymous.h"
#include "wifi_direct_mock.h"
#include "wifi_direct_utils.h"
#include "g_enhance_lnn_func.h"
#include "dsoftbus_enhance_interface.h"

#define WIFI_DEVICE_ABILITY_ID 1120
using namespace testing::ext;
using testing::_;
using ::testing::Return;

constexpr int32_t FREQ1 = 1500;
constexpr int32_t FREQ2 = 2437;
constexpr int32_t FREQ3 = 5300;
constexpr int32_t FREQ4 = 5745;
constexpr int32_t CHAN1 = 6;
constexpr int32_t CHAN2 = 149;
constexpr int32_t INVALID_CHANNEL = -1;

namespace OHOS::SoftBus {
class WifiDirectUtilsTest : public testing::Test {
public:
    static void SetUpTestCase() { }
    static void TearDownTestCase() { }
    void SetUp() override { }
    void TearDown() override { }
};

/*
 * @tc.name: BytesToIntTest
 * @tc.desc: check BytesToInt method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntTest, TestSize.Level1)
{
    std::vector<uint8_t> data = { 58, 76 };
    auto ret = WifiDirectUtils::BytesToInt(data);
    EXPECT_EQ(ret, 19514);
}

/*
 * @tc.name: ToBinary
 * @tc.desc: check ToBinary method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToBinaryTest, TestSize.Level1)
{
    std::string input("010203040506070809");
    std::vector<uint8_t> expected = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    auto ret = WifiDirectUtils::ToBinary(input);
    EXPECT_EQ(ret, expected);

    input = "zzzz";
    std::vector<uint8_t> expected2 = { '\0', '\0' };
    ret = WifiDirectUtils::ToBinary(input);
    EXPECT_EQ(ret, expected2);
}

/*
 * @tc.name: ChannelToFrequencyTest
 * @tc.desc: check ChannelToFrequency method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelToFrequencyTest, TestSize.Level1)
{
    int32_t channel = 36;
    int32_t frequency = 5180;
    auto ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, frequency);

    channel = -1;
    frequency = -1;
    ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, frequency);

    channel = CHAN1;
    frequency = FREQ2;
    ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, frequency);
}

/*
 * @tc.name: FrequencyToChannelTest
 * @tc.desc: check FrequencyToChannel method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, FrequencyToChannelTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::FrequencyToChannel(FREQ2);
    EXPECT_EQ(ret, CHAN1);
    ret = WifiDirectUtils::FrequencyToChannel(FREQ4);
    EXPECT_EQ(ret, CHAN2);
    ret = WifiDirectUtils::FrequencyToChannel(FREQ1);
    EXPECT_EQ(ret, INVALID_CHANNEL);
}

/*
 * @tc.name: UuidToNetworkIdTest
 * @tc.desc: check UuidToNetworkId method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, UuidToNetworkIdTest, TestSize.Level1)
{
    std::string uuid = "0123456789ABCDEF";
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetNetworkIdByUuid(_, _, _))
        .WillRepeatedly([&networkId](const std::string &uuid, char *buf, uint32_t len) {
            (void)strcpy_s(buf, len, networkId);
            return SOFTBUS_OK;
        });

    auto ret = WifiDirectUtils::UuidToNetworkId(uuid);
    EXPECT_EQ(ret, networkId);
}

/*
 * @tc.name: GetLocalPtkTest
 * @tc.desc: check GetLocalPtk method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalPtkTest, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnGetLocalDefaultPtkByUuid = LnnGetLocalDefaultPtkByUuid;
    pfnLnnEnhanceFuncList->lnnGetLocalPtkByUuid = LnnGetLocalPtkByUuid;
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    std::string remoteDeviceId = "01234567890ABCDEF";
    char ptkBytes[PTK_DEFAULT_LEN] = { 3 };
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetLocalPtkByUuid).WillRepeatedly(Return(SOFTBUS_NOT_FIND));
    EXPECT_CALL(mock, LnnGetLocalDefaultPtkByUuid(_, _, _))
        .WillRepeatedly([&ptkBytes](const std::string &uuid, char *localPtk, uint32_t len) {
            (void)strcpy_s(localPtk, len, ptkBytes);
            return SOFTBUS_OK;
        });
    WifiDirectUtils::GetLocalPtk(networkId);

    EXPECT_CALL(mock, LnnGetLocalPtkByUuid).WillRepeatedly(Return(SOFTBUS_OK));
    WifiDirectUtils::GetLocalPtk(networkId);
}

/*
 * @tc.name: GetRemotePtkTest
 * @tc.desc: check GetRemotePtk method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemotePtkTest, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnGetRemoteDefaultPtkByUuid = LnnGetRemoteDefaultPtkByUuid;
    char remoteNetworkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteByteInfo).WillOnce(Return(-1));
    auto ret = WifiDirectUtils::GetRemotePtk(remoteNetworkId);
    EXPECT_EQ(ret.size(), 0);

    EXPECT_CALL(mock, LnnGetRemoteByteInfo).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteDefaultPtkByUuid).WillRepeatedly(Return(SOFTBUS_OK));
    WifiDirectUtils::GetRemotePtk(remoteNetworkId);
}

/*
 * @tc.name: IsRemoteSupportTlvTest
 * @tc.desc: check IsRemoteSupportTlv method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsRemoteSupportTlvTest, TestSize.Level1)
{
    std::string remoteDeviceId = "01234567890ABCDEF";
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetNetworkIdByUuid).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteBoolInfoIgnoreOnline(_, _, _))
        .WillRepeatedly([](const std::string &networkId, InfoKey key, bool *info) {
            *info = true;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::IsRemoteSupportTlv(remoteDeviceId);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: IsInChannelListTest
 * @tc.desc: check IsInChannelList method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsInChannelListTest, TestSize.Level1)
{
    std::string channels = "";
    auto ret = WifiDirectUtils::StringToChannelList(channels);
    EXPECT_EQ(ret.empty(), true);

    channels = "36##37";
    ret = WifiDirectUtils::StringToChannelList(channels);
    EXPECT_EQ(ret.empty(), false);
}

/*
 * @tc.name: StringToChannelListTest
 * @tc.desc: check StringToChannelList method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToChannelListTest, TestSize.Level1)
{
    int32_t channel = 36;
    std::vector<int> channelArray = { 35, 36, 37 };
    auto ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: ToWifiDirectRoleTest
 * @tc.desc: check ToWifiDirectRole method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToWifiDirectRoleTest, TestSize.Level1)
{
    LinkInfo::LinkMode mode = LinkInfo::LinkMode::INVALID;
    auto ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_INVALID);

    mode = LinkInfo::LinkMode::NONE;
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_NONE);

    mode = LinkInfo::LinkMode::STA;
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_INVALID);

    mode = LinkInfo::LinkMode::AP;
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_INVALID);

    mode = LinkInfo::LinkMode::GO;
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_GO);

    mode = LinkInfo::LinkMode::GC;
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_GC);

    mode = LinkInfo::LinkMode::HML;
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_HML);

    mode = static_cast<LinkInfo::LinkMode>(17);
    ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_INVALID);
}

/*
 * @tc.name: BandWidthEnumToNumberTest
 * @tc.desc: check BandWidthEnumToNumber method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BandWidthEnumToNumberTest, TestSize.Level1)
{
    WifiDirectBandWidth bandWidth = BAND_WIDTH_160M;
    auto ret = WifiDirectUtils::BandWidthEnumToNumber(bandWidth);
    EXPECT_EQ(ret, WifiDirectUtils::BAND_WIDTH_160M_NUMBER);

    bandWidth = BAND_WIDTH_20M;
    ret = WifiDirectUtils::BandWidthEnumToNumber(bandWidth);
    EXPECT_EQ(ret, WifiDirectUtils::BAND_WIDTH_80M_NUMBER);
}

/*
 * @tc.name: CalculateStringLengthTest
 * @tc.desc: check CalculateStringLength method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthTest, TestSize.Level1)
{
    std::string str("123456");
    auto ret = WifiDirectUtils::CalculateStringLength(str.c_str(), str.size() + 1);
    EXPECT_EQ(ret, 6);
}

/*
 * @tc.name: SyncLnnInfoForP2pTest
 * @tc.desc: check SyncLnnInfoForP2p method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SyncLnnInfoForP2pTest, TestSize.Level1)
{
    WifiDirectApiRole role = WIFI_DIRECT_API_ROLE_GC;
    const std::string localMac = "11:22:33:44:55";
    const std::string goMac = "11:22:33:44:66";

    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnSetLocalNumInfo).WillOnce(Return(-1))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnSetLocalStrInfo).WillOnce(Return(-1))
        .WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnSyncP2pInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SyncLnnInfoForP2p(role, localMac, goMac));
}

/*
 * @tc.name: DurationStatisticEndTest
 * @tc.desc: check DurationStatistic::End method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, DurationStatisticEndTest, TestSize.Level1)
{
    int32_t requestid = 0;
    DurationStatistic::GetInstance().End(requestid);

    DurationStatistic::GetInstance().calculators_[requestid] =
        OHOS::SoftBus::DurationStatisticCalculatorFactory::GetInstance().NewInstance(
            WIFI_DIRECT_CONNECT_TYPE_AUTH_NEGO_P2P);

    DurationStatistic::GetInstance().End(requestid);
    DurationStatisticCalculatorFactory::GetInstance().creator_ = [](enum WifiDirectConnectType type) {
        return std::make_shared<P2pCalculator>(P2pCalculator::GetInstance());
    };
    auto ptr = OHOS::SoftBus::DurationStatisticCalculatorFactory::GetInstance().NewInstance(
        WIFI_DIRECT_CONNECT_TYPE_AUTH_NEGO_P2P);
    EXPECT_EQ(requestid, 0);
    EXPECT_NE(ptr, nullptr);
}

/*
 * @tc.name: GetLocalIpv4InfosTest
 * @tc.desc: check GetLocalIpv4Infos method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalIpv4InfosTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::GetLocalIpv4Infos();
    EXPECT_EQ(ret.empty(), true);
}

/*
 * @tc.name: IpStringToIntArrayTest
 * @tc.desc: check IpStringToIntArray method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArrayTest, TestSize.Level1)
{
    static char addrString[] = "255.255.255.0";
    static const uint32_t LEN = 4;
    uint32_t addrArray[LEN];
    auto ret = WifiDirectUtils::IpStringToIntArray(addrString, addrArray, LEN);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: ChannelListToStringTest
 * @tc.desc: check ChannelListToString method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelListToStringTest, TestSize.Level1)
{
    std::vector<int> channels = {36, 40, 52};
    auto ret = WifiDirectUtils::ChannelListToString(channels);
    EXPECT_EQ(ret, "36##40##52");
}

/*
 * @tc.name: IsDfsChannelTest
 * @tc.desc: check IsDfsChannel method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDfsChannelTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::IsDfsChannel(FREQ2);
    EXPECT_EQ(ret, false);
    ret = WifiDirectUtils::IsDfsChannel(FREQ3);
    EXPECT_EQ(ret, true);
    ret = WifiDirectUtils::IsDfsChannel(FREQ4);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: CheckLinkAtDfsChannelConflictTest
 * @tc.desc: check CheckLinkAtDfsChannelConflict method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CheckLinkAtDfsChannelConflictTest, TestSize.Level1)
{
    std::string uuid = "0123456789ABCDEF";
    std::string remoteDeviceId = "abcdefg";
    int32_t frequency = FREQ4;
    InnerLink::LinkType linkType = InnerLink::LinkType::HML;
    int32_t type = HO_OS_TYPE;
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetNetworkIdByUuid(_, _, _))
        .WillRepeatedly([&networkId](const std::string &uuid, char *buf, uint32_t len) {
            (void)strcpy_s(buf, len, networkId);
            return SOFTBUS_OK;
        });
    EXPECT_CALL(mock, LnnGetOsTypeByNetworkId).WillOnce(Return(SOFTBUS_WIFI_DIRECT_INIT_FAILED));
    auto ret = WifiDirectUtils::CheckLinkAtDfsChannelConflict(uuid, linkType);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(mock, LnnGetOsTypeByNetworkId).WillRepeatedly(Return(SOFTBUS_OK));
    ret = WifiDirectUtils::CheckLinkAtDfsChannelConflict(uuid, linkType);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(mock, LnnGetOsTypeByNetworkId(_, _))
        .WillRepeatedly([&type](const char *networkId, int32_t *osType) {
            *osType = type;
            return SOFTBUS_OK;
        });
    LinkManager::GetInstance().ProcessIfAbsent(
        InnerLink::LinkType::HML, remoteDeviceId, [frequency](InnerLink &innerLink) {
            innerLink.SetFrequency(frequency);
        });
    ret = WifiDirectUtils::CheckLinkAtDfsChannelConflict(uuid, linkType);
    EXPECT_EQ(ret, false);
    LinkManager::GetInstance().RemoveLink(InnerLink::LinkType::HML, remoteDeviceId);

    frequency = FREQ3;
    linkType = InnerLink::LinkType::P2P;
    LinkManager::GetInstance().ProcessIfAbsent(
        InnerLink::LinkType::P2P, remoteDeviceId, [frequency](InnerLink &innerLink) {
            innerLink.SetFrequency(frequency);
        });
    ret = WifiDirectUtils::CheckLinkAtDfsChannelConflict(uuid, linkType);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: WifiDirectAnonymizeIpTest
 * @tc.desc: check WifiDirectAnonymizeIp method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, WifiDirectAnonymizeIpTest, TestSize.Level1)
{
    std::string ip = "192";
    auto ret = WifiDirectAnonymizeIp(ip);
    EXPECT_EQ(ret, "**2");
    ip = "192.168";
    ret = WifiDirectAnonymizeIp(ip);
    EXPECT_EQ(ret, "1****68");
    ip = "192..";
    ret = WifiDirectAnonymizeIp(ip);
    EXPECT_EQ(ret, "1***.");
    ip = "70:60";
    ret = WifiDirectAnonymizeIp(ip);
    EXPECT_EQ(ret, "7***0");
    ip = "192.168.1.2";
    ret = WifiDirectAnonymizeIp(ip);
    EXPECT_EQ(ret, "192.168.1.*");
    ip = "70:f8:56:s5:80:9a";
    ret = WifiDirectAnonymizeIp(ip);
    EXPECT_EQ(ret, "70:f*********0:9a");
}

/*
 * @tc.name: WifiDirectAnonymizeSsidTest
 * @tc.desc: check WifiDirectAnonymizeSsid method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, WifiDirectAnonymizeSsidTest, TestSize.Level1)
{
    std::string ssid = "";
    auto ret = WifiDirectAnonymizeSsid(ssid);
    EXPECT_EQ(ret, "");
    ssid = "te";
    ret = WifiDirectAnonymizeSsid(ssid);
    EXPECT_EQ(ret, "*e");
    ssid = "aaaaaaaa";
    ret = WifiDirectAnonymizeSsid(ssid);
    EXPECT_EQ(ret, "aa****aa");
}

/*
 * @tc.name: WifiDirectAnonymizePskTest
 * @tc.desc: check WifiDirectAnonymizePsk method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, WifiDirectAnonymizePskTest, TestSize.Level1)
{
    std::string psk = "";
    WifiDirectConnectInfo conInfo{};
    ConnEventExtra conEventExtra = { 0 };
    WifiDirectInterfaceMock mock;
    conInfo.dfxInfo.linkType = STATISTIC_TRIGGER_HML;
    DurationStatistic::GetInstance().Record(1, TOTAL_START);
    DurationStatistic::GetInstance().Record(1, TOTAL_END);
    EXPECT_CALL(mock, LnnGetLocalStrInfo).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetOsTypeByNetworkId).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteNumInfo).WillOnce(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetLocalNumInfo).WillOnce(Return(SOFTBUS_OK));
    WifiDirectDfx::GetInstance().ReportConnEventExtra(conEventExtra, conInfo);
    std::shared_ptr<Wifi::WifiDevice> mockDevice = Wifi::WifiDevice::GetInstance(0);
    EXPECT_CALL(*mockDevice, GetLinkedInfo).WillRepeatedly(Return(Wifi::WIFI_OPT_SUCCESS));
    auto ret = WifiDirectAnonymizePsk(psk);
    EXPECT_EQ(ret, "");
    psk = "1234";
    ret = WifiDirectAnonymizePsk(psk);
    EXPECT_EQ(ret, "1**4");
    psk = "123456789";
    ret = WifiDirectAnonymizePsk(psk);
    EXPECT_EQ(ret, "12*****89");
}

/*
 * @tc.name: WifiDirectAnonymizePtkTest
 * @tc.desc: check WifiDirectAnonymizePtk method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, WifiDirectAnonymizePtkTest, TestSize.Level1)
{
    std::string ptk = "";
    auto ret = WifiDirectAnonymizePtk(ptk);
    EXPECT_EQ(ret, "");
    ptk = "123456";
    ret = WifiDirectAnonymizePtk(ptk);
    EXPECT_EQ(ret, "1***56");
    ptk = "123456789000000";
    ret = WifiDirectAnonymizePtk(ptk);
    EXPECT_EQ(ret, "123********0000");
}

/*
 * @tc.name: WifiDirectAnonymizeDataTest
 * @tc.desc: check WifiDirectAnonymizeData method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, WifiDirectAnonymizeDataTest, TestSize.Level1)
{
    std::string data = "";
    auto ret = WifiDirectAnonymizeData(data);
    EXPECT_EQ(ret, "");
    data = "12345";
    ret = WifiDirectAnonymizeData(data);
    EXPECT_EQ(ret, "1***5");
    data = "0123456789";
    ret = WifiDirectAnonymizeData(data);
    EXPECT_EQ(ret, "01*****789");
}

/*
 * @tc.name: GetLocalConnSubFeatureTest
 * @tc.desc: check GetLocalConnSubFeature method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalConnSubFeatureTest, TestSize.Level1)
{
    uint64_t feature = 0;
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalNumU64Info(_, _)).WillOnce([](InfoKey key, uint64_t *info) {
        *info = 1;
        return SOFTBUS_OK;
    });

    auto ret = WifiDirectUtils::GetLocalConnSubFeature(feature);
    EXPECT_EQ(feature, 1);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: GetRemoteConnSubFeatureTest
 * @tc.desc: check GetRemoteConnSubFeature method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteConnSubFeatureTest, TestSize.Level1)
{
    uint64_t feature = 0;
    std::string networkId = "1234567890";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNumU64Info(_, _, _))
        .WillOnce([](const std::string &networkId, InfoKey key, uint64_t *info) {
            return SOFTBUS_OK;
        });

    auto ret = WifiDirectUtils::GetRemoteConnSubFeature(networkId, feature);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: IsDeviceOnline
 * @tc.desc: test IsDeviceOnline method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDeviceOnlineTest, TestSize.Level1)
{
    char remoteNetworkId[NETWORK_ID_BUF_LEN] = { 1 };
    auto ret = WifiDirectUtils::IsDeviceOnline(remoteNetworkId);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: MacArrayToString
 * @tc.desc: test MacArrayToString method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacArrayToStringTest, TestSize.Level1)
{
    uint8_t mac[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

    auto ret = WifiDirectUtils::MacArrayToString(mac, 6);
    EXPECT_EQ(ret, "01:02:03:04:05:06");
}

/*
 * @tc.name: GetRemoteOsVersion
 * @tc.desc: test GetRemoteOsVersion method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteOsVersionTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteNodeInfoById).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    auto ret = WifiDirectUtils::GetRemoteOsVersion(networkId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetRemoteScreenStatus
 * @tc.desc: test GetRemoteScreenStatus method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteScreenStatusTest, TestSize.Level1)
{
    int screenStatus = 0;
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };

    auto ret = WifiDirectUtils::GetRemoteScreenStatus(networkId);
    EXPECT_EQ(ret, screenStatus);
}

/*
 * @tc.name: RemoteDeviceIdToMac
 * @tc.desc: test RemoteDeviceIdToMac method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, RemoteDeviceIdToMacTest, TestSize.Level1)
{
    std::string remoteDeviceId = "01234567890ABCDEF";
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::RemoteDeviceIdToMac(remoteDeviceId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: SupportHml
 * @tc.desc: test SupportHml method and SupportHmlTwo method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SupportHmlTest, TestSize.Level1)
{
    bool ret = WifiDirectUtils::SupportHml();
    EXPECT_EQ(ret, true);

    ret = WifiDirectUtils::SupportHmlTwo();
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: CompareIgnoreCase
 * @tc.desc: test CompareIgnoreCase method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CompareIgnoreCaseTest, TestSize.Level1)
{
    int32_t result = 0;
    std::string left = "test";
    std::string right = "test";

    int32_t ret = WifiDirectUtils::CompareIgnoreCase(left, right);
    EXPECT_EQ(ret, result);

    WifiDirectUtils::ParallelFlowEnter();
    WifiDirectUtils::ParallelFlowExit();
}

/*
 * @tc.name: ConvertPassiveErrorCode
 * @tc.desc: test ConvertPassiveErrorCode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ConvertPassiveErrorCodeTest, TestSize.Level1)
{
    int32_t code = SOFTBUS_CONN_ACTIVE_TYPE_NO_CONFLICT;
    EXPECT_EQ(SOFTBUS_CONN_PASSIVE_TYPE_NO_CONFLICT, WifiDirectUtils::ConvertPassiveErrorCode(code));
    code = SOFTBUS_CONN_P2P_GO_NO_EXIST;
    EXPECT_EQ(SOFTBUS_CONN_P2P_GO_NO_EXIST, WifiDirectUtils::ConvertPassiveErrorCode(code));
    code = SOFTBUS_CONN_NEARBY_CONTROL_CHANNEL_CONNECT_FAILED;
    EXPECT_EQ(SOFTBUS_CONN_NEARBY_CONTROL_CHANNEL_CONNECT_FAILED, WifiDirectUtils::ConvertPassiveErrorCode(code));
}

/*
 * @tc.name: BytesToIntWithInvalidSizeTest
 * @tc.desc: test BytesToInt with size > sizeof(uint32_t)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntWithInvalidSizeTest, TestSize.Level1)
{
    std::vector<uint8_t> data = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    auto ret = WifiDirectUtils::BytesToInt(data);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: BytesToIntWithPointerTest
 * @tc.desc: test BytesToInt with pointer and size parameters
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntWithPointerTest, TestSize.Level1)
{
    std::vector<uint8_t> data = { 0x01, 0x02, 0x03, 0x04 };
    auto ret = WifiDirectUtils::BytesToInt(data.data(), data.size());
    EXPECT_EQ(ret, 67305985);
}

/*
 * @tc.name: IntToBytesTest
 * @tc.desc: test IntToBytes method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IntToBytesTest, TestSize.Level1)
{
    uint32_t data = 0x12345678;
    std::vector<uint8_t> out;
    WifiDirectUtils::IntToBytes(data, sizeof(data), out);
    EXPECT_EQ(out.size(), sizeof(data));
    EXPECT_EQ(out[0], 0x78);
    EXPECT_EQ(out[1], 0x56);
    EXPECT_EQ(out[2], 0x34);
    EXPECT_EQ(out[3], 0x12);
}

/*
 * @tc.name: ToStringTest
 * @tc.desc: test ToString method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToStringTest, TestSize.Level1)
{
    std::vector<uint8_t> input = { 0x01, 0x02, 0x0F, 0xFF };
    auto ret = WifiDirectUtils::ToString(input);
    EXPECT_EQ(ret, "01020fff");
}

/*
 * @tc.name: Is2GBandTest
 * @tc.desc: test Is2GBand method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, Is2GBandTest, TestSize.Level1)
{
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQ2), true);
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQ3), false);
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQ1), false);
}

/*
 * @tc.name: Is5GBandTest
 * @tc.desc: test Is5GBand method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, Is5GBandTest, TestSize.Level1)
{
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQ2), false);
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQ3), true);
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQ4), true);
}

/*
 * @tc.name: GetLocalNetworkIdTest
 * @tc.desc: test GetLocalNetworkId method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalNetworkIdTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalStrInfo).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetLocalNetworkId();
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetLocalUuidTest
 * @tc.desc: test GetLocalUuid method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalUuidTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalStrInfo).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetLocalUuid();
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: SetLocalWifiDirectMacTest
 * @tc.desc: test SetLocalWifiDirectMac method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SetLocalWifiDirectMacTest, TestSize.Level1)
{
    std::string mac = "AA:BB:CC:DD:EE:FF";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnSetLocalStrInfo).WillOnce(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SetLocalWifiDirectMac(mac));
}

/*
 * @tc.name: MacStringToArrayTest
 * @tc.desc: test MacStringToArray method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayTest, TestSize.Level1)
{
    std::string macString = "AA:BB:CC:DD:EE:FF";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 6);
    EXPECT_EQ(ret[0], 0xAA);
    EXPECT_EQ(ret[5], 0xFF);
}

/*
 * @tc.name: MacStringToArrayEmptyTest
 * @tc.desc: test MacStringToArray with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayEmptyTest, TestSize.Level1)
{
    std::string macString = "";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: MacStringToArrayInvalidTest
 * @tc.desc: test MacStringToArray with invalid string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayInvalidTest, TestSize.Level1)
{
    std::string macString = "GG:HH:II:JJ:KK:LL";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: StringToIntTest
 * @tc.desc: test StringToInt method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToIntTest, TestSize.Level1)
{
    std::string validStr = "12345";
    int32_t result = 0;
    auto ret = WifiDirectUtils::StringToInt(validStr, result);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(result, 12345);
}

/*
 * @tc.name: StringToIntInvalidTest
 * @tc.desc: test StringToInt with invalid string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToIntInvalidTest, TestSize.Level1)
{
    std::string invalidStr = "abc";
    int32_t result = 0;
    auto ret = WifiDirectUtils::StringToInt(invalidStr, result);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: BandWidthNumberToEnumTest
 * @tc.desc: test BandWidthNumberToEnum method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BandWidthNumberToEnumTest, TestSize.Level1)
{
    int bandWidth = WifiDirectUtils::BAND_WIDTH_160M_NUMBER;
    auto ret = WifiDirectUtils::BandWidthNumberToEnum(bandWidth);
    EXPECT_EQ(ret, BAND_WIDTH_160M);

    bandWidth = WifiDirectUtils::BAND_WIDTH_80M_NUMBER;
    ret = WifiDirectUtils::BandWidthNumberToEnum(bandWidth);
    EXPECT_EQ(ret, BAND_WIDTH_80M);
}

/*
 * @tc.name: IsDeviceIdTest
 * @tc.desc: test IsDeviceId method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDeviceIdTest, TestSize.Level1)
{
    std::string validUuid = "0123456789012345678901234567890101234567890123456789012345678901";
    auto ret = WifiDirectUtils::IsDeviceId(validUuid);
    EXPECT_EQ(ret, true);

    std::string invalidUuid = "012345678901234";
    ret = WifiDirectUtils::IsDeviceId(invalidUuid);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: GetOsTypeTest
 * @tc.desc: test GetOsType method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetOsTypeTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetOsTypeByNetworkId).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetOsType(networkId);
    EXPECT_EQ(ret, OH_OS_TYPE);
}

/*
 * @tc.name: GetDeviceTypeTest
 * @tc.desc: test GetDeviceType method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetDeviceTypeTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteNumInfo).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetDeviceType(networkId);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: GetLocalDeviceTypeTest
 * @tc.desc: test GetDeviceType (local) method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalDeviceTypeTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetLocalNumInfo).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetDeviceType();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: GetLocalScreenStatusTest
 * @tc.desc: test GetLocalScreenStatus method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalScreenStatusTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetLocalBoolInfo).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetLocalScreenStatus();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CompareIgnoreCaseNotEqualTest
 * @tc.desc: test CompareIgnoreCase with different strings
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CompareIgnoreCaseNotEqualTest, TestSize.Level1)
{
    std::string left = "test";
    std::string right = "TEST123";
    int32_t ret = WifiDirectUtils::CompareIgnoreCase(left, right);
    EXPECT_NE(ret, 0);
}

/*
 * @tc.name: ChannelListToStringEmptyTest
 * @tc.desc: test ChannelListToString with empty vector
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelListToStringEmptyTest, TestSize.Level1)
{
    std::vector<int> channels;
    auto ret = WifiDirectUtils::ChannelListToString(channels);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: SplitStringTest
 * @tc.desc: test SplitString method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringTest, TestSize.Level1)
{
    std::string input = "apple##banana##orange";
    std::string delimiter = "##";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 3);
    EXPECT_EQ(ret[0], "apple");
    EXPECT_EQ(ret[1], "banana");
    EXPECT_EQ(ret[2], "orange");
}

/*
 * @tc.name: SplitStringNoDelimiterTest
 * @tc.desc: test SplitString with no delimiter in string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringNoDelimiterTest, TestSize.Level1)
{
    std::string input = "apple";
    std::string delimiter = "##";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0], "apple");
}

/*
 * @tc.name: SplitStringEmptyTest
 * @tc.desc: test SplitString with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringEmptyTest, TestSize.Level1)
{
    std::string input = "";
    std::string delimiter = "##";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: NetworkIdToUuidTest
 * @tc.desc: test NetworkIdToUuid method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, NetworkIdToUuidTest, TestSize.Level1)
{
    std::string networkId = "testNetworkId";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::NetworkIdToUuid(networkId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetChloadTest
 * @tc.desc: test GetChload method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetChloadTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    std::shared_ptr<Wifi::WifiDevice> mockDevice = Wifi::WifiDevice::GetInstance(WIFI_DEVICE_ABILITY_ID);
    EXPECT_CALL(*mockDevice, GetLinkedInfo).WillOnce(Return(Wifi::WIFI_OPT_SUCCESS));
    auto ret = WifiDirectUtils::GetChload();
    EXPECT_GE(ret, -1);
}

/*
 * @tc.name: RemoteMacToDeviceIdTest
 * @tc.desc: test RemoteMacToDeviceId method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, RemoteMacToDeviceIdTest, TestSize.Level1)
{
    std::string invaildMac = "FFFFFFFFFFFF";
    WifiDirectInterfaceMock mock;

    auto ret = WifiDirectUtils::RemoteMacToDeviceId(invaildMac);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: MacArrayToStringEmptyTest
 * @tc.desc: test MacArrayToString with empty array
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacArrayToStringEmptyTest, TestSize.Level1)
{
    std::vector<uint8_t> macArray;
    auto ret = WifiDirectUtils::MacArrayToString(macArray);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: StringToChannelListInvalidIntTest
 * @tc.desc: test StringToChannelList with invalid integer string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToChannelListInvalidIntTest, TestSize.Level1)
{
    std::string channels = "36##abc##40";
    auto ret = WifiDirectUtils::StringToChannelList(channels);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: IpStringToIntArrayNullTest
 * @tc.desc: test IpStringToIntArray with null string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArrayNullTest, TestSize.Level1)
{
    uint32_t addrArray[4];
    auto ret = WifiDirectUtils::IpStringToIntArray(nullptr, addrArray, 4);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: IpStringToIntArraySmallSizeTest
 * @tc.desc: test IpStringToIntArray with small array size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArraySmallSizeTest, TestSize.Level1)
{
    static char addrString[] = "192.168.1.1";
    uint32_t addrArray[2];
    auto ret = WifiDirectUtils::IpStringToIntArray(addrString, addrArray, 2);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: IpStringToIntArrayInvalidFormatTest
 * @tc.desc: test IpStringToIntArray with invalid format
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArrayInvalidFormatTest, TestSize.Level1)
{
    static char addrString[] = "192.168.1";
    uint32_t addrArray[4];
    auto ret = WifiDirectUtils::IpStringToIntArray(addrString, addrArray, 4);
    EXPECT_EQ(ret, SOFTBUS_CONN_SCAN_IP_NUMBER_FAILED);
}

/*
 * @tc.name: ToBinaryEmptyTest
 * @tc.desc: test ToBinary with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToBinaryEmptyTest, TestSize.Level1)
{
    std::string input = "";
    auto ret = WifiDirectUtils::ToBinary(input);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: IsInChannelListFalseTest
 * @tc.desc: test IsInChannelList when channel is not in list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsInChannelListFalseTest, TestSize.Level1)
{
    int32_t channel = 100;
    std::vector<int> channelArray = { 36, 40, 44 };
    auto ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: IsInChannelListEmptyTest
 * @tc.desc: test IsInChannelList with empty list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsInChannelListEmptyTest, TestSize.Level1)
{
    int32_t channel = 36;
    std::vector<int> channelArray;
    auto ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: GetLocalConnSubFeatureFailedTest
 * @tc.desc: test GetLocalConnSubFeature when it fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalConnSubFeatureFailedTest, TestSize.Level1)
{
    uint64_t feature = 0;
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalNumU64Info).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetLocalConnSubFeature(feature);
    EXPECT_NE(ret, SOFTBUS_OK);
}

/*
 * @tc.name: GetRemoteConnSubFeatureFailedTest
 * @tc.desc: test GetRemoteConnSubFeature when it fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteConnSubFeatureFailedTest, TestSize.Level1)
{
    uint64_t feature = 0;
    std::string networkId = "1234567890";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNumU64Info).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetRemoteConnSubFeature(networkId, feature);
    EXPECT_NE(ret, SOFTBUS_OK);
}

/*
 * @tc.name: CalculateStringLengthZeroTest
 * @tc.desc: test CalculateStringLength with all null bytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthZeroTest, TestSize.Level1)
{
    char str[10] = { 0 };
    auto ret = WifiDirectUtils::CalculateStringLength(str, sizeof(str));
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: CalculateStringLengthNoNullTest
 * @tc.desc: test CalculateStringLength with no null bytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthNoNullTest, TestSize.Level1)
{
    char str[6] = { 'a', 'b', 'c', 'd', 'e', 'f' };
    auto ret = WifiDirectUtils::CalculateStringLength(str, sizeof(str));
    EXPECT_EQ(ret, 6);
}

/*
 * @tc.name: SplitStringWithTrailingDelimiterTest
 * @tc.desc: test SplitString with input ending with delimiter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringWithTrailingDelimiterTest, TestSize.Level1)
{
    std::string input = "apple##banana##";
    std::string delimiter = "##";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 2);
    EXPECT_EQ(ret[0], "apple");
    EXPECT_EQ(ret[1], "banana");
}

/*
 * @tc.name: SplitStringConsecutiveDelimiterTest
 * @tc.desc: test SplitString with consecutive delimiters
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringConsecutiveDelimiterTest, TestSize.Level1)
{
    std::string input = "apple####banana";
    std::string delimiter = "##";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 3);
    EXPECT_EQ(ret[0], "apple");
    EXPECT_EQ(ret[1], "");
    EXPECT_EQ(ret[2], "banana");
}

/*
 * @tc.name: SplitStringSingleCharDelimiterTest
 * @tc.desc: test SplitString with single char delimiter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringSingleCharDelimiterTest, TestSize.Level1)
{
    std::string input = "a:b:c:d";
    std::string delimiter = ":";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 4);
    EXPECT_EQ(ret[0], "a");
    EXPECT_EQ(ret[1], "b");
    EXPECT_EQ(ret[2], "c");
    EXPECT_EQ(ret[3], "d");
}

/*
 * @tc.name: SplitStringDelimiterAtStartTest
 * @tc.desc: test SplitString with delimiter at the start
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SplitStringDelimiterAtStartTest, TestSize.Level1)
{
    std::string input = "##apple##banana";
    std::string delimiter = "##";
    auto ret = WifiDirectUtils::SplitString(input, delimiter);
    EXPECT_EQ(ret.size(), 3);
    EXPECT_EQ(ret[0], "");
    EXPECT_EQ(ret[1], "apple");
    EXPECT_EQ(ret[2], "banana");
}

/*
 * @tc.name: IntToBytesInvalidLenTest
 * @tc.desc: test IntToBytes with len greater than sizeof(uint32_t)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IntToBytesInvalidLenTest, TestSize.Level1)
{
    uint32_t data = 0x12345678;
    std::vector<uint8_t> out;
    WifiDirectUtils::IntToBytes(data, sizeof(data) + 1, out);
    EXPECT_EQ(out.size(), 0);
}

/*
 * @tc.name: IntToBytesZeroLenTest
 * @tc.desc: test IntToBytes with zero len
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IntToBytesZeroLenTest, TestSize.Level1)
{
    uint32_t data = 0x12345678;
    std::vector<uint8_t> out;
    WifiDirectUtils::IntToBytes(data, 0, out);
    EXPECT_EQ(out.size(), 0);
}

/*
 * @tc.name: IntToBytesPartialLenTest
 * @tc.desc: test IntToBytes with partial len (less than sizeof(uint32_t))
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IntToBytesPartialLenTest, TestSize.Level1)
{
    uint32_t data = 0x12345678;
    std::vector<uint8_t> out;
    WifiDirectUtils::IntToBytes(data, 2, out);
    EXPECT_EQ(out.size(), 2);
    EXPECT_EQ(out[0], 0x78);
    EXPECT_EQ(out[1], 0x56);
}

/*
 * @tc.name: IntToBytesSingleByteTest
 * @tc.desc: test IntToBytes with len=1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IntToBytesSingleByteTest, TestSize.Level1)
{
    uint32_t data = 0xABCD1234;
    std::vector<uint8_t> out;
    WifiDirectUtils::IntToBytes(data, 1, out);
    EXPECT_EQ(out.size(), 1);
    EXPECT_EQ(out[0], 0x34);
}

/*
 * @tc.name: IntToBytesToExistingVectorTest
 * @tc.desc: test IntToBytes appending to existing vector
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IntToBytesToExistingVectorTest, TestSize.Level1)
{
    uint32_t data = 0x12345678;
    std::vector<uint8_t> out = { 0xFF, 0xEE };
    WifiDirectUtils::IntToBytes(data, sizeof(data), out);
    EXPECT_EQ(out.size(), 6);
    EXPECT_EQ(out[0], 0xFF);
    EXPECT_EQ(out[1], 0xEE);
    EXPECT_EQ(out[2], 0x78);
    EXPECT_EQ(out[5], 0x12);
}

/*
 * @tc.name: ToStringEmptyTest
 * @tc.desc: test ToString with empty input vector
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToStringEmptyTest, TestSize.Level1)
{
    std::vector<uint8_t> input;
    auto ret = WifiDirectUtils::ToString(input);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: ToStringSingleByteTest
 * @tc.desc: test ToString with single byte
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToStringSingleByteTest, TestSize.Level1)
{
    std::vector<uint8_t> input = { 0x0A };
    auto ret = WifiDirectUtils::ToString(input);
    EXPECT_EQ(ret, "0a");
}

/*
 * @tc.name: ToStringAllZerosTest
 * @tc.desc: test ToString with all zero bytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToStringAllZerosTest, TestSize.Level1)
{
    std::vector<uint8_t> input = { 0x00, 0x00, 0x00 };
    auto ret = WifiDirectUtils::ToString(input);
    EXPECT_EQ(ret, "000000");
}

/*
 * @tc.name: ToBinaryOddLengthTest
 * @tc.desc: test ToBinary with odd length input
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToBinaryOddLengthTest, TestSize.Level1)
{
    std::string input = "ABC";
    auto ret = WifiDirectUtils::ToBinary(input);
    EXPECT_EQ(ret.size(), 2);
    EXPECT_EQ(ret[0], 0xAB);
    EXPECT_EQ(ret[1], 0xC);
}

/*
 * @tc.name: ToBinarySingleCharTest
 * @tc.desc: test ToBinary with single character
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToBinarySingleCharTest, TestSize.Level1)
{
    std::string input = "F";
    auto ret = WifiDirectUtils::ToBinary(input);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0], 0xF);
}

/*
 * @tc.name: ToBinaryUpperCaseTest
 * @tc.desc: test ToBinary with uppercase hex characters
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToBinaryUpperCaseTest, TestSize.Level1)
{
    std::string input = "0A1B2C";
    auto ret = WifiDirectUtils::ToBinary(input);
    EXPECT_EQ(ret.size(), 3);
    EXPECT_EQ(ret[0], 0x0A);
    EXPECT_EQ(ret[1], 0x1B);
    EXPECT_EQ(ret[2], 0x2C);
}

/*
 * @tc.name: ChannelToFrequencyMiddleRangeTest
 * @tc.desc: test ChannelToFrequency with channel in middle invalid range (14-33)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelToFrequencyMiddleRangeTest, TestSize.Level1)
{
    int32_t channel = 20;
    auto ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, FREQUENCY_INVALID);

    channel = 14;
    ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, FREQUENCY_INVALID);

    channel = 33;
    ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, FREQUENCY_INVALID);
}

/*
 * @tc.name: ChannelToFrequency2GBoundaryTest
 * @tc.desc: test ChannelToFrequency at 2G band boundaries
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelToFrequency2GBoundaryTest, TestSize.Level1)
{
    int32_t channel = CHANNEL_2G_FIRST;
    auto ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, FREQUENCY_2G_FIRST);

    channel = CHANNEL_2G_LAST;
    ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, FREQUENCY_2G_LAST);
}

/*
 * @tc.name: ChannelToFrequency5GBoundaryTest
 * @tc.desc: test ChannelToFrequency at 5G band boundaries
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelToFrequency5GBoundaryTest, TestSize.Level1)
{
    int32_t channel = CHANNEL_5G_FIRST;
    auto ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, FREQUENCY_5G_FIRST);

    channel = CHANNEL_5G_LAST;
    ret = WifiDirectUtils::ChannelToFrequency(channel);
    EXPECT_EQ(ret, (CHANNEL_5G_LAST - CHANNEL_5G_FIRST) * FREQUENCY_STEP + FREQUENCY_5G_FIRST);
}

/*
 * @tc.name: FrequencyToChannel2GBoundaryTest
 * @tc.desc: test FrequencyToChannel at 2G band boundaries
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, FrequencyToChannel2GBoundaryTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::FrequencyToChannel(FREQUENCY_2G_FIRST);
    EXPECT_EQ(ret, CHANNEL_2G_FIRST);

    ret = WifiDirectUtils::FrequencyToChannel(FREQUENCY_2G_LAST);
    EXPECT_EQ(ret, CHANNEL_2G_LAST);
}

/*
 * @tc.name: FrequencyToChannel5GBoundaryTest
 * @tc.desc: test FrequencyToChannel at 5G band boundaries
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, FrequencyToChannel5GBoundaryTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::FrequencyToChannel(FREQUENCY_5G_FIRST);
    EXPECT_EQ(ret, CHANNEL_5G_FIRST);

    ret = WifiDirectUtils::FrequencyToChannel(FREQUENCY_5G_LAST);
    EXPECT_EQ(ret, (FREQUENCY_5G_LAST - FREQUENCY_5G_FIRST) / FREQUENCY_STEP + CHANNEL_5G_FIRST);
}

/*
 * @tc.name: GetRecommendChannelFromLnnTest
 * @tc.desc: test GetRecommendChannelFromLnn method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRecommendChannelFromLnnTest, TestSize.Level1)
{
    std::string networkId = "testNetworkId";
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetRecommendChannelFromLnn(networkId);
    EXPECT_EQ(ret, SOFTBUS_ERR);
}

/*
 * @tc.name: NetworkIdToUuidFailedTest
 * @tc.desc: test NetworkIdToUuid when LnnGetRemoteStrInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, NetworkIdToUuidFailedTest, TestSize.Level1)
{
    std::string networkId = "testNetworkId";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::NetworkIdToUuid(networkId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: UuidToNetworkIdFailedTest
 * @tc.desc: test UuidToNetworkId when LnnGetNetworkIdByUuid fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, UuidToNetworkIdFailedTest, TestSize.Level1)
{
    std::string uuid = "0123456789ABCDEF";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetNetworkIdByUuid).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::UuidToNetworkId(uuid);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetLocalNetworkIdFailedTest
 * @tc.desc: test GetLocalNetworkId when LnnGetLocalStrInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalNetworkIdFailedTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalStrInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetLocalNetworkId();
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetLocalUuidFailedTest
 * @tc.desc: test GetLocalUuid when LnnGetLocalStrInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalUuidFailedTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalStrInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetLocalUuid();
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetRemotePtkNonZeroPtkTest
 * @tc.desc: test GetRemotePtk when remote byte info is non-zero
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemotePtkNonZeroPtkTest, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnGetRemoteDefaultPtkByUuid = LnnGetRemoteDefaultPtkByUuid;
    char remoteNetworkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteByteInfo(_, _, _, _))
        .WillRepeatedly([](const char *networkId, InfoKey key, uint8_t *info, uint32_t len) {
            info[0] = 0x42;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::GetRemotePtk(remoteNetworkId);
    EXPECT_NE(ret.size(), 0);
    EXPECT_EQ(ret[0], 0x42);
}

/*
 * @tc.name: IsRemoteSupportTlvFailedTest
 * @tc.desc: test IsRemoteSupportTlv when LnnGetRemoteBoolInfoIgnoreOnline fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsRemoteSupportTlvFailedTest, TestSize.Level1)
{
    std::string remoteDeviceId = "01234567890ABCDEF";
    WifiDirectInterfaceMock mock;

    EXPECT_CALL(mock, LnnGetNetworkIdByUuid).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteBoolInfoIgnoreOnline).WillRepeatedly(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::IsRemoteSupportTlv(remoteDeviceId);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: IsLocalSupportTlvTrueTest
 * @tc.desc: test IsLocalSupportTlv when feature is supported
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsLocalSupportTlvTrueTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetFeatureCapabilty).WillRepeatedly(Return(0xFFFF));
    EXPECT_CALL(mock, IsFeatureSupport).WillRepeatedly(Return(true));
    auto ret = WifiDirectUtils::IsLocalSupportTlv();
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: IsLocalSupportTlvFalseTest
 * @tc.desc: test IsLocalSupportTlv when feature is not supported
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsLocalSupportTlvFalseTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetFeatureCapabilty).WillRepeatedly(Return(0));
    EXPECT_CALL(mock, IsFeatureSupport).WillRepeatedly(Return(false));
    auto ret = WifiDirectUtils::IsLocalSupportTlv();
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: IsDeviceOnlineFalseTest
 * @tc.desc: test IsDeviceOnline when device is not online
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDeviceOnlineFalseTest, TestSize.Level1)
{
    char remoteNetworkId[NETWORK_ID_BUF_LEN] = { 1 };
    auto ret = WifiDirectUtils::IsDeviceOnline(remoteNetworkId);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: MacStringToArrayOutOfRangeTest
 * @tc.desc: test MacStringToArray with out of range value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayOutOfRangeTest, TestSize.Level1)
{
    std::string macString = "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: MacStringToArrayPartialValidTest
 * @tc.desc: test MacStringToArray with partial valid hex
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayPartialValidTest, TestSize.Level1)
{
    std::string macString = "AA:BB:CC:DD:EE";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 5);
    EXPECT_EQ(ret[0], 0xAA);
    EXPECT_EQ(ret[4], 0xEE);
}

/*
 * @tc.name: MacArrayToStringSingleByteTest
 * @tc.desc: test MacArrayToString with single byte
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacArrayToStringSingleByteTest, TestSize.Level1)
{
    std::vector<uint8_t> macArray = { 0xAB };
    auto ret = WifiDirectUtils::MacArrayToString(macArray);
    EXPECT_EQ(ret, "ab");
}

/*
 * @tc.name: MacArrayToStringPointerTest
 * @tc.desc: test MacArrayToString with pointer and size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacArrayToStringPointerTest, TestSize.Level1)
{
    uint8_t mac[3] = { 0x01, 0x02, 0x03 };
    auto ret = WifiDirectUtils::MacArrayToString(mac, 3);
    EXPECT_EQ(ret, "01:02:03");
}

/*
 * @tc.name: MacArrayToStringPointerZeroSizeTest
 * @tc.desc: test MacArrayToString with pointer and zero size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacArrayToStringPointerZeroSizeTest, TestSize.Level1)
{
    uint8_t mac[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
    auto ret = WifiDirectUtils::MacArrayToString(mac, 0);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetInterfaceMacAddrInvalidInterfaceTest
 * @tc.desc: test GetInterfaceMacAddr with non-existent interface
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetInterfaceMacAddrInvalidInterfaceTest, TestSize.Level1)
{
    std::string interface = "non_existent_interface_xyz";
    auto ret = WifiDirectUtils::GetInterfaceMacAddr(interface);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: GetInterfaceMacAddrLongNameTest
 * @tc.desc: test GetInterfaceMacAddr with too long interface name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetInterfaceMacAddrLongNameTest, TestSize.Level1)
{
    std::string interface(256, 'a');
    auto ret = WifiDirectUtils::GetInterfaceMacAddr(interface);
    EXPECT_EQ(ret.size(), 0);
}

/*
 * @tc.name: GetInterfaceIpStringInvalidInterfaceTest
 * @tc.desc: test GetInterfaceIpString with non-existent interface
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetInterfaceIpStringInvalidInterfaceTest, TestSize.Level1)
{
    std::string interface = "non_existent_interface_xyz";
    std::string ip;
    auto ret = WifiDirectUtils::GetInterfaceIpString(interface, ip);
    EXPECT_NE(ret, SOFTBUS_OK);
}

/*
 * @tc.name: GetInterfaceIpStringLongNameTest
 * @tc.desc: test GetInterfaceIpString with too long interface name
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetInterfaceIpStringLongNameTest, TestSize.Level1)
{
    std::string interface(256, 'a');
    std::string ip;
    auto ret = WifiDirectUtils::GetInterfaceIpString(interface, ip);
    EXPECT_EQ(ret, SOFTBUS_CONN_COPY_INTERFACE_NAME_FAILED);
}

/*
 * @tc.name: ShowLinkInfoListTest
 * @tc.desc: test ShowLinkInfoList method
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ShowLinkInfoListTest, TestSize.Level1)
{
    std::string banana = "testLinkList";
    std::vector<LinkInfo> linkList;
    LinkInfo info1("wlan0", "p2p-wlan0", LinkInfo::LinkMode::GO, LinkInfo::LinkMode::GC);
    linkList.push_back(info1);
    LinkInfo info2("wlan1", "p2p-wlan1", LinkInfo::LinkMode::HML, LinkInfo::LinkMode::HML);
    linkList.push_back(info2);
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::ShowLinkInfoList(banana, linkList));
}

/*
 * @tc.name: ShowLinkInfoListEmptyTest
 * @tc.desc: test ShowLinkInfoList with empty list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ShowLinkInfoListEmptyTest, TestSize.Level1)
{
    std::string banana = "emptyList";
    std::vector<LinkInfo> linkList;
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::ShowLinkInfoList(banana, linkList));
}

/*
 * @tc.name: SerialFlowEnterExitTest
 * @tc.desc: test SerialFlowEnter and SerialFlowExit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SerialFlowEnterExitTest, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SerialFlowEnter());
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SerialFlowExit());
}

/*
 * @tc.name: SerialParallelFlowTest
 * @tc.desc: test serial and parallel flow together
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SerialParallelFlowTest, TestSize.Level1)
{
    WifiDirectUtils::SerialFlowEnter();
    WifiDirectUtils::SerialFlowExit();
    WifiDirectUtils::ParallelFlowEnter();
    WifiDirectUtils::ParallelFlowExit();
}

/*
 * @tc.name: GetOsTypeFailedTest
 * @tc.desc: test GetOsType when LnnGetOsTypeByNetworkId fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetOsTypeFailedTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetOsTypeByNetworkId).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetOsType(networkId);
    EXPECT_EQ(ret, SOFTBUS_ERR);
}

/*
 * @tc.name: GetDeviceTypeFailedTest
 * @tc.desc: test GetDeviceType (remote) when LnnGetRemoteNumInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetDeviceTypeFailedTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNumInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetDeviceType(networkId);
    EXPECT_EQ(ret, SOFTBUS_ERR);
}

/*
 * @tc.name: GetLocalDeviceTypeFailedTest
 * @tc.desc: test GetDeviceType (local) when LnnGetLocalNumInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalDeviceTypeFailedTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalNumInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetDeviceType();
    EXPECT_EQ(ret, SOFTBUS_ERR);
}

/*
 * @tc.name: GetMetaTypeNullTest
 * @tc.desc: test GetMetaType with null networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetMetaTypeNullTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::GetMetaType(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: GetMetaTypeSuccessTest
 * @tc.desc: test GetMetaType when LnnGetRemoteNumInfo succeeds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetMetaTypeSuccessTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNumInfo(_, _, _))
        .WillOnce([](const char *networkId, InfoKey key, int32_t *info) {
            *info = 5;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::GetMetaType(networkId);
    EXPECT_EQ(ret, 5);
}

/*
 * @tc.name: GetMetaTypeFailedTest
 * @tc.desc: test GetMetaType when LnnGetRemoteNumInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetMetaTypeFailedTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNumInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetMetaType(networkId);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: GetRemoteOsVersionNullTest
 * @tc.desc: test GetRemoteOsVersion with null networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteOsVersionNullTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::GetRemoteOsVersion(nullptr);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetRemoteOsVersionSuccessTest
 * @tc.desc: test GetRemoteOsVersion when LnnGetRemoteNodeInfoById succeeds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteOsVersionSuccessTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNodeInfoById).WillRepeatedly(Return(SOFTBUS_OK));
    auto ret = WifiDirectUtils::GetRemoteOsVersion(networkId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetRemoteOsVersionDefaultTest
 * @tc.desc: test GetRemoteOsVersion returns empty string with default node info
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteOsVersionDefaultTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNodeInfoById).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    auto ret = WifiDirectUtils::GetRemoteOsVersion(networkId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetRemoteScreenStatusNullTest
 * @tc.desc: test GetRemoteScreenStatus with null networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteScreenStatusNullTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::GetRemoteScreenStatus(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
 * @tc.name: GetRemoteScreenStatusSuccessTest
 * @tc.desc: test GetRemoteScreenStatus when LnnGetRemoteNodeInfoByKey succeeds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteScreenStatusSuccessTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    auto ret = WifiDirectUtils::GetRemoteScreenStatus(networkId);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: GetRemoteScreenStatusDefaultTest
 * @tc.desc: test GetRemoteScreenStatus returns default value (screen off)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteScreenStatusDefaultTest, TestSize.Level1)
{
    char networkId[NETWORK_ID_BUF_LEN] = { 1 };
    auto ret = WifiDirectUtils::GetRemoteScreenStatus(networkId);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: RemoteDeviceIdToMacFailedTest
 * @tc.desc: test RemoteDeviceIdToMac when LnnGetRemoteStrInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, RemoteDeviceIdToMacFailedTest, TestSize.Level1)
{
    std::string remoteDeviceId = "01234567890ABCDEF";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetNetworkIdByUuid).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnGetRemoteStrInfo).WillRepeatedly(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::RemoteDeviceIdToMac(remoteDeviceId);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: RemoteDeviceIdToMacSuccessTest
 * @tc.desc: test RemoteDeviceIdToMac when LnnGetRemoteStrInfo succeeds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, RemoteDeviceIdToMacSuccessTest, TestSize.Level1)
{
    std::string remoteDeviceId = "01234567890ABCDEF";
    char remoteMac[MAC_ADDR_STR_LEN] = "11:22:33:44:55:66";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetNetworkIdByUuid(_, _, _))
        .WillRepeatedly([&remoteMac](const std::string &uuid, char *buf, uint32_t len) {
            (void)strcpy_s(buf, len, "networkId");
            return SOFTBUS_OK;
        });
    EXPECT_CALL(mock, LnnGetRemoteStrInfo(_, _, _, _))
        .WillRepeatedly([&remoteMac](const std::string &networkId, InfoKey key, char *info, uint32_t len) {
            (void)strcpy_s(info, len, remoteMac);
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::RemoteDeviceIdToMac(remoteDeviceId);
    EXPECT_EQ(ret, "11:22:33:44:55:66");
}

/*
 * @tc.name: RemoteMacToDeviceIdEmptyTest
 * @tc.desc: test RemoteMacToDeviceId with no online nodes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, RemoteMacToDeviceIdEmptyTest, TestSize.Level1)
{
    std::string remoteMac = "11:22:33:44:55:66";
    auto ret = WifiDirectUtils::RemoteMacToDeviceId(remoteMac);
    EXPECT_EQ(ret, "");
}

/*
 * @tc.name: GetLocalScreenStatusFailedTest
 * @tc.desc: test GetLocalScreenStatus when LnnGetLocalBoolInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalScreenStatusFailedTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalBoolInfo).WillOnce(Return(SOFTBUS_ERR));
    auto ret = WifiDirectUtils::GetLocalScreenStatus();
    EXPECT_EQ(ret, SOFTBUS_ERR);
}

/*
 * @tc.name: GetLocalScreenStatusScreenOnTest
 * @tc.desc: test GetLocalScreenStatus when screen is on
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalScreenStatusScreenOnTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalBoolInfo(_, _, _))
        .WillOnce([](InfoKey key, bool *info, uint32_t len) {
            *info = true;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::GetLocalScreenStatus();
    EXPECT_EQ(ret, WIFI_DIRECT_SCREEN_ON);
}

/*
 * @tc.name: GetLocalScreenStatusScreenOffTest
 * @tc.desc: test GetLocalScreenStatus when screen is off
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalScreenStatusScreenOffTest, TestSize.Level1)
{
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalBoolInfo(_, _, _))
        .WillOnce([](InfoKey key, bool *info, uint32_t len) {
            *info = false;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::GetLocalScreenStatus();
    EXPECT_EQ(ret, WIFI_DIRECT_SCREEN_OFF);
}

/*
 * @tc.name: ConvertPassiveErrorCodeBoundaryTest
 * @tc.desc: test ConvertPassiveErrorCode at boundary values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ConvertPassiveErrorCodeBoundaryTest, TestSize.Level1)
{
    int32_t code = SOFTBUS_CONN_SHORT_RANGE_BASE;
    EXPECT_EQ(code, WifiDirectUtils::ConvertPassiveErrorCode(code));

    code = SOFTBUS_CONN_SHORT_RANGE_BASE + 1;
    EXPECT_EQ(code + 1, WifiDirectUtils::ConvertPassiveErrorCode(code));

    code = SOFTBUS_CONN_PASSIVE_TYPE_STA_P2P_HML_CHIP_CONFLICT;
    EXPECT_EQ(code + 1, WifiDirectUtils::ConvertPassiveErrorCode(code));

    code = SOFTBUS_CONN_PASSIVE_TYPE_STA_P2P_HML_CHIP_CONFLICT + 1;
    EXPECT_EQ(code, WifiDirectUtils::ConvertPassiveErrorCode(code));

    code = 0;
    EXPECT_EQ(0, WifiDirectUtils::ConvertPassiveErrorCode(code));

    code = -1;
    EXPECT_EQ(-1, WifiDirectUtils::ConvertPassiveErrorCode(code));
}

/*
 * @tc.name: SyncLnnInfoForP2pAllFailTest
 * @tc.desc: test SyncLnnInfoForP2p when all set calls fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SyncLnnInfoForP2pAllFailTest, TestSize.Level1)
{
    WifiDirectApiRole role = WIFI_DIRECT_API_ROLE_GO;
    const std::string localMac = "11:22:33:44:55:66";
    const std::string goMac = "11:22:33:44:55:77";

    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnSetLocalNumInfo).WillRepeatedly(Return(SOFTBUS_ERR));
    EXPECT_CALL(mock, LnnSetLocalStrInfo).WillRepeatedly(Return(SOFTBUS_ERR));
    EXPECT_CALL(mock, LnnSyncP2pInfo).WillRepeatedly(Return(SOFTBUS_ERR));
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SyncLnnInfoForP2p(role, localMac, goMac));
}

/*
 * @tc.name: SyncLnnInfoForP2pAllSuccessTest
 * @tc.desc: test SyncLnnInfoForP2p when all calls succeed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SyncLnnInfoForP2pAllSuccessTest, TestSize.Level1)
{
    WifiDirectApiRole role = WIFI_DIRECT_API_ROLE_NONE;
    const std::string localMac = "AA:BB:CC:DD:EE:FF";
    const std::string goMac = "AA:BB:CC:DD:EE:00";

    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnSetLocalNumInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnSetLocalStrInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_CALL(mock, LnnSyncP2pInfo).WillRepeatedly(Return(SOFTBUS_OK));
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SyncLnnInfoForP2p(role, localMac, goMac));
}

/*
 * @tc.name: IsDeviceIdExactLengthTest
 * @tc.desc: test IsDeviceId at exact UUID_BUF_LEN-1 boundary
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDeviceIdExactLengthTest, TestSize.Level1)
{
    std::string exactUuid(UUID_BUF_LEN - 1, 'a');
    auto ret = WifiDirectUtils::IsDeviceId(exactUuid);
    EXPECT_EQ(ret, true);

    std::string shortUuid(UUID_BUF_LEN - 2, 'a');
    ret = WifiDirectUtils::IsDeviceId(shortUuid);
    EXPECT_EQ(ret, false);

    std::string longUuid(UUID_BUF_LEN, 'a');
    ret = WifiDirectUtils::IsDeviceId(longUuid);
    EXPECT_EQ(ret, false);

    std::string emptyUuid;
    ret = WifiDirectUtils::IsDeviceId(emptyUuid);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: CalculateStringLengthSingleCharTest
 * @tc.desc: test CalculateStringLength with single non-null char
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthSingleCharTest, TestSize.Level1)
{
    char str[5] = { 0, 0, 0, 'X', 0 };
    auto ret = WifiDirectUtils::CalculateStringLength(str, sizeof(str));
    EXPECT_EQ(ret, 4);
}

/*
 * @tc.name: CalculateStringLengthAllNullTest
 * @tc.desc: test CalculateStringLength with all null bytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthAllNullTest, TestSize.Level1)
{
    char str[8] = { 0 };
    auto ret = WifiDirectUtils::CalculateStringLength(str, sizeof(str));
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: CalculateStringLengthSizeOneTest
 * @tc.desc: test CalculateStringLength with size=1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthSizeOneTest, TestSize.Level1)
{
    char str = 'A';
    auto ret = WifiDirectUtils::CalculateStringLength(&str, 1);
    EXPECT_EQ(ret, 1);

    char str2 = '\0';
    ret = WifiDirectUtils::CalculateStringLength(&str2, 1);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: CalculateStringLengthZeroSizeTest
 * @tc.desc: test CalculateStringLength with size=0
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CalculateStringLengthZeroSizeTest, TestSize.Level1)
{
    char str[4] = { 'a', 'b', 'c', 'd' };
    auto ret = WifiDirectUtils::CalculateStringLength(str, 0);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: CompareIgnoreCaseCaseInsensitiveTest
 * @tc.desc: test CompareIgnoreCase with different cases but same letters
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CompareIgnoreCaseCaseInsensitiveTest, TestSize.Level1)
{
    std::string left = "HELLO";
    std::string right = "hello";
    int32_t ret = WifiDirectUtils::CompareIgnoreCase(left, right);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: CompareIgnoreCaseBothEmptyTest
 * @tc.desc: test CompareIgnoreCase with both empty strings
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CompareIgnoreCaseBothEmptyTest, TestSize.Level1)
{
    std::string left = "";
    std::string right = "";
    int32_t ret = WifiDirectUtils::CompareIgnoreCase(left, right);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: CompareIgnoreCaseLeftLessTest
 * @tc.desc: test CompareIgnoreCase where left is lexicographically less
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CompareIgnoreCaseLeftLessTest, TestSize.Level1)
{
    std::string left = "abc";
    std::string right = "abd";
    int32_t ret = WifiDirectUtils::CompareIgnoreCase(left, right);
    EXPECT_LT(ret, 0);
}

/*
 * @tc.name: CompareIgnoreCaseLeftGreaterTest
 * @tc.desc: test CompareIgnoreCase where left is lexicographically greater
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, CompareIgnoreCaseLeftGreaterTest, TestSize.Level1)
{
    std::string left = "abd";
    std::string right = "abc";
    int32_t ret = WifiDirectUtils::CompareIgnoreCase(left, right);
    EXPECT_GT(ret, 0);
}

/*
 * @tc.name: ChannelListToStringSingleElementTest
 * @tc.desc: test ChannelListToString with single element
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelListToStringSingleElementTest, TestSize.Level1)
{
    std::vector<int> channels = { 36 };
    auto ret = WifiDirectUtils::ChannelListToString(channels);
    EXPECT_EQ(ret, "36");
}

/*
 * @tc.name: ChannelListToStringLargeListTest
 * @tc.desc: test ChannelListToString with many elements
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ChannelListToStringLargeListTest, TestSize.Level1)
{
    std::vector<int> channels = { 1, 6, 11, 36, 40, 44, 48, 52, 149, 165 };
    auto ret = WifiDirectUtils::ChannelListToString(channels);
    EXPECT_EQ(ret, "1##6##11##36##40##44##48##52##149##165");
}

/*
 * @tc.name: StringToChannelListSingleTest
 * @tc.desc: test StringToChannelList with single channel
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToChannelListSingleTest, TestSize.Level1)
{
    std::string channels = "36";
    auto ret = WifiDirectUtils::StringToChannelList(channels);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0], 36);
}

/*
 * @tc.name: StringToChannelListNegativeTest
 * @tc.desc: test StringToChannelList with negative number
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToChannelListNegativeTest, TestSize.Level1)
{
    std::string channels = "-1";
    auto ret = WifiDirectUtils::StringToChannelList(channels);
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0], -1);
}

/*
 * @tc.name: StringToIntEmptyTest
 * @tc.desc: test StringToInt with empty string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToIntEmptyTest, TestSize.Level1)
{
    std::string str = "";
    int32_t result = 42;
    auto ret = WifiDirectUtils::StringToInt(str, result);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: StringToIntNegativeTest
 * @tc.desc: test StringToInt with negative number
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToIntNegativeTest, TestSize.Level1)
{
    std::string str = "-100";
    int32_t result = 0;
    auto ret = WifiDirectUtils::StringToInt(str, result);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(result, -100);
}

/*
 * @tc.name: StringToIntOverflowTest
 * @tc.desc: test StringToInt with overflow value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToIntOverflowTest, TestSize.Level1)
{
    std::string str = "99999999999999999999";
    int32_t result = 0;
    auto ret = WifiDirectUtils::StringToInt(str, result);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: StringToIntPartialValidTest
 * @tc.desc: test StringToInt with partially valid string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, StringToIntPartialValidTest, TestSize.Level1)
{
    std::string str = "123abc";
    int32_t result = 0;
    auto ret = WifiDirectUtils::StringToInt(str, result);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: BytesToIntSingleByteTest
 * @tc.desc: test BytesToInt with single byte
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntSingleByteTest, TestSize.Level1)
{
    std::vector<uint8_t> data = { 0x42 };
    auto ret = WifiDirectUtils::BytesToInt(data);
    EXPECT_EQ(ret, 0x42);
}

/*
 * @tc.name: BytesToIntThreeBytesTest
 * @tc.desc: test BytesToInt with three bytes
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntThreeBytesTest, TestSize.Level1)
{
    std::vector<uint8_t> data = { 0x78, 0x56, 0x34 };
    auto ret = WifiDirectUtils::BytesToInt(data);
    EXPECT_EQ(ret, 0x345678);
}

/*
 * @tc.name: BytesToIntEmptyTest
 * @tc.desc: test BytesToInt with empty vector
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntEmptyTest, TestSize.Level1)
{
    std::vector<uint8_t> data;
    auto ret = WifiDirectUtils::BytesToInt(data);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: BytesToIntPointerZeroSizeTest
 * @tc.desc: test BytesToInt with pointer and zero size
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntPointerZeroSizeTest, TestSize.Level1)
{
    uint8_t data[4] = { 0x01, 0x02, 0x03, 0x04 };
    auto ret = WifiDirectUtils::BytesToInt(data, 0);
    EXPECT_EQ(ret, 0);
}

/*
 * @tc.name: BytesToIntPointerSingleByteTest
 * @tc.desc: test BytesToInt with pointer and single byte
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BytesToIntPointerSingleByteTest, TestSize.Level1)
{
    uint8_t data = 0xFF;
    auto ret = WifiDirectUtils::BytesToInt(&data, 1);
    EXPECT_EQ(ret, 0xFF);
}

/*
 * @tc.name: Is2GBandBoundaryTest
 * @tc.desc: test Is2GBand at boundary frequencies
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, Is2GBandBoundaryTest, TestSize.Level1)
{
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQUENCY_2G_FIRST), true);
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQUENCY_2G_LAST), true);
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQUENCY_2G_FIRST - 1), false);
    EXPECT_EQ(WifiDirectUtils::Is2GBand(FREQUENCY_2G_LAST + 1), false);
}

/*
 * @tc.name: Is5GBandBoundaryTest
 * @tc.desc: test Is5GBand at boundary frequencies
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, Is5GBandBoundaryTest, TestSize.Level1)
{
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQUENCY_5G_FIRST), true);
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQUENCY_5G_LAST), true);
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQUENCY_5G_FIRST - 1), false);
    EXPECT_EQ(WifiDirectUtils::Is5GBand(FREQUENCY_5G_LAST + 1), false);
}

/*
 * @tc.name: BandWidthNumberToEnumExactly160MTest
 * @tc.desc: test BandWidthNumberToEnum with exactly 160M number
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BandWidthNumberToEnumExactly160MTest, TestSize.Level1)
{
    int bandWidth = WifiDirectUtils::BAND_WIDTH_160M_NUMBER;
    auto ret = WifiDirectUtils::BandWidthNumberToEnum(bandWidth);
    EXPECT_EQ(ret, BAND_WIDTH_160M);
}

/*
 * @tc.name: BandWidthNumberToEnumLessThan160MTest
 * @tc.desc: test BandWidthNumberToEnum with value less than 160M
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BandWidthNumberToEnumLessThan160MTest, TestSize.Level1)
{
    int bandWidth = WifiDirectUtils::BAND_WIDTH_160M_NUMBER - 1;
    auto ret = WifiDirectUtils::BandWidthNumberToEnum(bandWidth);
    EXPECT_EQ(ret, BAND_WIDTH_80M);
}

/*
 * @tc.name: BandWidthEnumToNumber80MTest
 * @tc.desc: test BandWidthEnumToNumber with BAND_WIDTH_80M
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, BandWidthEnumToNumber80MTest, TestSize.Level1)
{
    auto ret = WifiDirectUtils::BandWidthEnumToNumber(BAND_WIDTH_80M);
    EXPECT_EQ(ret, WifiDirectUtils::BAND_WIDTH_80M_NUMBER);
}

/*
 * @tc.name: IsInChannelListFirstElementTest
 * @tc.desc: test IsInChannelList where channel is first element
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsInChannelListFirstElementTest, TestSize.Level1)
{
    int32_t channel = 36;
    std::vector<int> channelArray = { 36, 40, 44 };
    auto ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: IsInChannelListLastElementTest
 * @tc.desc: test IsInChannelList where channel is last element
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsInChannelListLastElementTest, TestSize.Level1)
{
    int32_t channel = 44;
    std::vector<int> channelArray = { 36, 40, 44 };
    auto ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: IsInChannelListSingleElementTest
 * @tc.desc: test IsInChannelList with single element list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsInChannelListSingleElementTest, TestSize.Level1)
{
    int32_t channel = 36;
    std::vector<int> channelArray = { 36 };
    auto ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, true);

    channel = 40;
    ret = WifiDirectUtils::IsInChannelList(channel, channelArray);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name: IpStringToIntArrayAllZerosTest
 * @tc.desc: test IpStringToIntArray with all zeros
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArrayAllZerosTest, TestSize.Level1)
{
    static char addrString[] = "0.0.0.0";
    static const uint32_t LEN = 4;
    uint32_t addrArray[LEN] = { 0 };
    auto ret = WifiDirectUtils::IpStringToIntArray(addrString, addrArray, LEN);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(addrArray[0], 0);
    EXPECT_EQ(addrArray[1], 0);
    EXPECT_EQ(addrArray[2], 0);
    EXPECT_EQ(addrArray[3], 0);
}

/*
 * @tc.name: IpStringToIntArrayMaxValuesTest
 * @tc.desc: test IpStringToIntArray with max values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArrayMaxValuesTest, TestSize.Level1)
{
    static char addrString[] = "255.255.255.255";
    static const uint32_t LEN = 4;
    uint32_t addrArray[LEN] = { 0 };
    auto ret = WifiDirectUtils::IpStringToIntArray(addrString, addrArray, LEN);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(addrArray[0], 255);
    EXPECT_EQ(addrArray[1], 255);
    EXPECT_EQ(addrArray[2], 255);
    EXPECT_EQ(addrArray[3], 255);
}

/*
 * @tc.name: IpStringToIntArrayExtraPartsTest
 * @tc.desc: test IpStringToIntArray with extra parts
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IpStringToIntArrayExtraPartsTest, TestSize.Level1)
{
    static char addrString[] = "192.168.1.1.1";
    static const uint32_t LEN = 4;
    uint32_t addrArray[LEN] = { 0 };
    auto ret = WifiDirectUtils::IpStringToIntArray(addrString, addrArray, LEN);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: ToWifiDirectRoleDefaultTest
 * @tc.desc: test ToWifiDirectRole with default case value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, ToWifiDirectRoleDefaultTest, TestSize.Level1)
{
    LinkInfo::LinkMode mode = static_cast<LinkInfo::LinkMode>(999);
    auto ret = WifiDirectUtils::ToWifiDirectRole(mode);
    EXPECT_EQ(ret, WifiDirectRole::WIFI_DIRECT_ROLE_INVALID);
}

/*
 * @tc.name: SupportHmlDefaultTrueTest
 * @tc.desc: test SupportHml returns true by default
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SupportHmlDefaultTrueTest, TestSize.Level1)
{
    bool ret = WifiDirectUtils::SupportHml();
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: SupportHmlTwoDefaultTrueTest
 * @tc.desc: test SupportHmlTwo returns true by default
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SupportHmlTwoDefaultTrueTest, TestSize.Level1)
{
    bool ret = WifiDirectUtils::SupportHmlTwo();
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name: GetRemoteConnSubFeatureSuccessWithValueTest
 * @tc.desc: test GetRemoteConnSubFeature succeeds with a value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetRemoteConnSubFeatureSuccessWithValueTest, TestSize.Level1)
{
    uint64_t feature = 0;
    std::string networkId = "1234567890";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetRemoteNumU64Info(_, _, _))
        .WillOnce([](const std::string &networkId, InfoKey key, uint64_t *info) {
            *info = 0xABCD;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::GetRemoteConnSubFeature(networkId, feature);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(feature, 0xABCD);
}

/*
 * @tc.name: GetLocalConnSubFeatureSuccessWithValueTest
 * @tc.desc: test GetLocalConnSubFeature succeeds with a value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, GetLocalConnSubFeatureSuccessWithValueTest, TestSize.Level1)
{
    uint64_t feature = 0;
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnGetLocalNumU64Info(_, _))
        .WillOnce([](InfoKey key, uint64_t *info) {
            *info = 0x1234;
            return SOFTBUS_OK;
        });
    auto ret = WifiDirectUtils::GetLocalConnSubFeature(feature);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(feature, 0x1234);
}

/*
 * @tc.name: SetLocalWifiDirectMacFailedTest
 * @tc.desc: test SetLocalWifiDirectMac when LnnSetLocalStrInfo fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, SetLocalWifiDirectMacFailedTest, TestSize.Level1)
{
    std::string mac = "AA:BB:CC:DD:EE:FF";
    WifiDirectInterfaceMock mock;
    EXPECT_CALL(mock, LnnSetLocalStrInfo).WillOnce(Return(SOFTBUS_ERR));
    EXPECT_NO_FATAL_FAILURE(WifiDirectUtils::SetLocalWifiDirectMac(mac));
}

/*
 * @tc.name: MacStringToArrayLowercaseTest
 * @tc.desc: test MacStringToArray with lowercase hex
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayLowercaseTest, TestSize.Level1)
{
    std::string macString = "aa:bb:cc:dd:ee:ff";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 6);
    EXPECT_EQ(ret[0], 0xaa);
    EXPECT_EQ(ret[5], 0xff);
}

/*
 * @tc.name: MacStringToArrayMixedCaseTest
 * @tc.desc: test MacStringToArray with mixed case hex
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, MacStringToArrayMixedCaseTest, TestSize.Level1)
{
    std::string macString = "Aa:Bb:Cc:Dd:Ee:Ff";
    auto ret = WifiDirectUtils::MacStringToArray(macString);
    EXPECT_EQ(ret.size(), 6);
    EXPECT_EQ(ret[0], 0xAA);
    EXPECT_EQ(ret[5], 0xFF);
}

/*
 * @tc.name: IsDfsChannelBoundaryTest
 * @tc.desc: test IsDfsChannel at boundary channels
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDfsChannelBoundaryTest, TestSize.Level1)
{
    int freqDfsFirst = (52 - CHANNEL_5G_FIRST) * FREQUENCY_STEP + FREQUENCY_5G_FIRST;
    int freqDfsLast = (64 - CHANNEL_5G_FIRST) * FREQUENCY_STEP + FREQUENCY_5G_FIRST;
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(freqDfsFirst), true);
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(freqDfsLast), true);

    int freqBeforeDfs = (51 - CHANNEL_5G_FIRST) * FREQUENCY_STEP + FREQUENCY_5G_FIRST;
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(freqBeforeDfs), false);

    int freqAfterDfs = (65 - CHANNEL_5G_FIRST) * FREQUENCY_STEP + FREQUENCY_5G_FIRST;
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(freqAfterDfs), false);
}

/*
 * @tc.name: IsDfsChannelInvalidFreqTest
 * @tc.desc: test IsDfsChannel with invalid frequency
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(WifiDirectUtilsTest, IsDfsChannelInvalidFreqTest, TestSize.Level1)
{
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(FREQ1), false);
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(0), false);
    EXPECT_EQ(WifiDirectUtils::IsDfsChannel(-1), false);
}

} // namespace OHOS::SoftBus