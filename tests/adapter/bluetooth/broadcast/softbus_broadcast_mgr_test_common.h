/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef SOFTBUS_BROADCAST_MGR_TEST_COMMON_H
#define SOFTBUS_BROADCAST_MGR_TEST_COMMON_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "securec.h"

#include "disc_log.h"
#include "message_handler.h"
#include "softbus_adapter_mem.h"
#include "softbus_broadcast_manager.h"
#include "softbus_broadcast_utils.h"
#include "softbus_error_code.h"

#define BC_ADV_FLAG             0x2
#define BC_ADV_TX_POWER_DEFAULT (-6)
#define BC_CHANNLE_MAP          0x0
#define BC_INTERNAL             48
#define MANUFACTURE_COMPANY_ID  0x027D
#define SERVICE_UUID            0xFDEE
#define SRV_TYPE_INVALID        (-1)
#define INVALID_ADV_POWER       127

namespace OHOS {
inline const uint8_t *BASE_FUZZ_DATA = nullptr;
inline size_t g_baseFuzzPos;
inline size_t g_baseFuzzSize = 0;

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

    void SetUp() override { }
    void TearDown() override { }

    static inline uint8_t BC_DATA_PAYLOAD[] = { 0x04, 0x05, 0x90, 0x00, 0x00, 0x10, 0x00, 0x18, 0xB9, 0x13, 0x3D, 0x28,
        0xFC, 0x0D, 0x7F, 0xAB, 0x21, 0x00, 0x30, 0x4F, 0x70, 0x65, 0x6E, 0x48 };
    static inline uint8_t RSP_DATA_PAYLOAD[] = { 0x61, 0x72, 0x6D, 0x6F, 0x6E, 0x79, 0x20, 0x33, 0x2E, 0x32, 0x00 };
};

inline void BuildBroadcastParam(BroadcastParam *bcParam)
{
    bcParam->minInterval = BC_INTERNAL;
    bcParam->maxInterval = BC_INTERNAL;
    bcParam->advType = SOFTBUS_BC_ADV_IND;
    bcParam->ownAddrType = SOFTBUS_BC_PUBLIC_DEVICE_ADDRESS;
    bcParam->peerAddrType = SOFTBUS_BC_PUBLIC_DEVICE_ADDRESS;
    bcParam->channelMap = BC_CHANNLE_MAP;
    bcParam->txPower = BC_ADV_TX_POWER_DEFAULT;
}

inline void BuildBroadcastPacketExceptPayload(BroadcastPacket *packet)
{
    packet->bcData.type = BC_DATA_TYPE_SERVICE;
    packet->bcData.id = SERVICE_UUID;
    packet->rspData.type = BC_DATA_TYPE_MANUFACTURER;
    packet->rspData.id = MANUFACTURE_COMPANY_ID;
    packet->isSupportFlag = true;
    packet->flag = BC_ADV_FLAG;
}

inline void BuildScanParam(BcScanParams *scanParam)
{
    scanParam->scanInterval = SOFTBUS_BC_SCAN_INTERVAL_P2;
    scanParam->scanWindow = SOFTBUS_BC_SCAN_WINDOW_P2;
    scanParam->scanType = SOFTBUS_BC_SCAN_TYPE_ACTIVE;
    scanParam->scanPhy = SOFTBUS_BC_SCAN_PHY_1M;
    scanParam->scanFilterPolicy = SOFTBUS_BC_SCAN_FILTER_POLICY_ACCEPT_ALL;
}

template <class T> T GetData()
{
    T object{};
    size_t objectSize = sizeof(object);
    if (BASE_FUZZ_DATA == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, BASE_FUZZ_DATA + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

inline LpScanParam BuildLpScanParam()
{
    LpScanParam lpScanParam;
    g_baseFuzzPos = 0;
    BcScanParams scanParam;
    scanParam.scanInterval = SOFTBUS_BC_SCAN_INTERVAL_P2;
    scanParam.scanWindow = SOFTBUS_BC_SCAN_WINDOW_P2;
    scanParam.scanType = GetData<bool>();
    scanParam.scanPhy = GetData<uint8_t>() % SOFTBUS_BC_SCAN_PHY_CODED;
    scanParam.scanFilterPolicy = GetData<uint8_t>() % SOFTBUS_BC_SCAN_FILTER_POLICY_ONLY_WHITE_LIST_AND_RPA;
    lpScanParam.scanParam = scanParam;

    return lpScanParam;
}

inline int32_t BuildLpBroadcastParam(LpBroadcastParam *lpBcParam)
{
    g_baseFuzzPos = 0;
    lpBcParam->bcHandle = GetData<int32_t>();
    BuildBroadcastParam(&lpBcParam->bcParam);
    BuildBroadcastPacketExceptPayload(&lpBcParam->packet);
    lpBcParam->packet.bcData.payloadLen = sizeof(SoftbusBroadcastMgrTest::BC_DATA_PAYLOAD);
    lpBcParam->packet.bcData.payload = SoftbusBroadcastMgrTest::BC_DATA_PAYLOAD;
    lpBcParam->packet.rspData.payloadLen = sizeof(SoftbusBroadcastMgrTest::RSP_DATA_PAYLOAD);
    lpBcParam->packet.rspData.payload = SoftbusBroadcastMgrTest::RSP_DATA_PAYLOAD;
    return SOFTBUS_OK;
}

// filter is released in UnRegisterScanListener
BcScanFilter *GetBcScanFilter(void);

inline void BleBcEnableCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BleBcDisableCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BleBcUpdateCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BleBcDataCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BcSetBroadcastingParamCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BcDisableBroadcastingCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BcEnableBroadcastingCallback(int32_t channel, int32_t status)
{
    DISC_LOGI(DISC_TEST, "channel=%{public}d, status=%{public}d", channel, status);
}

inline void BleOnScanStart(int32_t listenerId, int32_t status)
{
    (void)listenerId;
    (void)status;
    DISC_LOGI(DISC_TEST, "BleOnScanStart");
}

inline void BleOnScanStop(int32_t listenerId, int32_t status)
{
    (void)listenerId;
    (void)status;
    DISC_LOGI(DISC_TEST, "BleOnScanStop");
}

inline void BleScanResultCallback(int32_t listenerId, const BroadcastReportInfo *reportInfo)
{
    (void)listenerId;
}

inline BroadcastCallback *GetBroadcastCallback()
{
    static BroadcastCallback g_bcCallback = {
        .OnStartBroadcastingCallback = BleBcEnableCallback,
        .OnStopBroadcastingCallback = BleBcDisableCallback,
        .OnUpdateBroadcastingCallback = BleBcUpdateCallback,
        .OnSetBroadcastingCallback = BleBcDataCallback,
        .OnSetBroadcastingParamCallback = BcSetBroadcastingParamCallback,
        .OnDisableBroadcastingCallback = BcDisableBroadcastingCallback,
        .OnEnableBroadcastingCallback = BcEnableBroadcastingCallback,
    };
    return &g_bcCallback;
}

inline ScanCallback *GetScanCallback()
{
    static ScanCallback g_scanListener = {
        .OnStartScanCallback = BleOnScanStart,
        .OnStopScanCallback = BleOnScanStop,
        .OnReportScanDataCallback = BleScanResultCallback,
    };
    return &g_scanListener;
}
} // namespace OHOS

#endif /* SOFTBUS_BROADCAST_MGR_TEST_COMMON_H */
