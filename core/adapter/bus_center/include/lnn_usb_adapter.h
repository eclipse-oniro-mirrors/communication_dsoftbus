/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef LNN_USB_ADAPTER_H
#define LNN_USB_ADAPTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NCM_DEVICE_NAME "ncm0"
#define NCM_HOST_NAME "wwan0"
#define NCM_DEVICE_MODE 0
#define NCM_HOST_MODE 1
#define NCM_TIME_OUT 10

int32_t StartUsbNcmAdapter(int32_t mode);

#ifdef __cplusplus
}
#endif
#endif // LNN_USB_ADAPTER_H