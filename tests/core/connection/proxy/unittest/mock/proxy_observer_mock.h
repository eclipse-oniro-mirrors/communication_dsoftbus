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

#ifndef PROXY_OBSERVER_MOCK_H
#define PROXY_OBSERVER_MOCK_H

#include <atomic>
#include <memory>
#include <string>
#include <vector>
#include <gmock/gmock.h>

#include "bluetooth_def.h"
#include "bluetooth_hfp_ag.h"
#include "bluetooth_host.h"
#include "bluetooth_remote_device.h"

// Mock device state used to configure BluetoothHost::GetPairedDevices and
// BluetoothRemoteDevice member function return values.
struct MockBtDevice {
    std::string address;
    int pairState = OHOS::Bluetooth::PAIR_NONE;   // PAIR_NONE / PAIR_PAIRING / PAIR_PAIRED
    bool isAclConnected = false;
    std::vector<std::string> uuids;
};

// gmock-based mock for the dsoftbus C utility functions that proxy_observer.cpp
// depends on (SoftBusGenerateStrHash, ConvertBytesToHexString). The extern "C"
// wrappers in proxy_observer_mock.cpp delegate to this mock so that the test can
// control success/failure of the hash/hex conversion path inside
// ConvertRealMacToHashMac.
class ProxyObserverUtilInterface {
public:
    virtual ~ProxyObserverUtilInterface() = default;
    virtual int32_t SoftBusGenerateStrHash(const unsigned char *str, uint32_t len, unsigned char *hash) = 0;
    virtual int32_t ConvertBytesToHexString(
        char *outBuf, uint32_t outBufLen, const unsigned char *inBuf, uint32_t inLen) = 0;
};

class ProxyObserverUtilMock : public ProxyObserverUtilInterface {
public:
    ProxyObserverUtilMock();
    ~ProxyObserverUtilMock() override;

    MOCK_METHOD3(SoftBusGenerateStrHash, int32_t(const unsigned char *, uint32_t, unsigned char *));
    MOCK_METHOD4(ConvertBytesToHexString, int32_t(char *, uint32_t, const unsigned char *, uint32_t));

    static ProxyObserverUtilMock *GetMock();

    // Default actions that mimic the real implementation behaviour so that
    // ConvertRealMacToHashMac produces a deterministic, predictable hash string.
    static int32_t ActionOfSoftBusGenerateStrHash(const unsigned char *str, uint32_t len, unsigned char *hash);
    static int32_t ActionOfConvertBytesToHexString(
        char *outBuf, uint32_t outBufLen, const unsigned char *inBuf, uint32_t inLen);

    // Compute the hash MAC that ConvertRealMacToHashMac would produce for the
    // given address when the default mock actions are used. The test uses this
    // to know which hash string to pass to IsPairedDevice / GetRealMac.
    static std::string ComputeHashMac(const std::string &addr);

private:
    static inline std::atomic<ProxyObserverUtilMock *> mock_ = nullptr;
};

// Global mock state for the Bluetooth C++ APIs. The link-time mock
// implementations of BluetoothHost / HandsFreeAudioGateway /
// BluetoothRemoteDevice member functions read from this state.
namespace ProxyObserverBtMock {
    // Configure the list of mock paired devices returned by GetPairedDevices.
    void SetMockDevices(const std::vector<MockBtDevice> &devices);
    // Configure the return value of GetPairedDevices (0 = success, non-zero = failure).
    void SetGetPairedDevicesResult(int32_t result);
    // Retrieve the observer captured during RegisterHfpListener.
    std::shared_ptr<OHOS::Bluetooth::HandsFreeAudioGatewayObserver> GetHfpObserver();
    std::shared_ptr<OHOS::Bluetooth::BluetoothRemoteDeviceObserver> GetRemoteDeviceObserver();
    // Reset all mock state to defaults.
    void Reset();
} // namespace ProxyObserverBtMock

#endif // PROXY_OBSERVER_MOCK_H
