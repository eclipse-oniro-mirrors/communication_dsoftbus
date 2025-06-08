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
#ifndef BR_PROXY_H
#define BR_PROXY_H

#include "trans_log.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BR_MAC_LEN          18
#define UUID_LEN            38
#define ERR_DESC_STR_LEN    128

typedef enum {
    CHANNEL_WAIT_RESUME = 0,
    CHANNEL_RESUME,
    CHANNEL_EXCEPTION_SOFTWARE_FAILED,
} ChannelState;

typedef enum {
    LINK_BR = 0,
} TSLinkType;

// 对应 TypeScript 中的 ChannelInfo 接口
typedef struct {
    TSLinkType linktype;
    char peerBRMacAddr[BR_MAC_LEN];
    char peerBRUuid[UUID_LEN];
    int32_t recvPri;
    bool recvPriSet;  // 用于标记 recvPri 是否被设置
} BrProxyChannelInfo;

typedef enum {
    DATA_RECEIVE,
    CHANNEL_STATE,
    LISTENER_TYPE_MAX,
} ListenerType;

#define COMM_PKGNAME_WECHAT "WeChatPkgName"
#define PKGNAME_MAX_LEN  30
#define DEFAULT_CHANNEL_ID (-1)

typedef struct {
    int32_t (*onChannelOpened)(int32_t channelId, int32_t result);
    void (*onDataReceived)(int32_t channelId, const char *data, uint32_t dataLen);
    void (*onChannelStatusChanged)(int32_t channelId, int32_t state);
} IBrProxyListener;

int32_t OpenBrProxy(BrProxyChannelInfo *channelInfo, IBrProxyListener *listener);
int32_t CloseBrProxy(int32_t channelId);
int32_t SendBrProxyData(int32_t channelId, char* data, uint32_t dataLen);
int32_t SetListenerState(int32_t channelId, ListenerType type, bool isEnable);
bool IsProxyChannelEnabled(int32_t uid);

int32_t ClientTransOnBrProxyOpened(int32_t channelId, const char *brMac, int32_t result);
int32_t ClientTransBrProxyDataReceived(int32_t channelId, const uint8_t *data, uint32_t len);
int32_t ClientTransBrProxyChannelChange(int32_t channelId, int32_t errCode);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // BR_PROXY_H