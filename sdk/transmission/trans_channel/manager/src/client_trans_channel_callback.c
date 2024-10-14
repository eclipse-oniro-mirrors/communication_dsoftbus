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

#include "client_trans_channel_callback.h"

#include "client_trans_auth_manager.h"
#include "client_trans_proxy_manager.h"
#include "client_trans_session_manager.h"
#include "client_trans_socket_manager.h"
#include "client_trans_statistics.h"
#include "client_trans_tcp_direct_manager.h"
#include "client_trans_tcp_direct_callback.h"
#include "client_trans_udp_manager.h"
#include "session.h"
#include "softbus_errcode.h"
#include "trans_log.h"

int32_t TransOnChannelOpened(const char *sessionName, const ChannelInfo *channel)
{
    if (sessionName == NULL || channel == NULL) {
        TRANS_LOGW(TRANS_SDK, "[client] invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = SOFTBUS_NO_INIT;
    int32_t udpPort = 0;
    switch (channel->channelType) {
        case CHANNEL_TYPE_AUTH:
            ret = ClientTransAuthOnChannelOpened(sessionName, channel);
            break;
        case CHANNEL_TYPE_PROXY:
            ret = ClientTransProxyOnChannelOpened(sessionName, channel);
            break;
        case CHANNEL_TYPE_TCP_DIRECT:
            ret = ClientTransTdcOnChannelOpened(sessionName, channel);
            break;
        case CHANNEL_TYPE_UDP:
            ret = TransOnUdpChannelOpened(sessionName, channel, &udpPort);
            break;
        default:
            TRANS_LOGE(TRANS_SDK, "[client] invalid type.");
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }

    AddSocketResource(sessionName, channel);
    if (channel->channelType == CHANNEL_TYPE_UDP && channel->isServer && udpPort > 0) {
        return udpPort;
    }

    return ret;
}

int32_t TransOnChannelOpenFailed(int32_t channelId, int32_t channelType, int32_t errCode)
{
    TRANS_LOGE(TRANS_SDK,
        "[client]: channelId=%{public}d, channelType=%{public}d, errCode=%{public}d",
        channelId, channelType, errCode);
    switch (channelType) {
        case CHANNEL_TYPE_AUTH:
            return ClientTransAuthOnChannelOpenFailed(channelId, errCode);
        case CHANNEL_TYPE_PROXY:
            return ClientTransProxyOnChannelOpenFailed(channelId, errCode);
        case CHANNEL_TYPE_TCP_DIRECT:
            return ClientTransTdcOnChannelOpenFailed(channelId, errCode);
        case CHANNEL_TYPE_UDP:
            return TransOnUdpChannelOpenFailed(channelId, errCode);
        case CHANNEL_TYPE_UNDEFINED:
            // channelId is sessionId
            return GetClientSessionCb()->OnSessionOpenFailed(channelId, channelType, errCode);
        default:
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
}

int32_t TransOnChannelLinkDown(const char *networkId, int32_t routeType)
{
    if (networkId == NULL) {
        TRANS_LOGE(TRANS_SDK, "[client] network id is null.");
        return SOFTBUS_INVALID_PARAM;
    }

    ClientTransOnLinkDown(networkId, routeType);
    return SOFTBUS_OK;
}

static int32_t NofifyChannelClosed(int32_t channelId, int32_t channelType, ShutdownReason reason)
{
    DeleteSocketResourceByChannelId(channelId, channelType);
    switch (channelType) {
        case CHANNEL_TYPE_AUTH:
            return ClientTransAuthOnChannelClosed(channelId, reason);
        case CHANNEL_TYPE_PROXY:
            return ClientTransProxyOnChannelClosed(channelId, reason);
        case CHANNEL_TYPE_UDP:
            return TransOnUdpChannelClosed(channelId, reason);
        case CHANNEL_TYPE_TCP_DIRECT:
            return ClientTransTdcOnSessionClosed(channelId, reason);
        default:
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
}

static int32_t NofifyCloseAckReceived(int32_t channelId, int32_t channelType)
{
    switch (channelType) {
        case CHANNEL_TYPE_UDP:
            return TransUdpOnCloseAckReceived(channelId);
        case CHANNEL_TYPE_AUTH:
        case CHANNEL_TYPE_PROXY:
        case CHANNEL_TYPE_TCP_DIRECT:
        default:
            TRANS_LOGI(TRANS_SDK, "recv unsupport channelType=%{public}d", channelType);
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
}

int32_t TransOnChannelClosed(int32_t channelId, int32_t channelType, int32_t messageType, ShutdownReason reason)
{
    TRANS_LOGI(TRANS_SDK,
        "channelId=%{public}d, channelType=%{public}d, messageType=%{public}d", channelId, channelType, messageType);
    switch (messageType) {
        case MESSAGE_TYPE_NOMAL:
            return NofifyChannelClosed(channelId, channelType, reason);
        case MESSAGE_TYPE_CLOSE_ACK:
            return NofifyCloseAckReceived(channelId, channelType);
        default:
            TRANS_LOGI(TRANS_SDK, "invalid messageType=%{public}d", messageType);
            return SOFTBUS_TRANS_INVALID_MESSAGE_TYPE;
    }
}

int32_t TransOnChannelMsgReceived(int32_t channelId, int32_t channelType,
    const void *data, unsigned int len, SessionPktType type)
{
    if (data == NULL) {
        TRANS_LOGE(TRANS_MSG, "param invalid");
        return SOFTBUS_INVALID_PARAM;
    }
    TRANS_LOGD(TRANS_MSG,
        "[client]: channelId=%{public}d, channelType=%{public}d", channelId, channelType);
    switch (channelType) {
        case CHANNEL_TYPE_AUTH:
            return ClientTransAuthOnDataReceived(channelId, data, len, type);
        case CHANNEL_TYPE_PROXY:
            return ClientTransProxyOnDataReceived(channelId, data, len, type);
        case CHANNEL_TYPE_TCP_DIRECT:
            return ClientTransTdcOnDataReceived(channelId, data, len, type);
        default:
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
}

int32_t TransOnChannelQosEvent(int32_t channelId, int32_t channelType, int32_t eventId,
    int32_t tvCount, const QosTv *tvList)
{
    if (tvList == NULL) {
        TRANS_LOGE(TRANS_MSG, "param invalid");
        return SOFTBUS_INVALID_PARAM;
    }
    TRANS_LOGI(TRANS_QOS,
        "[client] TransOnQosEvent: channelId=%{public}d, channelType=%{public}d, eventId=%{public}d", channelId,
        channelType, eventId);
    switch (channelType) {
        case CHANNEL_TYPE_UDP:
            return TransOnUdpChannelQosEvent(channelId, eventId, tvCount, tvList);
        default:
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
}

int32_t TransSetChannelInfo(const char* sessionName, int32_t sessionId, int32_t channleId, int32_t channelType)
{
    return ClientTransSetChannelInfo(sessionName, sessionId, channleId, channelType);
}

int32_t TransOnChannelBind(int32_t channelId, int32_t channelType)
{
    TRANS_LOGI(TRANS_SDK, "channelId=%{public}d, channelType=%{public}d", channelId, channelType);
    switch (channelType) {
        case CHANNEL_TYPE_PROXY:
            return ClientTransProxyOnChannelBind(channelId, channelType);
        case CHANNEL_TYPE_TCP_DIRECT:
            return ClientTransTdcOnChannelBind(channelId, channelType);
        case CHANNEL_TYPE_UDP:
            return TransOnUdpChannelBind(channelId, channelType);
        case CHANNEL_TYPE_AUTH:
        case CHANNEL_TYPE_UNDEFINED:
            TRANS_LOGW(TRANS_SDK, "The channel no need OnChannelBind");
            return SOFTBUS_OK;
        default:
            return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
    return SOFTBUS_OK;
}