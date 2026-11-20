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

#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "conn_event.h"
#include "conn_log.h"
#include "far_field_proxy_adapter.h"
#include "securec.h"
#include "softbus_error_code.h"
#include "softbus_init_common.h"

using namespace testing::ext;

namespace {
constexpr const char *TEST_BR_MAC = "11:22:33:44:55:66";
constexpr const char *TEST_UUID = "0000FEEA-0000-1000-8000-00805F9B34FB";
constexpr int32_t ADAPTER_REF_DRAIN_TIMES = 4;

// must match the callback type registered by the adapter for RegisterDfxEventCallback
typedef int32_t (*DfxEventCallbackFunc)(const DfxEventExtra *event);

// ----- behavior controls of the fake far-field so -----
bool g_dlopenShouldFail = false;
std::string g_missingSymbol;
int32_t g_initProxyRet = 0;
int32_t g_registerCallbackRet = 0;
bool g_isDeviceSupportRet = true;
int32_t g_openP2PRet = 0;
int32_t g_closeP2PRet = 0;
int32_t g_sendMsgRet = 0;
int32_t g_refreshRet = 0;
P2PState g_p2pState = P2P_STATE_CONNECT;

// ----- observation of the fake far-field so -----
int32_t g_fakeSoToken = 0; // its address acts as the fake so handle
int32_t g_initProxyCallCnt = 0;
int32_t g_deinitProxyCallCnt = 0;
int32_t g_registerCallbackCallCnt = 0;
int32_t g_isDeviceSupportCallCnt = 0;
int32_t g_openP2PCallCnt = 0;
int32_t g_closeP2PCallCnt = 0;
int32_t g_getP2PStateCallCnt = 0;
int32_t g_sendMsgCallCnt = 0;
int32_t g_refreshCallCnt = 0;
int32_t g_dlcloseCallCnt = 0;
bool g_lastCloseNeedSocket = false;
uint32_t g_lastSendDataLen = 0;
uint8_t g_lastSendFirstByte = 0;
P2PDeviceInfo g_lastDevice = {};
FarFieldCallbackSt g_capturedSoCallback = {};
DfxEventCallbackFunc g_capturedDfxCallback = nullptr;

void TestOnP2PStateChanged(const P2PDeviceInfo *device, P2PState state, int32_t reason)
{
    (void)device;
    (void)state;
    (void)reason;
}

void TestOnRemoteEvent(const P2PDeviceInfo *device, RemoteEvent event)
{
    (void)device;
    (void)event;
}

void TestOnRecvP2PMsg(const P2PDeviceInfo *device, const uint8_t *msgBody, uint32_t len)
{
    (void)device;
    (void)msgBody;
    (void)len;
}

// a second callback set, used to tell FarFieldAdapterRegisterCallback input
// apart from the manager callback registered during FarFieldAdapterInit
void TestOnP2PStateChanged2(const P2PDeviceInfo *device, P2PState state, int32_t reason)
{
    (void)device;
    (void)state;
    (void)reason;
}

void TestOnRemoteEvent2(const P2PDeviceInfo *device, RemoteEvent event)
{
    (void)device;
    (void)event;
}

void TestOnRecvP2PMsg2(const P2PDeviceInfo *device, const uint8_t *msgBody, uint32_t len)
{
    (void)device;
    (void)msgBody;
    (void)len;
}

FarFieldCallbackSt g_managerCallback = {
    .p2pStateCallback = TestOnP2PStateChanged,
    .remoteEventCallback = TestOnRemoteEvent,
    .recvP2PMsgCallback = TestOnRecvP2PMsg,
};

// ----- fake far-field so entry points -----
int32_t FakeFarFieldInitProxy(void)
{
    g_initProxyCallCnt++;
    return g_initProxyRet;
}

int32_t FakeFarFieldDeinitProxy(void)
{
    g_deinitProxyCallCnt++;
    return 0;
}

bool FakeFarFieldIsDeviceSupport(const P2PDeviceInfo *device)
{
    g_isDeviceSupportCallCnt++;
    if (device != nullptr) {
        g_lastDevice = *device;
    }
    return g_isDeviceSupportRet;
}

int32_t FakeFarFieldRegisterCallback(const FarFieldCallbackSt *callback)
{
    g_registerCallbackCallCnt++;
    if (callback != nullptr) {
        g_capturedSoCallback = *callback;
    }
    return g_registerCallbackRet;
}

int32_t FakeFarFieldOpenP2P(const P2PDeviceInfo *device)
{
    g_openP2PCallCnt++;
    if (device != nullptr) {
        g_lastDevice = *device;
    }
    return g_openP2PRet;
}

int32_t FakeFarFieldCloseP2P(const P2PDeviceInfo *device, bool isNeedCloseSocket)
{
    g_closeP2PCallCnt++;
    g_lastCloseNeedSocket = isNeedCloseSocket;
    if (device != nullptr) {
        g_lastDevice = *device;
    }
    return g_closeP2PRet;
}

P2PState FakeFarFieldGetP2PState(const P2PDeviceInfo *device)
{
    g_getP2PStateCallCnt++;
    if (device != nullptr) {
        g_lastDevice = *device;
    }
    return g_p2pState;
}

int32_t FakeFarFieldSendMsg(const P2PDeviceInfo *device, const uint8_t *data, uint32_t len)
{
    g_sendMsgCallCnt++;
    g_lastSendDataLen = len;
    if (data != nullptr && len > 0) {
        g_lastSendFirstByte = data[0];
    }
    if (device != nullptr) {
        g_lastDevice = *device;
    }
    return g_sendMsgRet;
}

int32_t FakeFarFieldRefresh(const P2PDeviceInfo *device)
{
    g_refreshCallCnt++;
    if (device != nullptr) {
        g_lastDevice = *device;
    }
    return g_refreshRet;
}

void FakeRegisterDfxEventCallback(DfxEventCallbackFunc callback)
{
    g_capturedDfxCallback = callback;
}

struct FakeSymbol {
    const char *name;
    void *addr;
};

FakeSymbol g_fakeSymbols[] = {
    { "FarFieldInitProxy", reinterpret_cast<void *>(&FakeFarFieldInitProxy) },
    { "FarFieldDeinitProxy", reinterpret_cast<void *>(&FakeFarFieldDeinitProxy) },
    { "FarFieldIsDeviceSupport", reinterpret_cast<void *>(&FakeFarFieldIsDeviceSupport) },
    { "FarFieldRegisterCallback", reinterpret_cast<void *>(&FakeFarFieldRegisterCallback) },
    { "FarFieldOpenP2P", reinterpret_cast<void *>(&FakeFarFieldOpenP2P) },
    { "FarFieldCloseP2P", reinterpret_cast<void *>(&FakeFarFieldCloseP2P) },
    { "FarFieldGetP2PState", reinterpret_cast<void *>(&FakeFarFieldGetP2PState) },
    { "FarFieldSendMsg", reinterpret_cast<void *>(&FakeFarFieldSendMsg) },
    { "FarFieldRefresh", reinterpret_cast<void *>(&FakeFarFieldRefresh) },
    { "RegisterDfxEventCallback", reinterpret_cast<void *>(&FakeRegisterDfxEventCallback) },
};

void ResetAdapterFakes()
{
    g_dlopenShouldFail = false;
    g_missingSymbol.clear();
    g_initProxyRet = 0;
    g_registerCallbackRet = 0;
    g_isDeviceSupportRet = true;
    g_openP2PRet = 0;
    g_closeP2PRet = 0;
    g_sendMsgRet = 0;
    g_refreshRet = 0;
    g_p2pState = P2P_STATE_CONNECT;
    g_initProxyCallCnt = 0;
    g_deinitProxyCallCnt = 0;
    g_registerCallbackCallCnt = 0;
    g_isDeviceSupportCallCnt = 0;
    g_openP2PCallCnt = 0;
    g_closeP2PCallCnt = 0;
    g_getP2PStateCallCnt = 0;
    g_sendMsgCallCnt = 0;
    g_refreshCallCnt = 0;
    g_dlcloseCallCnt = 0;
    g_lastCloseNeedSocket = false;
    g_lastSendDataLen = 0;
    g_lastSendFirstByte = 0;
    g_lastDevice = {};
    g_capturedSoCallback = {};
    g_capturedDfxCallback = nullptr;
}

P2PDeviceInfo BuildTestDevice()
{
    P2PDeviceInfo device = {};
    strcpy_s(device.brMac, BT_MAC_LEN, TEST_BR_MAC);
    strcpy_s(device.uuid, UUID_STRING_LEN, TEST_UUID);
    return device;
}
} // namespace

extern "C" {
// intercept the so loading entry of the adapter under test
SOFTBUS_DEFAULT_USED_EXPORT int32_t SoftBusDlopen(SoftBusHandleType type, void **dllHandle)
{
    (void)type;
    if (dllHandle == nullptr) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (g_dlopenShouldFail) {
        *dllHandle = nullptr;
        return SOFTBUS_NETWORK_DLOPEN_FAILED;
    }
    *dllHandle = &g_fakeSoToken;
    return SOFTBUS_OK;
}
} // extern "C"

// musl redirects dlsym to __dlsym_time64 on 32-bit targets (see the __REDIR in dlfcn.h);
// the asm label must match the symbol name the adapter under test references.
#if _REDIR_TIME64
#define DLSYM_LINK_NAME "__dlsym_time64"
#else
#define DLSYM_LINK_NAME "dlsym"
#endif

// Dlsym/Dlclose intercept symbol resolving/closing for the adapter under test: asm labels
// emit them under the real libc symbol names, so references from the adapter bind to them
// at static link time, while hidden visibility keeps them out of the dynamic symbol table.
// They must never interpose for shared libraries in the process: a process-wide dlsym
// interposition crashes library constructors that run before this binary's own static
// initializers (observed with libffrt.so calling dlsym in ffrt_init).
__attribute__((visibility("hidden"))) void *Dlsym(void *handle, const char *symbol) __asm__(DLSYM_LINK_NAME);
__attribute__((visibility("hidden"))) void *Dlsym(void *handle, const char *symbol)
{
    if (handle == nullptr || symbol == nullptr) {
        return nullptr;
    }
    if (g_missingSymbol == symbol) {
        return nullptr;
    }
    for (const auto &entry : g_fakeSymbols) {
        if (std::strcmp(entry.name, symbol) == 0) {
            return entry.addr;
        }
    }
    return nullptr;
}

__attribute__((visibility("hidden"))) int Dlclose(void *handle) __asm__("dlclose");
__attribute__((visibility("hidden"))) int Dlclose(void *handle)
{
    (void)handle;
    g_dlcloseCallCnt++;
    return 0;
}

class FarFieldProxyAdapterTest : public testing::Test {
public:
    void SetUp() override
    {
        ResetAdapterFakes();
        ASSERT_EQ(FarFieldAdapterManagerInit(&g_managerCallback), SOFTBUS_OK);
    }

    void TearDown() override
    {
        // drain any so reference the case left behind so the next case starts clean
        for (int32_t i = 0; i < ADAPTER_REF_DRAIN_TIMES; i++) {
            FarFieldAdapterDeinit();
        }
        FarFieldAdapterManagerDeinit();
        ResetAdapterFakes();
    }
};

/*
 * @tc.name: FarFieldProxyAdapterTest001
 * @tc.desc: test FarFieldAdapterManagerInit and ManagerDeinit round-trip
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest001, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest001 in");
    // SetUp already initialized the manager; re-init and deinit must be safe
    int32_t ret = FarFieldAdapterManagerInit(&g_managerCallback);
    EXPECT_EQ(ret, SOFTBUS_OK);
    FarFieldAdapterManagerDeinit();
    ret = FarFieldAdapterManagerInit(&g_managerCallback);
    EXPECT_EQ(ret, SOFTBUS_OK);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest001 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest002
 * @tc.desc: test FarFieldAdapterInit fails when dlopen fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest002, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest002 in");
    g_dlopenShouldFail = true;
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_NETWORK_DLOPEN_FAILED);
    EXPECT_EQ(g_registerCallbackCallCnt, 0);
    EXPECT_EQ(g_initProxyCallCnt, 0);

    // adapter stays uninitialized after the failed load
    P2PDeviceInfo device = BuildTestDevice();
    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest002 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest003
 * @tc.desc: test FarFieldAdapterInit fails when a required symbol is missing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest003, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest003 in");
    g_missingSymbol = "FarFieldInitProxy";
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_SYMBOL_NOT_FOUND);
    EXPECT_EQ(g_dlcloseCallCnt, 1);

    // the failed load is fully rolled back, a retry with all symbols succeeds
    g_missingSymbol.clear();
    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    FarFieldAdapterDeinit(); // deinit closes the so handle as well
    EXPECT_EQ(g_dlcloseCallCnt, 2);

    // a missing symbol late in the table is detected as well
    g_missingSymbol = "FarFieldRefresh";
    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_SYMBOL_NOT_FOUND);
    EXPECT_EQ(g_dlcloseCallCnt, 3);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest003 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest004
 * @tc.desc: test FarFieldAdapterInit success registers manager callback and dfx callback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest004, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest004 in");
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_registerCallbackCallCnt, 1);
    EXPECT_EQ(g_initProxyCallCnt, 1);
    // the callback registered into the so is the one given at ManagerInit
    EXPECT_EQ(g_capturedSoCallback.p2pStateCallback, &TestOnP2PStateChanged);
    EXPECT_EQ(g_capturedSoCallback.remoteEventCallback, &TestOnRemoteEvent);
    EXPECT_EQ(g_capturedSoCallback.recvP2PMsgCallback, &TestOnRecvP2PMsg);
    EXPECT_NE(g_capturedDfxCallback, nullptr);

    FarFieldAdapterDeinit();
    EXPECT_EQ(g_deinitProxyCallCnt, 1);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest004 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest005
 * @tc.desc: test repeated FarFieldAdapterInit only references the so
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest005, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest005 in");
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_ALREADY_INITIALIZED);
    EXPECT_EQ(g_initProxyCallCnt, 1);

    FarFieldAdapterDeinit(); // refCount 2 -> 1, the so stays loaded
    EXPECT_EQ(g_deinitProxyCallCnt, 0);
    FarFieldAdapterDeinit(); // refCount 1 -> 0, the so is unloaded
    EXPECT_EQ(g_deinitProxyCallCnt, 1);

    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_initProxyCallCnt, 2);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest005 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest006
 * @tc.desc: test FarFieldAdapterInit fails when so registerCallback fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest006, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest006 in");
    g_registerCallbackRet = SOFTBUS_ERR;
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_ERROR);
    EXPECT_EQ(g_initProxyCallCnt, 0);

    P2PDeviceInfo device = BuildTestDevice();
    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest006 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest007
 * @tc.desc: test FarFieldAdapterInit fails when so initProxy fails
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest007, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest007 in");
    g_initProxyRet = SOFTBUS_ERR;
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_ERROR);
    EXPECT_EQ(g_initProxyCallCnt, 1);

    P2PDeviceInfo device = BuildTestDevice();
    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest007 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest008
 * @tc.desc: test FarFieldAdapterInit succeeds without the optional dfx symbol
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest008, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest008 in");
    g_missingSymbol = "RegisterDfxEventCallback";
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_capturedDfxCallback, nullptr);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest008 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest009
 * @tc.desc: test FarFieldAdapterIsDeviceSupport loads and unloads the so by itself
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest009, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest009 in");
    P2PDeviceInfo device = BuildTestDevice();
    bool supported = FarFieldAdapterIsDeviceSupport(&device);
    EXPECT_TRUE(supported);
    EXPECT_EQ(g_isDeviceSupportCallCnt, 1);
    EXPECT_STREQ(g_lastDevice.brMac, TEST_BR_MAC);
    EXPECT_STREQ(g_lastDevice.uuid, TEST_UUID);
    // the so was loaded for the query and unloaded right after
    EXPECT_EQ(g_deinitProxyCallCnt, 1);
    EXPECT_EQ(g_dlcloseCallCnt, 1);

    int32_t ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest009 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest010
 * @tc.desc: test FarFieldAdapterIsDeviceSupport returns false
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest010, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest010 in");
    g_isDeviceSupportRet = false;
    P2PDeviceInfo device = BuildTestDevice();
    bool supported = FarFieldAdapterIsDeviceSupport(&device);
    EXPECT_FALSE(supported);

    g_dlopenShouldFail = true;
    supported = FarFieldAdapterIsDeviceSupport(&device);
    EXPECT_FALSE(supported);
    EXPECT_EQ(g_isDeviceSupportCallCnt, 1);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest010 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest011
 * @tc.desc: test FarFieldAdapterIsDeviceSupport keeps the initialized adapter alive
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest011, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest011 in");
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);

    P2PDeviceInfo device = BuildTestDevice();
    bool supported = FarFieldAdapterIsDeviceSupport(&device);
    EXPECT_TRUE(supported);
    // the query only references the so, it must not tear the adapter down
    EXPECT_EQ(g_deinitProxyCallCnt, 0);

    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_openP2PCallCnt, 1);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest011 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest012
 * @tc.desc: test FarFieldAdapterRegisterCallback with null callback or before init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest012, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest012 in");
    int32_t ret = FarFieldAdapterRegisterCallback(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    FarFieldCallbackSt callback = {
        .p2pStateCallback = TestOnP2PStateChanged2,
        .remoteEventCallback = TestOnRemoteEvent2,
        .recvP2PMsgCallback = TestOnRecvP2PMsg2,
    };
    ret = FarFieldAdapterRegisterCallback(&callback);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest012 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest013
 * @tc.desc: test FarFieldAdapterRegisterCallback forwards to the so after init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest013, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest013 in");
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_registerCallbackCallCnt, 1);

    FarFieldCallbackSt callback = {
        .p2pStateCallback = TestOnP2PStateChanged2,
        .remoteEventCallback = TestOnRemoteEvent2,
        .recvP2PMsgCallback = TestOnRecvP2PMsg2,
    };
    ret = FarFieldAdapterRegisterCallback(&callback);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_registerCallbackCallCnt, 2);
    EXPECT_EQ(g_capturedSoCallback.p2pStateCallback, &TestOnP2PStateChanged2);
    EXPECT_EQ(g_capturedSoCallback.remoteEventCallback, &TestOnRemoteEvent2);
    EXPECT_EQ(g_capturedSoCallback.recvP2PMsgCallback, &TestOnRecvP2PMsg2);

    g_registerCallbackRet = SOFTBUS_ERR;
    ret = FarFieldAdapterRegisterCallback(&callback);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest013 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest014
 * @tc.desc: test FarFieldAdapterOpenP2P with invalid param, before init and after init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest014, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest014 in");
    P2PDeviceInfo device = BuildTestDevice();
    int32_t ret = FarFieldAdapterOpenP2P(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);

    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_openP2PCallCnt, 1);
    EXPECT_STREQ(g_lastDevice.brMac, TEST_BR_MAC);
    EXPECT_STREQ(g_lastDevice.uuid, TEST_UUID);

    g_openP2PRet = SOFTBUS_ERR;
    ret = FarFieldAdapterOpenP2P(&device);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest014 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest015
 * @tc.desc: test FarFieldAdapterCloseP2P with invalid param, before init and after init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest015, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest015 in");
    P2PDeviceInfo device = BuildTestDevice();
    int32_t ret = FarFieldAdapterCloseP2P(nullptr, true);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = FarFieldAdapterCloseP2P(&device, true);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);

    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = FarFieldAdapterCloseP2P(&device, true);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_closeP2PCallCnt, 1);
    EXPECT_TRUE(g_lastCloseNeedSocket);
    EXPECT_STREQ(g_lastDevice.brMac, TEST_BR_MAC);

    g_closeP2PRet = SOFTBUS_ERR;
    ret = FarFieldAdapterCloseP2P(&device, false);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    EXPECT_FALSE(g_lastCloseNeedSocket);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest015 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest016
 * @tc.desc: test FarFieldAdapterGetP2PState with invalid param, before init and after init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest016, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest016 in");
    P2PDeviceInfo device = BuildTestDevice();
    P2PState state = FarFieldAdapterGetP2PState(nullptr);
    EXPECT_EQ(state, P2P_STATE_INVALID);
    state = FarFieldAdapterGetP2PState(&device);
    EXPECT_EQ(state, P2P_STATE_INVALID);

    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    g_p2pState = P2P_STATE_DISCONNECT;
    state = FarFieldAdapterGetP2PState(&device);
    EXPECT_EQ(state, P2P_STATE_DISCONNECT);
    EXPECT_EQ(g_getP2PStateCallCnt, 1);
    EXPECT_STREQ(g_lastDevice.brMac, TEST_BR_MAC);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest016 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest017
 * @tc.desc: test FarFieldAdapterSendMsg with invalid params
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest017, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest017 in");
    P2PDeviceInfo device = BuildTestDevice();
    const uint8_t data[] = { 0x01, 0x02 };
    int32_t ret = FarFieldAdapterSendMsg(nullptr, data, sizeof(data));
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = FarFieldAdapterSendMsg(&device, nullptr, sizeof(data));
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = FarFieldAdapterSendMsg(&device, data, 0);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    // params are valid but the adapter is not initialized yet
    ret = FarFieldAdapterSendMsg(&device, data, sizeof(data));
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest017 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest018
 * @tc.desc: test FarFieldAdapterSendMsg forwards data to the so after init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest018, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest018 in");
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);

    P2PDeviceInfo device = BuildTestDevice();
    const uint8_t data[] = { 0xAA, 0xBB, 0xCC };
    ret = FarFieldAdapterSendMsg(&device, data, sizeof(data));
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_sendMsgCallCnt, 1);
    EXPECT_EQ(g_lastSendDataLen, sizeof(data));
    EXPECT_EQ(g_lastSendFirstByte, 0xAA);
    EXPECT_STREQ(g_lastDevice.brMac, TEST_BR_MAC);

    g_sendMsgRet = SOFTBUS_ERR;
    ret = FarFieldAdapterSendMsg(&device, data, sizeof(data));
    EXPECT_EQ(ret, SOFTBUS_ERR);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest018 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest019
 * @tc.desc: test FarFieldAdapterRefresh with invalid param, before init and after init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest019, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest019 in");
    P2PDeviceInfo device = BuildTestDevice();
    int32_t ret = FarFieldAdapterRefresh(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = FarFieldAdapterRefresh(&device);
    EXPECT_EQ(ret, FAR_FIELD_ADAPTER_NOT_INITIALIZED);

    ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    ret = FarFieldAdapterRefresh(&device);
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_refreshCallCnt, 1);
    EXPECT_STREQ(g_lastDevice.brMac, TEST_BR_MAC);

    g_refreshRet = SOFTBUS_ERR;
    ret = FarFieldAdapterRefresh(&device);
    EXPECT_EQ(ret, SOFTBUS_ERR);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest019 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest020
 * @tc.desc: test the dfx event callback registered into the so
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest020, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest020 in");
    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    ASSERT_NE(g_capturedDfxCallback, nullptr);

    ret = g_capturedDfxCallback(nullptr);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);

    char func[] = "FakeDfxFunc";
    char hostPkg[] = "fake_host_pkg";
    DfxEventExtra extra = {
        .func = func,
        .line = 1,
        .bizScene = 1,
        .bizStage = 1,
        .stageRes = 0,
        .errorCode = 0,
        .hostPkg = hostPkg,
        .connectReason = 0,
        .costTime = 0,
        .sendMsgCnt = 0,
        .receiveMsgCnt = 0,
        .negotiateTime = 0,
        .linkTime = 0,
    };
    ret = g_capturedDfxCallback(&extra);
    EXPECT_EQ(ret, SOFTBUS_OK);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest020 out");
}

/*
 * @tc.name: FarFieldProxyAdapterTest021
 * @tc.desc: test FarFieldAdapterDeinit without init is a safe no-op
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(FarFieldProxyAdapterTest, FarFieldProxyAdapterTest021, TestSize.Level1)
{
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest021 in");
    FarFieldAdapterDeinit();
    EXPECT_EQ(g_deinitProxyCallCnt, 0);
    EXPECT_EQ(g_dlcloseCallCnt, 0);

    int32_t ret = FarFieldAdapterInit();
    EXPECT_EQ(ret, SOFTBUS_OK);
    EXPECT_EQ(g_initProxyCallCnt, 1);
    CONN_LOGI(CONN_PROXY, "FarFieldProxyAdapterTest021 out");
}
