/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DISC_COAP_MOCK_H
#define DISC_COAP_MOCK_H

#include <atomic>
#include <gmock/gmock.h>
#include "bus_center_info_key.h"
#include "bus_center_manager.h"
#include "nstackx.h"
#include "softbus_config_type.h"

class DiscCoapInterface {
public:
    virtual int32_t NSTACKX_RegisterServiceDataV2(const struct NSTACKX_ServiceData *param, uint32_t cnt) = 0;
    virtual int32_t LnnGetLocalNumInfo(InfoKey key, int32_t *info) = 0;
    virtual int32_t LnnGetLocalNum64Info(InfoKey key, int64_t *info) = 0;
    virtual int32_t LnnGetLocalStrInfo(InfoKey key, char *info, uint32_t len) = 0;
    virtual int32_t LnnGetLocalNumInfoByIfnameIdx(InfoKey key, int32_t *info, int32_t ifIdx) = 0;
    virtual int32_t LnnGetLocalStrInfoByIfnameIdx(InfoKey key, char *info, uint32_t len, int32_t ifIdx) = 0;
};

class DiscCoapMock : public DiscCoapInterface {
public:
    static DiscCoapMock* GetMock()
    {
        return mock.load();
    }

    DiscCoapMock();
    ~DiscCoapMock();

    void SetupSuccessStub();

    MOCK_METHOD(int32_t, NSTACKX_RegisterServiceDataV2,
        (const struct NSTACKX_ServiceData *param, uint32_t cnt), (override));
    MOCK_METHOD(int32_t, LnnGetLocalNumInfo, (InfoKey key, int32_t *info), (override));
    MOCK_METHOD(int32_t, LnnGetLocalNum64Info, (InfoKey key, int64_t *info), (override));
    MOCK_METHOD(int32_t, LnnGetLocalStrInfo, (InfoKey key, char *info, uint32_t len), (override));
    MOCK_METHOD(int32_t, LnnGetLocalNumInfoByIfnameIdx, (InfoKey key, int32_t *info, int32_t ifIdx), (override));
    MOCK_METHOD(int32_t, LnnGetLocalStrInfoByIfnameIdx,
        (InfoKey key, char *info, uint32_t len, int32_t ifIdx), (override));

    static int32_t ActionOfNstackInit(const NSTACKX_Parameter *parameter);
    static int32_t ActionOfRegisterServiceDataV2(const struct NSTACKX_ServiceData *param, uint32_t cnt);
    static int32_t ActionOfLnnGetLocalNumInfo(InfoKey key, int32_t *info);
    static int32_t ActionOfLnnGetLocalNum64Info(InfoKey key, int64_t *info);
    static int32_t ActionOfLnnGetLocalStrInfo(InfoKey key, char *info, uint32_t len);
    static int32_t ActionOfLnnGetLocalNumInfoByIfnameIdx(InfoKey key, int32_t *info, int32_t ifIdx);
    static int32_t ActionOfLnnGetLocalStrInfoByIfnameIdx(InfoKey key, char *info, uint32_t len, int32_t ifIdx);

private:
    static inline std::atomic<DiscCoapMock*> mock = nullptr;
};

#endif