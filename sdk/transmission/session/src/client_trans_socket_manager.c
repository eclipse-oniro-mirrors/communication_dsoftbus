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

#include "client_trans_socket_manager.h"

#include <securec.h>

#include "anonymizer.h"
#include "client_bus_center_manager.h"
#include "client_trans_channel_manager.h"
#include "client_trans_file_listener.h"
#include "client_trans_proxy_file_manager.h"
#include "client_trans_tcp_direct_manager.h"
#include "client_trans_udp_manager.h"
#include "softbus_adapter_mem.h"
#include "softbus_app_info.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_socket.h"
#include "softbus_utils.h"
#include "trans_log.h"
#include "trans_server_proxy.h"

#define NETWORK_ID_LEN 7
#define HML_IP_PREFIX "172.30."
#define GET_ROUTE_TYPE(type) ((type) & 0xff)
#define GET_CONN_TYPE(type) (((type) >> 8) & 0xff)

#define DISTRIBUTED_DATA_SESSION "distributeddata-default"

bool IsValidSessionParam(const SessionParam *param)
{
    if ((param == NULL) ||
        (param->sessionName == NULL) ||
        (param->peerSessionName == NULL) ||
        (param->peerDeviceId == NULL) ||
        (param->groupId == NULL) ||
        (param->attr == NULL)) {
        return false;
    }
    return true;
}

SessionInfo *CreateNewSession(const SessionParam *param)
{
    if (param == NULL) {
        TRANS_LOGE(TRANS_SDK, "param is null");
        return NULL;
    }
    SessionInfo *session = (SessionInfo*)SoftBusCalloc(sizeof(SessionInfo));
    if (session == NULL) {
        TRANS_LOGE(TRANS_SDK, "calloc failed");
        return NULL;
    }

    if (strcpy_s(session->info.peerSessionName, SESSION_NAME_SIZE_MAX, param->peerSessionName) != EOK ||
        strcpy_s(session->info.peerDeviceId, DEVICE_ID_SIZE_MAX, param->peerDeviceId) != EOK ||
        strcpy_s(session->info.groupId, GROUP_ID_SIZE_MAX, param->groupId) != EOK ||
        memcpy_s(session->linkType, sizeof(param->attr->linkType), param->attr->linkType,
            sizeof(param->attr->linkType)) != EOK) {
        TRANS_LOGE(TRANS_SDK, "strcpy failed");
        SoftBusFree(session);
        return NULL;
    }

    session->sessionId = INVALID_SESSION_ID;
    session->channelId = INVALID_CHANNEL_ID;
    session->channelType = CHANNEL_TYPE_BUTT;
    session->isServer = false;
    session->role = SESSION_ROLE_INIT;
    session->enableStatus = ENABLE_STATUS_INIT;
    session->info.flag = param->attr->dataType;
    session->isEncrypt = true;
    session->isAsync = false;
    session->lifecycle.sessionState = SESSION_STATE_INIT;
    return session;
}

NO_SANITIZE("cfi") DestroySessionInfo *CreateDestroySessionNode(SessionInfo *sessionNode,
    const ClientSessionServer *server)
{
    if (sessionNode == NULL || server == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return NULL;
    }
    DestroySessionInfo *destroyNode = (DestroySessionInfo *)SoftBusCalloc(sizeof(DestroySessionInfo));
    if (destroyNode == NULL) {
        TRANS_LOGE(TRANS_SDK, "destroyList malloc fail.");
        return NULL;
    }
    destroyNode->sessionId = sessionNode->sessionId;
    destroyNode->channelId = sessionNode->channelId;
    destroyNode->channelType = sessionNode->channelType;
    if (memcpy_s(destroyNode->sessionName, SESSION_NAME_SIZE_MAX, server->sessionName, SESSION_NAME_SIZE_MAX) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy_s sessionName fail.");
        SoftBusFree(destroyNode);
        return NULL;
    }
    if (memcpy_s(destroyNode->pkgName, PKG_NAME_SIZE_MAX, server->pkgName, PKG_NAME_SIZE_MAX) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy_s pkgName fail.");
        SoftBusFree(destroyNode);
        return NULL;
    }
    destroyNode->OnSessionClosed = server->listener.session.OnSessionClosed;
    destroyNode->OnShutdown = sessionNode->isServer ? server->listener.socketServer.OnShutdown :
        server->listener.socketClient.OnShutdown;
    return destroyNode;
}

NO_SANITIZE("cfi") void ClientDestroySession(const ListNode *destroyList, ShutdownReason reason)
{
    if (destroyList == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    if (IsListEmpty(destroyList)) {
        TRANS_LOGD(TRANS_SDK, "destroyList is empty fail.");
        return;
    }
    DestroySessionInfo *destroyNode = NULL;
    DestroySessionInfo *destroyNodeNext = NULL;
    TRANS_LOGD(TRANS_SDK, "enter.");
    LIST_FOR_EACH_ENTRY_SAFE(destroyNode, destroyNodeNext, destroyList, DestroySessionInfo, node) {
        int32_t id = destroyNode->sessionId;
        (void)ClientDeleteRecvFileList(id);
        (void)ClientTransCloseChannel(destroyNode->channelId, destroyNode->channelType);
        if (destroyNode->OnSessionClosed != NULL) {
            destroyNode->OnSessionClosed(id);
        } else if (destroyNode->OnShutdown != NULL) {
            destroyNode->OnShutdown(id, reason);
            (void)TryDeleteEmptySessionServer(destroyNode->pkgName, destroyNode->sessionName);
        }
        ListDelete(&(destroyNode->node));
        SoftBusFree(destroyNode);
    }
    TRANS_LOGD(TRANS_SDK, "ok");
}

void DestroyClientSessionServer(ClientSessionServer *server, ListNode *destroyList)
{
    if (server == NULL || destroyList == NULL) {
        TRANS_LOGW(TRANS_SDK, "invalid param");
        return;
    }

    if (!IsListEmpty(&(server->sessionList))) {
        SessionInfo *sessionNode = NULL;
        SessionInfo *sessionNodeNext = NULL;
        LIST_FOR_EACH_ENTRY_SAFE(sessionNode, sessionNodeNext, &(server->sessionList), SessionInfo, node) {
            DestroySessionInfo *destroyNode = CreateDestroySessionNode(sessionNode, server);
            if (destroyNode == NULL) {
                continue;
            }
            DestroySessionId();
            ListDelete(&sessionNode->node);
            ListAdd(destroyList, &(destroyNode->node));
            SoftBusFree(sessionNode);
        }
    }

    ListDelete(&(server->node));
    char *tmpName = NULL;
    Anonymize(server->sessionName, &tmpName);
    TRANS_LOGI(TRANS_SDK, "destroy session server sessionName=%{public}s", tmpName);
    AnonymizeFree(tmpName);
    SoftBusFree(server);
}

ClientSessionServer *GetNewSessionServer(SoftBusSecType type, const char *sessionName,
    const char *pkgName, const ISessionListener *listener)
{
    if (sessionName == NULL || pkgName == NULL || listener == NULL) {
        TRANS_LOGW(TRANS_SDK, "invalid param");
        return NULL;
    }
    ClientSessionServer *server = (ClientSessionServer *)SoftBusCalloc(sizeof(ClientSessionServer));
    if (server == NULL) {
        return NULL;
    }
    server->type = type;
    if (strcpy_s(server->pkgName, sizeof(server->pkgName), pkgName) != EOK) {
        goto EXIT_ERR;
    }
    if (strcpy_s(server->sessionName, sizeof(server->sessionName), sessionName) != EOK) {
        goto EXIT_ERR;
    }
    if (memcpy_s(&server->listener.session, sizeof(ISessionListener), listener, sizeof(ISessionListener)) != EOK) {
        goto EXIT_ERR;
    }
    server->listener.isSocketListener = false;
    server->isSrvEncryptedRawStream = false;

    ListInit(&server->node);
    ListInit(&server->sessionList);
    return server;

EXIT_ERR:
    if (server != NULL) {
        SoftBusFree(server);
    }
    return NULL;
}

SessionInfo *CreateNonEncryptSessionInfo(const char *sessionName)
{
    if (sessionName == NULL) {
        TRANS_LOGW(TRANS_SDK, "Invalid param");
        return NULL;
    }
    if (!IsValidString(sessionName, SESSION_NAME_SIZE_MAX - 1)) {
        TRANS_LOGW(TRANS_SDK, "Invalid param");
        return NULL;
    }
    SessionInfo *session = (SessionInfo *)SoftBusCalloc(sizeof(SessionInfo));
    if (session == NULL) {
        return NULL;
    }
    session->channelType = CHANNEL_TYPE_AUTH;
    session->isEncrypt = false;
    if (strcpy_s(session->info.peerSessionName, SESSION_NAME_SIZE_MAX, sessionName) != EOK) {
        SoftBusFree(session);
        return NULL;
    }
    return session;
}

static int32_t ClientTransGetTdcIp(int32_t channelId, char *myIp, int32_t ipLen)
{
    TcpDirectChannelInfo channel;
    if (TransTdcGetInfoById(channelId, &channel) == NULL) {
        TRANS_LOGE(TRANS_SDK, "not found Tdc channel by channelId=%{public}d", channelId);
        return SOFTBUS_TRANS_TDC_CHANNEL_NOT_FOUND;
    }

    if (strcpy_s(myIp, ipLen, channel.detail.myIp) != EOK) {
        TRANS_LOGE(TRANS_SDK, "strcpy_s ip failed, len=%{public}zu", strlen(channel.detail.myIp));
        return SOFTBUS_STRCPY_ERR;
    }

    return SOFTBUS_OK;
}

static int32_t ClientTransGetUdpIp(int32_t channelId, char *myIp, int32_t ipLen)
{
    UdpChannel channel;
    if (TransGetUdpChannel(channelId, &channel) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "not found Udp channel by channelId=%{public}d", channelId);
        return SOFTBUS_TRANS_UDP_CHANNEL_NOT_FOUND;
    }

    if (strcpy_s(myIp, ipLen, channel.info.myIp) != EOK) {
        TRANS_LOGE(TRANS_SDK, "strcpy_s ip failed, len=%{public}zu", strlen(channel.info.myIp));
        return SOFTBUS_STRCPY_ERR;
    }

    return SOFTBUS_OK;
}

// determine connection type based on IP
static bool ClientTransCheckHmlIp(const char *ip)
{
    if (strncmp(ip, HML_IP_PREFIX, NETWORK_ID_LEN) == 0) {
        return true;
    }

    return false;
}

// determine connection type based on IP, delete session when connection type and parameter connType are consistent
static bool ClientTransCheckNeedDel(SessionInfo *sessionNode, int32_t routeType, int32_t connType)
{
    if (connType == TRANS_CONN_ALL) {
        if (routeType != ROUTE_TYPE_ALL && sessionNode->routeType != routeType) {
            return false;
        }
        return true;
    }
    /*
    * only when the function OnWifiDirectDeviceOffLine is called can reach this else branch,
    * and routeType is WIFI_P2P, the connType is hml or p2p
    */
    if (sessionNode->routeType != routeType) {
        return false;
    }

    char myIp[IP_LEN] = {0};
    if (sessionNode->channelType == CHANNEL_TYPE_UDP) {
        if (ClientTransGetUdpIp(sessionNode->channelId, myIp, sizeof(myIp)) != SOFTBUS_OK) {
            return false;
        }
    } else if (sessionNode->channelType == CHANNEL_TYPE_TCP_DIRECT) {
        if (ClientTransGetTdcIp(sessionNode->channelId, myIp, sizeof(myIp)) != SOFTBUS_OK) {
            return false;
        }
    } else {
        TRANS_LOGW(TRANS_SDK, "check channelType=%{public}d", sessionNode->channelType);
        return false;
    }

    bool isHml = ClientTransCheckHmlIp(myIp);
    if (connType == TRANS_CONN_HML && isHml) {
        return true;
    } else if (connType == TRANS_CONN_P2P && !isHml) {
        return true;
    }

    return false;
}

void DestroyClientSessionByNetworkId(const ClientSessionServer *server,
    const char *networkId, int32_t type, ListNode *destroyList)
{
    if (server == NULL || networkId == NULL || destroyList == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    SessionInfo *sessionNode = NULL;
    SessionInfo *sessionNodeNext = NULL;
    // connType is set only in function OnWifiDirectDeviceOffLine, others is TRANS_CONN_ALL, and routeType is WIFI_P2P
    int32_t routeType = GET_ROUTE_TYPE(type);
    int32_t connType = GET_CONN_TYPE(type);

    LIST_FOR_EACH_ENTRY_SAFE(sessionNode, sessionNodeNext, &(server->sessionList), SessionInfo, node) {
        if (strcmp(sessionNode->info.peerDeviceId, networkId) != 0) {
            continue;
        }

        if (!ClientTransCheckNeedDel(sessionNode, routeType, connType)) {
            continue;
        }

        TRANS_LOGI(TRANS_SDK, "channelId=%{public}d, channelType=%{public}d, routeType=%{public}d, type=%{public}d",
            sessionNode->channelId, sessionNode->channelType, sessionNode->routeType, type);
        DestroySessionInfo *destroyNode = CreateDestroySessionNode(sessionNode, server);
        if (destroyNode == NULL) {
            continue;
        }
        /*
         * When the channel type is UDP and the business type is file, trigger DFILE_ON_CLEAR_POLICY_FILE_LIST event
         * before cleaning up sessionNode.
         */
        if (sessionNode->channelType == CHANNEL_TYPE_UDP && sessionNode->businessType == BUSINESS_TYPE_FILE) {
            ClientEmitFileEvent(sessionNode->channelId);
        }
        DestroySessionId();
        ListDelete(&sessionNode->node);
        ListAdd(destroyList, &(destroyNode->node));
        SoftBusFree(sessionNode);
    }
}

SessionServerInfo *CreateSessionServerInfoNode(const ClientSessionServer *clientSessionServer)
{
    if (clientSessionServer == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return NULL;
    }

    SessionServerInfo *infoNode = (SessionServerInfo *)SoftBusCalloc(sizeof(SessionServerInfo));
    if (infoNode == NULL) {
        TRANS_LOGE(TRANS_SDK, "failed to malloc SessionServerInfo.");
        return NULL;
    }

    if (strcpy_s(infoNode->pkgName, SESSION_NAME_SIZE_MAX, clientSessionServer->pkgName) != EOK) {
        SoftBusFree(infoNode);
        TRANS_LOGE(TRANS_SDK, "failed to strcpy pkgName.");
        return NULL;
    }

    if (strcpy_s(infoNode->sessionName, SESSION_NAME_SIZE_MAX, clientSessionServer->sessionName) != EOK) {
        SoftBusFree(infoNode);
        TRANS_LOGE(TRANS_SDK, "failed to strcpy sessionName.");
        return NULL;
    }

    return infoNode;
}

ClientSessionServer *GetNewSocketServer(SoftBusSecType type, const char *sessionName, const char *pkgName)
{
    if (sessionName == NULL || pkgName == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return NULL;
    }
    ClientSessionServer *server = (ClientSessionServer *)SoftBusCalloc(sizeof(ClientSessionServer));
    if (server == NULL) {
        return NULL;
    }
    server->type = type;
    if (strcpy_s(server->pkgName, sizeof(server->pkgName), pkgName) != EOK) {
        goto EXIT_ERR;
    }
    if (strcpy_s(server->sessionName, sizeof(server->sessionName), sessionName) != EOK) {
        goto EXIT_ERR;
    }
    server->isSrvEncryptedRawStream = false;
    ListInit(&server->node);
    ListInit(&server->sessionList);
    return server;

EXIT_ERR:
    if (server != NULL) {
        SoftBusFree(server);
    }
    return NULL;
}

bool IsDistributedDataSession(const char *sessionName)
{
    if (sessionName == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return false;
    }
    uint32_t distributedDataSessionLen = strlen(DISTRIBUTED_DATA_SESSION);
    if (strlen(sessionName) < distributedDataSessionLen ||
        strncmp(sessionName, DISTRIBUTED_DATA_SESSION, distributedDataSessionLen) != 0) {
        return false;
    }
    return true;
}

bool IsDifferentDataType(const SessionInfo *sessionInfo, int dataType, bool isEncyptedRawStream)
{
    if (sessionInfo == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return false;
    }
    if (sessionInfo->info.flag != dataType) {
        return true;
    }

    if (dataType != RAW_STREAM) {
        return false;
    }

    return sessionInfo->isEncyptedRawStream != isEncyptedRawStream;
}

static void ClientInitSession(SessionInfo *session, const SessionParam *param)
{
    session->sessionId = INVALID_SESSION_ID;
    session->channelId = INVALID_CHANNEL_ID;
    session->channelType = CHANNEL_TYPE_BUTT;
    session->isServer = false;
    session->role = SESSION_ROLE_INIT;
    session->enableStatus = ENABLE_STATUS_INIT;
    session->info.flag = param->attr->dataType;
    session->info.streamType = param->attr->attr.streamAttr.streamType;
    session->isEncrypt = true;
    session->isAsync = false;
    session->lifecycle.sessionState = SESSION_STATE_INIT;
}

SessionInfo *CreateNewSocketSession(const SessionParam *param)
{
    if (param == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return NULL;
    }
    SessionInfo *session = (SessionInfo *)SoftBusCalloc(sizeof(SessionInfo));
    if (session == NULL) {
        TRANS_LOGE(TRANS_SDK, "calloc failed");
        return NULL;
    }

    if (param->peerSessionName != NULL &&
        strcpy_s(session->info.peerSessionName, SESSION_NAME_SIZE_MAX, param->peerSessionName) != EOK) {
        char *anonySessionName = NULL;
        Anonymize(param->peerSessionName, &anonySessionName);
        TRANS_LOGI(TRANS_SDK, "strcpy peerName failed, peerName=%{public}s, peerNameLen=%{public}zu",
            anonySessionName, strlen(param->peerSessionName));
        AnonymizeFree(anonySessionName);
        SoftBusFree(session);
        return NULL;
    }

    if (param->peerDeviceId != NULL &&
        strcpy_s(session->info.peerDeviceId, DEVICE_ID_SIZE_MAX, param->peerDeviceId) != EOK) {
        char *anonyNetworkId = NULL;
        Anonymize(param->peerDeviceId, &anonyNetworkId);
        TRANS_LOGI(TRANS_SDK, "strcpy peerDeviceId failed, peerDeviceId=%{public}s, peerDeviceIdLen=%{public}zu",
            anonyNetworkId, strlen(param->peerDeviceId));
        AnonymizeFree(anonyNetworkId);
        SoftBusFree(session);
        return NULL;
    }

    if (strcpy_s(session->info.groupId, GROUP_ID_SIZE_MAX, param->groupId) != EOK ||
        memcpy_s(session->linkType, sizeof(param->attr->linkType), param->attr->linkType,
            sizeof(param->attr->linkType)) != EOK) {
        TRANS_LOGE(TRANS_SDK, "strcpy failed");
        SoftBusFree(session);
        return NULL;
    }

    if (SoftBusCondInit(&session->lifecycle.callbackCond) != SOFTBUS_OK) {
        SoftBusFree(session);
        TRANS_LOGE(TRANS_SDK, "callbackCond Init failed");
        return NULL;
    }

    ClientInitSession(session, param);
    return session;
}

int32_t CheckBindSocketInfo(const SessionInfo *session)
{
    if (session == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (!IsValidString(session->info.peerSessionName, SESSION_NAME_SIZE_MAX - 1) ||
        !IsValidString(session->info.peerDeviceId, DEVICE_ID_SIZE_MAX - 1)) {
        char *anonySessionName = NULL;
        char *anonyNetworkId = NULL;
        Anonymize(session->info.peerSessionName, &anonySessionName);
        Anonymize(session->info.peerDeviceId, &anonyNetworkId);
        TRANS_LOGI(TRANS_SDK, "invalid peerName=%{public}s, peerNameLen=%{public}zu, peerNetworkId=%{public}s, "
                              "peerNetworkIdLen=%{public}zu", anonySessionName,
            strlen(session->info.peerSessionName), anonyNetworkId, strlen(session->info.peerDeviceId));
        AnonymizeFree(anonyNetworkId);
        AnonymizeFree(anonySessionName);
        return SOFTBUS_INVALID_PARAM;
    }

    if (session->info.flag < TYPE_MESSAGE || session->info.flag >= TYPE_BUTT) {
        TRANS_LOGE(TRANS_SDK, "invalid dataType");
        return SOFTBUS_INVALID_PARAM;
    }

    return SOFTBUS_OK;
}

void FillSessionParam(SessionParam *param, SessionAttribute *tmpAttr,
    ClientSessionServer *serverNode, SessionInfo *sessionNode)
{
    if (param == NULL || tmpAttr == NULL || serverNode == NULL || sessionNode == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    tmpAttr->fastTransData = NULL;
    tmpAttr->fastTransDataSize = 0;
    tmpAttr->dataType = sessionNode->info.flag;
    tmpAttr->attr.streamAttr.streamType = sessionNode->info.streamType;
    tmpAttr->linkTypeNum = 0;
    param->sessionName = serverNode->sessionName;
    param->peerSessionName = sessionNode->info.peerSessionName;
    param->peerDeviceId = sessionNode->info.peerDeviceId;
    param->groupId = "reserved";
    param->attr = tmpAttr;
    param->isQosLane = true;
}

void ClientConvertRetVal(int32_t socket, int32_t *retOut)
{
    if (retOut == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    SocketLifecycleData lifecycle;
    (void)memset_s(&lifecycle, sizeof(SocketLifecycleData), 0, sizeof(SocketLifecycleData));
    int32_t ret = GetSocketLifecycleAndSessionNameBySessionId(socket, NULL, &lifecycle);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "get info fail, socket=%{public}d", socket);
        return;
    }

    if (lifecycle.bindErrCode == SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "bindErrCode is SOFTBUS_OK, socket=%{public}d", socket);
        return;
    }
    *retOut = lifecycle.bindErrCode;
}

void ClientCleanUpIdleTimeoutSocket(const ListNode *destroyList)
{
    if (destroyList == NULL || IsListEmpty(destroyList)) {
        TRANS_LOGD(TRANS_SDK, "destroyList is empty.");
        return;
    }
    DestroySessionInfo *destroyNode = NULL;
    DestroySessionInfo *destroyNodeNext = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(destroyNode, destroyNodeNext, destroyList, DestroySessionInfo, node) {
        int32_t id = destroyNode->sessionId;
        (void)ClientDeleteRecvFileList(id);
        (void)ClientTransCloseChannel(destroyNode->channelId, destroyNode->channelType);
        TRANS_LOGI(TRANS_SDK, "session is idle, sessionId=%{public}d", id);
        if (destroyNode->OnShutdown != NULL) {
            destroyNode->OnShutdown(id, SHUTDOWN_REASON_TIMEOUT);
            (void)TryDeleteEmptySessionServer(destroyNode->pkgName, destroyNode->sessionName);
        }
        ListDelete(&(destroyNode->node));
        SoftBusFree(destroyNode);
    }
    TRANS_LOGD(TRANS_SDK, "ok");
}

void ClientCheckWaitTimeOut(SessionInfo *sessionNode, int32_t waitOutSocket[], uint32_t capacity, uint32_t *num)
{
    if (sessionNode == NULL || waitOutSocket == NULL || num == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    if (sessionNode->enableStatus == ENABLE_STATUS_SUCCESS) {
        return;
    }

    sessionNode->lifecycle.waitTime += TIMER_TIMEOUT;
    if (sessionNode->lifecycle.maxWaitTime == 0 ||
        sessionNode->lifecycle.waitTime <= sessionNode->lifecycle.maxWaitTime) {
        TRANS_LOGD(TRANS_SDK, "no wait timeout, socket=%{public}d", sessionNode->sessionId);
        return;
    }

    TRANS_LOGW(TRANS_SDK, "bind time out socket=%{public}d", sessionNode->sessionId);
    // stop check time out
    sessionNode->lifecycle.maxWaitTime = 0;

    uint32_t tmpNum = *num;
    if (tmpNum + 1 > capacity) {
        TRANS_LOGE(TRANS_SDK, "socket num invalid tmpNum=%{public}u, capacity=%{public}u", tmpNum, capacity);
        return;
    }
    waitOutSocket[tmpNum] = sessionNode->sessionId;
    *num = tmpNum + 1;
}

void ClientCleanUpWaitTimeoutSocket(int32_t waitOutSocket[], uint32_t waitOutNum)
{
    if (waitOutSocket == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    bool tmpIsServer = false;
    SessionListenerAdapter callback = { 0 };
    for (uint32_t i = 0; i < waitOutNum; ++i) {
        TRANS_LOGI(TRANS_SDK, "time out shutdown socket=%{public}d", waitOutSocket[i]);
        ClientGetSessionCallbackAdapterById(waitOutSocket[i], &callback, &tmpIsServer);
        if (callback.socketClient.OnError != NULL) {
            (void)callback.socketClient.OnError(waitOutSocket[i], SOFTBUS_TRANS_STOP_BIND_BY_TIMEOUT);
        }
        ClientShutdown(waitOutSocket[i], SOFTBUS_TRANS_STOP_BIND_BY_TIMEOUT);
    }
}

void ClientUpdateIdleTimeout(const ClientSessionServer *serverNode, SessionInfo *sessionNode, ListNode *destroyList)
{
    if (serverNode == NULL || sessionNode == NULL || destroyList == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    if (sessionNode->role != SESSION_ROLE_CLIENT || sessionNode->enableStatus != ENABLE_STATUS_SUCCESS) {
        return;
    }

    sessionNode->timeout += TIMER_TIMEOUT;
    if (sessionNode->maxIdleTime == 0 || sessionNode->timeout <= sessionNode->maxIdleTime) {
        return;
    }

    DestroySessionInfo *destroyNode = CreateDestroySessionNode(sessionNode, serverNode);
    if (destroyNode == NULL) {
        TRANS_LOGE(TRANS_SDK, "failed to create destory session Node, sessionId=%{public}d", sessionNode->sessionId);
        return;
    }
    ListAdd(destroyList, &(destroyNode->node));
    DestroySessionId();
    ListDelete(&sessionNode->node);
    SoftBusFree(sessionNode);
}

int32_t ReCreateSessionServerToServer(ListNode *sessionServerInfoList)
{
    TRANS_LOGD(TRANS_SDK, "enter.");
    if (sessionServerInfoList == NULL) {
        TRANS_LOGE(TRANS_INIT, "session server list not init");
        return SOFTBUS_INVALID_PARAM;
    }

    SessionServerInfo *infoNode = NULL;
    SessionServerInfo *infoNodeNext = NULL;
    char *tmpName = NULL;
    LIST_FOR_EACH_ENTRY_SAFE(infoNode, infoNodeNext, sessionServerInfoList, SessionServerInfo, node) {
        int32_t ret = ServerIpcCreateSessionServer(infoNode->pkgName, infoNode->sessionName);
        Anonymize(infoNode->sessionName, &tmpName);
        TRANS_LOGI(TRANS_SDK, "sessionName=%{public}s, pkgName=%{public}s, ret=%{public}d",
            tmpName, infoNode->pkgName, ret);
        AnonymizeFree(tmpName);
        ListDelete(&infoNode->node);
        SoftBusFree(infoNode);
    }

    TRANS_LOGI(TRANS_SDK, "ok");
    return SOFTBUS_OK;
}

void FillDfsSocketParam(
    SessionParam *param, SessionAttribute *tmpAttr, ClientSessionServer *serverNode, SessionInfo *sessionNode)
{
    if (param == NULL || tmpAttr == NULL || serverNode == NULL || sessionNode == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return;
    }
    tmpAttr->fastTransData = NULL;
    tmpAttr->fastTransDataSize = 0;
    tmpAttr->dataType = sessionNode->info.flag;
    tmpAttr->attr.streamAttr.streamType = sessionNode->info.streamType;
    // 2 means has two linkType
    tmpAttr->linkTypeNum = 2;
    tmpAttr->linkType[0] = LINK_TYPE_WIFI_WLAN_5G;
    tmpAttr->linkType[1] = LINK_TYPE_WIFI_WLAN_2G;
    param->sessionName = serverNode->sessionName;
    param->peerSessionName = sessionNode->info.peerSessionName;
    param->peerDeviceId = sessionNode->info.peerDeviceId;
    param->groupId = "reserved";
    param->attr = tmpAttr;
    param->isQosLane = false;
    param->qosCount = 0;
    (void)memset_s(param->qos, sizeof(param->qos), 0, sizeof(param->qos));
    param->isAsync = false;
}

int32_t GetQosValue(const QosTV *qos, uint32_t qosCount, QosType type, int32_t *value, int32_t defVal)
{
    if (!IsValidQosInfo(qos, qosCount) || value == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    if (qos == NULL || qosCount == 0) {
        TRANS_LOGW(TRANS_SDK, "no qos info, use defVal");
        *value = defVal;
        return SOFTBUS_OK;
    }

    for (uint32_t i = 0; i < qosCount; i++) {
        if (qos[i].qos != type) {
            continue;
        }
        *value = qos[i].value;
        return SOFTBUS_OK;
    }
    *value = defVal;
    return SOFTBUS_OK;
}

int32_t ClientGrantPermission(int uid, int pid, const char *busName)
{
    if (uid < 0 || pid < 0 || busName == NULL) {
        TRANS_LOGW(TRANS_SDK, "invalid parameter");
        return SOFTBUS_INVALID_PARAM;
    }
    char *tmpName = NULL;
    Anonymize(busName, &tmpName);
    TRANS_LOGI(TRANS_SDK, "sessionName=%{public}s", tmpName);
    AnonymizeFree(tmpName);
    int32_t ret = ServerIpcGrantPermission(uid, pid, busName);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "server grant permission failed, ret=%{public}d", ret);
    }
    return ret;
}

int32_t ClientRemovePermission(const char *busName)
{
    if (busName == NULL) {
        TRANS_LOGW(TRANS_SDK, "invalid parameter");
        return SOFTBUS_INVALID_PARAM;
    }
    char *tmpName = NULL;
    Anonymize(busName, &tmpName);
    TRANS_LOGI(TRANS_SDK, "sessionName=%{public}s", tmpName);
    AnonymizeFree(tmpName);
    int32_t ret = ServerIpcRemovePermission(busName);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "server remove permission failed, ret=%{public}d", ret);
    }
    return ret;
}

int32_t ClientDeleteSocketSession(int32_t sessionId)
{
    if (sessionId <= 0) {
        TRANS_LOGE(TRANS_SDK, "Invalid sessionId=%{public}d", sessionId);
        return SOFTBUS_INVALID_PARAM;
    }

    char pkgName[PKG_NAME_SIZE_MAX] = { 0 };
    char sessionName[SESSION_NAME_SIZE_MAX] = { 0 };
    int32_t ret = DeleteSocketSession(sessionId, pkgName, sessionName);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "failed delete session");
        return ret;
    }

    ret = TryDeleteEmptySessionServer(pkgName, sessionName);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "delete empty session server failed, ret=%{public}d", ret);
        return ret;
    }
    return SOFTBUS_OK;
}