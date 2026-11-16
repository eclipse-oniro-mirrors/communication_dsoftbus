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

#include "proxy_observer_mock.h"

#include <cstring>
#include <securec.h>

#include "conn_log.h"
#include "softbus_common.h"
#include "softbus_error_code.h"
#include "softbus_utils.h"

// ---------------------------------------------------------------------------
// Global mock state for Bluetooth C++ APIs
// ---------------------------------------------------------------------------
namespace ProxyObserverBtMock {
static std::vector<MockBtDevice> g_mockDevices;
static int32_t g_getPairedDevicesResult = 0; // 0 = success
static std::shared_ptr<OHOS::Bluetooth::HandsFreeAudioGatewayObserver> g_hfpObserver;
static std::shared_ptr<OHOS::Bluetooth::BluetoothRemoteDeviceObserver> g_remoteDeviceObserver;

void SetMockDevices(const std::vector<MockBtDevice> &devices)
{
    g_mockDevices = devices;
}

void SetGetPairedDevicesResult(int32_t result)
{
    g_getPairedDevicesResult = result;
}

std::shared_ptr<OHOS::Bluetooth::HandsFreeAudioGatewayObserver> GetHfpObserver()
{
    return g_hfpObserver;
}

std::shared_ptr<OHOS::Bluetooth::BluetoothRemoteDeviceObserver> GetRemoteDeviceObserver()
{
    return g_remoteDeviceObserver;
}

void Reset()
{
    g_mockDevices.clear();
    g_getPairedDevicesResult = 0;
    g_hfpObserver.reset();
    g_remoteDeviceObserver.reset();
}

// Look up the mock device entry whose address matches the given BluetoothRemoteDevice.
// Returns nullptr when no entry matches.
static const MockBtDevice *FindMockDevice(const std::string &addr)
{
    for (const auto &dev : g_mockDevices) {
        if (dev.address == addr) {
            return &dev;
        }
    }
    return nullptr;
}
} // namespace ProxyObserverBtMock

// ---------------------------------------------------------------------------
// Link-time mock implementations of Bluetooth C++ member functions.
//
// These definitions override the symbols from libbtframework.so at link time
// because object-file definitions take precedence over shared-library ones.
// They read from ProxyObserverBtMock global state so the test can control
// the return values.
// ---------------------------------------------------------------------------

// BluetoothHost::GetPairedDevices — returns the mock device list.
int32_t OHOS::Bluetooth::BluetoothHost::GetPairedDevices(
    int transport, std::vector<OHOS::Bluetooth::BluetoothRemoteDevice> &pairedDevices) const
{
    if (ProxyObserverBtMock::g_getPairedDevicesResult != 0) {
        return ProxyObserverBtMock::g_getPairedDevicesResult;
    }
    pairedDevices.clear();
    for (const auto &dev : ProxyObserverBtMock::g_mockDevices) {
        pairedDevices.emplace_back(dev.address, OHOS::Bluetooth::BT_TRANSPORT_BREDR);
    }
    return 0;
}

// BluetoothHost::RegisterRemoteDeviceObserver — captures the observer.
void OHOS::Bluetooth::BluetoothHost::RegisterRemoteDeviceObserver(
    std::shared_ptr<OHOS::Bluetooth::BluetoothRemoteDeviceObserver> observer)
{
    ProxyObserverBtMock::g_remoteDeviceObserver = observer;
}

// HandsFreeAudioGateway::RegisterObserver — captures the observer.
void OHOS::Bluetooth::HandsFreeAudioGateway::RegisterObserver(
    std::shared_ptr<OHOS::Bluetooth::HandsFreeAudioGatewayObserver> observer)
{
    ProxyObserverBtMock::g_hfpObserver = observer;
}

// BluetoothRemoteDevice::GetPairState — returns the mock pair state.
int OHOS::Bluetooth::BluetoothRemoteDevice::GetPairState(int &pairState) const
{
    const MockBtDevice *dev = ProxyObserverBtMock::FindMockDevice(address_);
    if (dev != nullptr) {
        pairState = dev->pairState;
        return OHOS::Bluetooth::BT_SUCCESS;
    }
    pairState = OHOS::Bluetooth::PAIR_NONE;
    return OHOS::Bluetooth::BT_FAILURE;
}

// BluetoothRemoteDevice::IsAclConnected — returns the mock acl state.
bool OHOS::Bluetooth::BluetoothRemoteDevice::IsAclConnected() const
{
    const MockBtDevice *dev = ProxyObserverBtMock::FindMockDevice(address_);
    if (dev != nullptr) {
        return dev->isAclConnected;
    }
    return false;
}

// BluetoothRemoteDevice::GetDeviceUuids — returns the mock uuid list.
int OHOS::Bluetooth::BluetoothRemoteDevice::GetDeviceUuids(std::vector<std::string> &uuids) const
{
    const MockBtDevice *dev = ProxyObserverBtMock::FindMockDevice(address_);
    if (dev != nullptr) {
        uuids = dev->uuids;
        return OHOS::Bluetooth::BT_SUCCESS;
    }
    uuids.clear();
    return OHOS::Bluetooth::BT_FAILURE;
}

// ---------------------------------------------------------------------------
// Link-time mock implementations of dsoftbus C utility functions.
//
// proxy_observer.cpp calls SoftBusGenerateStrHash and ConvertBytesToHexString
// inside ConvertRealMacToHashMac. These wrappers delegate to the gmock so the
// test can drive both the success and failure branches.
// ---------------------------------------------------------------------------

ProxyObserverUtilMock::ProxyObserverUtilMock()
{
    mock_.store(this);
}

ProxyObserverUtilMock::~ProxyObserverUtilMock()
{
    mock_.store(nullptr);
}

ProxyObserverUtilMock *ProxyObserverUtilMock::GetMock()
{
    return mock_.load();
}

// Deterministic mock hash: fill the 32-byte buffer by cycling through the
// input bytes. This is NOT real SHA-256, but it is predictable so the test
// can compute the expected hash MAC string via ComputeHashMac().
int32_t ProxyObserverUtilMock::ActionOfSoftBusGenerateStrHash(
    const unsigned char *str, uint32_t len, unsigned char *hash)
{
    if (str == nullptr || hash == nullptr || len == 0) {
        return SOFTBUS_INVALID_PARAM;
    }
    for (uint32_t i = 0; i < SHA_256_HASH_LEN; i++) {
        hash[i] = str[i % len];
    }
    return SOFTBUS_OK;
}

int32_t ProxyObserverUtilMock::ActionOfConvertBytesToHexString(
    char *outBuf, uint32_t outBufLen, const unsigned char *inBuf, uint32_t inLen)
{
    if (outBuf == nullptr || inBuf == nullptr || outBufLen < (inLen * 2 + 1)) {
        return SOFTBUS_INVALID_PARAM;
    }
    static const char hexChars[] = "0123456789abcdef";
    for (uint32_t i = 0; i < inLen; i++) {
        outBuf[i * 2] = hexChars[(inBuf[i] >> 4) & 0x0F];
        outBuf[i * 2 + 1] = hexChars[inBuf[i] & 0x0F];
    }
    outBuf[inLen * 2] = '\0';
    return SOFTBUS_OK;
}

std::string ProxyObserverUtilMock::ComputeHashMac(const std::string &addr)
{
    if (addr.empty()) {
        return "";
    }
    unsigned char hash[SHA_256_HASH_LEN] = {0};
    ActionOfSoftBusGenerateStrHash(
        reinterpret_cast<const unsigned char *>(addr.c_str()), addr.length(), hash);
    char hex[HEXIFY_LEN(SHA_256_HASH_LEN)] = {0}; // 65 bytes
    ActionOfConvertBytesToHexString(hex, sizeof(hex), hash, SHA_256_HASH_LEN);
    // ConvertRealMacToHashMac returns the second half (substr(32, 32)).
    return std::string(hex).substr(SHA_256_HASH_LEN, SHA_256_HASH_LEN);
}

extern "C" {
// Override SoftBusGenerateStrHash from softbus_adapter.
int32_t SoftBusGenerateStrHash(const unsigned char *str, uint32_t len, unsigned char *hash)
{
    auto mocker = ProxyObserverUtilMock::GetMock();
    if (mocker == nullptr) {
        return SOFTBUS_ERR;
    }
    return mocker->SoftBusGenerateStrHash(str, len, hash);
}

// Override ConvertBytesToHexString from softbus_utils.
int32_t ConvertBytesToHexString(char *outBuf, uint32_t outBufLen, const unsigned char *inBuf, uint32_t inLen)
{
    auto mocker = ProxyObserverUtilMock::GetMock();
    if (mocker == nullptr) {
        return SOFTBUS_ERR;
    }
    return mocker->ConvertBytesToHexString(outBuf, outBufLen, inBuf, inLen);
}

// Override ConvertAnonymizeMacAddress from softbus_conn_common so that the
// test does not need to pull in the full softbus_conn_common.c translation
// unit (which has many additional dependencies). The anonymized output is
// simply a truncated copy of the input — sufficient for the CONN_LOGW call
// inside proxy_observer.cpp.
void ConvertAnonymizeMacAddress(char *outAnomize, uint32_t anomizeLen, const char *mac, uint32_t macLen)
{
    if (outAnomize == nullptr || mac == nullptr || anomizeLen == 0) {
        return;
    }
    uint32_t copyLen = (macLen < anomizeLen) ? macLen : (anomizeLen - 1);
    if (strncpy_s(outAnomize, anomizeLen, mac, copyLen) != EOK) {
        outAnomize[0] = '\0';
    }
}
} // extern "C"
