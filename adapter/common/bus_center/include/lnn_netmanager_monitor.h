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

#ifndef LNN_NETMANAGER_MONITOR_H
#define LNN_NETMANAGER_MONITOR_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

int32_t ConfigNetLinkUp(const char *ifName);
int32_t ConfigLocalIp(const char *ifName, const char *localIp);
int32_t ConfigRoute(const int32_t id, const char *ifName, const char *destination, const char *gateway);
int32_t ConfigLocalIpv6(const char *ifName, const char *localIpv6);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* LNN_NETMANAGER_MONITOR_H */