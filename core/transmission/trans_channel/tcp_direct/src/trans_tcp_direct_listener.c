/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "trans_tcp_direct_listener.h"

#include <securec.h>

#include "auth_interface.h"
#include "bus_center_manager.h"
#include "lnn_distributed_net_ledger.h"
#include "lnn_ohos_account_adapter.h"
#include "softbus_adapter_crypto.h"
#include "legacy/softbus_adapter_hitrace.h"
#include "softbus_access_token_adapter.h"
#include "softbus_adapter_mem.h"
#include "softbus_base_listener.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "softbus_message_open_channel.h"
#include "softbus_socket.h"
#include "softbus_utils.h"
#include "trans_channel_common.h"
#include "trans_channel_manager.h"
#include "trans_event.h"
#include "trans_log.h"
#include "trans_session_account_adapter.h"
#include "trans_tcp_direct_message.h"
#include "trans_tcp_direct_sessionconn.h"
#include "trans_uk_manager.h"

#define ID_OFFSET (1)
#define OHOS_TYPE_UNKNOWN (-1)
#define OH_OS_TYPE 10
#define HO_OS_TYPE 11

uint32_t SwitchAuthLinkTypeToFlagType(AuthLinkType type)
{
    switch (type) {
        case AUTH_LINK_TYPE_BR:
            return FLAG_BR;
        case AUTH_LINK_TYPE_BLE:
            return FLAG_BLE;
        case AUTH_LINK_TYPE_P2P:
            return FLAG_P2P;
        case AUTH_LINK_TYPE_ENHANCED_P2P:
            return FLAG_ENHANCE_P2P;
        case AUTH_LINK_TYPE_SESSION_KEY:
            return FLAG_SESSION_KEY;
        default:
            return FLAG_WIFI;
    }
}

int32_t GetCipherFlagByAuthId(AuthHandle authHandle, uint32_t *flag, bool *isAuthServer, bool isLegacyOs)
{
    if (flag == NULL || isAuthServer == NULL) {
        TRANS_LOGE(TRANS_CTRL, "param invalid");
        return SOFTBUS_INVALID_PARAM;
    }
    AuthConnInfo info;
    int32_t ret = AuthGetServerSide(authHandle.authId, isAuthServer);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "get auth server side fail authId=%{public}" PRId64, authHandle.authId);
        return ret;
    }
    ret = AuthGetConnInfo(authHandle, &info);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "get authinfo fail authId=%{public}" PRId64, authHandle.authId);
        return ret;
    }
    // In order to be compatible with legacyOs versions that only has AUTH_P2P
    if (isLegacyOs && info.type == AUTH_LINK_TYPE_ENHANCED_P2P) {
        *flag = FLAG_P2P;
        TRANS_LOGW(TRANS_CTRL,
            "peer device is legacyOs, change flag form P2P_ENHANCE to P2P flag=0x%{public}x", *flag);
        return SOFTBUS_OK;
    }
    *flag = SwitchAuthLinkTypeToFlagType(info.type);
    TRANS_LOGI(TRANS_CTRL, "get auth link type=%{public}d, flag=0x%{public}x", info.type, *flag);
    return SOFTBUS_OK;
}

static int32_t TransPostBytes(SessionConn *conn, bool isAuthServer, uint32_t cipherFlag, const UkIdInfo *ukIdInfo)
{
    uint64_t seq = TransTdcGetNewSeqId();
    if (isAuthServer) {
        seq |= AUTH_CONN_SERVER_SIDE;
    }

    char *bytes = PackRequest(&conn->appInfo);
    if (bytes == NULL) {
        TRANS_LOGE(TRANS_CTRL,
            "Pack Request failed channelId=%{public}d, fd=%{public}d",
            conn->channelId, conn->appInfo.fd);
        return SOFTBUS_TRANS_PACK_REQUEST_FAILED;
    }
    TdcPacketHead packetHead = {
        .magicNumber = MAGIC_NUMBER,
        .module = IsValidUkInfo(ukIdInfo) ? MODULE_UK_ENCYSESSION : MODULE_SESSION,
        .seq = seq,
        .flags = (FLAG_REQUEST | cipherFlag),
        .dataLen = strlen(bytes), /* reset after encrypt */
    };
    if (conn->isMeta) {
        packetHead.flags |= FLAG_AUTH_META;
    }
    int32_t ret = TransTdcPostBytes(conn->channelId, &packetHead, bytes, ukIdInfo);
    cJSON_free(bytes);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL,
            "TransTdc post bytes failed channelId=%{public}d, fd=%{public}d, ret=%{public}d",
            conn->channelId, conn->appInfo.fd, ret);
        return ret;
    }
    return SOFTBUS_OK;
}

static int32_t StartNegotiateUk(SessionConn *conn)
{
    TRANS_LOGI(
        TRANS_CTRL, "negotiate uk start. channelId=%{public}d, fd=%{public}d", conn->channelId, conn->appInfo.fd);
    uint64_t seq = TransTdcGetNewSeqId();
    char *buf = PackUkRequest(&(conn->appInfo));
    if (buf == NULL) {
        TRANS_LOGE(TRANS_CTRL,
            "pack uk negotiate request failed channelId=%{public}d, fd=%{public}d",
            conn->channelId, conn->appInfo.fd);
        return SOFTBUS_TRANS_PACK_REQUEST_FAILED;
    }
    bool isAuthServer = false;
    uint32_t cipherFlag = FLAG_WIFI;
    bool isLegacyOs = IsPeerDeviceLegacyOs(conn->appInfo.osType);
    if (GetCipherFlagByAuthId(conn->authHandle, &cipherFlag, &isAuthServer, isLegacyOs) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL,
            "get cipher flag failed channelId=%{public}d, fd=%{public}d",
            conn->channelId, conn->appInfo.fd);
        cJSON_free(buf);
        return SOFTBUS_TRANS_GET_CIPHER_FAILED;
    }
    if (isAuthServer) {
        seq |= AUTH_CONN_SERVER_SIDE;
    }
    TdcPacketHead packetHead = {
        .magicNumber = MAGIC_NUMBER,
        .module = MODULE_UK_NEGOSESSION,
        .seq = seq,
        .flags = (FLAG_REQUEST | cipherFlag),
        .dataLen = strlen(buf), /* reset after encrypt */
    };
    int32_t ret = TransTdcPostBytes(conn->channelId, &packetHead, buf, NULL);
    cJSON_free(buf);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL,
            "Trans tdc post uk negotiate request failed, channelId=%{public}d, fd=%{public}d, ret=%{public}d",
            conn->channelId, conn->appInfo.fd, ret);
        return ret;
    }
    return SOFTBUS_OK;
}

int32_t StartVerifySession(SessionConn *conn, const UkIdInfo *ukIdInfo)
{
    TRANS_CHECK_AND_RETURN_RET_LOGE(conn != NULL, SOFTBUS_INVALID_PARAM, TRANS_CTRL, "invalid param.");
    TRANS_LOGI(TRANS_CTRL, "verify enter. channelId=%{public}d, fd=%{public}d", conn->channelId, conn->appInfo.fd);
    if (SoftBusGenerateSessionKey(conn->appInfo.sessionKey, SESSION_KEY_LENGTH) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL,
            "Generate SessionKey failed channelId=%{public}d, fd=%{public}d",
            conn->channelId, conn->appInfo.fd);
        return SOFTBUS_TRANS_TCP_GENERATE_SESSIONKEY_FAILED;
    }
    SetSessionKeyByChanId(conn->channelId, conn->appInfo.sessionKey, sizeof(conn->appInfo.sessionKey));
    bool isAuthServer = false;
    uint32_t cipherFlag = FLAG_WIFI;
    bool isLegacyOs = IsPeerDeviceLegacyOs(conn->appInfo.osType);
    if (GetCipherFlagByAuthId(conn->authHandle, &cipherFlag, &isAuthServer, isLegacyOs)) {
        TRANS_LOGE(TRANS_CTRL,
            "get cipher flag failed channelId=%{public}d, fd=%{public}d",
            conn->channelId, conn->appInfo.fd);
        return SOFTBUS_TRANS_GET_CIPHER_FAILED;
    }
    int32_t ret = TransPostBytes(conn, isAuthServer, cipherFlag, ukIdInfo);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL,
            "TransTdc post bytes failed channelId=%{public}d, fd=%{public}d, ret=%{public}d",
            conn->channelId, conn->appInfo.fd, ret);
        return ret;
    }
    TRANS_LOGI(TRANS_CTRL, "verify ok. channelId=%{public}d, fd=%{public}d", conn->channelId, conn->appInfo.fd);

    return SOFTBUS_OK;
}

static void TransSetTcpDirectConnectType(int32_t *connectType, ListenerModule module)
{
    if (module >= DIRECT_CHANNEL_SERVER_HML_START && module <= DIRECT_CHANNEL_SERVER_HML_END) {
        *connectType = CONNECT_HML;
    } else if (module == DIRECT_CHANNEL_SERVER_P2P) {
        *connectType = CONNECT_P2P;
    } else if (module == DIRECT_CHANNEL_SERVER_WIFI || module == DIRECT_CHANNEL_SERVER_USB) {
        *connectType = CONNECT_TCP;
    }
}

static RouteType TransGetRouteTypeByModule(ListenerModule module)
{
    if (module == DIRECT_CHANNEL_SERVER_USB) {
        return WIFI_USB;
    }
    return (module == DIRECT_CHANNEL_SERVER_P2P) ? WIFI_P2P : WIFI_STA;
}

static int32_t CreateSessionConnNode(ListenerModule module, int fd, int32_t chanId, const ConnectOption *clientAddr)
{
    SessionConn *conn = (SessionConn *)SoftBusCalloc(sizeof(SessionConn));
    if (conn == NULL) {
        TRANS_LOGE(TRANS_CTRL, "malloc fail in create session conn node.");
        return SOFTBUS_MALLOC_ERR;
    }
    conn->appInfo.myData.apiVersion = API_V2;
    conn->appInfo.fd = fd;
    conn->serverSide = true;
    conn->channelId = chanId;
    conn->status = TCP_DIRECT_CHANNEL_STATUS_CONNECTING;
    conn->timeout = 0;
    conn->listenMod = module;
    conn->authHandle.authId = AUTH_INVALID_ID;
    conn->appInfo.routeType = TransGetRouteTypeByModule(module);
    conn->appInfo.peerData.port = clientAddr->socketOption.port;
    TransSetTcpDirectConnectType(&conn->appInfo.connectType, module);
    int32_t ret =
        LnnGetLocalStrInfo(STRING_KEY_UUID, conn->appInfo.myData.deviceId, sizeof(conn->appInfo.myData.deviceId));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "get local deviceId failed.");
        SoftBusFree(conn);
        return ret;
    }
    if (strcpy_s(conn->appInfo.peerData.addr, sizeof(conn->appInfo.peerData.addr), clientAddr->socketOption.addr) !=
        EOK) {
        TRANS_LOGE(TRANS_CTRL, "copy ip to app info failed.");
        SoftBusFree(conn);
        return SOFTBUS_STRCPY_ERR;
    }
    conn->appInfo.protocol = clientAddr->socketOption.protocol;

    const char *authState = "";
    if (strcpy_s(conn->appInfo.myData.authState, sizeof(conn->appInfo.myData.authState), authState) != EOK) {
        TRANS_LOGE(TRANS_CTRL, "copy auth state to app info failed.");
        SoftBusFree(conn);
        return SOFTBUS_STRCPY_ERR;
    }
    ret = TransTdcAddSessionConn(conn);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "add session conn node failed.");
        SoftBusFree(conn);
        return ret;
    }
    ret = AddTrigger(module, fd, READ_TRIGGER);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "add trigger failed, delete session conn.");
        TransDelSessionConnById(chanId);
        return ret;
    }

    return SOFTBUS_OK;
}

static int32_t TdcOnConnectEvent(ListenerModule module, int cfd, const ConnectOption *clientAddr)
{
    TRANS_CHECK_AND_RETURN_RET_LOGE(cfd >= 0 && clientAddr != NULL, SOFTBUS_INVALID_PARAM, TRANS_CTRL,
        "invalid param, cfd=%{public}d", cfd);
    int32_t ret;
    int32_t channelId = GenerateChannelId(true);
    TransEventExtra extra = {
        .socketName = NULL,
        .peerNetworkId = NULL,
        .calleePkg = NULL,
        .callerPkg = NULL,
        .socketFd = cfd,
        .channelId = channelId
    };
    if (channelId <= INVALID_CHANNEL_ID) {
        ret = SOFTBUS_TRANS_INVALID_CHANNEL_ID;
        extra.result = EVENT_STAGE_RESULT_FAILED;
        extra.errcode = ret;
        TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL_SERVER, EVENT_STAGE_START_CONNECT, extra);
        TRANS_LOGE(TRANS_CTRL, "channelId is invalid");
        DelTrigger(module, cfd, RW_TRIGGER);
        TransTdcSocketReleaseFd(module, cfd);
        return ret;
    }
    ret = TransSrvAddDataBufNode(channelId, cfd); // fd != channelId
    if (ret != SOFTBUS_OK) {
        extra.result = EVENT_STAGE_RESULT_FAILED;
        extra.errcode = ret;
        TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL_SERVER, EVENT_STAGE_START_CONNECT, extra);
        TRANS_LOGE(TRANS_CTRL, "create srv data buf node failed.");
        DelTrigger(module, cfd, RW_TRIGGER);
        TransTdcSocketReleaseFd(module, cfd);
        return ret;
    }
    ret = CreateSessionConnNode(module, cfd, channelId, clientAddr);
    if (ret != SOFTBUS_OK) {
        extra.result = EVENT_STAGE_RESULT_FAILED;
        extra.errcode = ret;
        TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL_SERVER, EVENT_STAGE_START_CONNECT, extra);
        TRANS_LOGE(TRANS_CTRL, "create session conn node fail, delete data buf node.");
        TransSrvDelDataBufNode(channelId);
        DelTrigger(module, cfd, RW_TRIGGER);
        TransTdcSocketReleaseFd(module, cfd);
        return ret;
    }
    extra.result = EVENT_STAGE_RESULT_OK;
    TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL_SERVER, EVENT_STAGE_START_CONNECT, extra);
    TRANS_LOGI(TRANS_CTRL,
        "tdc conn event cfd=%{public}d, channelId=%{public}d, module=%{public}d", cfd, channelId, module);
    return SOFTBUS_OK;
}

void CloseTcpDirectFd(ListenerModule module, int32_t fd)
{
#ifndef __LITEOS_M__
    if (fd < 0) {
        TRANS_LOGE(TRANS_CTRL, "invalid fd.");
        return;
    }
    if (DelTrigger(module, fd, RW_TRIGGER) == SOFTBUS_NOT_FIND) {
        ConnCloseSocket(fd);
    }
#else
    (void)fd;
#endif
}

static void TransProcDataRes(ListenerModule module, int32_t errCode, int32_t channelId, int32_t fd, int32_t pktModule)
{
    SessionConn conn;
    int32_t ret = GetSessionConnById(channelId, &conn);
    if (errCode != SOFTBUS_OK) {
        TransEventExtra extra = {
            .socketName = NULL,
            .peerNetworkId = NULL,
            .calleePkg = NULL,
            .callerPkg = NULL,
            .channelId = channelId,
            .socketFd = fd,
            .errcode = errCode,
            .result = EVENT_STAGE_RESULT_FAILED
        };

        if (ret != SOFTBUS_OK || !conn.serverSide) {
            TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL, EVENT_STAGE_HANDSHAKE_REPLY, extra);
        } else {
            TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL_SERVER, EVENT_STAGE_HANDSHAKE_REPLY, extra);
        }
        (void)memset_s(conn.appInfo.sessionKey, sizeof(conn.appInfo.sessionKey), 0, sizeof(conn.appInfo.sessionKey));
        DelTrigger(module, fd, READ_TRIGGER);
        TransTdcSocketReleaseFd(module, fd);
        (void)NotifyChannelOpenFailed(channelId, errCode);
    } else if (pktModule == MODULE_UK_NEGOSESSION) {
        AddTrigger(module, fd, READ_TRIGGER);
        return;
    } else {
        if (ret != SOFTBUS_OK || conn.serverSide) {
            return;
        }
        DelTrigger(module, fd, READ_TRIGGER);
        CloseTcpDirectFd(module, fd);
    }
    TransDelSessionConnById(channelId);
    TransSrvDelDataBufNode(channelId);
}

static int32_t ProcessSocketInEvent(SessionConn *conn, int fd)
{
    int32_t pktModule = 0;
    int32_t ret = TransTdcSrvRecvData(conn->listenMod, conn->channelId, conn->authHandle.type, &pktModule);
    if (ret == SOFTBUS_DATA_NOT_ENOUGH) {
        return SOFTBUS_OK;
    }
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "Trans Srv Recv Data, ret=%{public}d.", ret);
    }
    TransProcDataRes(conn->listenMod, ret, conn->channelId, fd, pktModule);
    return ret;
}

static int32_t ProcessSocketOutEvent(SessionConn *conn, int fd)
{
    int32_t ret = SOFTBUS_TCP_SOCKET_ERR;
    if (conn->serverSide) {
        return ret;
    }
    DelTrigger(conn->listenMod, fd, WRITE_TRIGGER);
    if (AddTrigger(conn->listenMod, fd, READ_TRIGGER) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "add trigger fail, module=%{public}d, fd=%{public}d", conn->listenMod, fd);
        return SOFTBUS_TRANS_ADD_TRIGGER_FAILED;
    }
    int32_t ukPolicy = GetUkPolicy(&(conn->appInfo));
    TRANS_LOGI(TRANS_CTRL, "uk policy=%{public}d", ukPolicy);
    if (ukPolicy == USE_NEGO_UK) {
        TRANS_LOGI(TRANS_CTRL, "negotiate uk, channelId=%{public}d", conn->channelId);
        ret = StartNegotiateUk(conn);
    } else {
        ret = StartVerifySession(conn, NULL);
    }
    TransEventExtra extra = {
        .socketName = NULL,
        .peerNetworkId = NULL,
        .calleePkg = NULL,
        .callerPkg = NULL,
        .socketFd = fd,
        .channelId = conn->channelId,
        .authId = conn->authHandle.authId,
        .errcode = ret,
        .result = (ret == SOFTBUS_OK) ? EVENT_STAGE_RESULT_OK : EVENT_STAGE_RESULT_FAILED
    };
    TRANS_EVENT(EVENT_SCENE_OPEN_CHANNEL, EVENT_STAGE_HANDSHAKE_START, extra);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "start verify session failed, ret=%{public}d", ret);
        DelTrigger(conn->listenMod, fd, READ_TRIGGER);
        TransTdcSocketReleaseFd(conn->listenMod, fd);
        (void)NotifyChannelOpenFailed(conn->channelId, ret);
        TransDelSessionConnById(conn->channelId);
        TransSrvDelDataBufNode(conn->channelId);
    }
    return ret;
}

static void ProcessSocketExceptionEvent(SessionConn *conn, int fd)
{
    TRANS_LOGE(TRANS_CTRL, "exception occurred.");
    DelTrigger(conn->listenMod, fd, EXCEPT_TRIGGER);
    TransTdcSocketReleaseFd(conn->listenMod, fd);
    TransDelSessionConnById(conn->channelId);
    TransSrvDelDataBufNode(conn->channelId);
}

static int32_t TdcOnDataEvent(ListenerModule module, int events, int fd)
{
    (void)module;
    SessionConn *conn = (SessionConn *)SoftBusCalloc(sizeof(SessionConn));
    if (conn == NULL) {
        TRANS_LOGE(TRANS_CTRL, "OnDataEvent malloc fail.");
        return SOFTBUS_MALLOC_ERR;
    }
    if (GetSessionConnByFd(fd, conn) != SOFTBUS_OK || conn->appInfo.fd != fd) {
        TRANS_LOGE(TRANS_CTRL, "fd is not exist tdc info. fd=%{public}d, appfd=%{public}d", fd, conn->appInfo.fd);
        for (uint32_t i = DIRECT_CHANNEL_SERVER_P2P; i <= DIRECT_CHANNEL_SERVER_HML_END; i++) {
            DelTrigger((ListenerModule)i, fd, READ_TRIGGER);
            DelTrigger((ListenerModule)i, fd, WRITE_TRIGGER);
            DelTrigger((ListenerModule)i, fd, EXCEPT_TRIGGER);
        }
        SoftBusFree(conn);
        TransTdcSocketReleaseFd(module, fd);
        return SOFTBUS_INVALID_FD;
    }
    SoftbusHitraceStart(SOFTBUS_HITRACE_ID_VALID, (uint64_t)(conn->channelId + ID_OFFSET));
    int32_t ret = SOFTBUS_TRANS_TDC_ON_DATA_EVENT_FAILED;
    if (events == SOFTBUS_SOCKET_IN) {
        ret = ProcessSocketInEvent(conn, fd);
    } else if (events == SOFTBUS_SOCKET_OUT) {
        ret = ProcessSocketOutEvent(conn, fd);
    } else if (events == SOFTBUS_SOCKET_EXCEPTION) {
        ProcessSocketExceptionEvent(conn, fd);
    }
    (void)memset_s(conn->appInfo.sessionKey, sizeof(conn->appInfo.sessionKey), 0, sizeof(conn->appInfo.sessionKey));
    SoftBusFree(conn);
    return ret;
}

int32_t TransTdcStartSessionListener(ListenerModule module, const LocalListenerInfo *info)
{
    if (info == NULL || (info->type != CONNECT_TCP && info->type != CONNECT_P2P && info->type != CONNECT_HML) ||
        info->socketOption.port < 0) {
        TRANS_LOGE(TRANS_CTRL, "Invalid para.");
        return SOFTBUS_INVALID_PARAM;
    }

    static SoftbusBaseListener sessionListener = {
        .onConnectEvent = TdcOnConnectEvent,
        .onDataEvent = TdcOnDataEvent
    };

    TRANS_LOGI(TRANS_CTRL, "set listener for module=%{public}d.", module);
    int serverPort = StartBaseListener(info, &sessionListener);
    return serverPort;
}

int32_t TransTdcStopSessionListener(ListenerModule module)
{
    TransTdcStopSessionProc(module);
    return StopBaseListener(module);
}

void TransTdcSocketReleaseFd(ListenerModule module, int32_t fd)
{
    if (fd < 0) {
        TRANS_LOGI(TRANS_SDK, "fd less than zero");
        return;
    }
    if (DelTrigger(module, fd, RW_TRIGGER) == SOFTBUS_NOT_FIND) {
        ConnShutdownSocket(fd);
    }
}
