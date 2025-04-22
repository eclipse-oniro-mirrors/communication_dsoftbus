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

#include "auth_tcp_connection.h"

#include <securec.h>

#include "auth_channel.h"
#include "auth_common.h"
#include "auth_log.h"
#include "auth_meta_manager.h"
#include "bus_center_manager.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_socket.h"
#include "softbus_base_listener.h"
#include "softbus_def.h"
#include "softbus_socket.h"
#include "softbus_tcp_socket.h"
#include "trans_lane_manager.h"

#define MAGIC_NUMBER             0xBABEFACE
#define AUTH_PKT_HEAD_LEN        24
#define AUTH_SOCKET_MAX_DATA_LEN (64 * 1024)
#define TCP_KEEPALIVE_TOS_VAL    180
#define RECV_DATA_TIMEOUT        (2 * 1000 * 1000)

typedef struct {
    int32_t keepaliveIdle;
    int32_t keepaliveIntvl;
    int32_t keepaliveCount;
    uint32_t userTimeout;
} TcpKeepaliveOption;

typedef struct {
    int32_t module;
    AuthChannelListener listener;
} InnerChannelListener;

static InnerChannelListener g_listener[] = {
    {
        .module = MODULE_AUTH_CHANNEL,
        .listener = { NULL, NULL },
    },
    {
        .module = MODULE_AUTH_MSG,
        .listener = { NULL, NULL },
    },
};

static SocketCallback g_callback = { NULL, NULL, NULL };
static ListNode g_wifiConnList = { &g_wifiConnList, &g_wifiConnList };
static SoftBusMutex g_wifiConnListLock;

static void NotifyChannelDisconnected(int32_t channelId);
static void NotifyChannelDataReceived(int32_t channelId, const SocketPktHead *head, const uint8_t *data);
int32_t __attribute__((weak)) RouteBuildServerAuthManager(int32_t cfd, const ConnectOption *clientAddr)
{
    (void)cfd;
    (void)clientAddr;
    return SOFTBUS_OK;
}

static uint32_t GetSocketPktSize(uint32_t len)
{
    return AUTH_PKT_HEAD_LEN + len;
}

static int32_t PackSocketPkt(const SocketPktHead *pktHead, const uint8_t *data, uint8_t *buf, uint32_t size)
{
    if (size < GetSocketPktSize(pktHead->len)) {
        AUTH_LOGE(AUTH_CONN, "buffer not enough.");
        return SOFTBUS_NO_ENOUGH_DATA;
    }
    uint32_t offset = 0;
    *(uint32_t *)buf = SoftBusHtoLl((uint32_t)pktHead->magic);
    offset += sizeof(uint32_t);
    *(uint32_t *)(buf + offset) = SoftBusHtoLl((uint32_t)pktHead->module);
    offset += sizeof(uint32_t);
    *(uint64_t *)(buf + offset) = SoftBusHtoLll((uint64_t)pktHead->seq);
    offset += sizeof(uint64_t);
    *(uint32_t *)(buf + offset) = SoftBusHtoLl((uint32_t)pktHead->flag);
    offset += sizeof(uint32_t);
    *(uint32_t *)(buf + offset) = SoftBusHtoLl(pktHead->len);
    offset += sizeof(uint32_t);
    if (memcpy_s(buf + offset, size - offset, data, pktHead->len) != EOK) {
        AUTH_LOGE(AUTH_CONN, "pack fail.");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t UnpackSocketPkt(const uint8_t *data, uint32_t len, SocketPktHead *head)
{
    if (len < GetSocketPktSize(0)) {
        AUTH_LOGE(AUTH_CONN, "head not enough.");
        return SOFTBUS_NO_ENOUGH_DATA;
    }
    uint32_t offset = 0;
    head->magic = (int32_t)SoftBusLtoHl(*(uint32_t *)data);
    offset += sizeof(uint32_t);
    head->module = (int32_t)SoftBusLtoHl(*(uint32_t *)(data + offset));
    offset += sizeof(uint32_t);
    head->seq = (int64_t)SoftBusLtoHll(*(uint64_t *)(data + offset));
    offset += sizeof(uint64_t);
    head->flag = (int32_t)SoftBusLtoHl(*(uint32_t *)(data + offset));
    offset += sizeof(uint32_t);
    head->len = SoftBusLtoHl(*(uint32_t *)(data + offset));
    return SOFTBUS_OK;
}

static void NotifyConnected(ListenerModule module, int32_t fd, bool isClient)
{
    if (g_callback.onConnected != NULL) {
        g_callback.onConnected(module, fd, isClient);
    }
}

static void NotifyDisconnected(ListenerModule module, int32_t fd)
{
    if (g_callback.onDisconnected != NULL) {
        g_callback.onDisconnected(module, fd);
    }
    NotifyChannelDisconnected(fd);
}

static uint32_t ModuleToDataType(int32_t module)
{
    switch (module) {
        case MODULE_TRUST_ENGINE:
            return DATA_TYPE_DEVICE_ID;
        case MODULE_AUTH_SDK:
            return DATA_TYPE_AUTH;
        case MODULE_AUTH_CONNECTION:
            return DATA_TYPE_DEVICE_INFO;
        case MODULE_AUTH_CANCEL:
            return DATA_TYPE_CANCEL_AUTH;
        case MODULE_USER_KEY_CONNECTION:
            return DATA_TYPE_UK_CONNECTION;
        default:
            break;
    }
    return DATA_TYPE_CONNECTION;
}

static void SessionNotifyDataReceived(ListenerModule module, int32_t fd,
    uint32_t len, const uint8_t *data)
{
    AuthDataHead head = {0};
    const uint8_t *body = UnpackAuthData(data, len, &head);
    if (body == NULL) {
        AUTH_LOGE(AUTH_CONN, "unpack auth data fail.");
        return;
    }
    if (g_callback.onDataReceived != NULL) {
        g_callback.onDataReceived(module, fd, &head, body);
    }
}

static void SessionKeyNotifyDataReceived(ListenerModule module, int32_t fd,
    uint32_t len, const uint8_t *data)
{
    AuthDataHead head = {0};
    const uint8_t *body = UnpackAuthData(data, len, &head);
    if (body == NULL) {
        AUTH_LOGE(AUTH_CONN, "unpack auth data fail.");
        return;
    }
    if (g_callback.onDataReceived != NULL) {
        g_callback.onDataReceived(module, fd, &head, body);
    }
}

static void NotifyDataReceived(ListenerModule module, int32_t fd,
    const SocketPktHead *pktHead, const uint8_t *data)
{
    if (pktHead->module == MODULE_AUTH_CHANNEL || pktHead->module == MODULE_AUTH_MSG) {
        NotifyChannelDataReceived(fd, pktHead, data);
        return;
    }
    if (pktHead->module == MODULE_META_AUTH) {
        AuthMetaNotifyDataReceived(fd, pktHead, data);
        return;
    }
    if (pktHead->module == MODULE_SESSION_AUTH) {
        SessionNotifyDataReceived(module, fd, pktHead->len, data);
        return;
    }
    if (pktHead->module == MODULE_SESSION_KEY_AUTH) {
        SessionKeyNotifyDataReceived(module, fd, pktHead->len, data);
        return;
    }
    AuthDataHead head = {
        .dataType = ModuleToDataType(pktHead->module),
        .module = pktHead->module,
        .seq = pktHead->seq,
        .flag = pktHead->flag,
        .len = pktHead->len,
    };
    if (g_callback.onDataReceived != NULL) {
        g_callback.onDataReceived(module, fd, &head, data);
    }
}

static int32_t RecvPacketHead(ListenerModule module, int32_t fd, SocketPktHead *head)
{
    uint8_t buf[AUTH_PKT_HEAD_LEN] = { 0 };
    uint32_t offset = 0;
    while (offset < AUTH_PKT_HEAD_LEN) {
        ssize_t recvLen =
            ConnRecvSocketData(fd, (char *)&buf[offset], (size_t)(sizeof(buf) - offset), RECV_DATA_TIMEOUT);
        if (recvLen < 0) {
            AUTH_LOGE(AUTH_CONN, "recv head fail.");
            (void)DelTrigger(module, fd, READ_TRIGGER);
            NotifyDisconnected(module, fd);
            return SOFTBUS_INVALID_DATA_HEAD;
        }
        offset += (uint32_t)recvLen;
    }
    return UnpackSocketPkt(buf, offset, head);
}

static uint8_t *RecvPacketData(int32_t fd, uint32_t len)
{
    uint8_t *data = (uint8_t *)SoftBusCalloc(len);
    if (data == NULL) {
        AUTH_LOGE(AUTH_CONN, "malloc data buf fail.");
        return NULL;
    }
    uint32_t offset = 0;
    while (offset < len) {
        ssize_t recvLen = ConnRecvSocketData(fd, (char *)(data + offset), (size_t)(len - offset), 0);
        if (recvLen < 0) {
            AUTH_LOGE(AUTH_CONN, "recv data fail.");
            SoftBusFree(data);
            return NULL;
        }
        offset += (uint32_t)recvLen;
    }
    return data;
}

typedef struct {
    ListNode node;
    int32_t fd;
} WifiConnInstance;

static bool RequireWifiConnListLock(void)
{
    if (SoftBusMutexLock(&g_wifiConnListLock) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "wifiConnList lock fail");
        return false;
    }
    return true;
}

static void ReleaseWifiConnListLock(void)
{
    if (SoftBusMutexUnlock(&g_wifiConnListLock) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "wifiConnList unlock fail");
    }
}

static int32_t AddWifiConnItem(int32_t fd)
{
    if (!RequireWifiConnListLock()) {
        AUTH_LOGE(AUTH_CONN, "RequireWifiConnListLock fail");
        return SOFTBUS_LOCK_ERR;
    }
    WifiConnInstance *connItem = (WifiConnInstance *)SoftBusCalloc(sizeof(WifiConnInstance));
    if (connItem == NULL) {
        AUTH_LOGE(AUTH_CONN, "malloc connItem fail");
        ReleaseWifiConnListLock();
        return SOFTBUS_MEM_ERR;
    }
    connItem->fd = fd;
    ListNodeInsert(&g_wifiConnList, &connItem->node);
    AUTH_LOGI(AUTH_CONN, "add wifi conn item. fd=%{public}d", fd);
    ReleaseWifiConnListLock();
    return SOFTBUS_OK;
}

bool IsExistWifiConnItemByConnId(int32_t fd)
{
    if (!RequireWifiConnListLock()) {
        AUTH_LOGE(AUTH_CONN, "RequireWifiConnListLock fail");
        return false;
    }
    WifiConnInstance *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &g_wifiConnList, WifiConnInstance, node) {
        if (item->fd != fd) {
            continue;
        }
        ReleaseWifiConnListLock();
        return true;
    }
    AUTH_LOGE(AUTH_CONN, "wifi conn item is not found. fd=%{public}d", fd);
    ReleaseWifiConnListLock();
    return false;
}

void DeleteWifiConnItemByConnId(int32_t fd)
{
    if (!RequireWifiConnListLock()) {
        AUTH_LOGE(AUTH_CONN, "RequireWifiConnListLock fail");
        return;
    }
    WifiConnInstance *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &g_wifiConnList, WifiConnInstance, node) {
        if (item->fd != fd) {
            continue;
        }
        AUTH_LOGI(AUTH_CONN, "delete wifi conn item. fd=%{public}d", fd);
        ListDelete(&item->node);
        SoftBusFree(item);
        break;
    }
    ReleaseWifiConnListLock();
}

static int32_t ProcessSocketOutEvent(ListenerModule module, int32_t fd)
{
    AUTH_LOGI(AUTH_CONN, "socket client connect succ: fd=%{public}d.", fd);
    (void)DelTrigger(module, fd, WRITE_TRIGGER);
    if (AddTrigger(module, fd, READ_TRIGGER) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "AddTrigger fail.");
        goto FAIL;
    }
    if (ConnToggleNonBlockMode(fd, true) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set none block mode fail.");
        goto FAIL;
    }
    if (AddWifiConnItem(fd) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "insert wifi conn item fail.");
        goto FAIL;
    }
    NotifyConnected(module, fd, true);
    return SOFTBUS_OK;

FAIL:
    (void)DelTrigger(module, fd, READ_TRIGGER);
    ConnShutdownSocket(fd);
    NotifyDisconnected(module, fd);
    return SOFTBUS_AUTH_SOCKET_EVENT_INVALID;
}

static int32_t ProcessSocketInEvent(ListenerModule module, int32_t fd)
{
    SocketPktHead head = { 0 };
    int32_t ret = RecvPacketHead(module, fd, &head);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    AUTH_LOGI(AUTH_CONN,
        "RecvSocketData: fd=%{public}d, module=%{public}d, seq=%{public}" PRId64 ", flag=%{public}d, len=%{public}u.",
        fd, head.module, head.seq, head.flag, head.len);
    if (head.len == 0 || head.len > AUTH_SOCKET_MAX_DATA_LEN) {
        AUTH_LOGW(AUTH_CONN, "data is out of size, abandon it.");
        return SOFTBUS_INVALID_DATA_HEAD;
    }
    if (head.magic != (int32_t)MAGIC_NUMBER) {
        AUTH_LOGE(AUTH_CONN, "magic number not match.");
        return SOFTBUS_INVALID_DATA_HEAD;
    }
    uint8_t *data = RecvPacketData(fd, head.len);
    if (data == NULL) {
        return SOFTBUS_DATA_NOT_ENOUGH;
    }
    NotifyDataReceived(module, fd, &head, data);
    SoftBusFree(data);
    return SOFTBUS_OK;
}

static bool IsEnhanceP2pModuleId(ListenerModule moduleId)
{
    if (moduleId >= AUTH_ENHANCED_P2P_START && moduleId <= AUTH_ENHANCED_P2P_END) {
        return true;
    }
    return false;
}

static int32_t OnConnectEvent(ListenerModule module, int32_t cfd, const ConnectOption *clientAddr)
{
    if (cfd < 0) {
        AUTH_LOGE(AUTH_CONN, "invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (ConnSetTcpKeepalive(cfd, (int32_t)DEFAULT_FREQ_CYCLE, TCP_KEEPALIVE_INTERVAL, TCP_KEEPALIVE_DEFAULT_COUNT) !=
        SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set keepalive fail!");
        ConnShutdownSocket(cfd);
        return SOFTBUS_NETWORK_SET_KEEPALIVE_OPTION_FAIL;
    }
    int32_t ipTos = TCP_KEEPALIVE_TOS_VAL;
    if (module == AUTH) {
        if (SoftBusSocketSetOpt(cfd, SOFTBUS_IPPROTO_IP_, SOFTBUS_IP_TOS_, &ipTos, sizeof(ipTos)) !=
            SOFTBUS_ADAPTER_OK) {
            AUTH_LOGE(AUTH_CONN, "set option fail!");
            ConnShutdownSocket(cfd);
            return SOFTBUS_NETWORK_SET_KEEPALIVE_OPTION_FAIL;
        }
    }
    if (AddTrigger(module, cfd, READ_TRIGGER) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "AddTrigger fail.");
        ConnShutdownSocket(cfd);
        return SOFTBUS_AUTH_ADD_TRIGGER_FAIL;
    }
    if (module != AUTH && module != AUTH_USB && module != AUTH_P2P && module != AUTH_RAW_P2P_CLIENT &&
        !IsEnhanceP2pModuleId(module)) {
        AUTH_LOGI(AUTH_CONN, "newip auth process");
        if (RouteBuildServerAuthManager(cfd, clientAddr) != SOFTBUS_OK) {
            AUTH_LOGE(AUTH_CONN, "build auth manager fail.");
            (void)DelTrigger(module, cfd, READ_TRIGGER);
            ConnShutdownSocket(cfd);
            return SOFTBUS_AUTH_MANAGER_BUILD_FAIL;
        }
        return SOFTBUS_OK;
    }
    if ((module == AUTH || module == AUTH_USB) && AddWifiConnItem(cfd) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "insert wifi conn item fail.");
        return SOFTBUS_MEM_ERR;
    }
    NotifyConnected(module, cfd, false);
    return SOFTBUS_OK;
}

static int32_t OnDataEvent(ListenerModule module, int32_t events, int32_t fd)
{
    if (events == SOFTBUS_SOCKET_OUT) {
        return ProcessSocketOutEvent(module, fd);
    } else if (events == SOFTBUS_SOCKET_IN) {
        return ProcessSocketInEvent(module, fd);
    }
    return SOFTBUS_AUTH_SOCKET_EVENT_INVALID;
}

int32_t SetSocketCallback(const SocketCallback *cb)
{
    CHECK_NULL_PTR_RETURN_VALUE(cb, SOFTBUS_INVALID_PARAM);
    if (memcpy_s(&g_callback, sizeof(SocketCallback), cb, sizeof(SocketCallback)) != EOK) {
        AUTH_LOGE(AUTH_CONN, "set SocketCallback fail.");
        return SOFTBUS_MEM_ERR;
    }
    return SOFTBUS_OK;
}

void UnsetSocketCallback(void)
{
    (void)memset_s(&g_callback, sizeof(SocketCallback), 0, sizeof(SocketCallback));
}

int32_t StartSocketListening(ListenerModule module, const LocalListenerInfo *info)
{
    SoftbusBaseListener listener = {
        .onConnectEvent = OnConnectEvent,
        .onDataEvent = OnDataEvent,
    };
    int32_t port = StartBaseListener(info, &listener);
    if (port <= 0) {
        AUTH_LOGE(AUTH_CONN, "StartBaseListener fail. port=%{public}d", port);
        return port;
    }
    return port;
}

void StopSocketListening(ListenerModule moduleId)
{
    AUTH_LOGI(AUTH_CONN, "stop socket listening. moduleId=%{public}d", moduleId);
    if (StopBaseListener(moduleId) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "StopBaseListener fail.");
    }
}

static int32_t AuthTcpCreateListener(ListenerModule module, int32_t fd, TriggerType trigger)
{
    if (!IsListenerNodeExist(module)) {
        SoftbusBaseListener listener = {
            .onConnectEvent = OnConnectEvent,
            .onDataEvent = OnDataEvent,
        };
        if (StartBaseClient(module, &listener) != SOFTBUS_OK) {
            AUTH_LOGE(AUTH_CONN, "StartBaseClient fail.");
        }
    }
    return AddTrigger(module, fd, trigger);
}

static int32_t SocketConnectInner(
    const char *localIp, const char *peerIp, int32_t port, ListenerModule module, bool isBlockMode)
{
    if (localIp == NULL || peerIp == NULL) {
        AUTH_LOGE(AUTH_CONN, "ip is invalid param.");
        return AUTH_INVALID_FD;
    }
    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = { .addr = "", .port = port, .moduleId = module, .protocol = LNN_PROTOCOL_IP }
    };
    if (strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), peerIp) != EOK) {
        AUTH_LOGE(AUTH_CONN, "copy remote ip fail.");
        return AUTH_INVALID_FD;
    }
    int32_t ret = ConnOpenClientSocket(&option, localIp, !isBlockMode);
    if (ret < 0) {
        AUTH_LOGE(AUTH_CONN, "ConnOpenClientSocket fail, error=%{public}d", ret);
        return ret;
    }
    int32_t fd = ret;
    TriggerType triggerMode = isBlockMode ? READ_TRIGGER : WRITE_TRIGGER;
    if (AuthTcpCreateListener(module, fd, triggerMode) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "AddTrigger fail.");
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    if (ConnSetTcpKeepalive(fd, (int32_t)DEFAULT_FREQ_CYCLE, TCP_KEEPALIVE_INTERVAL, TCP_KEEPALIVE_DEFAULT_COUNT) !=
        SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set tcp keep alive fail.");
        (void)DelTrigger(module, fd, triggerMode);
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    return fd;
}

int32_t SocketConnectDeviceWithAllIp(const char *localIp, const char *peerIp, int32_t port, bool isBlockMode)
{
    return SocketConnectInner(localIp, peerIp, port, AUTH_RAW_P2P_CLIENT, isBlockMode);
}

int32_t SocketSetDevice(int32_t fd, bool isBlockMode)
{
    if (fd < 0) {
        AUTH_LOGE(AUTH_CONN, "ConnOpenClientSocket fail, fd=%{public}d", fd);
        return SOFTBUS_INVALID_FD;
    }
    TriggerType triggerMode = isBlockMode ? READ_TRIGGER : WRITE_TRIGGER;
    SoftbusBaseListener listener = {
        .onConnectEvent = OnConnectEvent,
        .onDataEvent = OnDataEvent,
    };
    if (StartBaseClient(AUTH, &listener) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "StartBaseClient fail.");
    }
    if (DelTrigger(AUTH_RAW_P2P_CLIENT, fd, RW_TRIGGER) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "DelTrigger fail.");
        ConnShutdownSocket(fd);
        return SOFTBUS_INVALID_FD;
    }
    if (AddTrigger(AUTH, fd, triggerMode) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "AddTrigger fail.");
        ConnShutdownSocket(fd);
        return SOFTBUS_INVALID_FD;
    }
    if (ConnSetTcpKeepalive(fd, (int32_t)DEFAULT_FREQ_CYCLE, TCP_KEEPALIVE_INTERVAL, TCP_KEEPALIVE_DEFAULT_COUNT) !=
        SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set tcp keep alive fail.");
        (void)DelTrigger(AUTH, fd, triggerMode);
        ConnShutdownSocket(fd);
        return SOFTBUS_INVALID_FD;
    }
    int32_t ipTos = TCP_KEEPALIVE_TOS_VAL;
    if (SoftBusSocketSetOpt(fd, SOFTBUS_IPPROTO_IP_, SOFTBUS_IP_TOS_, &ipTos, sizeof(ipTos)) != SOFTBUS_ADAPTER_OK) {
        AUTH_LOGE(AUTH_CONN, "set option fail.");
        (void)DelTrigger(AUTH, fd, triggerMode);
        ConnShutdownSocket(fd);
        return SOFTBUS_INVALID_FD;
    }
    return SOFTBUS_OK;
}

static ConnectOption GetConnectOptionByIfname(int32_t ifnameIdx, int32_t port)
{
    ConnectOption option = {
        .type = CONNECT_TCP, .socketOption = { .addr = "", .port = port, .moduleId = AUTH, .protocol = LNN_PROTOCOL_IP }
    };
    if (ifnameIdx == USB_IF) {
        option.socketOption.moduleId = AUTH_USB;
        option.socketOption.protocol = LNN_PROTOCOL_USB;
    }
    return option;
}

int32_t SocketConnectDevice(const char *ip, int32_t port, bool isBlockMode, int32_t ifnameIdx)
{
    CHECK_NULL_PTR_RETURN_VALUE(ip, AUTH_INVALID_FD);
    char localIp[MAX_ADDR_LEN] = { 0 };
    if (LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_IP, localIp, MAX_ADDR_LEN, ifnameIdx) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "get local ip fail.");
        return AUTH_INVALID_FD;
    }
    ConnectOption option = GetConnectOptionByIfname(ifnameIdx, port);
    if (ifnameIdx == USB_IF) {
        if (LnnGetLocalStrInfoByIfnameIdx(STRING_KEY_IP6_WITH_IF, localIp, MAX_ADDR_LEN, USB_IF) != SOFTBUS_OK) {
            AUTH_LOGE(AUTH_CONN, "get local ip failed");
            return SOFTBUS_NETWORK_GET_NODE_INFO_ERR;
        }
    }
    if (strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), ip) != EOK) {
        AUTH_LOGE(AUTH_CONN, "copy remote ip fail.");
        return AUTH_INVALID_FD;
    }
    int32_t ret = ConnOpenClientSocket(&option, localIp, !isBlockMode);
    if (ret < 0) {
        AUTH_LOGE(AUTH_CONN, "ConnOpenClientSocket fail, error=%{public}d", ret);
        return ret;
    }
    int32_t fd = ret;
    TriggerType triggerMode = isBlockMode ? READ_TRIGGER : WRITE_TRIGGER;
    ListenerModule module = (ifnameIdx == USB_IF) ? AUTH_USB : AUTH;
    if (AddTrigger(module, fd, triggerMode) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "AddTrigger fail.");
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    if (ConnSetTcpKeepalive(fd, (int32_t)DEFAULT_FREQ_CYCLE, TCP_KEEPALIVE_INTERVAL, TCP_KEEPALIVE_DEFAULT_COUNT) !=
        SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set tcp keep alive fail.");
        (void)DelTrigger(module, fd, triggerMode);
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    int32_t ipTos = TCP_KEEPALIVE_TOS_VAL;
    if (SoftBusSocketSetOpt(fd, SOFTBUS_IPPROTO_IP_, SOFTBUS_IP_TOS_, &ipTos, sizeof(ipTos)) != SOFTBUS_ADAPTER_OK) {
        AUTH_LOGE(AUTH_CONN, "set option fail.");
        (void)DelTrigger(module, fd, triggerMode);
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    return fd;
}

int32_t NipSocketConnectDevice(ListenerModule module, const char *addr, int32_t port, bool isBlockMode)
{
    if (addr == NULL) {
        AUTH_LOGE(AUTH_CONN, "addr is invalid param.");
        return AUTH_INVALID_FD;
    }
    ConnectOption option = {
        .type = CONNECT_TCP,
        .socketOption = { .addr = "", .port = port, .moduleId = module, .protocol = LNN_PROTOCOL_NIP }
    };
    if (strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), addr) != EOK) {
        AUTH_LOGE(AUTH_CONN, "copy remote ip fail.");
        return AUTH_INVALID_FD;
    }
    int32_t fd = ConnOpenClientSocket(&option, BIND_ADDR_ALL, !isBlockMode);
    if (fd < 0) {
        AUTH_LOGE(AUTH_CONN, "ConnOpenClientSocket fail.");
        return AUTH_INVALID_FD;
    }
    TriggerType triggerMode = isBlockMode ? READ_TRIGGER : WRITE_TRIGGER;
    if (AddTrigger(module, fd, triggerMode) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "AddTrigger fail.");
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    if (ConnSetTcpKeepalive(fd, (int32_t)DEFAULT_FREQ_CYCLE, TCP_KEEPALIVE_INTERVAL, TCP_KEEPALIVE_DEFAULT_COUNT) !=
        SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set tcp keep alive fail.");
        (void)DelTrigger(module, fd, triggerMode);
        ConnShutdownSocket(fd);
        return AUTH_INVALID_FD;
    }
    return fd;
}

void SocketDisconnectDevice(ListenerModule module, int32_t fd)
{
    if (fd < 0) {
        AUTH_LOGD(AUTH_CONN, "invalid fd, maybe has shutdown. fd=%{public}d", fd);
        return;
    }
    (void)DelTrigger(module, fd, RW_TRIGGER);
    ConnShutdownSocket(fd);
}

int32_t SocketPostBytes(int32_t fd, const AuthDataHead *head, const uint8_t *data)
{
    CHECK_NULL_PTR_RETURN_VALUE(head, SOFTBUS_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(data, SOFTBUS_INVALID_PARAM);
    uint32_t size = GetSocketPktSize(head->len);
    uint8_t *buf = (uint8_t *)SoftBusCalloc(size);
    if (buf == NULL) {
        AUTH_LOGE(AUTH_CONN, "malloc pkt err.");
        return SOFTBUS_MALLOC_ERR;
    }
    SocketPktHead pktHead = {
        .magic = MAGIC_NUMBER,
        .module = head->module,
        .seq = head->seq,
        .flag = head->flag,
        .len = head->len,
    };
    if (PackSocketPkt(&pktHead, data, buf, size) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "pack socket pkt fail.");
        SoftBusFree(buf);
        return SOFTBUS_AUTH_PACK_SOCKET_PKT_FAIL;
    }

    AUTH_LOGI(AUTH_CONN, "fd=%{public}d, module=%{public}d, seq=%{public}" PRId64 ", flag=%{public}d, len=%{public}u.",
        fd, pktHead.module, pktHead.seq, pktHead.flag, pktHead.len);
    ssize_t ret = ConnSendSocketData(fd, (const char *)buf, (size_t)size, 0);
    SoftBusFree(buf);
    if (ret != (ssize_t)size) {
        AUTH_LOGE(AUTH_CONN, "fail. ret=%{public}zd", ret);
        return SOFTBUS_TCP_SOCKET_ERR;
    }
    return SOFTBUS_OK;
}

int32_t SocketGetConnInfo(int32_t fd, AuthConnInfo *connInfo, bool *isServer, int32_t ifnameIdx)
{
    CHECK_NULL_PTR_RETURN_VALUE(connInfo, SOFTBUS_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(isServer, SOFTBUS_INVALID_PARAM);
    SocketAddr socket;
    if (ifnameIdx == USB_IF) {
        if (ConnGetPeerSocketAddr6(fd, &socket) != SOFTBUS_OK) {
            AUTH_LOGE(AUTH_CONN, "fail, fd=%{public}d.", fd);
            return SOFTBUS_AUTH_GET_PEER_SOCKET_ADDR_FAIL;
        }
    } else {
        if (ConnGetPeerSocketAddr(fd, &socket) != SOFTBUS_OK) {
            AUTH_LOGE(AUTH_CONN, "fail, fd=%{public}d.", fd);
            return SOFTBUS_AUTH_GET_PEER_SOCKET_ADDR_FAIL;
        }
    }
    int32_t localPort = ConnGetLocalSocketPort(fd);
    if (localPort <= 0) {
        AUTH_LOGE(AUTH_CONN, "fail, fd=%{public}d.", fd);
        return SOFTBUS_INVALID_PORT;
    }
    connInfo->type = AUTH_LINK_TYPE_WIFI;
    if (ifnameIdx == USB_IF) {
        connInfo->type = AUTH_LINK_TYPE_USB;
    }
    if (strcpy_s(connInfo->info.ipInfo.ip, sizeof(connInfo->info.ipInfo.ip), socket.addr) != EOK) {
        AUTH_LOGE(AUTH_CONN, "copy ip fail, fd=%{public}d.", fd);
        return SOFTBUS_MEM_ERR;
    }
    connInfo->info.ipInfo.port = socket.port;
    int32_t serverPort = 0;
    if (LnnGetLocalNumInfoByIfnameIdx(NUM_KEY_AUTH_PORT, &serverPort, ifnameIdx) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "get local auth port fail.");
    }
    *isServer = (serverPort != localPort);
    return SOFTBUS_OK;
}

/* Auth Channel */
static void NotifyChannelDataReceived(int32_t channelId, const SocketPktHead *head, const uint8_t *data)
{
    uint32_t i;
    AuthChannelListener *listener = NULL;
    for (i = 0; i < sizeof(g_listener) / sizeof(InnerChannelListener); i++) {
        if (g_listener[i].module == head->module) {
            listener = &g_listener[i].listener;
            break;
        }
    }
    if (listener == NULL || listener->onDataReceived == NULL) {
        AUTH_LOGE(AUTH_CONN, "AuthChannelListener not set.");
        return;
    }

    AuthChannelData channelData = { 0 };
    channelData.module = head->module;
    channelData.seq = head->seq;
    channelData.flag = head->flag;
    channelData.len = head->len;
    channelData.data = data;
    listener->onDataReceived(channelId, &channelData);
}

static void NotifyChannelDisconnected(int32_t channelId)
{
    uint32_t i;
    for (i = 0; i < sizeof(g_listener) / sizeof(InnerChannelListener); i++) {
        if (g_listener[i].listener.onDisconnected != NULL) {
            g_listener[i].listener.onDisconnected(channelId);
        }
    }
}

int32_t RegAuthChannelListener(int32_t module, const AuthChannelListener *listener)
{
    if (listener == NULL || listener->onDataReceived == NULL) {
        AUTH_LOGE(AUTH_CONN, "invalid listener.");
        return SOFTBUS_INVALID_PARAM;
    }
    uint32_t i;
    for (i = 0; i < sizeof(g_listener) / sizeof(InnerChannelListener); i++) {
        if (g_listener[i].module == module) {
            g_listener[i].listener.onDataReceived = listener->onDataReceived;
            g_listener[i].listener.onDisconnected = listener->onDisconnected;
            return SOFTBUS_OK;
        }
    }
    AUTH_LOGE(AUTH_CONN, "unknown module. module=%{public}d", module);
    return SOFTBUS_INVALID_PARAM;
}

void UnregAuthChannelListener(int32_t module)
{
    uint32_t i;
    for (i = 0; i < sizeof(g_listener) / sizeof(InnerChannelListener); i++) {
        if (g_listener[i].module == module) {
            g_listener[i].listener.onDataReceived = NULL;
            g_listener[i].listener.onDisconnected = NULL;
            return;
        }
    }
}

int32_t AuthOpenChannelWithAllIp(const char *localIp, const char *remoteIp, int32_t port)
{
    if (localIp == NULL || remoteIp == NULL || port <= 0) {
        AUTH_LOGE(AUTH_CONN, "invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    int32_t fd = SocketConnectDeviceWithAllIp(localIp, remoteIp, port, true);
    if (fd < 0) {
        AUTH_LOGE(AUTH_CONN, "connect fail.");
        return INVALID_CHANNEL_ID;
    }
    AUTH_LOGI(AUTH_CONN, "open auth channel succ, channelId=%{public}d.", fd);
    return fd;
}

int32_t AuthOpenChannel(const char *ip, int32_t port, int32_t ifnameIdx)
{
    if (ip == NULL || port <= 0) {
        AUTH_LOGE(AUTH_CONN, "invalid param.");
        return INVALID_CHANNEL_ID;
    }
    int32_t fd = SocketConnectDevice(ip, port, true, ifnameIdx);
    if (fd < 0) {
        AUTH_LOGE(AUTH_CONN, "connect fail.");
        return INVALID_CHANNEL_ID;
    }
    AUTH_LOGI(AUTH_CONN, "open auth channel succ, channelId=%{public}d.", fd);
    return fd;
}

void AuthCloseChannel(int32_t channelId, int32_t moduleId)
{
    AUTH_LOGI(AUTH_CONN, "close auth channel, moduleId=%{public}d, id=%{public}d.", moduleId, channelId);
    SocketDisconnectDevice((ListenerModule)moduleId, channelId);
}

int32_t AuthPostChannelData(int32_t channelId, const AuthChannelData *data)
{
    if (channelId < 0 || data == NULL || data->data == NULL || data->len == 0) {
        AUTH_LOGE(AUTH_CONN, "invalid param, channelId=%{public}d.", channelId);
        return SOFTBUS_INVALID_PARAM;
    }
    AuthDataHead head = {
        .dataType = DATA_TYPE_CONNECTION,
        .module = data->module,
        .seq = data->seq,
        .flag = data->flag,
        .len = data->len,
    };
    return SocketPostBytes(channelId, &head, data->data);
}

static int32_t GetTcpKeepaliveOptionByCycle(ModeCycle cycle, TcpKeepaliveOption *tcpKeepaliveOption)
{
    if (tcpKeepaliveOption == NULL) {
        AUTH_LOGE(AUTH_CONN, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    switch (cycle) {
        case HIGH_FREQ_CYCLE:
            tcpKeepaliveOption->keepaliveCount = TCP_KEEPALIVE_HIGH_COUNT;
            tcpKeepaliveOption->userTimeout = TCP_KEEPALIVE_HIGH_USER_TIMEOUT;
            break;
        case MID_FREQ_CYCLE:
            tcpKeepaliveOption->keepaliveCount = TCP_KEEPALIVE_MID_COUNT;
            tcpKeepaliveOption->userTimeout = TCP_KEEPALIVE_MID_USER_TIMEOUT;
            break;
        case LOW_FREQ_CYCLE:
            tcpKeepaliveOption->keepaliveCount = TCP_KEEPALIVE_LOW_COUNT;
            tcpKeepaliveOption->userTimeout = TCP_KEEPALIVE_LOW_USER_TIMEOUT;
            break;
        case DEFAULT_FREQ_CYCLE:
            tcpKeepaliveOption->keepaliveCount = TCP_KEEPALIVE_DEFAULT_COUNT;
            tcpKeepaliveOption->userTimeout = TCP_KEEPALIVE_DEFAULT_USER_TIMEOUT;
            break;
        default:
            AUTH_LOGE(AUTH_CONN, "no match cycle, cycle=%{public}d", cycle);
            return SOFTBUS_INVALID_PARAM;
    }
    tcpKeepaliveOption->keepaliveIdle = (int32_t)cycle;
    tcpKeepaliveOption->keepaliveIntvl = TCP_KEEPALIVE_INTERVAL;
    return SOFTBUS_OK;
}

int32_t AuthSetTcpKeepaliveOption(int32_t fd, ModeCycle cycle)
{
    if (fd <= 0 || cycle < HIGH_FREQ_CYCLE || cycle > DEFAULT_FREQ_CYCLE) {
        AUTH_LOGE(AUTH_CONN, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    TcpKeepaliveOption tcpKeepaliveOption = { 0 };
    int32_t ret = GetTcpKeepaliveOptionByCycle(cycle, &tcpKeepaliveOption);
    if (ret != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "get tcp keepalive option by cycle fail");
        return ret;
    }
    if (ConnSetTcpUserTimeOut(fd, tcpKeepaliveOption.userTimeout) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set TCP_USER_TIMEOUT fail, fd=%{public}d", fd);
        return SOFTBUS_ADAPTER_ERR;
    }
    if (ConnSetTcpKeepalive(fd, tcpKeepaliveOption.keepaliveIdle, tcpKeepaliveOption.keepaliveIntvl,
            tcpKeepaliveOption.keepaliveCount) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "set tcp keepalive fail, fd=%{public}d", fd);
        return SOFTBUS_ADAPTER_ERR;
    }

    AUTH_LOGI(AUTH_CONN,
        "set tcp keepalive successful, fd=%{public}d, keepaliveIdle=%{public}d, keepaliveIntvl=%{public}d, "
        "keepaliveCount=%{public}d, userTimeout=%{public}u",
        fd, tcpKeepaliveOption.keepaliveIdle, tcpKeepaliveOption.keepaliveIntvl, tcpKeepaliveOption.keepaliveCount,
        tcpKeepaliveOption.userTimeout);
    return SOFTBUS_OK;
}

int32_t WifiConnListLockInit(void)
{
    if (SoftBusMutexInit(&g_wifiConnListLock, NULL) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "wifiConnList mutex init fail");
        return SOFTBUS_LOCK_ERR;
    }
    return SOFTBUS_OK;
}

void WifiConnListLockDeinit(void)
{
    if (SoftBusMutexDestroy(&g_wifiConnListLock) != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_CONN, "wifiConnList mutex destroy fail");
    }
}