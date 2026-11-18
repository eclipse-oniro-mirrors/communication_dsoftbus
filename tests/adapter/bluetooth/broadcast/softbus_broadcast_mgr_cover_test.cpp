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

#include "securec.h"
#include <unistd.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "disc_log.h"
#include "g_enhance_adapter_func.h"
#include "softbus_adapter_mem.h"
#include "softbus_ble_mock.h"
#include "softbus_broadcast_mgr_mock.h"
#include "softbus_broadcast_mgr_test_common.h"
#include "softbus_error_code.h"

using namespace testing::ext;
using ::testing::Return;
using namespace testing;

namespace OHOS {
// ---------------------------------------------------------------------------
// Coverage tests driven by the configurable stub medium interface.
// ---------------------------------------------------------------------------
#define TEST_FILTER_INDEX_MAX 33
#define STUB_BC_NUM_SEVEN 7

// Filter with address and deviceName fields set
static BcScanFilter *GetBcScanFilterFull(void)
{
    BcScanFilter *filter = GetBcScanFilter();
    const char addr[] = "11:22:33:44:55:66";
    const char devName[] = "stubDev";
    filter->address = (int8_t *)SoftBusCalloc(sizeof(addr));
    filter->deviceName = (int8_t *)SoftBusCalloc(sizeof(devName));
    if (memcpy_s(filter->address, sizeof(addr), addr, sizeof(addr)) != EOK ||
        memcpy_s(filter->deviceName, sizeof(devName), devName, sizeof(devName)) != EOK) {
        DISC_LOGE(DISC_TEST, "memcpy failed");
    }
    return filter;
}

static BcScanFilter *GetManufactureScanFilter(void)
{
    uint8_t manuData[] = { 0x04, 0x05 };
    uint8_t manuMask[] = { 0xFF, 0xFF };
    BcScanFilter *filter = (BcScanFilter *)SoftBusCalloc(sizeof(BcScanFilter));
    filter->manufactureId = MANUFACTURE_COMPANY_ID;
    filter->manufactureDataLength = sizeof(manuData);
    filter->manufactureData = (uint8_t *)SoftBusCalloc(sizeof(manuData));
    filter->manufactureDataMask = (uint8_t *)SoftBusCalloc(sizeof(manuMask));
    if (memcpy_s(filter->manufactureData, sizeof(manuData), manuData, sizeof(manuData)) != EOK ||
        memcpy_s(filter->manufactureDataMask, sizeof(manuMask), manuMask, sizeof(manuMask)) != EOK) {
        DISC_LOGE(DISC_TEST, "memcpy failed");
    }
    return filter;
}

static BcScanFilter *GetServiceUuidScanFilter(void)
{
    uint8_t uuidData[] = { 0xAA, 0xBB };
    uint8_t uuidMask[] = { 0xFF, 0xFF };
    BcScanFilter *filter = (BcScanFilter *)SoftBusCalloc(sizeof(BcScanFilter));
    filter->serviceUuidId = SERVICE_UUID;
    filter->serviceUuidDataLength = sizeof(uuidData);
    filter->serviceUuidData = (uint8_t *)SoftBusCalloc(sizeof(uuidData));
    filter->serviceUuidDataMask = (uint8_t *)SoftBusCalloc(sizeof(uuidMask));
    if (memcpy_s(filter->serviceUuidData, sizeof(uuidData), uuidData, sizeof(uuidData)) != EOK ||
        memcpy_s(filter->serviceUuidDataMask, sizeof(uuidMask), uuidMask, sizeof(uuidMask)) != EOK) {
        DISC_LOGE(DISC_TEST, "memcpy failed");
    }
    return filter;
}

static void InvokeScanReport(uint8_t bcType, uint16_t bcDataId, uint8_t *bcPayload, uint16_t bcLen,
    uint8_t rspType, uint16_t rspId, uint8_t *rspPayload, uint16_t rspLen)
{
    SoftBusBcScanResult report = {};
    report.data.bcData.type = (SoftbusBcDataType)bcType;
    report.data.bcData.id = bcDataId;
    report.data.bcData.payload = bcPayload;
    report.data.bcData.payloadLen = bcLen;
    report.data.rspData.type = (SoftbusBcDataType)rspType;
    report.data.rspData.id = rspId;
    report.data.rspData.payload = rspPayload;
    report.data.rspData.payloadLen = rspLen;
    g_stubScanCallback->OnReportScanDataCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, &report);
}

static void BuildValidPacket(BroadcastParam *bcParam, BroadcastPacket *packet)
{
    BuildBroadcastParam(bcParam);
    BuildBroadcastPacketExceptPayload(packet);
    packet->bcData.payloadLen = sizeof(SoftbusBroadcastMgrTest::BC_DATA_PAYLOAD);
    packet->bcData.payload = SoftbusBroadcastMgrTest::BC_DATA_PAYLOAD;
    packet->rspData.payloadLen = sizeof(SoftbusBroadcastMgrTest::RSP_DATA_PAYLOAD);
    packet->rspData.payload = SoftbusBroadcastMgrTest::RSP_DATA_PAYLOAD;
}

/*
 * @tc.name: MgrCoverEnhanceInit001
 * @tc.desc: Cover SoftbusBleAdapterInitPacked branch calling the enhance func
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverEnhanceInit001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverEnhanceInit001 begin ----");
    ManagerMock managerMock;

    AdapterEnhanceFuncListGet()->softbusBleAdapterInit = ActionOfRegisterStubMediumInterface;
    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    AdapterEnhanceFuncListGet()->softbusBleAdapterInit = nullptr;

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverEnhanceInit001 end ----");
}

/*
 * @tc.name: MgrCoverAdapterCbNotAvailable001
 * @tc.desc: Cover "bcManager not available"/mismatch branches of the adapter broadcast callbacks
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverAdapterCbNotAvailable001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverAdapterCbNotAvailable001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    const SoftbusBroadcastCallback *cb = g_stubBcCallback;
    EXPECT_TRUE(cb != nullptr);
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));

    // After unregister the manager slot is reset to (BROADCAST_PROTOCOL_BUTT, -1, isUsed=false),
    // invoking the callbacks with that identity drives the "not available" branches.
    cb->OnStartBroadcastingCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);
    cb->OnStopBroadcastingCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);
    cb->OnSetBroadcastingCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);
    cb->OnSetBroadcastingParamCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);
    cb->OnEnableBroadcastingCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);
    cb->OnDisableBroadcastingCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverAdapterCbNotAvailable001 end ----");
}

/*
 * @tc.name: MgrCoverStopCbFailStatus001
 * @tc.desc: Cover the status != SUCCESS branch of BcStopBroadcastingCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStopCbFailStatus001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStopCbFailStatus001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    ASSERT_TRUE(g_stubBcCallback != nullptr);
    g_stubBcCallback->OnStopBroadcastingCallback(BROADCAST_PROTOCOL_BLE, STUB_BC_ID_BASE, SOFTBUS_BC_STATUS_FAIL);

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStopCbFailStatus001 end ----");
}

/*
 * @tc.name: MgrCoverStartReportMax001
 * @tc.desc: Cover UpdateBcMaxExtra report/return branches and g_bcOverMaxNum increase
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStartReportMax001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStartReportMax001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);

    int32_t bcId[STUB_BC_NUM_SEVEN];
    for (int32_t i = 0; i < STUB_BC_NUM_SEVEN; i++) {
        bcId[i] = -1;
        EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
            SRV_TYPE_DIS, &bcId[i], GetBroadcastCallback()));
    }
    // 5th start covers g_bcCurrentNum > BC_DFX_REPORT_NUM, 7th covers g_bcCurrentNum >= MAX_BLE_ADV_NUM
    for (int32_t i = 0; i < STUB_BC_NUM_SEVEN; i++) {
        EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId[i], &bcParam, &packet));
    }
    for (int32_t i = 0; i < STUB_BC_NUM_SEVEN - 1; i++) {
        EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId[i]));
    }
    // g_bcCurrentNum < g_bcMaxNum now, covers the early return in UpdateBcMaxExtra
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId[0], &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId[0]));
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId[STUB_BC_NUM_SEVEN - 1]));
    for (int32_t i = 0; i < STUB_BC_NUM_SEVEN; i++) {
        EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId[i]));
    }
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStartReportMax001 end ----");
}

/*
 * @tc.name: MgrCoverStartWaitSignal001
 * @tc.desc: Cover StartBroadcastingWaitSignal path when isAdvertising && !isStarted
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStartWaitSignal001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStartWaitSignal001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, SoftBusCondWait).WillRepeatedly(Return(SOFTBUS_OK));

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));

    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    // Stop without invoking the adapter callback: isStarted=false while isAdvertising stays true
    g_stubMediumConfig.invokeStopBcCb = false;
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStartWaitSignal001 end ----");
}

/*
 * @tc.name: MgrCoverStartAdapterFail001
 * @tc.desc: Cover StartBroadcasting adapter call failure branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStartAdapterFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStartAdapterFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));

    g_stubMediumConfig.startBcRet = SOFTBUS_ERR;
    g_stubMediumConfig.invokeStartBcCb = false;
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_ERR, StartBroadcasting(bcId, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStartAdapterFail001 end ----");
}

/*
 * @tc.name: MgrCoverSetDataAbnormal001
 * @tc.desc: Cover SetBroadcastingData func-NULL and adapter-failure branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverSetDataAbnormal001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverSetDataAbnormal001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumInterface.SetBroadcastingData = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, SetBroadcastingData(bcId, &packet));
    ResetStubMediumInterfaceFuncs();
    g_stubMediumConfig.setBcDataRet = SOFTBUS_ERR;
    EXPECT_EQ(SOFTBUS_ERR, SetBroadcastingData(bcId, &packet));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverSetDataAbnormal001 end ----");
}

/*
 * @tc.name: MgrCoverDisableAbnormal001
 * @tc.desc: Cover DisableBroadcasting func-NULL and adapter-failure branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverDisableAbnormal001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverDisableAbnormal001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumInterface.DisableBroadcasting = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, DisableBroadcasting(bcId));
    ResetStubMediumInterfaceFuncs();
    g_stubMediumConfig.disableBcRet = SOFTBUS_ERR;
    EXPECT_EQ(SOFTBUS_ERR, DisableBroadcasting(bcId));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverDisableAbnormal001 end ----");
}

/*
 * @tc.name: MgrCoverEnableFuncNull001
 * @tc.desc: Cover EnableBroadcasting func-NULL branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverEnableFuncNull001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverEnableFuncNull001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumInterface.EnableBroadcasting = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, EnableBroadcasting(bcId));

    ResetStubMediumInterfaceFuncs();
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverEnableFuncNull001 end ----");
}

/*
 * @tc.name: MgrCoverSetParamFuncNull001
 * @tc.desc: Cover SetBroadcastingParam func-NULL branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverSetParamFuncNull001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverSetParamFuncNull001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumInterface.SetBroadcastingParam = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, SetBroadcastingParam(bcId, &bcParam));

    ResetStubMediumInterfaceFuncs();
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverSetParamFuncNull001 end ----");
}

/*
 * @tc.name: MgrCoverPerformSetParamWait001
 * @tc.desc: Cover DisableBroadcastingWaitSignal branch in PerformSetBroadcastingParam
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverPerformSetParamWait001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverPerformSetParamWait001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, SoftBusCondWait).WillRepeatedly(Return(SOFTBUS_OK));

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    // Disable/enable succeed without adapter callbacks: isDisableCb stays false
    g_stubMediumConfig.invokeDisableBcCb = false;
    g_stubMediumConfig.invokeEnableBcCb = false;
    EXPECT_EQ(SOFTBUS_OK, SetBroadcastingParam(bcId, &bcParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverPerformSetParamWait001 end ----");
}

/*
 * @tc.name: MgrCoverStopAbnormal001
 * @tc.desc: Cover LogStopSrvType HB branch, StopBroadcasting interface check and null cb branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStopAbnormal001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStopAbnormal001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);

    // HB srvType hits the SRV_TYPE_HB branch of LogStopSrvType
    int32_t hbBcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_HB, &hbBcId, GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(hbBcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(hbBcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(hbBcId));

    // interface check failure branch
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    g_stubMediumInterface.StopBroadcasting = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StopBroadcasting(bcId));
    ResetStubMediumInterfaceFuncs();

    // registered callback without OnStopBroadcastingCallback hits the null cb branch
    BroadcastCallback partialCb = {};
    partialCb.OnStartBroadcastingCallback = BleBcEnableCallback;
    int32_t partialBcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE, SRV_TYPE_DIS, &partialBcId, &partialCb));
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(partialBcId, &bcParam, &packet));
    EXPECT_EQ(SOFTBUS_BC_MGR_INVALID_BC_ID, StopBroadcasting(partialBcId));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(partialBcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStopAbnormal001 end ----");
}

/*
 * @tc.name: MgrCoverUnRegisterBcAbnormal001
 * @tc.desc: Cover UnRegisterBroadcaster func-NULL and adapter-failure branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverUnRegisterBcAbnormal001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverUnRegisterBcAbnormal001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));

    g_stubMediumInterface.UnRegisterBroadcaster = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, UnRegisterBroadcaster(bcId));
    ResetStubMediumInterfaceFuncs();
    g_stubMediumConfig.unRegisterBroadcasterRet = SOFTBUS_ERR;
    EXPECT_EQ(SOFTBUS_ERR, UnRegisterBroadcaster(bcId));
    g_stubMediumConfig.unRegisterBroadcasterRet = SOFTBUS_OK;
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverUnRegisterBcAbnormal001 end ----");
}

/*
 * @tc.name: MgrCoverRegisterBcAdapterFail001
 * @tc.desc: Cover RegisterBroadcaster adapter call failure branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverRegisterBcAdapterFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverRegisterBcAdapterFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    g_stubMediumConfig.registerBroadcasterRet = SOFTBUS_ERR;
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_ERR, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverRegisterBcAdapterFail001 end ----");
}

/*
 * @tc.name: MgrCoverCondInitFail001
 * @tc.desc: Cover all SoftBusCondInit failure branches in InitializeBroadcaster
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverCondInitFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverCondInitFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    for (int32_t i = 0; i < 4; i++) {
        SetStubCondInitFailCountdown(i);
        int32_t bcId = -1;
        EXPECT_EQ(SOFTBUS_ERR, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
            SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    }

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverCondInitFail001 end ----");
}

/*
 * @tc.name: MgrCoverRegisterScanAdapterFail001
 * @tc.desc: Cover RegisterScanListener adapter call failure branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverRegisterScanAdapterFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverRegisterScanAdapterFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    g_stubMediumConfig.registerScanListenerRet = SOFTBUS_ERR;
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_ERR, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverRegisterScanAdapterFail001 end ----");
}

/*
 * @tc.name: MgrCoverScanFreqMap001
 * @tc.desc: Cover GetScanFreq/GetScanIntervalAndWindow mapping branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverScanFreqMap001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverScanFreqMap001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));

    scanParam.scanInterval = SOFTBUS_BC_SCAN_INTERVAL_120_P50;
    scanParam.scanWindow = SOFTBUS_BC_SCAN_WINDOW_60_P50;
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    scanParam.scanInterval = SOFTBUS_BC_SCAN_INTERVAL_P10_LONG_RANGE;
    scanParam.scanWindow = SOFTBUS_BC_SCAN_WINDOW_P10_LONG_RANGE;
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    scanParam.scanInterval = SOFTBUS_BC_SCAN_INTERVAL_P100_LONG_RANGE;
    scanParam.scanWindow = SOFTBUS_BC_SCAN_WINDOW_P100_LONG_RANGE;
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverScanFreqMap001 end ----");
}

/*
 * @tc.name: MgrCoverScanFuncNull001
 * @tc.desc: Cover StartScan/StopScan interface check failure branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverScanFuncNull001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverScanFuncNull001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));

    g_stubMediumInterface.StartScan = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StartScan(listenerId, &scanParam));
    ResetStubMediumInterfaceFuncs();

    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    g_stubMediumInterface.StopScan = nullptr;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, StopScan(listenerId));
    ResetStubMediumInterfaceFuncs();

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverScanFuncNull001 end ----");
}

/*
 * @tc.name: MgrCoverReportServiceFilter001
 * @tc.desc: Cover CheckScanResultDataIsMatch/CheckServiceIsMatch branches via scan report callback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportServiceFilter001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportServiceFilter001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t matchData[] = { 0x04, 0x05, 0x90 };
    uint8_t shortData[] = { 0x04, 0x05 };
    uint8_t mismatchData[] = { 0xFF, 0xFF, 0xFF };
    // service data matches
    InvokeScanReport(BC_DATA_TYPE_SERVICE, SERVICE_UUID, matchData, sizeof(matchData), 0, 0, nullptr, 0);
    // payload too short
    InvokeScanReport(BC_DATA_TYPE_SERVICE, SERVICE_UUID, shortData, sizeof(shortData), 0, 0, nullptr, 0);
    // serviceId mismatch
    InvokeScanReport(BC_DATA_TYPE_SERVICE, MANUFACTURE_COMPANY_ID, matchData, sizeof(matchData), 0, 0, nullptr, 0);
    // service data byte mismatch
    InvokeScanReport(BC_DATA_TYPE_SERVICE, SERVICE_UUID, mismatchData, sizeof(mismatchData), 0, 0, nullptr, 0);
    // unsupported data type
    InvokeScanReport(0x7F, SERVICE_UUID, matchData, sizeof(matchData), 0, 0, nullptr, 0);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportServiceFilter001 end ----");
}

/*
 * @tc.name: MgrCoverReportManufactureFilter001
 * @tc.desc: Cover CheckManufactureIsMatch branches via scan report callback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportManufactureFilter001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportManufactureFilter001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetManufactureScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t matchData[] = { 0x04, 0x05 };
    uint8_t shortData[] = { 0x04 };
    uint8_t mismatchData[] = { 0xFF, 0xFF };
    // manufacture data matches
    InvokeScanReport(BC_DATA_TYPE_MANUFACTURER, MANUFACTURE_COMPANY_ID, matchData, sizeof(matchData),
        0, 0, nullptr, 0);
    // payload too short
    InvokeScanReport(BC_DATA_TYPE_MANUFACTURER, MANUFACTURE_COMPANY_ID, shortData, sizeof(shortData),
        0, 0, nullptr, 0);
    // manufacture data byte mismatch
    InvokeScanReport(BC_DATA_TYPE_MANUFACTURER, MANUFACTURE_COMPANY_ID, mismatchData, sizeof(mismatchData),
        0, 0, nullptr, 0);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportManufactureFilter001 end ----");
}

/*
 * @tc.name: MgrCoverReportUuidFilter001
 * @tc.desc: Cover CheckServiceUuidIsMatch branch via scan report callback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportUuidFilter001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportUuidFilter001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetServiceUuidScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t matchData[] = { 0xAA, 0xBB };
    InvokeScanReport(BC_DATA_TYPE_SERVICE_UUID, SERVICE_UUID, matchData, sizeof(matchData), 0, 0, nullptr, 0);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportUuidFilter001 end ----");
}

/*
 * @tc.name: MgrCoverReportApproach001
 * @tc.desc: Cover CheckScanResultDataIsMatchApproach no-match branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportApproach001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportApproach001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_APPROACH, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t bcData[] = { 0x04, 0x05, 0x90 };
    uint8_t rspData[] = { 0x01, 0x02, 0x03 };
    // bcData type unsupported, rspData service type not matching the filter
    InvokeScanReport(0x7F, SERVICE_UUID, bcData, sizeof(bcData),
        BC_DATA_TYPE_SERVICE, MANUFACTURE_COMPANY_ID, rspData, sizeof(rspData));

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportApproach001 end ----");
}

/*
 * @tc.name: MgrCoverReportConstraint001
 * @tc.desc: Cover DiscIsOsAccountConstraint branch in BcReportScanDataCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportConstraint001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportConstraint001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, DiscIsOsAccountConstraint).WillRepeatedly(Return(true));

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t data[] = { 0x04, 0x05, 0x90 };
    InvokeScanReport(BC_DATA_TYPE_SERVICE, SERVICE_UUID, data, sizeof(data), 0, 0, nullptr, 0);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportConstraint001 end ----");
}

/*
 * @tc.name: MgrCoverReportLockFail001
 * @tc.desc: Cover scanLock failure branch in BcReportScanDataCallback after DeInitBroadcastMgr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportLockFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportLockFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    // DeInit destroys g_scanLock, invoking the report callback hits the lock failure branch
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    uint8_t data[] = { 0x04, 0x05, 0x90 };
    InvokeScanReport(BC_DATA_TYPE_SERVICE, SERVICE_UUID, data, sizeof(data), 0, 0, nullptr, 0);

    // re-init to clean up the still-registered listener
    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportLockFail001 end ----");
}

/*
 * @tc.name: MgrCoverBuildPacketCallocFail001
 * @tc.desc: Cover BuildBroadcastPacket rsp/uuid payload build failure branches via calloc injection
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverBuildPacketCallocFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverBuildPacketCallocFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t bcData[] = { 0x04, 0x05, 0x90 };
    uint8_t rspData[] = { 0x01, 0x02, 0x03 };
    uint8_t uuidData[] = { 0xAA, 0xBB };
    // fail the rspData payload calloc (bcData calloc + dump calloc pass)
    SetStubCallocFailCountdown(2);
    SoftBusBcScanResult report = {};
    report.data.bcData.type = BROADCAST_DATA_TYPE_SERVICE;
    report.data.bcData.id = SERVICE_UUID;
    report.data.bcData.payload = bcData;
    report.data.bcData.payloadLen = sizeof(bcData);
    report.data.rspData.type = BROADCAST_DATA_TYPE_MANUFACTURER;
    report.data.rspData.id = MANUFACTURE_COMPANY_ID;
    report.data.rspData.payload = rspData;
    report.data.rspData.payloadLen = sizeof(rspData);
    report.data.uuidData.type = (SoftbusBcDataType)BC_DATA_TYPE_SERVICE_UUID;
    report.data.uuidData.id = SERVICE_UUID;
    report.data.uuidData.payload = uuidData;
    report.data.uuidData.payloadLen = sizeof(uuidData);
    g_stubScanCallback->OnReportScanDataCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, &report);

    // fail the uuidData payload calloc
    SetStubCallocFailCountdown(4);
    g_stubScanCallback->OnReportScanDataCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, &report);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverBuildPacketCallocFail001 end ----");
}

/*
 * @tc.name: MgrCoverFilterIndexExhaust001
 * @tc.desc: Cover GetFilterIndex exhaustion and CopySoftBusBcScanFilter invalid filterIndex branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverFilterIndexExhaust001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverFilterIndexExhaust001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerIdA = -1;
    int32_t listenerIdB = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerIdA, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_OOP, &listenerIdB, GetScanCallback()));

    // listener A occupies all filter indexes
    BcScanFilter *filters = (BcScanFilter *)SoftBusCalloc(sizeof(BcScanFilter) * TEST_FILTER_INDEX_MAX);
    for (int32_t i = 0; i < TEST_FILTER_INDEX_MAX; i++) {
        filters[i] = *GetBcScanFilter();
    }
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerIdA, filters, TEST_FILTER_INDEX_MAX));

    // no index left for listener B, GetFilterIndex fails and filterIndex stays 0
    BcScanFilter *filterB = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerIdB, filterB, 1));

    // CopySoftBusBcScanFilter hits the invalid filterIndex log, SetAdvFilterParam succeeds
    LpScanParam lpScanParam = BuildLpScanParam();
    lpScanParam.listenerId = listenerIdB;
    LpBroadcastParam lpBcParam = {};
    lpBcParam.bcHandle = -1;
    BuildBroadcastParam(&lpBcParam.bcParam);
    EXPECT_TRUE(BroadcastSetAdvDeviceParam(SOFTBUS_VIRTUAL_SCAN_TYPE, &lpBcParam, &lpScanParam));

    // release occupied indexes
    BcScanFilter *filterA2 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerIdA, filterA2, 1));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerIdA));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerIdB));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverFilterIndexExhaust001 end ----");
}

/*
 * @tc.name: MgrCoverFilterAddFail001
 * @tc.desc: Cover CompareFilterAndGetIndex add-index failure and SetScanFilter failure branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverFilterAddFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverFilterAddFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerIdA = -1;
    int32_t listenerIdB = -1;
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerIdA, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_OOP, &listenerIdB, GetScanCallback()));

    BcScanFilter *filterA = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerIdA, filterA, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerIdA, &scanParam));

    // listener B occupies remaining indexes (2..33)
    BcScanFilter *filters = (BcScanFilter *)SoftBusCalloc(sizeof(BcScanFilter) * (TEST_FILTER_INDEX_MAX - 1));
    for (int32_t i = 0; i < TEST_FILTER_INDEX_MAX - 1; i++) {
        filters[i] = *GetBcScanFilter();
    }
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerIdB, filters, TEST_FILTER_INDEX_MAX - 1));

    // setting a different filter while scanning can not allocate an index
    BcScanFilter *filterNew = GetBcScanFilter();
    filterNew->serviceData[0] = filterNew->serviceData[0] ^ 0xFF;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, SetScanFilter(listenerIdA, filterNew, 1));
    SoftBusFree(filterNew->serviceData);
    SoftBusFree(filterNew->serviceDataMask);
    SoftBusFree(filterNew);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerIdA));
    BcScanFilter *filterB2 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerIdB, filterB2, 1));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerIdA));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerIdB));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverFilterAddFail001 end ----");
}

/*
 * @tc.name: MgrCoverCombineFilterCallocFail001
 * @tc.desc: Cover CombineSoftbusBcScanFilters convert failure branch via calloc injection
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverCombineFilterCallocFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverCombineFilterCallocFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilterFull();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));

    // adapterFilter calloc passes, filter address calloc fails
    SetStubCallocFailCountdown(1);
    EXPECT_EQ(SOFTBUS_BC_MGR_START_SCAN_NO_FILTER, StartScan(listenerId, &scanParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverCombineFilterCallocFail001 end ----");
}

/*
 * @tc.name: MgrCoverLpFilterCallocFail001
 * @tc.desc: Cover ConstructBcScanFilter malloc failure branch via calloc injection
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverLpFilterCallocFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverLpFilterCallocFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));

    LpScanParam lpScanParam = BuildLpScanParam();
    lpScanParam.listenerId = listenerId;
    LpBroadcastParam lpBcParam = {};
    lpBcParam.bcHandle = -1;
    BuildBroadcastParam(&lpBcParam.bcParam);
    SetStubCallocFailCountdown(0);
    EXPECT_FALSE(BroadcastSetAdvDeviceParam(SOFTBUS_VIRTUAL_SCAN_TYPE, &lpBcParam, &lpScanParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverLpFilterCallocFail001 end ----");
}

/*
 * @tc.name: MgrCoverLpSetAdvParamSuccess001
 * @tc.desc: Cover BroadcastSetScanParam/BroadcastSetAdvDeviceParam success branches
 *           and scan filter address/deviceName copy-release branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverLpSetAdvParamSuccess001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverLpSetAdvParamSuccess001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilterFull();
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));

    LpScanParam lpScanParam = BuildLpScanParam();
    lpScanParam.listenerId = listenerId;
    LpBroadcastParam lpBcParam = {};
    lpBcParam.bcHandle = -1;
    BuildBroadcastParam(&lpBcParam.bcParam);
    // bcHandle == -1, scan-only mode success
    EXPECT_TRUE(BroadcastSetAdvDeviceParam(SOFTBUS_VIRTUAL_SCAN_TYPE, &lpBcParam, &lpScanParam));

    BuildLpBroadcastParam(&lpBcParam);
    // bcHandle != -1, full mode success
    EXPECT_TRUE(BroadcastSetAdvDeviceParam(SOFTBUS_VIRTUAL_SCAN_TYPE, &lpBcParam, &lpScanParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverLpSetAdvParamSuccess001 end ----");
}

/*
 * @tc.name: MgrCoverScanReportChannelFail001
 * @tc.desc: Cover BroadcastSetScanReportChannelToLpDevice adapter failure branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverScanReportChannelFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverScanReportChannelFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));

    g_stubMediumConfig.setScanReportChannelRet = SOFTBUS_ERR;
    EXPECT_EQ(SOFTBUS_ERR, BroadcastSetScanReportChannelToLpDevice(listenerId, true));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverScanReportChannelFail001 end ----");
}

/*
 * @tc.name: MgrCoverGetHandleFuncNull001
 * @tc.desc: Cover BroadcastGetBroadcastHandle func-NULL branch and BroadcastIsLpDeviceAvailable true branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverGetHandleFuncNull001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverGetHandleFuncNull001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));

    g_stubMediumInterface.GetBroadcastHandle = nullptr;
    int32_t handle = 0;
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, BroadcastGetBroadcastHandle(bcId, &handle));
    ResetStubMediumInterfaceFuncs();
    EXPECT_TRUE(BroadcastIsLpDeviceAvailable());

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverGetHandleFuncNull001 end ----");
}

/*
 * @tc.name: MgrCoverTwoScanStopUpdate001
 * @tc.desc: Cover CheckNeedUpdateScan/CheckNeedUnRegisterScanListener true branches and
 *           CheckAndStopScan needUpdate success path, ProcessFliterChanged add-filter path
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverTwoScanStopUpdate001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverTwoScanStopUpdate001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId1 = -1;
    int32_t listenerId2 = -1;
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId1, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_OOP, &listenerId2, GetScanCallback()));

    BcScanFilter *filter1 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId1, filter1, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId1, &scanParam));

    BcScanFilter *filter2 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId2, filter2, 1));
    // channel is scanning and listener2 filter changed, covers ProcessFliterChanged add-filter path
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId2, &scanParam));

    // change filter while scanning to make deleteSize > 0
    BcScanFilter *filter1New = GetBcScanFilter();
    filter1New->serviceData[0] = filter1New->serviceData[0] ^ 0xFF;
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId1, filter1New, 1));

    // needUpdate branch of CheckAndStopScan with deleteSize > 0, success path
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId1, &scanParam));
    // listener2 still scanning, unregister listener1 keeps the adapter scan listener registered
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId1));

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId2));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId2));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverTwoScanStopUpdate001 end ----");
}

/*
 * @tc.name: MgrCoverTwoScanStopUpdateFail001
 * @tc.desc: Cover CheckAndStopScan needUpdate SetScanParams failure branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverTwoScanStopUpdateFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverTwoScanStopUpdateFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId1 = -1;
    int32_t listenerId2 = -1;
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId1, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_OOP, &listenerId2, GetScanCallback()));

    BcScanFilter *filter1 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId1, filter1, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId1, &scanParam));
    BcScanFilter *filter2 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId2, filter2, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId2, &scanParam));

    BcScanFilter *filter1New = GetBcScanFilter();
    filter1New->serviceData[0] = filter1New->serviceData[0] ^ 0xFF;
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId1, filter1New, 1));

    g_stubMediumConfig.setScanParamsRet = SOFTBUS_ERR;
    EXPECT_EQ(SOFTBUS_ERR, StopScan(listenerId1));
    g_stubMediumConfig.setScanParamsRet = SOFTBUS_OK;
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId1));
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId2));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId1));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId2));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverTwoScanStopUpdateFail001 end ----");
}

/*
 * @tc.name: MgrCoverScanStopAdapterFail001
 * @tc.desc: Cover CheckAndStopScan StopScan adapter failure branch and
 *           UnRegisterScanListener stop-scan failure branch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverScanStopAdapterFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverScanStopAdapterFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetBcScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));

    g_stubMediumConfig.stopScanRet = SOFTBUS_ERR;
    EXPECT_EQ(SOFTBUS_ERR, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_ERR, UnRegisterScanListener(listenerId));
    g_stubMediumConfig.stopScanRet = SOFTBUS_OK;
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));

    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverScanStopAdapterFail001 end ----");
}

/*
 * @tc.name: MgrCoverSleScanStart001
 * @tc.desc: Cover StartScanSub SLE branch when the channel is already scanning
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverSleScanStart001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverSleScanStart001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcastMediumFunction(BROADCAST_PROTOCOL_SLE, &g_stubSleMediumInterface));

    int32_t listenerId1 = -1;
    int32_t listenerId2 = -1;
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_SLE,
        SRV_TYPE_DIS, &listenerId1, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_SLE,
        SRV_TYPE_OOP, &listenerId2, GetScanCallback()));

    BcScanFilter *filter1 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId1, filter1, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId1, &scanParam));

    BcScanFilter *filter2 = GetBcScanFilter();
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId2, filter2, 1));
    // SLE branch: stop the channel scan first, then perform normal start scan
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId2, &scanParam));

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId2));
    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId1));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId2));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId1));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverSleScanStart001 end ----");
}

static void BleOnScanStateChanged(int32_t resultCode, bool isStartScan)
{
    (void)resultCode;
    (void)isStartScan;
}

static void BleOnLpDeviceInfo(const BroadcastUuid *uuid, int32_t type, uint8_t *data, uint32_t dataSize)
{
    (void)uuid;
    (void)type;
    (void)data;
    (void)dataSize;
}

static ScanCallback *GetScanCallbackFull()
{
    static ScanCallback g_scanListenerFull = {
        .OnStartScanCallback = BleOnScanStart,
        .OnStopScanCallback = BleOnScanStop,
        .OnReportScanDataCallback = BleScanResultCallback,
        .OnScanStateChanged = BleOnScanStateChanged,
        .OnLpDeviceInfoCallback = BleOnLpDeviceInfo,
    };
    return &g_scanListenerFull;
}

static BleAsyncCallbackFunc g_capturedDelayFunc = nullptr;

/*
 * @tc.name: MgrCoverDelayReport001
 * @tc.desc: Cover DelayReportBroadcast with/without recorded bc extra and re-register failure
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverDelayReport001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverDelayReport001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, BleAsyncCallbackDelayHelper)
        .WillRepeatedly(DoAll(SaveArg<1>(&g_capturedDelayFunc), Return(SOFTBUS_OK)));

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    ASSERT_TRUE(g_capturedDelayFunc != nullptr);
    // no bc extra recorded, resets g_bcMaxNum/g_bcOverMaxNum
    g_capturedDelayFunc(nullptr);

    // record one advertising broadcaster into g_bcManagerExtra
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    // isOn == 1 branch, reports the recorded extra
    g_capturedDelayFunc(nullptr);

    // re-register delay helper failure branch
    EXPECT_CALL(managerMock, BleAsyncCallbackDelayHelper).WillRepeatedly(Return(SOFTBUS_ERR));
    g_capturedDelayFunc(nullptr);

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverDelayReport001 end ----");
}

/*
 * @tc.name: MgrCoverReportCurrentFail001
 * @tc.desc: Cover ReportCurrentBroadcast with startBcResult false while a broadcaster is advertising
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverReportCurrentFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverReportCurrentFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);

    int32_t bcId1 = -1;
    int32_t bcId2 = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId1, GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId2, GetBroadcastCallback()));
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId1, &bcParam, &packet));

    g_stubMediumConfig.startBcRet = SOFTBUS_ERR;
    g_stubMediumConfig.invokeStartBcCb = false;
    EXPECT_EQ(SOFTBUS_ERR, StartBroadcasting(bcId2, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId1));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId1));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId2));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverReportCurrentFail001 end ----");
}

/*
 * @tc.name: MgrCoverUpdateBcCb001
 * @tc.desc: Cover BcUpdateBroadcastingCallback match and not-available branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverUpdateBcCb001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverUpdateBcCb001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    ASSERT_TRUE(g_stubBcCallback != nullptr);

    g_stubBcCallback->OnUpdateBroadcastingCallback(BROADCAST_PROTOCOL_BLE,
        STUB_BC_ID_BASE, SOFTBUS_BC_STATUS_SUCCESS);
    g_stubBcCallback->OnUpdateBroadcastingCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverUpdateBcCb001 end ----");
}

/*
 * @tc.name: MgrCoverScanStartStopCb001
 * @tc.desc: Cover BcOnStartScanCallback/BcStopScanCallback success, fail and not-available branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverScanStartStopCb001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverScanStartStopCb001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallbackFull()));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    // success status sets isScanning, fail status only reports
    g_stubScanCallback->OnStartScanCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, SOFTBUS_BC_STATUS_SUCCESS);
    g_stubScanCallback->OnStartScanCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, SOFTBUS_BC_STATUS_FAIL);
    // not-available branch
    g_stubScanCallback->OnStartScanCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);

    // fail status logs the else branch, success status clears isScanning
    g_stubScanCallback->OnStopScanCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, SOFTBUS_BC_STATUS_FAIL);
    g_stubScanCallback->OnStopScanCallback(BROADCAST_PROTOCOL_BLE, STUB_SCAN_ID, SOFTBUS_BC_STATUS_SUCCESS);
    g_stubScanCallback->OnStopScanCallback(BROADCAST_PROTOCOL_BUTT, -1, SOFTBUS_BC_STATUS_SUCCESS);

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverScanStartStopCb001 end ----");
}

/*
 * @tc.name: MgrCoverScanStateChanged001
 * @tc.desc: Cover BcScanStateChanged loop and continue branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverScanStateChanged001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverScanStateChanged001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallbackFull()));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    g_stubScanCallback->OnScanStateChanged(BROADCAST_PROTOCOL_BLE, 0, true);
    g_stubScanCallback->OnScanStateChanged(BROADCAST_PROTOCOL_SLE, 1, false);

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverScanStateChanged001 end ----");
}

/*
 * @tc.name: MgrCoverLpDeviceInfoCb001
 * @tc.desc: Cover BcLpDeviceInfoCallback and ConvertBroadcastUuid success,
 *           not-available and calloc failure branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverLpDeviceInfoCb001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverLpDeviceInfoCb001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallbackFull()));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t uuidData[] = { 0x01, 0x02, 0x03 };
    SoftbusBroadcastUuid uuid = {
        .uuidLen = (uint8_t)sizeof(uuidData),
        .uuid = uuidData,
    };
    uint8_t data[] = { 0xAA, 0xBB };
    // match branch, user callback invoked
    g_stubScanCallback->OnLpDeviceInfoCallback(BROADCAST_PROTOCOL_BLE, &uuid, 1, data, sizeof(data));
    // not-available branch
    g_stubScanCallback->OnLpDeviceInfoCallback(BROADCAST_PROTOCOL_SLE, &uuid, 1, data, sizeof(data));
    // ConvertBroadcastUuid calloc failure branch
    SetStubCallocFailCountdown(0);
    g_stubScanCallback->OnLpDeviceInfoCallback(BROADCAST_PROTOCOL_BLE, &uuid, 1, data, sizeof(data));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverLpDeviceInfoCb001 end ----");
}

/*
 * @tc.name: MgrCoverUuidMatchBranches001
 * @tc.desc: Cover CheckServiceUuidIsMatch short-payload, id-mismatch and byte-mismatch branches
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverUuidMatchBranches001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverUuidMatchBranches001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t listenerId = -1;
    BcScanFilter *filter = GetServiceUuidScanFilter();
    BcScanParams scanParam = {};
    BuildScanParam(&scanParam);

    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));
    EXPECT_EQ(SOFTBUS_OK, SetScanFilter(listenerId, filter, 1));
    EXPECT_EQ(SOFTBUS_OK, StartScan(listenerId, &scanParam));
    ASSERT_TRUE(g_stubScanCallback != nullptr);

    uint8_t matchData[] = { 0xAA, 0xBB };
    uint8_t shortData[] = { 0xAA };
    uint8_t mismatchData[] = { 0xFF, 0xFF };
    InvokeScanReport(BC_DATA_TYPE_SERVICE_UUID, SERVICE_UUID, matchData, sizeof(matchData), 0, 0, nullptr, 0);
    InvokeScanReport(BC_DATA_TYPE_SERVICE_UUID, SERVICE_UUID, shortData, sizeof(shortData), 0, 0, nullptr, 0);
    InvokeScanReport(BC_DATA_TYPE_SERVICE_UUID, MANUFACTURE_COMPANY_ID, matchData, sizeof(matchData),
        0, 0, nullptr, 0);
    InvokeScanReport(BC_DATA_TYPE_SERVICE_UUID, SERVICE_UUID, mismatchData, sizeof(mismatchData), 0, 0, nullptr, 0);

    EXPECT_EQ(SOFTBUS_OK, StopScan(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverUuidMatchBranches001 end ----");
}

/*
 * @tc.name: MgrCoverStartWaitTimeout001
 * @tc.desc: Cover SoftBusCondWaitSec timeout and StartBroadcastingWaitSignal full recovery path
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStartWaitTimeout001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStartWaitTimeout001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, SoftBusCondWait).WillRepeatedly(Return(SOFTBUS_ERR));

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumConfig.invokeStopBcCb = false;
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    // both cond waits fail, covers the wait-timeout and wait-stop-failed branches
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStartWaitTimeout001 end ----");
}

/*
 * @tc.name: MgrCoverStartWaitRecovery001
 * @tc.desc: Cover StartBroadcastingWaitSignal recovery path where the second wait succeeds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverStartWaitRecovery001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverStartWaitRecovery001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, SoftBusCondWait)
        .WillOnce(Return(SOFTBUS_ERR))
        .WillOnce(Return(SOFTBUS_OK))
        .RetiresOnSaturation();

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumConfig.invokeStopBcCb = false;
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    // first cond wait fails, second cond wait succeeds after adapter stop
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverStartWaitRecovery001 end ----");
}

/*
 * @tc.name: MgrCoverGetTimeFail001
 * @tc.desc: Cover SoftBusGetTime failure branch in SoftBusCondWaitSec
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverGetTimeFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverGetTimeFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumConfig.invokeStopBcCb = false;
    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    SetStubGetTimeFail(true);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));
    SetStubGetTimeFail(false);

    EXPECT_EQ(SOFTBUS_OK, StopBroadcasting(bcId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverGetTimeFail001 end ----");
}

/*
 * @tc.name: MgrCoverDisableWaitFail001
 * @tc.desc: Cover DisableBroadcastingWaitSignal wait-failure recovery path
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverDisableWaitFail001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverDisableWaitFail001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);
    EXPECT_CALL(managerMock, SoftBusCondWait).WillRepeatedly(Return(SOFTBUS_ERR));

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    BroadcastParam bcParam = {};
    BroadcastPacket packet = {};
    BuildValidPacket(&bcParam, &packet);
    EXPECT_EQ(SOFTBUS_OK, StartBroadcasting(bcId, &bcParam, &packet));

    g_stubMediumConfig.invokeDisableBcCb = false;
    EXPECT_EQ(SOFTBUS_BC_MGR_WAIT_COND_FAIL, SetBroadcastingParam(bcId, &bcParam));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverDisableWaitFail001 end ----");
}

/*
 * @tc.name: MgrCoverRegisterInfoDump001
 * @tc.desc: Cover RegisterInfoDump with used and unused manager slots
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SoftbusBroadcastMgrTest, MgrCoverRegisterInfoDump001, TestSize.Level1)
{
    DISC_LOGI(DISC_TEST, "MgrCoverRegisterInfoDump001 begin ----");
    ManagerMock managerMock;
    EXPECT_CALL(managerMock, SoftbusBleAdapterInit).WillRepeatedly(ActionOfRegisterStubMediumInterface);

    EXPECT_EQ(SOFTBUS_OK, InitBroadcastMgr());
    int32_t bcId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterBroadcaster(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &bcId, GetBroadcastCallback()));
    int32_t listenerId = -1;
    EXPECT_EQ(SOFTBUS_OK, RegisterScanListener(BROADCAST_PROTOCOL_BLE,
        SRV_TYPE_DIS, &listenerId, GetScanCallback()));

    EXPECT_EQ(SOFTBUS_OK, InvokeStubBcMgrDump(STDERR_FILENO));

    EXPECT_EQ(SOFTBUS_OK, UnRegisterScanListener(listenerId));
    EXPECT_EQ(SOFTBUS_OK, UnRegisterBroadcaster(bcId));
    EXPECT_EQ(SOFTBUS_OK, DeInitBroadcastMgr());
    DISC_LOGI(DISC_TEST, "MgrCoverRegisterInfoDump001 end ----");
}
} // namespace OHOS
