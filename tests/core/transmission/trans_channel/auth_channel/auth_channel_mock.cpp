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

#include <gtest/gtest.h>
#include <securec.h>

#include "auth_channel_mock.h"
#include "softbus_error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
void *g_authChannelInterface;
AuthChannelInterfaceMock::AuthChannelInterfaceMock()
{
    g_authChannelInterface = reinterpret_cast<void *>(this);
}

AuthChannelInterfaceMock::~AuthChannelInterfaceMock()
{
    g_authChannelInterface = nullptr;
}

static AuthChannelInterface *GetAuthChannelInterface()
{
    return reinterpret_cast<AuthChannelInterface *>(g_authChannelInterface);
}

extern "C" {
int32_t LnnInitGetDeviceName(LnnDeviceNameHandler handler)
{
    return GetAuthChannelInterface()->LnnInitGetDeviceName(handler);
}

int32_t LnnGetSettingDeviceName(char *deviceName, uint32_t len)
{
    return GetAuthChannelInterface()->LnnGetSettingDeviceName(deviceName, len);
}

int32_t AuthChannelInterfaceMock::ActionOfLnnInitGetDeviceName(LnnDeviceNameHandler handler)
{
    if (handler == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    g_deviceNameHandler = handler;
    return SOFTBUS_OK;
}
}
}