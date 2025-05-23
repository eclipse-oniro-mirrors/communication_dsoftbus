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
#ifndef CLIENT_TRANS_STATISTICS_H
#define CLIENT_TRANS_STATISTICS_H

#include "trans_network_statistics.h"

#ifdef __cplusplus
extern "C" {
#endif

void AddSocketResource(const char *sessionName, const ChannelInfo *channel);

void UpdateChannelStatistics(int32_t socketId, int64_t len);

void DeleteSocketResourceByChannelId(int32_t channelId, int32_t channelType);

void DeleteSocketResourceBySocketId(int32_t socketId);

int32_t ClientTransStatisticsInit(void);

void ClientTransStatisticsDeinit(void);

#ifdef __cplusplus
}
#endif
#endif // CLIENT_TRANS_STATISTICS_H