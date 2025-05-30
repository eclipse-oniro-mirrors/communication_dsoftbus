/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef SOFTBUS_ADAPTER_MEM_H
#define SOFTBUS_ADAPTER_MEM_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_MALLOC_SIZE (512 * 1024 * 1024) // 512M

void *SoftBusMalloc(unsigned int size);
void *SoftBusCalloc(unsigned int size);
void SoftBusFree(void *pt);
void SoftBusClearFree(void *pt, unsigned int size);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif // SOFTBUS_ADAPTER_MEM_H