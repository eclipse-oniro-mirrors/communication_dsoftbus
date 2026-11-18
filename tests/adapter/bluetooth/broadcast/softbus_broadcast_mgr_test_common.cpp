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

#include "softbus_broadcast_mgr_test_common.h"

namespace OHOS {
// filter is released in UnRegisterScanListener
BcScanFilter *GetBcScanFilter(void)
{
    unsigned char serviceData[] = { 0x04, 0x05, 0x90 };
    unsigned char serviceDataMask[] = { 0xFF, 0xFF, 0xFF };
    int32_t serviceDataLength = sizeof(serviceData);

    BcScanFilter *filter = static_cast<BcScanFilter *>(SoftBusCalloc(sizeof(BcScanFilter)));
    DISC_CHECK_AND_RETURN_RET_LOGW(filter != nullptr, nullptr, DISC_TEST, "malloc filter failed");

    unsigned char *data = static_cast<unsigned char *>(SoftBusCalloc(serviceDataLength));
    unsigned char *mask = static_cast<unsigned char *>(SoftBusCalloc(serviceDataLength));
    if (data == nullptr || mask == nullptr) {
        goto EXIT;
    }
    if (memcpy_s(data, serviceDataLength, serviceData, serviceDataLength) != EOK) {
        goto EXIT;
    }
    if (memcpy_s(mask, serviceDataLength, serviceDataMask, serviceDataLength) != EOK) {
        goto EXIT;
    }

    filter->serviceId = SERVICE_UUID;
    filter->serviceData = data;
    filter->serviceDataMask = mask;
    filter->serviceDataLength = serviceDataLength;
    return filter;
EXIT:
    SoftBusFree(filter);
    SoftBusFree(data);
    SoftBusFree(mask);
    return nullptr;
}
} // namespace OHOS
