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
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "securec.h"

#include "disc_log.h"
#include "message_handler.h"
#include "softbus_adapter_mem.h"
#include "softbus_ble_mock.h"
#include "softbus_broadcast_utils.h"
#include "softbus_error_code.h"

using namespace testing::ext;
using ::testing::Return;

#define BC_ADV_FLAG                  0x2
#define BC_ADV_TX_POWER_DEFAULT      (-6)
#define BC_CHANNLE_MAP               0x0
#define BC_INTERNAL                  48
#define MANUFACTURE_COMPANY_ID       0x027D
#define SERVICE_UUID                 0xFDEE
#define SRV_TYPE_INVALID             (-1)

namespace OHOS {
class SoftbusBroadcastMgrTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        LooperInit();
    }

    static void TearDownTestCase()
    {
        LooperDeinit();
    }

    void SetUp() override {}
    void TearDown() override {}

    static constexpr char BC_DATA_PAYLOAD[] = {0x04, 0x05, 0x90, 0x00, 0x00, 0x10, 0x00, 0x18, 0xB9, 0x13, 0x3D, 0x28,
        0xFC, 0x0D, 0x7F, 0xAB, 0x21, 0x00, 0x30, 0x4F, 0x70, 0x65, 0x6E, 0x48};
    static constexpr char RSP_DATA_PAYLOAD[] = {0x61, 0x72, 0x6D, 0x6F, 0x6E, 0x79, 0x20, 0x33, 0x2E, 0x32, 0x00};
};

static void BuildBroadcastParam(BroadcastParam *bcParam)
{
    bcParam->minInterval = BC_INTERNAL;
    bcParam->maxInterval = BC_INTERNAL;
    bcParam->advType = SOFTBUS_BC_ADV_IND;
    bcParam->ownAddrType = SOFTBUS_BC_PUBLIC_DEVICE_ADDRESS;
    bcParam->peerAddrType = SOFTBUS_BC_PUBLIC_DEVICE_ADDRESS;
    bcParam->channelMap = BC_CHANNLE_MAP;
    bcParam->txPower = BC_ADV_TX_POWER_DEFAULT;
}

static void BuildBroadcastPacketExceptPayload(BroadcastPacket *packet)
{
    packet->bcData.type = BC_DATA_TYPE_SERVICE;
    packet->bcData.id = SERVICE_UUID;
    packet->rspData.type = BC_DATA_TYPE_MANUFACTURER;
    packet->rspData.id = MANUFACTURE_COMPANY_ID;
    packet->isSupportFlag = true;
    packet->flag = BC_ADV_FLAG;
}

static void BuildScanParam(BcScanParams *scanParam)
{
    scanParam->scanInterval = SOFTBUS_BC_SCAN_INTERVAL_P2;
    scanParam->scanWindow = SOFTBUS_BC_SCAN_WINDOW_P2;
    scanParam->scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE;
    scanParam->scanPhy = SOFTBUS_BC_SCAN_PHY_1M;
    scanParam->scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL;
}

// filter is released in UnRegisterScanListener
static BcScanFilter *GetBcScanFilter(void)
{
    unsigned char serviceData[] = {0x04, 0x05, 0x90};
    unsigned char serviceDataMask[] = {0xFF, 0xFF, 0xFF};
    int serviceDataLength = sizeof(serviceData);

    BcScanFilter *filter = static_cast<BcScanFilter *>(SoftBusCalloc(sizeof(BcScanFilter)));
    DISC_CHECK_AND_RETURN_RET_LOGW(filter != nullptr, nullptr, DISC_TEST, "malloc filter failed");
    
    unsigned char *data = static_cast<unsigned char *>(SoftBusCalloc(serviceDataLength));
    unsigned char *mask = static_cast<unsigned char *>(SoftBusCalloc(serviceDataLength));
    if (data == nullptr || mask == nullptr) {
        goto EXIT;
    }
    if (memcpy_s(data, serviceDataLength, serviceData, serviceDataLength) != EOK) {
        goto EXIT;
    }
    if (memcpy_s(mask, serviceDataLength, serviceDataMask, serviceDataLength) != EOK) {
        goto EXIT;
    }

    filter->serviceUuid = SERVICE_UUID;
    filter->serviceData = data;
    filter->serviceDataMask = mask;
    filter->serviceDataLength = serviceDataLength;
    return filter;
EXIT:
    SoftBusFree(filter);
    SoftBusFree(data);
    SoftBusFree(mask);
    return nullptr;
}

static void BleBcEnableCallback(int channel, int status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

static void BleBcDisableCallback(int channel, int status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

static void BleBcUpdateCallback(int channel, int status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

static void BleBcDataCallback(int channel, int status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

static void BleOnScanStart(int listenerId, int status)
{
    (void)listenerId;
    (void)status;
    DISC_LOGI(DISC_TEST, "BleOnScanStart");
}

static void BleOnScanStop(int listenerId, int status)
{
    (void)listenerId;
    (void)status;
    DISC_LOGI(DISC_TEST, "BleOnScanStop");
}

static void BleScanResultCallback(int listenerId, const BroadcastReportInfo *reportInfo)
{
    (void)listenerId;
}

static BroadcastCallback *GetBroadcastCallback()
{
    static BroadcastCallback g_bcCallback = {
        .OnStartBroadcastingCallback = BleBcEnableCallback,
        .OnStopBroadcastingCallback = BleBcDisableCallback,
        .OnUpdateBroadcastingCallback = BleBcUpdateCallback,
        .OnSetBroadcastingCallback = BleBcDataCallback,
    };
    return &g_bcCallback;
}

static ScanCallback *GetScanCallback()
{
    static ScanCallback g_scanListener = {
        .OnStartScanCallback = BleOnScanStart,
        .OnStopScanCallback = BleOnScanStop,
        .OnReportScanDataCallback = BleScanResultCallback,
    };
    return &g_scanListener;
}

/*
 * @tc.name: SoftbusBroadcastMgrInit001
 * @tc.desc: Init successful.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastMgrInit001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrInit001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrInit001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastMgrInit002
 * @tc.desc: Repeated initializations successful.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastMgrInit002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrInit002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrInit002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastMgrDeInit001
 * @tc.desc: Repeated deinitializations successful.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastMgrDeInit001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrDeInit001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrDeInit001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastMgrDeInit002
 * @tc.desc: Deinit without initialization.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastMgrDeInit002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrDeInit002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastMgrDeInit002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastInterface001
 * @tc.desc: Calls the interfaces without initialization.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastInterface001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastInterface001 begin ----");
    ManagerMock managerMock;

    int32_t bcId = -1;
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_ERR, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_ERR, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_ERR, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_ERR, UnRegisterScanListener(listenerId));

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));

    EXPECT_EQ(SOFTBUS_ERR, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_ERR, SetBroadcastingData(bcId, &packet));
    EXPECT_EQ(SOFTBUS_ERR, StopBroadcasting(bcId));

    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_ERR, StartScan(listenerId, &scanParam));
    EXPECT_EQ(SOFTBUS_ERR, StopScan(listenerId));
    EXPECT_FALSE(BroadcastIsLpDeviceAvailable());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastInterface001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastRegisterBroadcaster001
 * @tc.desc: Invalid parameter, register broadcaster fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastRegisterBroadcaster001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterBroadcaster001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, RegisterBroadcaster(static_cast<BaseServiceType>(SRV_TYPE_INVALID), &bcId,
        GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, RegisterBroadcaster(SRV_TYPE_DIS, nullptr, GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, nullptr));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterBroadcaster001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastRegisterBroadcaster002
 * @tc.desc: Register broadcaster and unregister broadcaster success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastRegisterBroadcaster002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterBroadcaster002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterBroadcaster002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastRegisterBroadcaster003
 * @tc.desc: Duplicate registration.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastRegisterBroadcaster003, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterBroadcaster003 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId[BC_NUM_MAX + 1] = {-1};
    for (int32_t i = 0; i < BC_NUM_MAX; i++) {
        EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId[i], GetBroadcastCallback()));
        EXPECT_TRUE(bcId[i] >= 0);
    }

    EXPECT_EQ(SOFTBUS_ERR, RegisterBroadcaster(SRV_TYPE_DIS, &bcId[BC_NUM_MAX], GetBroadcastCallback()));
    
    for (int32_t i = 0; i < BC_NUM_MAX; i++) {
        EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId[i]));
    }

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterBroadcaster003 end ----");
}

/*
 * @tc.name: SoftbusBroadcastUnRegisterBroadcaster001
 * @tc.desc: Unregister without registration.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastUnRegisterBroadcaster001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterBroadcaster001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t invalidId = -1;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterBroadcaster(invalidId));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterBroadcaster(BC_NUM_MAX));
    invalidId = 1;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterBroadcaster(invalidId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterBroadcaster001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastUnRegisterBroadcaster002
 * @tc.desc: Unregister when broadcasting.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastUnRegisterBroadcaster002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterBroadcaster002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterBroadcaster002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastRegisterScanListener001
 * @tc.desc: Invalid parameter, register listener fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastRegisterScanListener001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterScanListener001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, RegisterScanListener(static_cast<BaseServiceType>(SRV_TYPE_INVALID), &listenerId,
        GetScanCallback()));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, RegisterScanListener(SRV_TYPE_DIS, nullptr, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, RegisterScanListener(SRV_TYPE_DIS, &listenerId, nullptr));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterScanListener001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastRegisterScanListener002
 * @tc.desc: Register listener and unregister listener success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastRegisterScanListener002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterScanListener002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterScanListener002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastRegisterScanListener003
 * @tc.desc: Duplicate registration.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastRegisterScanListener003, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterScanListener003 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);
    EXPECT_EQ(SOFTBUS_ERR, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_SH, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);
    EXPECT_EQ(SOFTBUS_ERR, RegisterScanListener(SRV_TYPE_SH, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastRegisterScanListener003 end ----");
}

/*
 * @tc.name: SoftbusBroadcastUnRegisterScanListener001
 * @tc.desc: Unregister without registration.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastUnRegisterScanListener001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterScanListener001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t invalidId = -1;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterScanListener(invalidId));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterScanListener(SCAN_NUM_MAX));
    invalidId = 0;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterScanListener(invalidId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterScanListener001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastUnRegisterScanListener002
 * @tc.desc: Unregister when scanning.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastUnRegisterScanListener002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterScanListener002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    uint8_t filterNum = 1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);

    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, filterNum));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUnRegisterScanListener002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartBroadcasting001
 * @tc.desc: Invalid parameter, start broadcasting fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartBroadcasting001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartBroadcasting001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartBroadcasting(bcId, nullptr, &packet));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartBroadcasting(bcId, &bcParam, nullptr));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartBroadcasting(bcId, &bcParam, &packet));

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));
    int32_t invalidBcId = -1;
    EXPECT_EQ(SOFTBUS_ERR, StartBroadcasting(invalidBcId, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartBroadcasting001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartBroadcasting002
 * @tc.desc: Start broadcasting and stop broadcasting success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartBroadcasting002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartBroadcasting002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));

    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartBroadcasting002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartBroadcasting003
 * @tc.desc: Duplicate start broadcasting.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartBroadcasting003, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartBroadcasting003 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));

    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_ERR, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartBroadcasting003 end ----");
}

/*
 * @tc.name: SoftbusBroadcastUpdateBroadcasting001
 * @tc.desc: Invalid parameter, update broadcasting fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastUpdateBroadcasting001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUpdateBroadcasting001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UpdateBroadcasting(bcId, nullptr, &packet));
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UpdateBroadcasting(bcId, &bcParam, nullptr));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUpdateBroadcasting001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastUpdateBroadcasting002
 * @tc.desc: Update broadcasting success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastUpdateBroadcasting002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUpdateBroadcasting002 begin ----");
        ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));

    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_OK, UpdateBroadcasting(bcId, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastUpdateBroadcasting002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastSetBroadcastingData001
 * @tc.desc: Invalid parameter, set broadcasting data fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastSetBroadcastingData001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetBroadcastingData001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t invalidId = -1;
    BroadcastPacket packet = {};
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, SetBroadcastingData(invalidId, nullptr));
    EXPECT_EQ(SOFTBUS_ERR, SetBroadcastingData(invalidId, &packet));
    EXPECT_EQ(SOFTBUS_ERR, SetBroadcastingData(BC_NUM_MAX, &packet));
    invalidId = 0;
    EXPECT_EQ(SOFTBUS_ERR, SetBroadcastingData(invalidId, &packet));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetBroadcastingData001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastSetBroadcastingData002
 * @tc.desc: Set broadcasting data without start.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastSetBroadcastingData002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetBroadcastingData002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastPacket packet = {};
    EXPECT_EQ(SOFTBUS_ERR, SetBroadcastingData(bcId, &packet));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetBroadcastingData002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastSetBroadcastingData003
 * @tc.desc: Set broadcasting data success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastSetBroadcastingData003, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetBroadcastingData003 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildBroadcastParam(&bcParam);
    BuildBroadcastPacketExceptPayload(&packet);

    packet.bcData.payloadLen = sizeof(BC_DATA_PAYLOAD);
    packet.bcData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(BC_DATA_PAYLOAD));
    packet.rspData.payloadLen = sizeof(RSP_DATA_PAYLOAD);
    packet.rspData.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(RSP_DATA_PAYLOAD));

    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_OK, SetBroadcastingData(bcId, &packet));
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetBroadcastingData003 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStopBroadcasting001
 * @tc.desc: Invalid parameter, stop broadcasting fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStopBroadcasting001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopBroadcasting001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());

    int32_t invalidId = -1;
    EXPECT_EQ(SOFTBUS_ERR, StopBroadcasting(invalidId));
    EXPECT_EQ(SOFTBUS_ERR, StopBroadcasting(BC_NUM_MAX));
    invalidId = 0;
    EXPECT_EQ(SOFTBUS_ERR, StopBroadcasting(invalidId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopBroadcasting001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStopBroadcasting002
 * @tc.desc: Stop broadcasting without start.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStopBroadcasting002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopBroadcasting002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_TRUE(bcId >= 0);

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopBroadcasting002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastSetScanFilter001
 * @tc.desc: Invalid parameter, set filter fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastSetScanFilter001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetScanFilter001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    uint8_t filterNum = 0;
    BcScanFilter filter = {};
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, SetScanFilter(listenerId, &filter, filterNum));

    filterNum = 1;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, SetScanFilter(listenerId, nullptr, filterNum));
    EXPECT_EQ(SOFTBUS_ERR, SetScanFilter(listenerId, &filter, filterNum));
    EXPECT_EQ(SOFTBUS_ERR, SetScanFilter(SCAN_NUM_MAX, &filter, filterNum));
    listenerId = 0;
    EXPECT_EQ(SOFTBUS_ERR, SetScanFilter(listenerId, &filter, filterNum));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetScanFilter001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastSetScanFilter002
 * @tc.desc: Set filter success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastSetScanFilter002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetScanFilter002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    uint8_t filterNum = 1;
    BcScanFilter *filter = GetBcScanFilter();

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);

    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, filterNum));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastSetScanFilter002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartScan001
 * @tc.desc: Invalid parameter, start scan fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartScan001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);
    EXPECT_EQ(SOFTBUS_ERR, StartScan(listenerId, &scanParam));

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartScan(listenerId, nullptr));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartScan002
 * @tc.desc: Start scan without filter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartScan002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);
    EXPECT_EQ(SOFTBUS_ERR, StartScan(listenerId, &scanParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan002 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartScan003
 * @tc.desc: Start scan and stop scan success.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartScan003, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan003 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    uint8_t filterNum = 1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);

    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, filterNum));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan003 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStartScan004
 * @tc.desc: Duplicate start scan.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStartScan004, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan004 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    uint8_t filterNum = 1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);

    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, filterNum));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStartScan004 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStopScan001
 * @tc.desc: Invalid parameter, stop scan fail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStopScan001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopScan001 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t invalidId = -1;
    EXPECT_EQ(SOFTBUS_ERR, StopScan(invalidId));
    EXPECT_EQ(SOFTBUS_ERR, StopScan(SCAN_NUM_MAX));
    invalidId = 0;
    EXPECT_EQ(SOFTBUS_ERR, StopScan(invalidId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopScan001 end ----");
}

/*
 * @tc.name: SoftbusBroadcastStopScan002
 * @tc.desc: Stop scan without start.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, SoftbusBroadcastStopScan002, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopScan002 begin ----");
    ManagerMock managerMock;

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_TRUE(listenerId >= 0);
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());

    DISC_LOGI(DISC_TEST, "SoftbusBroadcastStopScan002 end ----");
}

} // namespace OHOS