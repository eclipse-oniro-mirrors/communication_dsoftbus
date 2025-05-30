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

#ifndef DISC_MANAGER_MOCK_H
#define DISC_MANAGER_MOCK_H

#include <gmock/gmock.h>

#include "disc_manager.h"
#include "lnn_ohos_account_adapter.h"

namespace OHOS {
class DiscManagerInterface {
public:
    DiscManagerInterface() {};
    virtual ~DiscManagerInterface() {};

    virtual int32_t DiscPublishService(const char *packageName, const PublishInfo *info, int32_t callingPid) = 0;
    virtual int32_t DiscStartScan(DiscModule moduleId, const PublishInfo *info, int32_t callingPid) = 0;
    virtual int32_t DiscUnPublishService(const char *packageName, int32_t publishId, int32_t callingPid) = 0;
    virtual int32_t DiscUnpublish(DiscModule moduleId, int32_t publishId, int32_t callingPid) = 0;
    virtual int32_t DiscStartDiscovery(
        const char *packageName, const SubscribeInfo *info, const IServerDiscInnerCallback *cb, int32_t callingPid) = 0;
    virtual int32_t DiscSetDiscoverCallback(DiscModule moduleId, const DiscInnerCallback *callback) = 0;
    virtual int32_t DiscStartAdvertise(DiscModule moduleId, const SubscribeInfo *info, int32_t callingPid) = 0;
    virtual int32_t DiscStopDiscovery(const char *packageName, int32_t subscribeId, int32_t callingPid) = 0;
    virtual int32_t DiscStopAdvertise(DiscModule moduleId, int32_t subscribeId, int32_t callingPid) = 0;
};
class DiscManagerInterfaceMock : public DiscManagerInterface {
public:
    DiscManagerInterfaceMock();
    ~DiscManagerInterfaceMock() override;
    MOCK_METHOD3(DiscPublishService, int32_t(const char *, const PublishInfo *, int32_t));
    MOCK_METHOD3(DiscStartScan, int32_t(DiscModule, const PublishInfo *, int32_t));
    MOCK_METHOD3(DiscUnPublishService, int32_t(const char *, int32_t, int32_t));
    MOCK_METHOD3(DiscUnpublish, int32_t(DiscModule, int32_t, int32_t));
    MOCK_METHOD4(DiscStartDiscovery,
        int32_t(const char *, const SubscribeInfo *, const IServerDiscInnerCallback *, int32_t));
    MOCK_METHOD2(DiscSetDiscoverCallback, int32_t(DiscModule, const DiscInnerCallback *));
    MOCK_METHOD3(DiscStartAdvertise, int32_t(DiscModule, const SubscribeInfo *, int32_t));
    MOCK_METHOD3(DiscStopDiscovery, int32_t(const char *, int32_t, int32_t));
    MOCK_METHOD3(DiscStopAdvertise, int32_t(DiscModule, int32_t, int32_t));
};
} // namespace OHOS
#endif // DISC_MANAGER_MOCK_H
