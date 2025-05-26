/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef CLIENT_TRANS_SESSION_MANAGER_H
#define CLIENT_TRANS_SESSION_MANAGER_H

#include "client_trans_session_adapter.h"
#include "client_trans_session_manager_struct.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t ClientAddNewSession(const char *sessionName, SessionInfo *session);

/**
 * @brief Add session.
 * @return if the operation is successful, return SOFTBUS_OK.
 * @return if session already added, return SOFTBUS_TRANS_SESSION_REPEATED.
 * @return return other error codes.
 */
int32_t ClientAddSession(const SessionParam *param, int32_t *sessionId, SessionEnableStatus *isEnabled);

int32_t ClientAddAuthSession(const char *sessionName, int32_t *sessionId);

int32_t ClientDeleteSessionServer(SoftBusSecType type, const char *sessionName);

int32_t ClientDeleteSession(int32_t sessionId);

int32_t ClientGetSessionDataById(int32_t sessionId, char *data, uint16_t len, TransSessionKey key);

int32_t ClientGetSessionIntegerDataById(int32_t sessionId, int *data, TransSessionKey key);

int32_t ClientGetChannelBySessionId(
    int32_t sessionId, int32_t *channelId, int32_t *type, SessionEnableStatus *enableStatus);

int32_t ClientSetChannelBySessionId(int32_t sessionId, TransInfo *transInfo);

int32_t ClientGetChannelBusinessTypeBySessionId(int32_t sessionId, int32_t *businessType);

int32_t GetEncryptByChannelId(int32_t channelId, int32_t channelType, int32_t *data);

int32_t GetSupportTlvAndNeedAckById(int32_t channelId, int32_t channelType, bool *supportTlv, bool *needAck);

int32_t ClientGetSessionStateByChannelId(int32_t channelId, int32_t channelType, SessionState *sessionState);

int32_t ClientGetSessionIdByChannelId(int32_t channelId, int32_t channelType, int32_t *sessionId, bool isClosing);

int32_t ClientGetSessionIsAsyncBySessionId(int32_t sessionId, bool *isAsync);

int32_t ClientGetRouteTypeByChannelId(int32_t channelId, int32_t channelType, int32_t *routeType);

int32_t ClientGetDataConfigByChannelId(int32_t channelId, int32_t channelType, uint32_t *dataConfig);

int32_t ClientEnableSessionByChannelId(const ChannelInfo *channel, int32_t *sessionId);

int32_t ClientGetSessionCallbackById(int32_t sessionId, ISessionListener *callback);

int32_t ClientGetSessionCallbackByName(const char *sessionName, ISessionListener *callback);

int32_t ClientAddSessionServer(SoftBusSecType type, const char *pkgName, const char *sessionName,
    const ISessionListener *listener);

int32_t ClientGetSessionSide(int32_t sessionId);

int32_t ClientGetFileConfigInfoById(int32_t sessionId, int32_t *fileEncrypt, int32_t *algorithm, int32_t *crc);

int TransClientInit(void);
void TransClientDeinit(void);

void ClientTransRegLnnOffline(void);

void ClientTransOnUserSwitch(void);

void ClientTransOnLinkDown(const char *networkId, int32_t routeType);

void ClientCleanAllSessionWhenServerDeath(ListNode *sessionServerInfoList);

int32_t CheckPermissionState(int32_t sessionId);

void PermissionStateChange(const char *pkgName, int32_t state);

int32_t ClientAddSocketServer(SoftBusSecType type, const char *pkgName, const char *sessionName);

int32_t ClientAddSocketSession(
    const SessionParam *param, bool isEncyptedRawStream, int32_t *sessionId, SessionEnableStatus *isEnabled);

int32_t ClientSetListenerBySessionId(int32_t sessionId, const ISocketListener *listener, bool isServer);

int32_t ClientIpcOpenSession(
    int32_t sessionId, const QosTV *qos, uint32_t qosCount, TransInfo *transInfo, bool isAsync);

int32_t ClientSetActionIdBySessionId(int32_t sessionId, uint32_t actionId);

int32_t ClientSetSocketState(int32_t socket, uint32_t maxIdleTimeout, SessionRole role);

int32_t ClientGetSessionCallbackAdapterByName(const char *sessionName, SessionListenerAdapter *callbackAdapter);

int32_t ClientGetSessionCallbackAdapterById(int32_t sessionId, SessionListenerAdapter *callbackAdapter, bool *isServer);

int32_t ClientGetPeerSocketInfoById(int32_t sessionId, PeerSocketInfo *peerSocketInfo);

bool IsSessionExceedLimit(void);

int32_t ClientResetIdleTimeoutById(int32_t sessionId);

int32_t ClientGetSessionNameByChannelId(int32_t channelId, int32_t channelType, char *sessionName, int32_t len);

int32_t ClientRawStreamEncryptDefOptGet(const char *sessionName, bool *isEncrypt);

int32_t ClientRawStreamEncryptOptGet(int32_t sessionId, int32_t channelId, int32_t channelType, bool *isEncrypt);

int32_t SetSessionIsAsyncById(int32_t sessionId, bool isAsync);

int32_t ClientTransSetChannelInfo(const char *sessionName, int32_t sessionId, int32_t channelId, int32_t channelType);

void DelSessionStateClosing(void);

void AddSessionStateClosing(void);

int32_t ClientHandleBindWaitTimer(int32_t socket, uint32_t maxWaitTime, TimerAction action);

inline bool IsValidQosInfo(const QosTV qos[], uint32_t qosCount)
{
    return (qos == NULL) ? (qosCount == 0) : (qosCount <= QOS_TYPE_BUTT);
}

int32_t SetSessionInitInfoById(int32_t sessionId);

int32_t ClientSetEnableStatusBySocket(int32_t socket, SessionEnableStatus enableStatus);

int32_t TryDeleteEmptySessionServer(const char *pkgName, const char *sessionName);

int32_t DeleteSocketSession(int32_t sessionId, char *pkgName, char *sessionName);

int32_t SetSessionStateBySessionId(int32_t sessionId, SessionState sessionState, int32_t optional);

int32_t GetSocketLifecycleAndSessionNameBySessionId(
    int32_t sessionId, char *sessionName, SocketLifecycleData *lifecycle);

int32_t ClientWaitSyncBind(int32_t socket);

int32_t ClientSignalSyncBind(int32_t socket, int32_t errCode);

int32_t ClientDfsIpcOpenSession(int32_t sessionId, TransInfo *transInfo);

void SocketServerStateUpdate(const char *sessionName);

int32_t ClientCancelAuthSessionTimer(int32_t sessionId);

int32_t ClientSetStatusClosingBySocket(int32_t socket, bool isClosing);

int32_t ClientGetChannelOsTypeBySessionId(int32_t sessionId, int32_t *osType);

int32_t ClientCacheQosEvent(int32_t socket, QoSEvent event, const QosTV *qos, uint32_t count);

int32_t ClientGetCachedQosEventBySocket(int32_t socket, CachedQosEvent *cachedQosEvent);

int32_t GetMaxIdleTimeBySocket(int32_t socket, uint32_t *maxIdleTime);

int32_t SetMaxIdleTimeBySocket(int32_t socket, uint32_t maxIdleTime);

void ClientTransOnPrivilegeClose(const char *peerNetworkId);

int32_t TransGetSupportTlvBySocket(int32_t socket, bool *supportTlv, int32_t *optValueSize);

int32_t TransSetNeedAckBySocket(int32_t socket, bool needAck);

bool IsRawAuthSession(const char *sessionName);

int32_t ClientGetSessionNameBySessionId(int32_t sessionId, char *sessionName);

int32_t GetIsAsyncAndTokenTypeBySessionId(int32_t sessionId, bool *isAsync, int32_t *tokenType);

#ifdef __cplusplus
}
#endif
#endif // CLIENT_TRANS_SESSION_MANAGER_H
