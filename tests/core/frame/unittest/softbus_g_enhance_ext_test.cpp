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

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include <gtest/gtest.h>

#include "auth_log.h"
#include "disc_coap_capability_public.h"
#include "g_enhance_adapter_func.h"
#include "g_enhance_adapter_func_pack.h"
#include "g_enhance_auth_func.h"
#include "g_enhance_auth_func_pack.h"
#include "g_enhance_conn_func.h"
#include "g_enhance_conn_func_pack.h"
#include "g_enhance_disc_func.h"
#include "g_enhance_disc_func_pack.h"
#include "g_enhance_lnn_func.h"
#include "g_enhance_lnn_func_pack.h"
#include "g_enhance_trans_func.h"
#include "g_enhance_trans_func_pack.h"
#include "lnn_heartbeat_medium_mgr.h"
#include "lnn_heartbeat_utils_struct.h"
#include "lnn_secure_storage_struct.h"
#include "softbus_broadcast_manager.h"
#include "softbus_error_code.h"
#include "softbus_init_common.h"
#include "softbus_server_frame.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {

#define TEST_SHORT_UDID_HASH_HEX_LEN 16
#define VIRTUAL_DEFAULT_SCORE 60
#define TEST_PID 1570
#define TEST_CHANNEL_ID 2531
#define TEST_REQUEST_ID 1251
#define TEST_BLE_POWER_MOCK (-17)
#define TEST_RANGE_CAPACITY_VALUE 100
#define TEST_SLE_LISTENER_ID 1
#define TEST_ANTI_SLEEP_TIMEOUT_MS 1000

static const char *TEST_PKG_NAME = "TEST_PKG_NAME";
static const char *TEST_UDID = "11223344";
static const char *TEST_NETWORK_ID = "11223344";

class SoftbusGEnhanceTest : public testing::Test {
public:
    SoftbusGEnhanceTest()
    {}
    ~SoftbusGEnhanceTest()
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

static void RegisterRadarCbForOpenSrcStub(void *callback)
{
    (void)callback;
}

#ifdef DISC_COMMUNITY
static int SoftBusGetBlePowerStub(int8_t *power)
{
    *power = TEST_BLE_POWER_MOCK;
    return SOFTBUS_OK;
}
#endif /* DISC_COMMUNITY */

static int SoftBusBleRangeStub(SoftBusRangeParam *param, int32_t *range)
{
    (void)param;
    *range = TEST_RANGE_CAPACITY_VALUE;
    return SOFTBUS_OK;
}

static int32_t SoftBusRegRangeCbStub(SoftBusRangeModule module, const SoftBusRangeCallback *callback)
{
    (void)module;
    (void)callback;
    return SOFTBUS_OK;
}

static void SoftBusUnregRangeCbStub(SoftBusRangeModule module)
{
    (void)module;
}

static int32_t SoftBusAddSleStateListenerStub(const SoftBusSleStateListener *listener, int32_t *listenerId)
{
    (void)listener;
    *listenerId = 1;
    return SOFTBUS_OK;
}

static bool IsSleEnabledStub(void)
{
    return true;
}

static void SoftBusRemoveSleStateListenerStub(int listenerId)
{
    (void)listenerId;
}

static int32_t GetSleRangeCapacityStub(void)
{
    return TEST_RANGE_CAPACITY_VALUE;
}

static int32_t GetLocalSleAddrStub(char *sleAddr, uint32_t sleAddrLen)
{
    (void)sleAddrLen;
    sleAddr[0] = '\0';
    return SOFTBUS_OK;
}

static void SoftbusSleAdapterInitStub(void) {}

static void SoftbusSleAdapterDeInitStub(void) {}

static void AntiSleepHoldRunningLockStub(int32_t timeOutMs)
{
    (void)timeOutMs;
}

static void AntiSleepUnHoldRunningLockStub(void) {}

/*
 * @tc.name: SoftbusGEnhanceTest040
 * @tc.desc: adapter pack forwarding branch test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SoftbusGEnhanceTest040, TestSize.Level1)
{
    AdapterEnhanceFuncList *pfnAdapterEnhanceFuncList = AdapterEnhanceFuncListGet();

    pfnAdapterEnhanceFuncList->registerRadarCbForOpenSrc = RegisterRadarCbForOpenSrcStub;
    EXPECT_NO_FATAL_FAILURE(RegisterRadarCbForOpenSrcPacked(nullptr));
    pfnAdapterEnhanceFuncList->registerRadarCbForOpenSrc = nullptr;
    EXPECT_NO_FATAL_FAILURE(RegisterRadarCbForOpenSrcPacked(nullptr));

    int32_t ret = SOFTBUS_OK;
#ifdef DISC_COMMUNITY
    int8_t power = 0;
    pfnAdapterEnhanceFuncList->softBusGetBlePower = nullptr;
    ret = SoftBusGetBlePowerPacked(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = SoftBusGetBlePowerPacked(&power);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnAdapterEnhanceFuncList->softBusGetBlePower = SoftBusGetBlePowerStub;
    power = 0;
    ret = SoftBusGetBlePowerPacked(&power);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(power, TEST_BLE_POWER_MOCK);
#endif

    int32_t range = 0;
    pfnAdapterEnhanceFuncList->softBusBleRange = nullptr;
    ret = SoftBusBleRangePacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    SoftBusRangeParam rangeParam = {0};
    ret = SoftBusBleRangePacked(&rangeParam, &range);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnAdapterEnhanceFuncList->softBusBleRange = SoftBusBleRangeStub;
    ret = SoftBusBleRangePacked(&rangeParam, &range);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(range, TEST_RANGE_CAPACITY_VALUE);
}

/*
 * @tc.name: SoftbusGEnhanceTest041
 * @tc.desc: adapter range callback and sle listener pack forwarding branch test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SoftbusGEnhanceTest041, TestSize.Level1)
{
    AdapterEnhanceFuncList *pfnAdapterEnhanceFuncList = AdapterEnhanceFuncListGet();

    pfnAdapterEnhanceFuncList->softBusRegRangeCb = SoftBusRegRangeCbStub;
    int32_t ret = SoftBusRegRangeCbPacked(MODULE_BLE_DISCOVERY, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnAdapterEnhanceFuncList->softBusRegRangeCb = nullptr;
    ret = SoftBusRegRangeCbPacked(MODULE_BLE_DISCOVERY, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnAdapterEnhanceFuncList->softBusUnregRangeCb = SoftBusUnregRangeCbStub;
    EXPECT_NO_FATAL_FAILURE(SoftBusUnregRangeCbPacked(MODULE_BLE_DISCOVERY));
    pfnAdapterEnhanceFuncList->softBusUnregRangeCb = nullptr;
    EXPECT_NO_FATAL_FAILURE(SoftBusUnregRangeCbPacked(MODULE_BLE_DISCOVERY));

    int32_t listenerId = 0;
    pfnAdapterEnhanceFuncList->softBusAddSleStateListener = SoftBusAddSleStateListenerStub;
    ret = SoftBusAddSleStateListenerPacked(nullptr, &listenerId);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(listenerId, TEST_SLE_LISTENER_ID);
    pfnAdapterEnhanceFuncList->softBusAddSleStateListener = nullptr;
    listenerId = 0;
    ret = SoftBusAddSleStateListenerPacked(nullptr, &listenerId);
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnAdapterEnhanceFuncList->isSleEnabled = IsSleEnabledStub;
    EXPECT_EQ(IsSleEnabledPacked(), true);
    pfnAdapterEnhanceFuncList->isSleEnabled = nullptr;
    EXPECT_EQ(IsSleEnabledPacked(), false);

    pfnAdapterEnhanceFuncList->softBusRemoveSleStateListener = SoftBusRemoveSleStateListenerStub;
    EXPECT_NO_FATAL_FAILURE(SoftBusRemoveSleStateListenerPacked(TEST_SLE_LISTENER_ID));
    pfnAdapterEnhanceFuncList->softBusRemoveSleStateListener = nullptr;
    EXPECT_NO_FATAL_FAILURE(SoftBusRemoveSleStateListenerPacked(TEST_SLE_LISTENER_ID));
}

/*
 * @tc.name: SoftbusGEnhanceTest043
 * @tc.desc: adapter sle address, sle adapter init and anti sleep pack forwarding branch test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SoftbusGEnhanceTest043, TestSize.Level1)
{
    AdapterEnhanceFuncList *pfnAdapterEnhanceFuncList = AdapterEnhanceFuncListGet();

    pfnAdapterEnhanceFuncList->getSleRangeCapacity = GetSleRangeCapacityStub;
    EXPECT_EQ(GetSleRangeCapacityPacked(), TEST_RANGE_CAPACITY_VALUE);
    pfnAdapterEnhanceFuncList->getSleRangeCapacity = nullptr;
    EXPECT_EQ(GetSleRangeCapacityPacked(), SOFTBUS_OK);

    char sleAddr[SOFTBUS_DEV_IDENTITY_LEN] = {0};
    pfnAdapterEnhanceFuncList->getLocalSleAddr = GetLocalSleAddrStub;
    int32_t ret = GetLocalSleAddrPacked(sleAddr, sizeof(sleAddr));
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnAdapterEnhanceFuncList->getLocalSleAddr = nullptr;
    ret = GetLocalSleAddrPacked(sleAddr, sizeof(sleAddr));
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnAdapterEnhanceFuncList->softbusSleAdapterInit = SoftbusSleAdapterInitStub;
    pfnAdapterEnhanceFuncList->softbusSleAdapterDeInit = SoftbusSleAdapterDeInitStub;
    EXPECT_NO_FATAL_FAILURE(SoftbusSleAdapterInitPacked());
    EXPECT_NO_FATAL_FAILURE(SoftbusSleAdapterDeInitPacked());
    pfnAdapterEnhanceFuncList->softbusSleAdapterInit = nullptr;
    pfnAdapterEnhanceFuncList->softbusSleAdapterDeInit = nullptr;
    EXPECT_NO_FATAL_FAILURE(SoftbusSleAdapterInitPacked());
    EXPECT_NO_FATAL_FAILURE(SoftbusSleAdapterDeInitPacked());

    pfnAdapterEnhanceFuncList->antiSleepHoldRunningLock = AntiSleepHoldRunningLockStub;
    pfnAdapterEnhanceFuncList->antiSleepUnHoldRunningLock = AntiSleepUnHoldRunningLockStub;
    EXPECT_NO_FATAL_FAILURE(AntiSleepHoldRunningLockPacked(TEST_ANTI_SLEEP_TIMEOUT_MS));
    EXPECT_NO_FATAL_FAILURE(AntiSleepUnHoldRunningLockPacked());
    pfnAdapterEnhanceFuncList->antiSleepHoldRunningLock = nullptr;
    pfnAdapterEnhanceFuncList->antiSleepUnHoldRunningLock = nullptr;
    EXPECT_NO_FATAL_FAILURE(AntiSleepHoldRunningLockPacked(TEST_ANTI_SLEEP_TIMEOUT_MS));
    EXPECT_NO_FATAL_FAILURE(AntiSleepUnHoldRunningLockPacked());
}

static int32_t ConnCoapStartServerListenStub(void)
{
    return SOFTBUS_OK;
}

static void ConnCoapStopServerListenStub(void) {}

static void SoftbusBleConflictNotifyDisconnectStub(const char *addr, const char *udid)
{
    (void)addr;
    (void)udid;
}

static void SoftbusBleConflictNotifyDateReceiveStub(int32_t underlayerHandle, const uint8_t *data, uint32_t dataLen)
{
    (void)underlayerHandle;
    (void)data;
    (void)dataLen;
}

static void SoftbusBleConflictNotifyConnectResultStub(uint32_t requestId, int32_t underlayerHandle, bool status)
{
    (void)requestId;
    (void)underlayerHandle;
    (void)status;
}

static void SoftbusBleConflictRegisterListenerStub(SoftBusBleConflictListener *listener)
{
    (void)listener;
}

static int32_t ConnExtInitStub(void)
{
    return SOFTBUS_OK;
}

static bool ConnBleDirectIsEnableStub(BleProtocolType protocol)
{
    (void)protocol;
    return true;
}

static int32_t ConnBleDirectConnectDeviceStub(const ConnectOption *option, uint32_t reqId, const ConnectResult *result)
{
    (void)option;
    (void)reqId;
    (void)result;
    return SOFTBUS_OK;
}

static int32_t ConnDirectConnectDeviceStub(const ConnectOption *option, uint32_t reqId, const ConnectResult *result)
{
    (void)option;
    (void)reqId;
    (void)result;
    return SOFTBUS_OK;
}

static ConnectFuncInterface *ConnSleInitStub(const ConnectCallback *callback)
{
    (void)callback;
    return nullptr;
}

static int32_t ConnPagingConnectInitStub(void)
{
    return SOFTBUS_OK;
}

/*
 * @tc.name: SoftbusGEnhanceTest042
 * @tc.desc: conn coap listen and ble conflict pack forwarding branch test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SoftbusGEnhanceTest042, TestSize.Level1)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();

    pfnConnEnhanceFuncList->connCoapStartServerListen = ConnCoapStartServerListenStub;
    int32_t ret = ConnCoapStartServerListenPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnConnEnhanceFuncList->connCoapStartServerListen = nullptr;
    ret = ConnCoapStartServerListenPacked();
    EXPECT_EQ(ret, SOFTBUS_FUNC_NOT_SUPPORT);

    pfnConnEnhanceFuncList->connCoapStopServerListen = ConnCoapStopServerListenStub;
    EXPECT_NO_FATAL_FAILURE(ConnCoapStopServerListenPacked());
    pfnConnEnhanceFuncList->connCoapStopServerListen = nullptr;
    EXPECT_NO_FATAL_FAILURE(ConnCoapStopServerListenPacked());

    pfnConnEnhanceFuncList->softbusBleConflictNotifyDisconnect = SoftbusBleConflictNotifyDisconnectStub;
    pfnConnEnhanceFuncList->softbusBleConflictNotifyDateReceive = SoftbusBleConflictNotifyDateReceiveStub;
    pfnConnEnhanceFuncList->softbusBleConflictNotifyConnectResult = SoftbusBleConflictNotifyConnectResultStub;
    pfnConnEnhanceFuncList->softbusBleConflictRegisterListener = SoftbusBleConflictRegisterListenerStub;
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictNotifyDisconnectPacked(nullptr, nullptr));
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictNotifyDateReceivePacked(0, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictNotifyConnectResultPacked(0, 0, false));
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictRegisterListenerPacked(nullptr));
    pfnConnEnhanceFuncList->softbusBleConflictNotifyDisconnect = nullptr;
    pfnConnEnhanceFuncList->softbusBleConflictNotifyDateReceive = nullptr;
    pfnConnEnhanceFuncList->softbusBleConflictNotifyConnectResult = nullptr;
    pfnConnEnhanceFuncList->softbusBleConflictRegisterListener = nullptr;
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictNotifyDisconnectPacked(nullptr, nullptr));
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictNotifyDateReceivePacked(0, nullptr, 0));
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictNotifyConnectResultPacked(0, 0, false));
    EXPECT_NO_FATAL_FAILURE(SoftbusBleConflictRegisterListenerPacked(nullptr));
}

/*
 * @tc.name: SoftbusGEnhanceTest044
 * @tc.desc: conn ext init, direct connect and sle pack forwarding branch test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SoftbusGEnhanceTest044, TestSize.Level1)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();

    pfnConnEnhanceFuncList->connExtInit = ConnExtInitStub;
    int32_t ret = ConnExtInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnConnEnhanceFuncList->connExtInit = nullptr;
    ret = ConnExtInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnConnEnhanceFuncList->connBleDirectIsEnable = ConnBleDirectIsEnableStub;
    EXPECT_EQ(ConnBleDirectIsEnablePacked(BLE_PROTOCOL_MAX), true);
    pfnConnEnhanceFuncList->connBleDirectIsEnable = nullptr;
    EXPECT_EQ(ConnBleDirectIsEnablePacked(BLE_PROTOCOL_MAX), false);

    pfnConnEnhanceFuncList->connBleDirectConnectDevice = ConnBleDirectConnectDeviceStub;
    ret = ConnBleDirectConnectDevicePacked(nullptr, 0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnConnEnhanceFuncList->connBleDirectConnectDevice = nullptr;
    ret = ConnBleDirectConnectDevicePacked(nullptr, 0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnConnEnhanceFuncList->connDirectConnectDevice = ConnDirectConnectDeviceStub;
    ret = ConnDirectConnectDevicePacked(nullptr, 0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnConnEnhanceFuncList->connDirectConnectDevice = nullptr;
    ret = ConnDirectConnectDevicePacked(nullptr, 0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnConnEnhanceFuncList->connSleInit = ConnSleInitStub;
    EXPECT_EQ(ConnSleInitPacked(nullptr), nullptr);
    pfnConnEnhanceFuncList->connSleInit = nullptr;
    EXPECT_EQ(ConnSleInitPacked(nullptr), nullptr);

    pfnConnEnhanceFuncList->connPagingConnectInit = ConnPagingConnectInitStub;
    ret = ConnPagingConnectInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnConnEnhanceFuncList->connPagingConnectInit = nullptr;
    ret = ConnPagingConnectInitPacked();
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
}

#ifdef SUPPORT_JSAPI
static int32_t SchedulerStartBroadcastStub(int32_t bcId, BroadcastContentType bcType, const BroadcastParam *bcParam,
    const BroadcastPacket *bcPacket)
{
    (void)bcId;
    (void)bcType;
    (void)bcParam;
    (void)bcPacket;
    return SOFTBUS_OK;
}

static int32_t SchedulerUpdateBroadcastStub(int32_t bcId, const BroadcastParam *bcParam,
    const BroadcastPacket *bcPacket)
{
    (void)bcId;
    (void)bcParam;
    (void)bcPacket;
    return SOFTBUS_OK;
}

static int32_t SchedulerSetBroadcastDataStub(int32_t bcId, const BroadcastPacket *bcPacket)
{
    (void)bcId;
    (void)bcPacket;
    return SOFTBUS_OK;
}

static int32_t SchedulerStopBroadcastStub(int32_t bcId)
{
    (void)bcId;
    return SOFTBUS_OK;
}

/*
 * @tc.name: SchedulerStartBroadcastPackedTest001
 * @tc.desc: SchedulerStartBroadcastPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerStartBroadcastPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerStartBroadcast = SchedulerStartBroadcastStub;
    int32_t ret = SchedulerStartBroadcastPacked(0, BC_TYPE_DISTRIB_CON, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SchedulerUpdateBroadcastPackedTest001
 * @tc.desc: SchedulerUpdateBroadcastPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerUpdateBroadcastPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerUpdateBroadcast = SchedulerUpdateBroadcastStub;
    int32_t ret = SchedulerUpdateBroadcastPacked(0, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SchedulerSetBroadcastDataPackedTest001
 * @tc.desc: SchedulerSetBroadcastDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerSetBroadcastDataPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerSetBroadcastData = SchedulerSetBroadcastDataStub;
    int32_t ret = SchedulerSetBroadcastDataPacked(0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SchedulerStopBroadcastPackedTest001
 * @tc.desc: SchedulerStopBroadcastPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerStopBroadcastPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerStopBroadcast = SchedulerStopBroadcastStub;
    int32_t ret = SchedulerStopBroadcastPacked(0);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SchedulerSetBroadcastParamPackedTest001
 * @tc.desc: SchedulerSetBroadcastParamPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerSetBroadcastParamPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerSetBroadcastParam = SchedulerSetBroadcastParamStub;
    int32_t ret = SchedulerSetBroadcastParamPacked(0, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SchedulerInitBroadcastPackedTest001
 * @tc.desc: SchedulerInitBroadcastPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerInitBroadcastPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerInitBroadcast = InitBroadcastMgrStub;
    int32_t ret = SchedulerInitBroadcastPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: SchedulerDeinitBroadcastPackedTest001
 * @tc.desc: SchedulerDeinitBroadcastPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SchedulerDeinitBroadcastPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->schedulerDeinitBroadcast = DeInitBroadcastMgrStub;
    int32_t ret = SchedulerDeinitBroadcastPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}
#endif

static int32_t DiscCoapExtInitStub(void)
{
    return SOFTBUS_OK;
}

static void DiscCoapExtDeinitStub(void) {}

/*
 * @tc.name: DiscCoapExtInitPackedTest001
 * @tc.desc: DiscCoapExtInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscCoapExtInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discCoapExtInit = nullptr;
    int32_t ret = DiscCoapExtInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->discCoapExtInit = DiscCoapExtInitStub;
    ret = DiscCoapExtInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscCoapExtDeinitPackedTest001
 * @tc.desc: DiscCoapExtDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscCoapExtDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discCoapExtDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscCoapExtDeinitPacked());
    pfnDiscEnhanceFuncList->discCoapExtDeinit = DiscCoapExtDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscCoapExtDeinitPacked());
}

#ifdef DSOFTBUS_FEATURE_DISC_SHARE_COAP
static void DiscCoapUpdateAbilityStub(uint32_t capability, const char *capabilityData, uint32_t dataLen,
    bool isPublish, bool isStart)
{
    (void)capability;
    (void)capabilityData;
    (void)dataLen;
    (void)isPublish;
    (void)isStart;
}

/*
 * @tc.name: DiscCoapUpdateAbilityPackedTest001
 * @tc.desc: DiscCoapUpdateAbilityPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscCoapUpdateAbilityPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discCoapUpdateAbility = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscCoapUpdateAbilityPacked(0, nullptr, 0, false, false));
    pfnDiscEnhanceFuncList->discCoapUpdateAbility = DiscCoapUpdateAbilityStub;
    EXPECT_NO_FATAL_FAILURE(DiscCoapUpdateAbilityPacked(0, nullptr, 0, false, false));
}
#endif

static DiscoveryBleDispatcherInterface *DiscTouchBleInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static DiscoveryBleDispatcherInterface *DiscOopBleInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static DiscoveryBleDispatcherInterface *DiscPcCollaborationInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static DiscoveryBleDispatcherInterface *DiscShareBleInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static DiscoveryBleDispatcherInterface *DiscApproachBleInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static DiscoveryBleDispatcherInterface *DiscVLinkBleInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static void DiscShareBleDeinitStub(void) {}

static void DiscApproachBleDeinitStub(void) {}

static void DiscVLinkBleDeinitStub(void) {}

static void DiscTouchBleDeinitStub(void) {}

static void DiscOopBleDeinitStub(void) {}

static void DiscPcCollaborationEventDeinitStub(void) {}

/*
 * @tc.name: DiscTouchBleInitPackedTest001
 * @tc.desc: DiscTouchBleInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscTouchBleInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discTouchBleInit = nullptr;
    DiscoveryBleDispatcherInterface *ret = DiscTouchBleInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discTouchBleInit = DiscTouchBleInitStub;
    ret = DiscTouchBleInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscOopBleInitPackedTest001
 * @tc.desc: DiscOopBleInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscOopBleInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discOopBleInit = nullptr;
    DiscoveryBleDispatcherInterface *ret = DiscOopBleInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discOopBleInit = DiscOopBleInitStub;
    ret = DiscOopBleInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscPcCollaborationInitPackedTest001
 * @tc.desc: DiscPcCollaborationInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscPcCollaborationInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discPcCollaborationBleInit = nullptr;
    DiscoveryBleDispatcherInterface *ret = DiscPcCollaborationInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discPcCollaborationBleInit = DiscPcCollaborationInitStub;
    ret = DiscPcCollaborationInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscShareBleInitPackedTest001
 * @tc.desc: DiscShareBleInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscShareBleInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discShareBleInit = nullptr;
    DiscoveryBleDispatcherInterface *ret = DiscShareBleInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discShareBleInit = DiscShareBleInitStub;
    ret = DiscShareBleInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscApproachBleInitPackedTest001
 * @tc.desc: DiscApproachBleInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscApproachBleInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discApproachBleInit = nullptr;
    DiscoveryBleDispatcherInterface *ret = DiscApproachBleInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discApproachBleInit = DiscApproachBleInitStub;
    ret = DiscApproachBleInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscVLinkBleInitPackedTest001
 * @tc.desc: DiscVLinkBleInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscVLinkBleInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discVLinkBleInit = nullptr;
    DiscoveryBleDispatcherInterface *ret = DiscVLinkBleInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discVLinkBleInit = DiscVLinkBleInitStub;
    ret = DiscVLinkBleInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscShareBleDeinitPackedTest001
 * @tc.desc: DiscShareBleDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscShareBleDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discShareBleDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscShareBleDeinitPacked());
    pfnDiscEnhanceFuncList->discShareBleDeinit = DiscShareBleDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscShareBleDeinitPacked());
}

/*
 * @tc.name: DiscApproachBleDeinitPackedTest001
 * @tc.desc: DiscApproachBleDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscApproachBleDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discApproachBleDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscApproachBleDeinitPacked());
    pfnDiscEnhanceFuncList->discApproachBleDeinit = DiscApproachBleDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscApproachBleDeinitPacked());
}

/*
 * @tc.name: DiscVLinkBleDeinitPackedTest001
 * @tc.desc: DiscVLinkBleDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscVLinkBleDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discVLinkBleDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscVLinkBleDeinitPacked());
    pfnDiscEnhanceFuncList->discVLinkBleDeinit = DiscVLinkBleDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscVLinkBleDeinitPacked());
}

/*
 * @tc.name: DiscTouchBleDeinitPackedTest001
 * @tc.desc: DiscTouchBleDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscTouchBleDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discTouchBleDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscTouchBleDeinitPacked());
    pfnDiscEnhanceFuncList->discTouchBleDeinit = DiscTouchBleDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscTouchBleDeinitPacked());
}

/*
 * @tc.name: DiscOopBleDeinitPackedTest001
 * @tc.desc: DiscOopBleDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscOopBleDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discOopBleDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscOopBleDeinitPacked());
    pfnDiscEnhanceFuncList->discOopBleDeinit = DiscOopBleDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscOopBleDeinitPacked());
}

/*
 * @tc.name: DiscPcCollaborationEventDeinitPackedTest001
 * @tc.desc: DiscPcCollaborationEventDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscPcCollaborationEventDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->pcCollaborationEventDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscPcCollaborationEventDeinitPacked());
    pfnDiscEnhanceFuncList->pcCollaborationEventDeinit = DiscPcCollaborationEventDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscPcCollaborationEventDeinitPacked());
}

static int32_t DiscApproachBleEventInitStub(void)
{
    return SOFTBUS_OK;
}

static int32_t DiscVLinkBleEventInitStub(void)
{
    return SOFTBUS_OK;
}

static int32_t DiscTouchBleEventInitStub(void)
{
    return SOFTBUS_OK;
}

static int32_t DiscOopBleEventInitStub(void)
{
    return SOFTBUS_OK;
}

static int32_t DiscPcCollaborationEventInitStub(void)
{
    return SOFTBUS_OK;
}

static void DiscTouchBleEventDeinitStub(void) {}

static void DiscApproachBleEventDeinitStub(void) {}

static void DiscVLinkBleEventDeinitStub(void) {}

static void DiscOopBleEventDeinitStub(void) {}

/*
 * @tc.name: DiscApproachBleEventInitPackedTest001
 * @tc.desc: DiscApproachBleEventInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscApproachBleEventInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discApproachBleEventInit = nullptr;
    int32_t ret = DiscApproachBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->discApproachBleEventInit = DiscApproachBleEventInitStub;
    ret = DiscApproachBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscVLinkBleEventInitPackedTest001
 * @tc.desc: DiscVLinkBleEventInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscVLinkBleEventInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discVLinkBleEventInit = nullptr;
    int32_t ret = DiscVLinkBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->discVLinkBleEventInit = DiscVLinkBleEventInitStub;
    ret = DiscVLinkBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscTouchBleEventInitPackedTest001
 * @tc.desc: DiscTouchBleEventInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscTouchBleEventInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discTouchBleEventInit = nullptr;
    int32_t ret = DiscTouchBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->discTouchBleEventInit = DiscTouchBleEventInitStub;
    ret = DiscTouchBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscOopBleEventInitPackedTest001
 * @tc.desc: DiscOopBleEventInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscOopBleEventInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discOopBleEventInit = nullptr;
    int32_t ret = DiscOopBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->discOopBleEventInit = DiscOopBleEventInitStub;
    ret = DiscOopBleEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscPcCollaborationEventInitPackedTest001
 * @tc.desc: DiscPcCollaborationEventInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscPcCollaborationEventInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discPcCollaborationEventInit = nullptr;
    int32_t ret = DiscPcCollaborationEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->discPcCollaborationEventInit = DiscPcCollaborationEventInitStub;
    ret = DiscPcCollaborationEventInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DiscTouchBleEventDeinitPackedTest001
 * @tc.desc: DiscTouchBleEventDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscTouchBleEventDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discTouchBleEventDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscTouchBleEventDeinitPacked());
    pfnDiscEnhanceFuncList->discTouchBleEventDeinit = DiscTouchBleEventDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscTouchBleEventDeinitPacked());
}

/*
 * @tc.name: DiscApproachBleEventDeinitPackedTest001
 * @tc.desc: DiscApproachBleEventDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscApproachBleEventDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discApproachBleEventDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscApproachBleEventDeinitPacked());
    pfnDiscEnhanceFuncList->discApproachBleEventDeinit = DiscApproachBleEventDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscApproachBleEventDeinitPacked());
}

/*
 * @tc.name: DiscVLinkBleEventDeinitPackedTest001
 * @tc.desc: DiscVLinkBleEventDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscVLinkBleEventDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discVLinkBleEventDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscVLinkBleEventDeinitPacked());
    pfnDiscEnhanceFuncList->discVLinkBleEventDeinit = DiscVLinkBleEventDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscVLinkBleEventDeinitPacked());
}

/*
 * @tc.name: DiscOopBleEventDeinitPackedTest001
 * @tc.desc: DiscOopBleEventDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscOopBleEventDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discOopBleEventDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscOopBleEventDeinitPacked());
    pfnDiscEnhanceFuncList->discOopBleEventDeinit = DiscOopBleEventDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscOopBleEventDeinitPacked());
}

static DiscoveryUsbDispatcherInterface *DiscUsbInitStub(DiscInnerCallback *discInnerCb)
{
    (void)discInnerCb;
    return nullptr;
}

static void DiscUsbDeinitStub(void) {}

/*
 * @tc.name: DiscUsbInitPackedTest001
 * @tc.desc: DiscUsbInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscUsbInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscInnerCallback discCb = {0};
    pfnDiscEnhanceFuncList->discUsbInit = nullptr;
    DiscoveryUsbDispatcherInterface *ret = DiscUsbInitPacked(&discCb);
    EXPECT_NE(ret, nullptr);
    pfnDiscEnhanceFuncList->discUsbInit = DiscUsbInitStub;
    ret = DiscUsbInitPacked(&discCb);
    EXPECT_EQ(ret, nullptr);
}

/*
 * @tc.name: DiscUsbDeinitPackedTest001
 * @tc.desc: DiscUsbDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscUsbDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discUsbDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscUsbDeinitPacked());
    pfnDiscEnhanceFuncList->discUsbDeinit = DiscUsbDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DiscUsbDeinitPacked());
}

static int32_t DistUpdatePublishParamStub(const char *cust, const char *extCust, bool isStart)
{
    (void)cust;
    (void)extCust;
    (void)isStart;
    return SOFTBUS_OK;
}

static int32_t DistDiscoveryStartActionPreLinkStub(void)
{
    return SOFTBUS_OK;
}

static int32_t DistDiscoveryStopActionPreLinkStub(void)
{
    return SOFTBUS_OK;
}

static int32_t DistPublishStopActionPreLinkStub(void)
{
    return SOFTBUS_OK;
}

static void DistGetActionParamStub(DiscActionParam *action)
{
    action->channelId = 1;
}

static bool DistActionProcessConPacketStub(DeviceWrapper *wrapperDevice, const uint8_t *key, uint32_t len)
{
    (void)wrapperDevice;
    (void)key;
    (void)len;
    return true;
}

static int32_t DistActionInitStub(DiscActionUpdateBleCallback *updateAdvCb, DiscInnerCallback *innerCb)
{
    (void)updateAdvCb;
    (void)innerCb;
    return SOFTBUS_OK;
}

static void DistActionDeinitStub(void) {}

/*
 * @tc.name: DistUpdatePublishParamPackedTest001
 * @tc.desc: DistUpdatePublishParamPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistUpdatePublishParamPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distUpdatePublishParam = nullptr;
    int32_t ret = DistUpdatePublishParamPacked(nullptr, nullptr, false);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->distUpdatePublishParam = DistUpdatePublishParamStub;
    ret = DistUpdatePublishParamPacked(nullptr, nullptr, false);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DistDiscoveryStartActionPreLinkPackedTest001
 * @tc.desc: DistDiscoveryStartActionPreLinkPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistDiscoveryStartActionPreLinkPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distDiscoveryStartActionPreLink = nullptr;
    int32_t ret = DistDiscoveryStartActionPreLinkPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->distDiscoveryStartActionPreLink = DistDiscoveryStartActionPreLinkStub;
    ret = DistDiscoveryStartActionPreLinkPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DistDiscoveryStopActionPreLinkPackedTest001
 * @tc.desc: DistDiscoveryStopActionPreLinkPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistDiscoveryStopActionPreLinkPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distDiscoveryStopActionPreLink = nullptr;
    int32_t ret = DistDiscoveryStopActionPreLinkPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->distDiscoveryStopActionPreLink = DistDiscoveryStopActionPreLinkStub;
    ret = DistDiscoveryStopActionPreLinkPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DistPublishStopActionPreLinkPackedTest001
 * @tc.desc: DistPublishStopActionPreLinkPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistPublishStopActionPreLinkPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distPublishStopActionPreLink = nullptr;
    int32_t ret = DistPublishStopActionPreLinkPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->distPublishStopActionPreLink = DistPublishStopActionPreLinkStub;
    ret = DistPublishStopActionPreLinkPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DistGetActionParamPackedTest001
 * @tc.desc: DistGetActionParamPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistGetActionParamPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    DiscActionParam action = {0};
    pfnDiscEnhanceFuncList->distGetActionParam = nullptr;
    DistGetActionParamPacked(&action);
    EXPECT_EQ(action.channelId, 0);
    pfnDiscEnhanceFuncList->distGetActionParam = DistGetActionParamStub;
    DistGetActionParamPacked(&action);
    EXPECT_EQ(action.channelId, 1);
}

/*
 * @tc.name: DistActionProcessConPacketPackedTest001
 * @tc.desc: DistActionProcessConPacketPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistActionProcessConPacketPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distActionProcessConPacket = nullptr;
    bool ret = DistActionProcessConPacketPacked(nullptr, nullptr, 0);
    EXPECT_FALSE(ret);
    pfnDiscEnhanceFuncList->distActionProcessConPacket = DistActionProcessConPacketStub;
    ret = DistActionProcessConPacketPacked(nullptr, nullptr, 0);
    EXPECT_TRUE(ret);
}

/*
 * @tc.name: DistActionInitPackedTest001
 * @tc.desc: DistActionInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistActionInitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distActionInit = nullptr;
    int32_t ret = DistActionInitPacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
    pfnDiscEnhanceFuncList->distActionInit = DistActionInitStub;
    ret = DistActionInitPacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

/*
 * @tc.name: DistActionDeinitPackedTest001
 * @tc.desc: DistActionDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DistActionDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->distActionDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(DistActionDeinitPacked());
    pfnDiscEnhanceFuncList->distActionDeinit = DistActionDeinitStub;
    EXPECT_NO_FATAL_FAILURE(DistActionDeinitPacked());
}

static bool IsUnknownDeviceStub(const char *bleMacStr)
{
    (void)bleMacStr;
    return true;
}

/*
 * @tc.name: IsUnknownDevicePackedTest001
 * @tc.desc: IsUnknownDevicePacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, IsUnknownDevicePackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->isUnknownDevice = nullptr;
    bool ret = IsUnknownDevicePacked("00:11:22:33:44:55");
    EXPECT_FALSE(ret);
    pfnDiscEnhanceFuncList->isUnknownDevice = IsUnknownDeviceStub;
    ret = IsUnknownDevicePacked("00:11:22:33:44:55");
    EXPECT_TRUE(ret);
}

static void PcCollaborationManagerDeinitStub(void) {}

/*
 * @tc.name: PcCollaborationManagerDeinitPackedTest001
 * @tc.desc: PcCollaborationManagerDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PcCollaborationManagerDeinitPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->pcCollaborationManagerDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(PcCollaborationManagerDeinitPacked());
    pfnDiscEnhanceFuncList->pcCollaborationManagerDeinit = PcCollaborationManagerDeinitStub;
    EXPECT_NO_FATAL_FAILURE(PcCollaborationManagerDeinitPacked());
}

static void DiscCoapReportNotificationStub(const NSTACKX_NotificationConfig *notification)
{
    (void)notification;
}

/*
 * @tc.name: DiscCoapReportNotificationPackedTest001
 * @tc.desc: DiscCoapReportNotificationPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscCoapReportNotificationPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    NSTACKX_NotificationConfig notification = {0};
    pfnDiscEnhanceFuncList->discCoapReportNotification = nullptr;
    EXPECT_NO_FATAL_FAILURE(DiscCoapReportNotificationPacked(&notification));
    pfnDiscEnhanceFuncList->discCoapReportNotification = DiscCoapReportNotificationStub;
    EXPECT_NO_FATAL_FAILURE(DiscCoapReportNotificationPacked(&notification));
}

#ifdef DSOFTBUS_FEATURE_DISC_COAP
static int32_t DiscCoapFillServiceDataStub(const PublishOption *option, char *outData, uint32_t outDataLen,
    uint32_t allCap)
{
    (void)option;
    (void)outData;
    (void)outDataLen;
    (void)allCap;
    return SOFTBUS_OK;
}

/*
 * @tc.name: DiscCoapFillServiceDataPackedTest001
 * @tc.desc: DiscCoapFillServiceDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, DiscCoapFillServiceDataPackedTest001, TestSize.Level1)
{
    DiscEnhanceFuncList *pfnDiscEnhanceFuncList = DiscEnhanceFuncListGet();
    pfnDiscEnhanceFuncList->discCoapFillServiceData = DiscCoapFillServiceDataStub;
    int32_t ret = DiscCoapFillServiceDataPacked(nullptr, nullptr, 0, 0);
    EXPECT_EQ(ret, SOFTBUS_OK);
}
#endif

static int32_t LnnProcessPushMsgStub(const uint8_t *data, uint32_t len)
{
    (void)data;
    (void)len;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnProcessPushMsgPackedTest001
 * @tc.desc: LnnProcessPushMsgPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnProcessPushMsgPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnProcessPushMsg = nullptr;
    int32_t ret = LnnProcessPushMsgPacked(nullptr, 0);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnProcessPushMsg = LnnProcessPushMsgStub;
    ret = LnnProcessPushMsgPacked(nullptr, 0);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnRegisterSleHeartbeatMediumMgrStub(void)
{
    return SOFTBUS_INVALID_PARAM;
}

/*
 * @tc.name: LnnRegisterSleHeartbeatMediumMgrPackedTest001
 * @tc.desc: LnnRegisterSleHeartbeatMediumMgrPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnRegisterSleHeartbeatMediumMgrPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnRegisterSleHeartbeatMediumMgr = nullptr;
    int32_t ret = LnnRegisterSleHeartbeatMediumMgrPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnLnnEnhanceFuncList->lnnRegisterSleHeartbeatMediumMgr = LnnRegisterSleHeartbeatMediumMgrStub;
    ret = LnnRegisterSleHeartbeatMediumMgrPacked();
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

static bool g_metaNodeExtLedgerDeinitCalled = false;

static void LnnDeinitMetaNodeExtLedgerStub(void)
{
    g_metaNodeExtLedgerDeinitCalled = true;
}

/*
 * @tc.name: LnnDeinitMetaNodeExtLedgerPackedTest001
 * @tc.desc: LnnDeinitMetaNodeExtLedgerPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnDeinitMetaNodeExtLedgerPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnDeinitMetaNodeExtLedger = nullptr;
    EXPECT_NO_FATAL_FAILURE(LnnDeinitMetaNodeExtLedgerPacked());

    pfnLnnEnhanceFuncList->lnnDeinitMetaNodeExtLedger = LnnDeinitMetaNodeExtLedgerStub;
    g_metaNodeExtLedgerDeinitCalled = false;
    EXPECT_NO_FATAL_FAILURE(LnnDeinitMetaNodeExtLedgerPacked());
    EXPECT_TRUE(g_metaNodeExtLedgerDeinitCalled);
}

static bool LnnIsSupportLpSparkFeatureStub(void)
{
    return true;
}

/*
 * @tc.name: LnnIsSupportLpSparkFeaturePackedTest001
 * @tc.desc: LnnIsSupportLpSparkFeaturePacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnIsSupportLpSparkFeaturePackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnIsSupportLpSparkFeature = nullptr;
    bool ret = LnnIsSupportLpSparkFeaturePacked();
    EXPECT_FALSE(ret);

    pfnLnnEnhanceFuncList->lnnIsSupportLpSparkFeature = LnnIsSupportLpSparkFeatureStub;
    ret = LnnIsSupportLpSparkFeaturePacked();
    EXPECT_TRUE(ret);
}

static bool LnnIsFeatureSupportDetailStub(void)
{
    return true;
}

/*
 * @tc.name: LnnIsFeatureSupportDetailPackedTest001
 * @tc.desc: LnnIsFeatureSupportDetailPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnIsFeatureSupportDetailPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->isFeatureSupportDetail = nullptr;
    bool ret = LnnIsFeatureSupportDetailPacked();
    EXPECT_FALSE(ret);

    pfnLnnEnhanceFuncList->isFeatureSupportDetail = LnnIsFeatureSupportDetailStub;
    ret = LnnIsFeatureSupportDetailPacked();
    EXPECT_TRUE(ret);
}

static bool g_updateLocalDeviceInfoToMlpsCalled = false;

static void UpdateLocalDeviceInfoToMlpsStub(const NodeInfo *localInfo)
{
    (void)localInfo;
    g_updateLocalDeviceInfoToMlpsCalled = true;
}

/*
 * @tc.name: UpdateLocalDeviceInfoToMlpsPackedTest001
 * @tc.desc: UpdateLocalDeviceInfoToMlpsPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, UpdateLocalDeviceInfoToMlpsPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->updateLocalDeviceInfoToMlps = nullptr;
    EXPECT_NO_FATAL_FAILURE(UpdateLocalDeviceInfoToMlpsPacked(nullptr));

    pfnLnnEnhanceFuncList->updateLocalDeviceInfoToMlps = UpdateLocalDeviceInfoToMlpsStub;
    g_updateLocalDeviceInfoToMlpsCalled = false;
    EXPECT_NO_FATAL_FAILURE(UpdateLocalDeviceInfoToMlpsPacked(nullptr));
    EXPECT_TRUE(g_updateLocalDeviceInfoToMlpsCalled);
}

static bool IsSupportLowLatencyStub(const TransReqInfo *reqInfo, const LaneLinkInfo *laneLinkInfo)
{
    (void)reqInfo;
    (void)laneLinkInfo;
    return true;
}

/*
 * @tc.name: IsSupportLowLatencyPackedTest001
 * @tc.desc: IsSupportLowLatencyPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, IsSupportLowLatencyPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->isSupportLowLatency = nullptr;
    bool ret = IsSupportLowLatencyPacked(nullptr, nullptr);
    EXPECT_FALSE(ret);

    pfnLnnEnhanceFuncList->isSupportLowLatency = IsSupportLowLatencyStub;
    ret = IsSupportLowLatencyPacked(nullptr, nullptr);
    EXPECT_TRUE(ret);
}

static char g_retrieveDeviceData[] = "deviceDataStub";

static int32_t LnnRetrieveDeviceDataStub(LnnDataType dataType, char **data, uint32_t *dataLen)
{
    (void)dataType;
    if (data != nullptr) {
        *data = g_retrieveDeviceData;
    }
    if (dataLen != nullptr) {
        *dataLen = sizeof(g_retrieveDeviceData) - 1;
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnRetrieveDeviceDataPackedTest001
 * @tc.desc: LnnRetrieveDeviceDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnRetrieveDeviceDataPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    char *data = nullptr;
    uint32_t dataLen = 0;
    pfnLnnEnhanceFuncList->lnnRetrieveDeviceData = nullptr;
    int32_t ret = LnnRetrieveDeviceDataPacked(LNN_DATA_TYPE_ACCOUNT_INFO, &data, &dataLen);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(dataLen, 0u);

    pfnLnnEnhanceFuncList->lnnRetrieveDeviceData = LnnRetrieveDeviceDataStub;
    ret = LnnRetrieveDeviceDataPacked(LNN_DATA_TYPE_ACCOUNT_INFO, &data, &dataLen);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_STREQ(data, "deviceDataStub");
    EXPECT_EQ(dataLen, 14u);
}

static int32_t LnnSaveDeviceDataStub(const char *data, LnnDataType dataType)
{
    (void)data;
    (void)dataType;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnSaveDeviceDataPackedTest001
 * @tc.desc: LnnSaveDeviceDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnSaveDeviceDataPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnSaveDeviceData = nullptr;
    int32_t ret = LnnSaveDeviceDataPacked("deviceData", LNN_DATA_TYPE_ACCOUNT_INFO);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnSaveDeviceData = LnnSaveDeviceDataStub;
    ret = LnnSaveDeviceDataPacked("deviceData", LNN_DATA_TYPE_ACCOUNT_INFO);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnVirtualLinkInitStub(void)
{
    return SOFTBUS_INVALID_PARAM;
}

/*
 * @tc.name: LnnVirtualLinkInitPackedTest001
 * @tc.desc: LnnVirtualLinkInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnVirtualLinkInitPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnVirtualLinkInit = nullptr;
    int32_t ret = LnnVirtualLinkInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnLnnEnhanceFuncList->lnnVirtualLinkInit = LnnVirtualLinkInitStub;
    ret = LnnVirtualLinkInitPacked();
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

static bool g_virtualLinkDeinitCalled = false;

static void LnnVirtualLinkDeinitStub(void)
{
    g_virtualLinkDeinitCalled = true;
}

/*
 * @tc.name: LnnVirtualLinkDeinitPackedTest001
 * @tc.desc: LnnVirtualLinkDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnVirtualLinkDeinitPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnVirtualLinkDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(LnnVirtualLinkDeinitPacked());

    pfnLnnEnhanceFuncList->lnnVirtualLinkDeinit = LnnVirtualLinkDeinitStub;
    g_virtualLinkDeinitCalled = false;
    EXPECT_NO_FATAL_FAILURE(LnnVirtualLinkDeinitPacked());
    EXPECT_TRUE(g_virtualLinkDeinitCalled);
}

static int32_t LnnRegisterPushListenerStub(void)
{
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnRegisterPushListenerPackedTest001
 * @tc.desc: LnnRegisterPushListenerPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnRegisterPushListenerPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnRegisterPushListener = nullptr;
    int32_t ret = LnnRegisterPushListenerPacked();
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnRegisterPushListener = LnnRegisterPushListenerStub;
    ret = LnnRegisterPushListenerPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnSetLocalChannelInfoStub(LnnVapType type, int32_t channelId)
{
    (void)type;
    (void)channelId;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnSetLocalChannelInfoPackedTest001
 * @tc.desc: LnnSetLocalChannelInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnSetLocalChannelInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnSetLocalChannelInfo = nullptr;
    int32_t ret = LnnSetLocalChannelInfoPacked(LNN_VAP_HML, TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnSetLocalChannelInfo = LnnSetLocalChannelInfoStub;
    ret = LnnSetLocalChannelInfoPacked(LNN_VAP_HML, TEST_CHANNEL_ID);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static bool IsDeviceHasRiskFactorStub(void)
{
    return true;
}

/*
 * @tc.name: IsDeviceHasRiskFactorPackedTest001
 * @tc.desc: IsDeviceHasRiskFactorPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, IsDeviceHasRiskFactorPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->isDeviceHasRiskFactor = nullptr;
    bool ret = IsDeviceHasRiskFactorPacked();
    EXPECT_FALSE(ret);

    pfnLnnEnhanceFuncList->isDeviceHasRiskFactor = IsDeviceHasRiskFactorStub;
    ret = IsDeviceHasRiskFactorPacked();
    EXPECT_TRUE(ret);
}

static bool g_deinitLocalUserInfoCalled = false;

static void LnnDeinitLocalUserInfoStub(void)
{
    g_deinitLocalUserInfoCalled = true;
}

/*
 * @tc.name: LnnDeinitLocalUserInfoPackedTest001
 * @tc.desc: LnnDeinitLocalUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnDeinitLocalUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnDeinitLocalUserInfo = nullptr;
    EXPECT_NO_FATAL_FAILURE(LnnDeinitLocalUserInfoPacked());

    pfnLnnEnhanceFuncList->lnnDeinitLocalUserInfo = LnnDeinitLocalUserInfoStub;
    g_deinitLocalUserInfoCalled = false;
    EXPECT_NO_FATAL_FAILURE(LnnDeinitLocalUserInfoPacked());
    EXPECT_TRUE(g_deinitLocalUserInfoCalled);
}

static int32_t LnnPackCloudSyncUserInfoStub(cJSON *json, const UserInfo *userInfo)
{
    (void)json;
    (void)userInfo;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnPackCloudSyncUserInfoPackedTest001
 * @tc.desc: LnnPackCloudSyncUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnPackCloudSyncUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnPackCloudSyncUserInfo = nullptr;
    int32_t ret = LnnPackCloudSyncUserInfoPacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnPackCloudSyncUserInfo = LnnPackCloudSyncUserInfoStub;
    ret = LnnPackCloudSyncUserInfoPacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnUnPackCloudSyncUserInfoStub(cJSON *json, UserInfo *userInfo)
{
    (void)json;
    (void)userInfo;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnUnPackCloudSyncUserInfoPackedTest001
 * @tc.desc: LnnUnPackCloudSyncUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnUnPackCloudSyncUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnUnPackCloudSyncUserInfo = nullptr;
    int32_t ret = LnnUnPackCloudSyncUserInfoPacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnUnPackCloudSyncUserInfo = LnnUnPackCloudSyncUserInfoStub;
    ret = LnnUnPackCloudSyncUserInfoPacked(nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static bool LnnCheckUserExistsByAccountIdStub(int64_t accountId)
{
    (void)accountId;
    return true;
}

/*
 * @tc.name: LnnCheckUserExistsByAccountIdPackedTest001
 * @tc.desc: LnnCheckUserExistsByAccountIdPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnCheckUserExistsByAccountIdPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnCheckUserExistsByAccountId = nullptr;
    bool ret = LnnCheckUserExistsByAccountIdPacked(1);
    EXPECT_FALSE(ret);

    pfnLnnEnhanceFuncList->lnnCheckUserExistsByAccountId = LnnCheckUserExistsByAccountIdStub;
    ret = LnnCheckUserExistsByAccountIdPacked(1);
    EXPECT_TRUE(ret);
}

static int32_t LnnSaveLocalUserInfoStub(void)
{
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnSaveLocalUserInfoPackedTest001
 * @tc.desc: LnnSaveLocalUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnSaveLocalUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnSaveLocalUserInfo = nullptr;
    int32_t ret = LnnSaveLocalUserInfoPacked();
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnSaveLocalUserInfo = LnnSaveLocalUserInfoStub;
    ret = LnnSaveLocalUserInfoPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnLoadLocalUserInfoStub(void)
{
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnLoadLocalUserInfoPackedTest001
 * @tc.desc: LnnLoadLocalUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnLoadLocalUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnLoadLocalUserInfo = nullptr;
    int32_t ret = LnnLoadLocalUserInfoPacked();
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnLoadLocalUserInfo = LnnLoadLocalUserInfoStub;
    ret = LnnLoadLocalUserInfoPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnSaveRemoteUserInfoStub(const char *udid, const UserInfo *userInfo)
{
    (void)udid;
    (void)userInfo;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnSaveRemoteUserInfoPackedTest001
 * @tc.desc: LnnSaveRemoteUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnSaveRemoteUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnSaveRemoteUserInfo = nullptr;
    int32_t ret = LnnSaveRemoteUserInfoPacked(TEST_UDID, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnSaveRemoteUserInfo = LnnSaveRemoteUserInfoStub;
    ret = LnnSaveRemoteUserInfoPacked(TEST_UDID, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnLoadRemoteUserInfoStub(void)
{
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnLoadRemoteUserInfoPackedTest001
 * @tc.desc: LnnLoadRemoteUserInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnLoadRemoteUserInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnLoadRemoteUserInfo = nullptr;
    int32_t ret = LnnLoadRemoteUserInfoPacked();
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnLoadRemoteUserInfo = LnnLoadRemoteUserInfoStub;
    ret = LnnLoadRemoteUserInfoPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnAsyncSaveDeviceDataStub(const char *data, LnnDataType dataType)
{
    (void)data;
    (void)dataType;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnAsyncSaveDeviceDataPackedTest001
 * @tc.desc: LnnAsyncSaveDeviceDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnAsyncSaveDeviceDataPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnAsyncSaveDeviceData = nullptr;
    int32_t ret = LnnAsyncSaveDeviceDataPacked("deviceData", LNN_DATA_TYPE_ACCOUNT_INFO);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnAsyncSaveDeviceData = LnnAsyncSaveDeviceDataStub;
    ret = LnnAsyncSaveDeviceDataPacked("deviceData", LNN_DATA_TYPE_ACCOUNT_INFO);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnDeleteDeviceDataStub(LnnDataType dataType)
{
    (void)dataType;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnDeleteDeviceDataPackedTest001
 * @tc.desc: LnnDeleteDeviceDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnDeleteDeviceDataPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnDeleteDeviceData = nullptr;
    int32_t ret = LnnDeleteDeviceDataPacked(LNN_DATA_TYPE_ACCOUNT_INFO);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnDeleteDeviceData = LnnDeleteDeviceDataStub;
    ret = LnnDeleteDeviceDataPacked(LNN_DATA_TYPE_ACCOUNT_INFO);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t LnnInitDecisionCenterV2Stub(void)
{
    return SOFTBUS_INVALID_PARAM;
}

/*
 * @tc.name: LnnInitDecisionCenterV2PackedTest001
 * @tc.desc: LnnInitDecisionCenterV2Packed function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnInitDecisionCenterV2PackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnInitDecisionCenterV2 = nullptr;
    int32_t ret = LnnInitDecisionCenterV2Packed();
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnLnnEnhanceFuncList->lnnInitDecisionCenterV2 = LnnInitDecisionCenterV2Stub;
    ret = LnnInitDecisionCenterV2Packed();
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

static bool g_deinitDecisionCenterV2Called = false;

static void LnnDeinitDecisionCenterV2Stub(void)
{
    g_deinitDecisionCenterV2Called = true;
}

/*
 * @tc.name: LnnDeinitDecisionCenterV2PackedTest001
 * @tc.desc: LnnDeinitDecisionCenterV2Packed function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnDeinitDecisionCenterV2PackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnDeinitDecisionCenterV2 = nullptr;
    EXPECT_NO_FATAL_FAILURE(LnnDeinitDecisionCenterV2Packed());

    pfnLnnEnhanceFuncList->lnnDeinitDecisionCenterV2 = LnnDeinitDecisionCenterV2Stub;
    g_deinitDecisionCenterV2Called = false;
    EXPECT_NO_FATAL_FAILURE(LnnDeinitDecisionCenterV2Packed());
    EXPECT_TRUE(g_deinitDecisionCenterV2Called);
}

static bool g_sdMgrDeathCallbackCalled = false;

static void SdMgrDeathCallbackStub(const char *pkgName)
{
    (void)pkgName;
    g_sdMgrDeathCallbackCalled = true;
}

/*
 * @tc.name: SdMgrDeathCallbackPackedTest001
 * @tc.desc: SdMgrDeathCallbackPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, SdMgrDeathCallbackPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->sdMgrDeathCallback = nullptr;
    EXPECT_NO_FATAL_FAILURE(SdMgrDeathCallbackPacked(TEST_PKG_NAME));

    pfnLnnEnhanceFuncList->sdMgrDeathCallback = SdMgrDeathCallbackStub;
    g_sdMgrDeathCallbackCalled = false;
    EXPECT_NO_FATAL_FAILURE(SdMgrDeathCallbackPacked(TEST_PKG_NAME));
    EXPECT_TRUE(g_sdMgrDeathCallbackCalled);
}

static int32_t AuthMetaGetIpByMetaNodeIdStub(const char *metaNodeId, char *ip, int32_t len)
{
    (void)metaNodeId;
    if (ip != nullptr && len > 0) {
        ip[0] = '1';
        ip[1] = '\0';
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: AuthMetaGetIpByMetaNodeIdPackedTest001
 * @tc.desc: AuthMetaGetIpByMetaNodeIdPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, AuthMetaGetIpByMetaNodeIdPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    char ip[64] = {0};
    pfnLnnEnhanceFuncList->authMetaGetIpByMetaNodeId = nullptr;
    int32_t ret = AuthMetaGetIpByMetaNodeIdPacked(TEST_NETWORK_ID, ip, sizeof(ip));
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_STREQ(ip, "");

    pfnLnnEnhanceFuncList->authMetaGetIpByMetaNodeId = AuthMetaGetIpByMetaNodeIdStub;
    ret = AuthMetaGetIpByMetaNodeIdPacked(TEST_NETWORK_ID, ip, sizeof(ip));
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_STREQ(ip, "1");
}

static int32_t AuthMetaGetLocalIpByMetaNodeIdStub(const char *metaNodeId, char *localIp, int32_t len)
{
    (void)metaNodeId;
    if (localIp != nullptr && len > 0) {
        localIp[0] = '1';
        localIp[1] = '\0';
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: AuthMetaGetLocalIpByMetaNodeIdPackedTest001
 * @tc.desc: AuthMetaGetLocalIpByMetaNodeIdPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, AuthMetaGetLocalIpByMetaNodeIdPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    char localIp[64] = {0};
    pfnLnnEnhanceFuncList->authMetaGetLocalIpByMetaNodeId = nullptr;
    int32_t ret = AuthMetaGetLocalIpByMetaNodeIdPacked(TEST_NETWORK_ID, localIp, sizeof(localIp));
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_STREQ(localIp, "");

    pfnLnnEnhanceFuncList->authMetaGetLocalIpByMetaNodeId = AuthMetaGetLocalIpByMetaNodeIdStub;
    ret = AuthMetaGetLocalIpByMetaNodeIdPacked(TEST_NETWORK_ID, localIp, sizeof(localIp));
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_STREQ(localIp, "1");
}

static bool IsSupportMcuFeatureStub(void)
{
    return true;
}

/*
 * @tc.name: IsSupportMcuFeaturePackedTest001
 * @tc.desc: IsSupportMcuFeaturePacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, IsSupportMcuFeaturePackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->isSupportMcuFeature = nullptr;
    bool ret = IsSupportMcuFeaturePacked();
    EXPECT_FALSE(ret);

    pfnLnnEnhanceFuncList->isSupportMcuFeature = IsSupportMcuFeatureStub;
    ret = IsSupportMcuFeaturePacked();
    EXPECT_TRUE(ret);
}

static bool g_sendDeviceStateToMcuCalled = false;

static void LnnSendDeviceStateToMcuStub(void *para)
{
    (void)para;
    g_sendDeviceStateToMcuCalled = true;
}

/*
 * @tc.name: LnnSendDeviceStateToMcuPackedTest001
 * @tc.desc: LnnSendDeviceStateToMcuPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnSendDeviceStateToMcuPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnSendDeviceStateToMcu = nullptr;
    EXPECT_NO_FATAL_FAILURE(LnnSendDeviceStateToMcuPacked(nullptr));

    pfnLnnEnhanceFuncList->lnnSendDeviceStateToMcu = LnnSendDeviceStateToMcuStub;
    g_sendDeviceStateToMcuCalled = false;
    EXPECT_NO_FATAL_FAILURE(LnnSendDeviceStateToMcuPacked(nullptr));
    EXPECT_TRUE(g_sendDeviceStateToMcuCalled);
}

static int32_t LnnInitMcuStub(void)
{
    return SOFTBUS_INVALID_PARAM;
}

/*
 * @tc.name: LnnInitMcuPackedTest001
 * @tc.desc: LnnInitMcuPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnInitMcuPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnInitMcu = nullptr;
    int32_t ret = LnnInitMcuPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);

    pfnLnnEnhanceFuncList->lnnInitMcu = LnnInitMcuStub;
    ret = LnnInitMcuPacked();
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

static NodeInfo g_remoteDevInfo = {};

static int32_t LnnGetAllRemoteDevInfoStub(NodeInfo **info, int32_t *nums)
{
    if (info != nullptr) {
        *info = &g_remoteDevInfo;
    }
    if (nums != nullptr) {
        *nums = 1;
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnGetAllRemoteDevInfoPackedTest001
 * @tc.desc: LnnGetAllRemoteDevInfoPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnGetAllRemoteDevInfoPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    NodeInfo *info = nullptr;
    int32_t nums = 0;
    pfnLnnEnhanceFuncList->lnnGetAllRemoteDevInfo = nullptr;
    int32_t ret = LnnGetAllRemoteDevInfoPacked(&info, &nums);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_EQ(nums, 0);

    pfnLnnEnhanceFuncList->lnnGetAllRemoteDevInfo = LnnGetAllRemoteDevInfoStub;
    ret = LnnGetAllRemoteDevInfoPacked(&info, &nums);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(nums, 1);
    EXPECT_EQ(info, &g_remoteDevInfo);
}

static int32_t LnnSendAgentDataStub(const char *udid, const char *data, uint32_t length,
    LnnEventExtra *extra, bool isAckMsg)
{
    (void)udid;
    (void)data;
    (void)length;
    (void)extra;
    (void)isAckMsg;
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnSendAgentDataPackedTest001
 * @tc.desc: LnnSendAgentDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnSendAgentDataPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->lnnSendAgentData = nullptr;
    int32_t ret = LnnSendAgentDataPacked(TEST_UDID, nullptr, 0, nullptr, false);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->lnnSendAgentData = LnnSendAgentDataStub;
    ret = LnnSendAgentDataPacked(TEST_UDID, nullptr, 0, nullptr, false);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static bool FarfieldParseModuleTypeStub(const uint8_t *data, uint32_t dataLen, uint32_t *moduleType)
{
    (void)data;
    (void)dataLen;
    if (moduleType != nullptr) {
        *moduleType = 1;
    }
    return true;
}

/*
 * @tc.name: FarfieldParseModuleTypePackedTest001
 * @tc.desc: FarfieldParseModuleTypePacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, FarfieldParseModuleTypePackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    uint32_t moduleType = 0;
    pfnLnnEnhanceFuncList->farfieldParseModuleType = nullptr;
    bool ret = FarfieldParseModuleTypePacked(nullptr, 0, &moduleType);
    EXPECT_FALSE(ret);
    EXPECT_EQ(moduleType, 0u);

    pfnLnnEnhanceFuncList->farfieldParseModuleType = FarfieldParseModuleTypeStub;
    ret = FarfieldParseModuleTypePacked(nullptr, 0, &moduleType);
    EXPECT_TRUE(ret);
    EXPECT_EQ(moduleType, 1u);
}

static int32_t PostLnnCloudEventStub(LnnCloudMsgType event, LnnCloudHandler handler,
    const void *obj, uint32_t size, uint64_t delayMs)
{
    (void)event;
    (void)handler;
    (void)obj;
    (void)size;
    (void)delayMs;
    return SOFTBUS_OK;
}

/*
 * @tc.name: PostLnnCloudEventPackedTest001
 * @tc.desc: PostLnnCloudEventPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PostLnnCloudEventPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->postLnnCloudEvent = nullptr;
    int32_t ret = PostLnnCloudEventPacked(MSG_TYPE_SOCKET_TIMEOUT, nullptr, nullptr, 0, 0);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->postLnnCloudEvent = PostLnnCloudEventStub;
    ret = PostLnnCloudEventPacked(MSG_TYPE_SOCKET_TIMEOUT, nullptr, nullptr, 0, 0);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t RemoveLnnCloudEventStub(LnnCloudMsgType event, LnnCloudRemoveCompareFunc func, void *param)
{
    (void)event;
    (void)func;
    (void)param;
    return SOFTBUS_OK;
}

/*
 * @tc.name: RemoveLnnCloudEventPackedTest001
 * @tc.desc: RemoveLnnCloudEventPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, RemoveLnnCloudEventPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->removeLnnCloudEvent = nullptr;
    int32_t ret = RemoveLnnCloudEventPacked(MSG_TYPE_SOCKET_TIMEOUT, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->removeLnnCloudEvent = RemoveLnnCloudEventStub;
    ret = RemoveLnnCloudEventPacked(MSG_TYPE_SOCKET_TIMEOUT, nullptr, nullptr);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static char g_allDevicesUdid[1][UDID_BUF_LEN] = { "testUdid" };

static int32_t LnnGetAllDevicesUdidStub(bool isCloud, char (**udidList)[UDID_BUF_LEN], uint32_t *count)
{
    (void)isCloud;
    if (udidList != nullptr) {
        *udidList = g_allDevicesUdid;
    }
    if (count != nullptr) {
        *count = 1;
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: LnnGetAllDevicesUdidPackedTest001
 * @tc.desc: LnnGetAllDevicesUdidPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, LnnGetAllDevicesUdidPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    char (*udidList)[UDID_BUF_LEN] = nullptr;
    uint32_t count = 0;
    pfnLnnEnhanceFuncList->lnnGetAllDevicesUdid = nullptr;
    int32_t ret = LnnGetAllDevicesUdidPacked(false, &udidList, &count);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_EQ(count, 0u);

    pfnLnnEnhanceFuncList->lnnGetAllDevicesUdid = LnnGetAllDevicesUdidStub;
    ret = LnnGetAllDevicesUdidPacked(false, &udidList, &count);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(count, 1u);
    EXPECT_STREQ(udidList[0], "testUdid");
}

static int32_t PerceptionBuildAdvDataStub(const PerceptionType *type, const PerceptionAdvParam *params,
    uint32_t paramCnt, BroadcastPacket *packet)
{
    (void)type;
    (void)params;
    (void)paramCnt;
    if (packet != nullptr) {
        packet->bcData.payloadLen = 1;
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: PerceptionBuildAdvDataPackedTest001
 * @tc.desc: PerceptionBuildAdvDataPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionBuildAdvDataPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    PerceptionType type = PERCEPTION_TYPE_COLLABORATIVE_WAKE;
    BroadcastPacket packet = {0};
    pfnLnnEnhanceFuncList->perceptionBuildAdvData = nullptr;
    int32_t ret = PerceptionBuildAdvDataPacked(&type, nullptr, 0, &packet);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_EQ(packet.bcData.payloadLen, 0);

    pfnLnnEnhanceFuncList->perceptionBuildAdvData = PerceptionBuildAdvDataStub;
    ret = PerceptionBuildAdvDataPacked(&type, nullptr, 0, &packet);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(packet.bcData.payloadLen, 1);
}

static BcScanFilter g_perceptionScanFilter[1] = {};

static int32_t PerceptionBuildScanFilterStub(BcScanFilter **filter, uint8_t *filterNum)
{
    if (filter != nullptr) {
        *filter = g_perceptionScanFilter;
    }
    if (filterNum != nullptr) {
        *filterNum = 1;
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: PerceptionBuildScanFilterPackedTest001
 * @tc.desc: PerceptionBuildScanFilterPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionBuildScanFilterPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    BcScanFilter *filter = nullptr;
    uint8_t filterNum = 0;
    pfnLnnEnhanceFuncList->perceptionBuildScanFilter = nullptr;
    int32_t ret = PerceptionBuildScanFilterPacked(&filter, &filterNum);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_EQ(filterNum, 0);

    pfnLnnEnhanceFuncList->perceptionBuildScanFilter = PerceptionBuildScanFilterStub;
    ret = PerceptionBuildScanFilterPacked(&filter, &filterNum);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(filterNum, 1);
    EXPECT_EQ(filter, &g_perceptionScanFilter[0]);
}

static PerceptionDeviceInfo g_perceptionDeviceInfo[1] = {};

static int32_t PerceptionGetDeviceInfoListStub(PerceptionType type, PerceptionDeviceInfo **list, uint32_t *count)
{
    (void)type;
    if (list != nullptr) {
        *list = g_perceptionDeviceInfo;
    }
    if (count != nullptr) {
        *count = 1;
    }
    return SOFTBUS_OK;
}

/*
 * @tc.name: PerceptionGetDeviceInfoListPackedTest001
 * @tc.desc: PerceptionGetDeviceInfoListPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionGetDeviceInfoListPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    PerceptionDeviceInfo *list = nullptr;
    uint32_t count = 0;
    pfnLnnEnhanceFuncList->perceptionGetDeviceInfoList = nullptr;
    int32_t ret = PerceptionGetDeviceInfoListPacked(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &list, &count);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);
    EXPECT_EQ(count, 0u);

    pfnLnnEnhanceFuncList->perceptionGetDeviceInfoList = PerceptionGetDeviceInfoListStub;
    ret = PerceptionGetDeviceInfoListPacked(PERCEPTION_TYPE_COLLABORATIVE_WAKE, &list, &count);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(count, 1u);
    EXPECT_EQ(list, &g_perceptionDeviceInfo[0]);
}

static int32_t PerceptionSyncStateStub(PerceptionState state)
{
    (void)state;
    return SOFTBUS_OK;
}

/*
 * @tc.name: PerceptionSyncStatePackedTest001
 * @tc.desc: PerceptionSyncStatePacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionSyncStatePackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->perceptionSyncState = nullptr;
    int32_t ret = PerceptionSyncStatePacked(PERCEPTION_STATE_START);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->perceptionSyncState = PerceptionSyncStateStub;
    ret = PerceptionSyncStatePacked(PERCEPTION_STATE_START);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t PerceptionSyncCycleStub(PerceptionType type, uint32_t cycle)
{
    (void)type;
    (void)cycle;
    return SOFTBUS_OK;
}

/*
 * @tc.name: PerceptionSyncCyclePackedTest001
 * @tc.desc: PerceptionSyncCyclePacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionSyncCyclePackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->perceptionSyncCycle = nullptr;
    int32_t ret = PerceptionSyncCyclePacked(PERCEPTION_TYPE_COLLABORATIVE_WAKE, 100);
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->perceptionSyncCycle = PerceptionSyncCycleStub;
    ret = PerceptionSyncCyclePacked(PERCEPTION_TYPE_COLLABORATIVE_WAKE, 100);
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static int32_t PerceptionEnhanceInitStub(void)
{
    return SOFTBUS_OK;
}

/*
 * @tc.name: PerceptionEnhanceInitPackedTest001
 * @tc.desc: PerceptionEnhanceInitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionEnhanceInitPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->perceptionEnhanceInit = nullptr;
    int32_t ret = PerceptionEnhanceInitPacked();
    EXPECT_EQ(ret, SOFTBUS_NOT_IMPLEMENT);

    pfnLnnEnhanceFuncList->perceptionEnhanceInit = PerceptionEnhanceInitStub;
    ret = PerceptionEnhanceInitPacked();
    EXPECT_EQ(ret, SOFTBUS_OK);
}

static bool g_perceptionEnhanceDeinitCalled = false;

static void PerceptionEnhanceDeinitStub(void)
{
    g_perceptionEnhanceDeinitCalled = true;
}

/*
 * @tc.name: PerceptionEnhanceDeinitPackedTest001
 * @tc.desc: PerceptionEnhanceDeinitPacked function test
 * @tc.type: FUNC
 * @tc.level: Level1
 * @tc.require:
 */
HWTEST_F(SoftbusGEnhanceTest, PerceptionEnhanceDeinitPackedTest001, TestSize.Level1)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    pfnLnnEnhanceFuncList->perceptionEnhanceDeinit = nullptr;
    EXPECT_NO_FATAL_FAILURE(PerceptionEnhanceDeinitPacked());

    pfnLnnEnhanceFuncList->perceptionEnhanceDeinit = PerceptionEnhanceDeinitStub;
    g_perceptionEnhanceDeinitCalled = false;
    EXPECT_NO_FATAL_FAILURE(PerceptionEnhanceDeinitPacked());
    EXPECT_TRUE(g_perceptionEnhanceDeinitCalled);
}
} // namespace OHOS
