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

#include "gtest/gtest.h"

#include "softbus_adapter_mem.h"
#include "softbus_ble_utils.h"
#include "softbus_broadcast_type.h"
#include "softbus_broadcast_utils.h"
#include "softbus_error_code.h"
#include <cstring>

using namespace testing::ext;

namespace OHOS {

/**
 * @tc.name: SoftbusBleUtilsTest_BtStatusToSoftBus001
 * @tc.desc: test bt status convert to softbus status
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, BtStatusToSoftBus001, TestSize.Level3)
{
    int32_t status = BtStatusToSoftBus(OHOS_BT_STATUS_SUCCESS);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_SUCCESS);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_FAIL);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_FAIL);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_NOT_READY);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_NOT_READY);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_NOMEM);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_NOMEM);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_BUSY);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_BUSY);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_DONE);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_DONE);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_UNSUPPORTED);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_UNSUPPORTED);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_PARM_INVALID);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_PARM_INVALID);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_UNHANDLED);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_UNHANDLED);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_AUTH_FAILURE);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_AUTH_FAILURE);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_RMT_DEV_DOWN);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_RMT_DEV_DOWN);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_AUTH_REJECTED);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_AUTH_REJECTED);

    status = BtStatusToSoftBus(OHOS_BT_STATUS_DUPLICATED_ADDR);
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_DUPLICATED_ADDR);

    int32_t invalidStatus = 100;
    status = BtStatusToSoftBus(static_cast<BtStatus>(invalidStatus));
    EXPECT_EQ(status, SOFTBUS_BC_STATUS_FAIL);
}

/**
 * @tc.name: SoftbusBleUtilsTest_SoftbusAdvParamToBt001
 * @tc.desc: test softbus adv param convert to bt adv params
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, SoftbusAdvParamToBt001, TestSize.Level3)
{
    SoftbusBroadcastParam softbusAdvParam = {};
    softbusAdvParam.minInterval = 1;
    softbusAdvParam.maxInterval = 1;
    softbusAdvParam.advType = 1;
    softbusAdvParam.advFilterPolicy = 1;
    softbusAdvParam.ownAddrType = 1;
    softbusAdvParam.peerAddrType = 1;
    softbusAdvParam.channelMap = 1;
    softbusAdvParam.duration = 1;
    softbusAdvParam.txPower = 1;

    BleAdvParams bleAdvParams = {};
    SoftbusAdvParamToBt(&softbusAdvParam, &bleAdvParams);

    EXPECT_EQ(bleAdvParams.minInterval, softbusAdvParam.minInterval);
    EXPECT_EQ(bleAdvParams.maxInterval, softbusAdvParam.maxInterval);
    EXPECT_EQ(bleAdvParams.advType, softbusAdvParam.advType);
    EXPECT_EQ(bleAdvParams.advFilterPolicy, softbusAdvParam.advFilterPolicy);
    EXPECT_EQ(bleAdvParams.ownAddrType, softbusAdvParam.ownAddrType);
    EXPECT_EQ(bleAdvParams.peerAddrType, softbusAdvParam.peerAddrType);
    EXPECT_EQ(bleAdvParams.channelMap, softbusAdvParam.channelMap);
    EXPECT_EQ(bleAdvParams.duration, softbusAdvParam.duration);
    EXPECT_EQ(bleAdvParams.txPower, softbusAdvParam.txPower);

    softbusAdvParam.advFilterPolicy = SOFTBUS_BC_ADV_FILTER_ALLOW_SCAN_ANY_CON_WLST;
    softbusAdvParam.advType = SOFTBUS_BC_ADV_SCAN_IND;
    SoftbusAdvParamToBt(&softbusAdvParam, &bleAdvParams);
    EXPECT_EQ(static_cast<int>(bleAdvParams.advFilterPolicy), static_cast<int>(softbusAdvParam.advFilterPolicy));
    EXPECT_EQ(static_cast<int>(bleAdvParams.advType), static_cast<int>(softbusAdvParam.advType));

    softbusAdvParam.advFilterPolicy = SOFTBUS_BC_ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST;
    softbusAdvParam.advType = SOFTBUS_BC_ADV_DIRECT_IND_LOW;
    SoftbusAdvParamToBt(&softbusAdvParam, &bleAdvParams);
    EXPECT_EQ(static_cast<int>(bleAdvParams.advFilterPolicy), static_cast<int>(softbusAdvParam.advFilterPolicy));
    EXPECT_EQ(static_cast<int>(bleAdvParams.advType), static_cast<int>(softbusAdvParam.advType));

    softbusAdvParam.advFilterPolicy = (SOFTBUS_BC_ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST
                                        + SOFTBUS_BC_ADV_FILTER_ALLOW_SCAN_WLST_CON_ANY);
    softbusAdvParam.advType = (SOFTBUS_BC_ADV_DIRECT_IND_LOW + SOFTBUS_BC_ADV_DIRECT_IND_HIGH);
    SoftbusAdvParamToBt(&softbusAdvParam, &bleAdvParams);
    EXPECT_EQ(static_cast<int>(bleAdvParams.advFilterPolicy),
             static_cast<int>(SOFTBUS_BC_ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY));
    EXPECT_EQ(static_cast<int>(bleAdvParams.advType), static_cast<int>(SOFTBUS_BC_ADV_IND));
}

/**
 * @tc.name: SoftbusBleUtilsTest_BtScanResultToSoftbus001
 * @tc.desc: test bt scan result convert to softbus scan result
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, BtScanResultToSoftbus001, TestSize.Level3)
{
    BtScanResultData btScanResult = {};
    btScanResult.eventType = 1;
    btScanResult.dataStatus = 1;
    btScanResult.addrType = 1;
    btScanResult.primaryPhy = 1;
    btScanResult.secondaryPhy = 1;
    btScanResult.advSid = 1;
    btScanResult.txPower = 1;
    btScanResult.rssi = 1;

    SoftBusBcScanResult softbusScanResult = {};
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);

    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.dataStatus, btScanResult.dataStatus);
    EXPECT_EQ(softbusScanResult.addrType, btScanResult.addrType);
    EXPECT_EQ(softbusScanResult.primaryPhy, btScanResult.primaryPhy);
    EXPECT_EQ(softbusScanResult.secondaryPhy, btScanResult.secondaryPhy);
    EXPECT_EQ(softbusScanResult.advSid, btScanResult.advSid);
    EXPECT_EQ(softbusScanResult.txPower, btScanResult.txPower);
    EXPECT_EQ(softbusScanResult.rssi, btScanResult.rssi);
}


/**
 * @tc.name: SoftbusBleUtilsTest_BtScanResultToSoftbus002
 * @tc.desc: test bt scan result convert to softbus scan result for BtScanDataStatusToSoftbus
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, BtScanResultToSoftbus002, TestSize.Level3)
{
    BtScanResultData btScanResult = {};
    btScanResult.secondaryPhy = 1;
    btScanResult.advSid = 1;
    btScanResult.txPower = 1;
    btScanResult.rssi = 1;

    SoftBusBcScanResult softbusScanResult = {};

    btScanResult.eventType = OHOS_BLE_EVT_NON_CONNECTABLE_NON_SCANNABLE;
    btScanResult.dataStatus = OHOS_BLE_DATA_INCOMPLETE_TRUNCATED;
    btScanResult.addrType = OHOS_BLE_PUBLIC_DEVICE_ADDRESS;
    btScanResult.primaryPhy = OHOS_BLE_SCAN_PHY_NO_PACKET;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.dataStatus, btScanResult.dataStatus);
    EXPECT_EQ(softbusScanResult.addrType, btScanResult.addrType);
    EXPECT_EQ(softbusScanResult.primaryPhy, btScanResult.primaryPhy);

    btScanResult.eventType = OHOS_BLE_EVT_SCANNABLE;
    btScanResult.dataStatus = (OHOS_BLE_DATA_INCOMPLETE_TRUNCATED +
                               OHOS_BLE_DATA_INCOMPLETE_MORE_TO_COME);
    btScanResult.addrType = OHOS_BLE_PUBLIC_IDENTITY_ADDRESS;
    btScanResult.primaryPhy = OHOS_BLE_SCAN_PHY_2M;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.dataStatus, SOFTBUS_BC_DATA_INCOMPLETE_TRUNCATED);
    EXPECT_EQ(softbusScanResult.addrType, btScanResult.addrType);
    EXPECT_EQ(softbusScanResult.primaryPhy, btScanResult.primaryPhy);

    btScanResult.eventType = OHOS_BLE_EVT_NON_CONNECTABLE_NON_SCANNABLE_DIRECTED;
    btScanResult.dataStatus = OHOS_BLE_DATA_INCOMPLETE_MORE_TO_COME;
    btScanResult.addrType = OHOS_BLE_RANDOM_STATIC_IDENTITY_ADDRESS;
    btScanResult.primaryPhy = OHOS_BLE_SCAN_PHY_CODED;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.addrType, btScanResult.addrType);
    EXPECT_EQ(softbusScanResult.primaryPhy, btScanResult.primaryPhy);

    btScanResult.eventType = OHOS_BLE_EVT_CONNECTABLE_DIRECTED;
    btScanResult.addrType = OHOS_BLE_UNRESOLVABLE_RANDOM_DEVICE_ADDRESS;
    btScanResult.primaryPhy = (OHOS_BLE_SCAN_PHY_CODED + OHOS_BLE_SCAN_PHY_1M);
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.addrType, btScanResult.addrType);
    EXPECT_EQ(softbusScanResult.primaryPhy, SOFTBUS_BC_SCAN_PHY_NO_PACKET);
}

/**
 * @tc.name: SoftbusBleUtilsTest_BtScanResultToSoftbus003
 * @tc.desc: test bt scan result convert to softbus scan result for BtScanDataStatusToSoftbus
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, BtScanResultToSoftbus003, TestSize.Level3)
{
    BtScanResultData btScanResult = {};
    btScanResult.secondaryPhy = 1;
    btScanResult.advSid = 1;
    btScanResult.txPower = 1;
    btScanResult.rssi = 1;

    SoftBusBcScanResult softbusScanResult = {};

    btScanResult.eventType = OHOS_BLE_EVT_SCANNABLE_DIRECTED;
    btScanResult.addrType = OHOS_BLE_NO_ADDRESS;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.addrType, btScanResult.addrType);

    btScanResult.eventType = OHOS_BLE_EVT_LEGACY_NON_CONNECTABLE;
    btScanResult.addrType = (OHOS_BLE_UNRESOLVABLE_RANDOM_DEVICE_ADDRESS -
                             OHOS_BLE_RANDOM_STATIC_IDENTITY_ADDRESS);
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);
    EXPECT_EQ(softbusScanResult.addrType, OHOS_BLE_NO_ADDRESS);

    btScanResult.eventType = OHOS_BLE_EVT_LEGACY_SCANNABLE;
    btScanResult.addrType = OHOS_BLE_RANDOM_DEVICE_ADDRESS;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);

    btScanResult.eventType = OHOS_BLE_EVT_LEGACY_CONNECTABLE_DIRECTED;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);

    btScanResult.eventType = OHOS_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV_SCAN;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);

    btScanResult.eventType = OHOS_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV;
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, btScanResult.eventType);

    btScanResult.eventType = (OHOS_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV + OHOS_BLE_EVT_CONNECTABLE);
    BtScanResultToSoftbus(&btScanResult, &softbusScanResult);
    EXPECT_EQ(softbusScanResult.eventType, SOFTBUS_BC_EVT_NON_CONNECTABLE_NON_SCANNABLE);
}

/**
 * @tc.name: SoftbusBleUtilsTest_BtScanResultToSoftbus004
 * @tc.desc: test bt scan result with invalid params
 * @tc.type: FUNC
 * @tc.require: 1
 */
 HWTEST(SoftbusBleUtilsTest, BtScanResultToSoftbus004, TestSize.Level3)
 {
    BtScanResultData *src = nullptr;
    SoftBusBcScanResult *dst = nullptr;
    BtScanResultToSoftbus(src, dst);
    EXPECT_EQ(dst, nullptr);

    BtScanResultData btScanResult = {};
    BtScanResultToSoftbus(&btScanResult, dst);
    EXPECT_EQ(dst, nullptr);
}

/**
 * @tc.name: SoftbusBleUtilsTest_SoftbusFilterToBt001
 * @tc.desc: test softbus scan filter convert to bt scan filter
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, SoftbusFilterToBt001, TestSize.Level3)
{
    SoftBusBcScanFilter softBusBcScanFilter = {};
    softBusBcScanFilter.address = (int8_t *)"address";
    softBusBcScanFilter.deviceName = (int8_t *)"deviceName";
    softBusBcScanFilter.serviceUuid = 1;
    softBusBcScanFilter.serviceDataLength = 1;
    softBusBcScanFilter.manufactureId = 1;
    softBusBcScanFilter.manufactureDataLength = 1;

    BleScanNativeFilter bleScanNativeFilter = {};
    SoftbusFilterToBt(&bleScanNativeFilter, &softBusBcScanFilter, 1);
    SoftBusFree(bleScanNativeFilter.serviceData);
    SoftBusFree(bleScanNativeFilter.serviceDataMask);

    EXPECT_EQ(bleScanNativeFilter.address, (char *)softBusBcScanFilter.address);
    EXPECT_EQ(bleScanNativeFilter.deviceName, (char *)softBusBcScanFilter.deviceName);
    EXPECT_EQ(bleScanNativeFilter.manufactureId, softBusBcScanFilter.manufactureId);
    EXPECT_EQ(bleScanNativeFilter.manufactureDataLength, softBusBcScanFilter.manufactureDataLength);
}

/**
 * @tc.name: SoftbusBleUtilsTest_FreeBtFilter001
 * @tc.desc: test free bt scan filter
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, FreeBtFilter001, TestSize.Level3)
{
    BleScanNativeFilter *bleScanNativeFilter = (BleScanNativeFilter *)calloc(1, sizeof(BleScanNativeFilter));
    FreeBtFilter(bleScanNativeFilter, 1);
}

/**
 * @tc.name: SoftbusBleUtilsTest_DumpBleScanFilter001
 * @tc.desc: test dump scan filter
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, DumpBleScanFilter001, TestSize.Level3)
{
    BleScanNativeFilter *bleScanNativeFilter = (BleScanNativeFilter *)calloc(1, sizeof(BleScanNativeFilter));
    DumpBleScanFilter(bleScanNativeFilter, 1);
    free(bleScanNativeFilter);
}

/**
 * @tc.name: SoftbusBleUtilsTest_GetBtScanMode001
 * @tc.desc: test get bt scan mode
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, GetBtScanMode001, TestSize.Level3)
{
    int32_t scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P2, SOFTBUS_BC_SCAN_WINDOW_P2);
    EXPECT_EQ(scanMode, OHOS_BLE_SCAN_MODE_OP_P2_60_3000);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P10, SOFTBUS_BC_SCAN_WINDOW_P10);
    EXPECT_EQ(scanMode, OHOS_BLE_SCAN_MODE_OP_P10_30_300);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P25, SOFTBUS_BC_SCAN_WINDOW_P25);
    EXPECT_EQ(scanMode, OHOS_BLE_SCAN_MODE_OP_P25_60_240);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P100, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_EQ(scanMode, OHOS_BLE_SCAN_MODE_OP_P100_1000_1000);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P2, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_EQ(scanMode, OHOS_BLE_SCAN_MODE_LOW_POWER);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P2_FAST, SOFTBUS_BC_SCAN_WINDOW_P2_FAST);
    EXPECT_EQ(scanMode, OHOS_BLE_SCAN_MODE_OP_P2_30_1500);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P2_FAST, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_NE(scanMode, OHOS_BLE_SCAN_MODE_OP_P2_30_1500);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P10, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_NE(scanMode, OHOS_BLE_SCAN_MODE_OP_P10_30_300);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P25, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_NE(scanMode, OHOS_BLE_SCAN_MODE_OP_P25_60_240);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P50, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_NE(scanMode, OHOS_BLE_SCAN_MODE_OP_P50_30_60);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P75, SOFTBUS_BC_SCAN_WINDOW_P100);
    EXPECT_NE(scanMode, OHOS_BLE_SCAN_MODE_OP_P75_30_40);

    scanMode = GetBtScanMode(SOFTBUS_BC_SCAN_INTERVAL_P100, SOFTBUS_BC_SCAN_WINDOW_P75);
    EXPECT_NE(scanMode, OHOS_BLE_SCAN_MODE_OP_P75_30_40);
}

/**
 * @tc.name: SoftbusBleUtilsTest_AssembleAdvData001
 * @tc.desc: test assemble ble adv data
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, AssembleAdvData001, TestSize.Level3)
{
    SoftbusBroadcastData *data = (SoftbusBroadcastData *)calloc(1, sizeof(SoftbusBroadcastData));
    data->isSupportFlag = true;
    data->flag = 1;
    SoftbusBroadcastPayload bcData;
    bcData.type = BROADCAST_DATA_TYPE_SERVICE;
    bcData.id = 1;
    uint8_t *payload = (uint8_t *)"00112233445566";
    bcData.payloadLen = 15;
    bcData.payload = payload;
    data->bcData = bcData;
    uint16_t dataLen = 0;
    uint8_t *advData = AssembleAdvData(data, &dataLen);
    uint16_t expectedDataLen =
        (data->isSupportFlag) ? bcData.payloadLen + BC_HEAD_LEN : bcData.payloadLen + BC_HEAD_LEN - BC_FLAG_LEN;
    EXPECT_EQ(dataLen, expectedDataLen);
    EXPECT_NE(advData, nullptr);

    SoftBusFree(advData);
    free(data);
}

/**
 * @tc.name: SoftbusBleUtilsTest_AssembleRspData001
 * @tc.desc: test assemble ble rsp data
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, AssembleRspData001, TestSize.Level3)
{
    SoftbusBroadcastPayload rspData = {};
    rspData.type = BROADCAST_DATA_TYPE_BUTT;
    rspData.id = 1;
    uint8_t *payload = (uint8_t *)"00112233445566";
    rspData.payloadLen = 15;
    rspData.payload = payload;
    uint16_t dataLen = 0;

    uint8_t *data = AssembleRspData(&rspData, &dataLen);
    EXPECT_NE(data, nullptr);
    uint16_t expectedDataLen = rspData.payloadLen + RSP_HEAD_LEN;
    EXPECT_EQ(dataLen, expectedDataLen);
    SoftBusFree(data);
}

/**
 * @tc.name: SoftbusBleUtilsTest_ParseScanResult001
 * @tc.desc: test parse ble scan result as softbus scan result
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, ParseScanResult001, TestSize.Level3)
{
    uint8_t *advData = (uint8_t *)"00112233445566";
    uint8_t advLen = 23;
    SoftBusBcScanResult softBusBcScanResult = {};
    int32_t ret = ParseScanResult(advData, advLen, &softBusBcScanResult);
    EXPECT_EQ(ret, SOFTBUS_OK);
    SoftBusFree(softBusBcScanResult.data.bcData.payload);
    SoftBusFree(softBusBcScanResult.data.rspData.payload);

    EXPECT_EQ(softBusBcScanResult.data.isSupportFlag, false);
    EXPECT_EQ(softBusBcScanResult.data.bcData.type, BROADCAST_DATA_TYPE_SERVICE);
}

/**
 * @tc.name: SoftbusBleUtilsTest_SoftbusSetManufactureFilterTest001
 * @tc.desc: test SoftbusSetManufactureFilter when success
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, SoftbusSetManufactureFilterTest001, TestSize.Level3)
{
    const uint8_t filterSize = 2;
    BleScanNativeFilter nativeFilter[filterSize];
    SoftbusSetManufactureFilter(nativeFilter, filterSize);

    for (uint8_t i = 0; i < filterSize; i++) {
        EXPECT_NE(nativeFilter[i].manufactureData, nullptr);
        EXPECT_EQ(nativeFilter[i].manufactureDataLength, 1);
        EXPECT_NE(nativeFilter[i].manufactureDataMask, nullptr);
        EXPECT_EQ(nativeFilter[i].manufactureId, 0x027D);
        SoftBusFree(nativeFilter[i].manufactureData);
        SoftBusFree(nativeFilter[i].manufactureDataMask);
    }
}

/**
 * @tc.name: SoftbusBleUtilsTest_SoftbusSetManufactureFilterTest002
 * @tc.desc: test SoftbusSetManufactureFilter when nativeFilter is nullptr
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, SoftbusSetManufactureFilterTest002, TestSize.Level3)
{
    const uint8_t filterSize = 2;
    SoftbusSetManufactureFilter(nullptr, filterSize);
    EXPECT_EQ(filterSize, 2);
}

/**
 * @tc.name: SoftbusBleUtilsTest_SoftbusSetManufactureFilterTest003
 * @tc.desc: test SoftbusSetManufactureFilter when filterSize = 0
 * @tc.type: FUNC
 * @tc.require: 1
 */
HWTEST(SoftbusBleUtilsTest, SoftbusSetManufactureFilterTest003, TestSize.Level3)
{
    const uint8_t filterSize = 0;
    BleScanNativeFilter nativeFilter[1];
    SoftbusSetManufactureFilter(nativeFilter, filterSize);
    EXPECT_EQ(filterSize, 0);
}
} // namespace OHOS
