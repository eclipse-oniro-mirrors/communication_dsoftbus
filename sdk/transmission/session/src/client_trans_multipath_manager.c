/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "client_trans_multipath_manager.h"

#include "softbus_error_code.h"
#include "trans_log.h"

#include "client_trans_channel_manager.h"
#include "client_trans_session_callback.h"
#include "client_trans_session_manager.h"
#include "client_trans_socket_manager.h"

#define LOW_BW               (384 * 1024)
#define TRANS_DEFAULT_MIN_BW 0

static uint32_t g_ctrlAsyncCloseCnt = 0;

int32_t TransMultipathGetEnabled(int32_t socket, bool *enabled)
{
    if (enabled == NULL) {
        TRANS_LOGE(TRANS_SDK, "invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (socket < 0) {
        TRANS_LOGE(TRANS_SDK, "invalid socket=%{public}d", socket);
        return SOFTBUS_TRANS_INVALID_SESSION_ID;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    if (GetSessionById(socket, &serverNode, &sessionNode) != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socket=%{public}d not found", socket);
        return SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND;
    }

    *enabled = sessionNode->enableMultipath;
    TRANS_LOGI(TRANS_SDK, "socket=%{public}d, enableMultipath=%{public}d", socket, *enabled);
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathSetEnabled(int32_t socket, bool enabled)
{
    if (socket < 0) {
        TRANS_LOGE(TRANS_INIT, "invalid socket=%{public}d", socket);
        return SOFTBUS_TRANS_INVALID_SESSION_ID;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    if (GetSessionById(socket, &serverNode, &sessionNode) != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socket=%{public}d not found", socket);
        return SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND;
    }

    sessionNode->enableMultipath = enabled;
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathSetStrategy(int32_t socket, MultipathStrategy strategy)
{
    if (socket <= 0) {
        TRANS_LOGE(TRANS_SDK, "invalid param");
        return SOFTBUS_INVALID_PARAM;
    }
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    if (GetSessionById(socket, &serverNode, &sessionNode) != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socket=%{public}d not found", socket);
        return SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND;
    }

    sessionNode->multipathStrategy = strategy;
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathCheckAndEnable(int32_t socket, const QosTV *qos, uint32_t qosCount)
{
    int32_t minBW = 0;
    bool enableMultipath = false;
    int32_t dataType = 0;
    int32_t ret = GetQosValue(qos, qosCount, QOS_TYPE_MIN_BW, &minBW, TRANS_DEFAULT_MIN_BW);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "get minBW failed, ret=%{public}d", ret);
        return ret;
    }

    if (minBW < 0) {
        TRANS_LOGE(TRANS_SDK, "invalid BW, minBW=%{public}d", minBW);
        return SOFTBUS_INVALID_PARAM;
    }
    ret = TransMultipathGetEnabled(socket, &enableMultipath);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "get enableMultipath failed, socket=%{public}d, ret=%{public}d", socket, ret);
        return ret;
    }

    ret = ClientGetDataTypeBySocket(socket, &dataType);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "get dataType failed, socket=%{public}d, ret=%{public}d", socket, ret);
        return ret;
    }
    if (enableMultipath) {
        TRANS_LOGI(TRANS_SDK, "set enableMultipath, socket=%{public}d", socket);
    }
    if (enableMultipath && (minBW <= LOW_BW || (dataType != DATA_TYPE_FILE && dataType != DATA_TYPE_BYTES))) {
        TRANS_LOGE(TRANS_SDK, "not multipath ability, minBW=%{public}d, dataType=%{public}d", minBW, dataType);
        TransMultipathSetEnabled(socket, false);
    }
    return SOFTBUS_OK;
}

int32_t TransMultipathSetChannel(int32_t sessionId, int32_t channelId, int32_t channelType)
{
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }
    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    if (GetSessionById(sessionId, &serverNode, &sessionNode) != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found", sessionId);
        return SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND;
    }
    if (sessionNode->lifecycle.sessionState == SESSION_STATE_CANCELLING) {
        int32_t bindErrCode = sessionNode->lifecycle.bindErrCode;
        TRANS_LOGW(TRANS_SDK, "socketFd=%{public}d already in cancelling state", sessionId);
        UnlockClientSessionServerList();
        return bindErrCode;
    }
    if (!sessionNode->enableMultipath || sessionNode->channelId == INVALID_CHANNEL_ID) {
        sessionNode->channelId = channelId;
        sessionNode->channelType = (ChannelType)channelType;
        sessionNode->lifecycle.sessionState = SESSION_STATE_OPENED;
    } else {
        sessionNode->channelIdReserve = channelId;
        sessionNode->channelTypeReserve = (ChannelType)channelType;
    }

    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

bool TransMultipathIsSessionActive(const char *sessionName, int32_t *multipathSessionId)
{
    if (sessionName == NULL || multipathSessionId == NULL) {
        TRANS_LOGW(TRANS_SDK, "Invalid param");
        return false;
    }
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return false;
    }
    ClientSessionServer *serverNode = NULL;
    ret = GetServerBySessionName(sessionName, &serverNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGI(TRANS_SDK, "current session is not a multiPath session");
        return false;
    }
    SessionInfo *sessionNode = NULL;
    LIST_FOR_EACH_ENTRY(sessionNode, &serverNode->sessionList, SessionInfo, node) {
        if (sessionNode->enableMultipath && sessionNode->channelId != INVALID_CHANNEL_ID) {
            *multipathSessionId = sessionNode->sessionId;
            UnlockClientSessionServerList();
            return true;
        }
    }
    UnlockClientSessionServerList();
    return false;
}

int32_t TransMultipathUpdateReserveChannel(int32_t sessionId, const ChannelInfo *channel)
{
    if (sessionId == INVALID_SESSION_ID || channel == NULL) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "can not find multi path socketId=%{public}d", sessionId);
        return ret;
    }
    if (sessionNode->channelId != INVALID_CHANNEL_ID && sessionNode->channelId != channel->channelId) {
        sessionNode->channelIdReserve = channel->channelId;
        sessionNode->channelTypeReserve = (ChannelType)channel->channelType;
        sessionNode->routeTypeReserve = channel->routeType;
        (void)TransMultipathMarkUpOnNode(sessionNode, channel->routeType);
    }
    TRANS_LOGI(TRANS_SDK,
        "mp socketId=%{public}d, channelId=%{public}d, channelIdReserve=%{public}d, routeType=%{public}d, "
        "routeTypeReserve=%{public}d",
        sessionId, sessionNode->channelId, sessionNode->channelIdReserve, sessionNode->routeType,
        sessionNode->routeTypeReserve);
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathGetReserveChannel(int32_t sessionId, int32_t *channelId, int32_t *channelType, int32_t *routeType)
{
    if (sessionId == INVALID_SESSION_ID || channelId == NULL || channelType == NULL || routeType == NULL) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found", sessionId);
        return ret;
    }
    *channelId = sessionNode->channelIdReserve;
    *channelType = sessionNode->channelTypeReserve;
    *routeType = sessionNode->routeTypeReserve;
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathGetRouteType(int32_t sessionId, int32_t *mainRouteType, int32_t *reserveRouteType)
{
    if (sessionId == INVALID_SESSION_ID || mainRouteType == NULL || reserveRouteType == NULL) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found", sessionId);
        return ret;
    }
    *mainRouteType = sessionNode->routeType;
    *reserveRouteType = sessionNode->routeTypeReserve;
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathClearReserveChannel(int32_t sessionId)
{
    if (sessionId == INVALID_SESSION_ID) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found", sessionId);
        return ret;
    }
    sessionNode->channelIdReserve = INVALID_CHANNEL_ID;
    sessionNode->channelTypeReserve = CHANNEL_TYPE_UNDEFINED;
    sessionNode->routeTypeReserve = -1;
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

int32_t TransMultipathGetChannelRole(int32_t sessionId, int32_t channelId, ChannelUseChooseState *useType)
{
    if (channelId == INVALID_CHANNEL_ID || sessionId == INVALID_SESSION_ID || useType == NULL) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found", sessionId);
        return ret;
    }
    if (sessionNode->channelId == channelId) {
        *useType = CHANNEL_USE_CHOOSE_FIRST;
    } else if (sessionNode->channelIdReserve == channelId) {
        *useType = CHANNEL_USE_CHOOSE_SECOND;
    } else {
        TRANS_LOGE(TRANS_SDK, "Invalid compare param");
        UnlockClientSessionServerList();
        return SOFTBUS_INVALID_PARAM;
    }
    UnlockClientSessionServerList();
    return SOFTBUS_OK;
}

bool TransMultipathNeedDelReserve(SessionInfo *sessionNode, int32_t routeType, bool *onlyReserveLinkDown)
{
    if (sessionNode == NULL || routeType == INVALID_ROUTE_TYPE) {
        TRANS_LOGW(TRANS_SDK, "Invalid param");
        return false;
    }
    if (!sessionNode->enableMultipath) {
        return false;
    }
    if ((sessionNode->routeType == routeType && sessionNode->routeTypeReserve != INVALID_ROUTE_TYPE) ||
        sessionNode->routeTypeReserve == routeType) {
        TRANS_LOGI(TRANS_SDK, "sessionId=%{public}d, type1=%{public}d, type2=%{public}d, routeType=%{public}d",
            sessionNode->sessionId, sessionNode->routeType, sessionNode->routeTypeReserve, routeType);
        if (onlyReserveLinkDown != NULL) {
            *onlyReserveLinkDown = sessionNode->routeType == routeType ? false : true;
        }
        return true;
    }
    return false;
}

bool TransMultipathDelReserveLinkDown(
    SessionInfo *sessionNode, const ClientSessionServer *server, ListNode *destroyList, int32_t routeType)
{
    if (sessionNode == NULL || destroyList == NULL) {
        TRANS_LOGW(TRANS_SDK, "Invalid param");
        return false;
    }
    bool onlyReserveLinkDown = true;
    if (!TransMultipathNeedDelReserve(sessionNode, routeType, &onlyReserveLinkDown)) {
        return false;
    }
    int32_t deadChannelId = onlyReserveLinkDown ? sessionNode->channelIdReserve : sessionNode->channelId;
    (void)TransMultipathMarkDownOnNode(sessionNode, deadChannelId);
    bool otherUp =
        (sessionNode->ctrlWirelessStatus == CTRL_CH_STATUS_UP) || (sessionNode->ctrlWiredStatus == CTRL_CH_STATUS_UP);
    if (!otherUp) {
        return false;
    }
    if (!onlyReserveLinkDown) {
        int32_t tmpId = sessionNode->channelId;
        sessionNode->channelId = sessionNode->channelIdReserve;
        sessionNode->channelIdReserve = tmpId;
        ChannelType tmpType = sessionNode->channelType;
        sessionNode->channelType = sessionNode->channelTypeReserve;
        sessionNode->channelTypeReserve = tmpType;
        int32_t tmpRoute = sessionNode->routeType;
        sessionNode->routeType = sessionNode->routeTypeReserve;
        sessionNode->routeTypeReserve = tmpRoute;
    }
    DestroySessionInfo *destroyNode = CreateDestroySessionNode(sessionNode, server, true);
    if (destroyNode != NULL) {
        (void)TransMultipathClearChannelSlotOnNode(sessionNode, deadChannelId);
        ListAdd(destroyList, &destroyNode->node);
    }
    return true;
}

void TransMultipathUpdateClosingState(SessionInfo *sessionNode, bool isClosing)
{
    if (sessionNode == NULL) {
        return;
    }
    if (sessionNode->enableMultipath) {
        sessionNode->isClosingReserve = isClosing;
    }
}

SOFTBUS_VISIBILITY void TransMultipathOnEvent(int32_t channelId, int32_t channelType, uint8_t changeType, int32_t linkType, int32_t reason)
{
    if (channelId == INVALID_CHANNEL_ID) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return;
    }
    MultipathEvent eventData = {
        .transitionType = changeType ? TRANSITION_TO_DUAL_PATH : TRANSITION_TO_SINGLE_PATH,
        .linkMediumType = (LinkMediumType)linkType,
        .reason = reason
    };
    TRANS_LOGI(TRANS_SDK,
        "handle on event, channelId=%{public}d, channelType=%{public}d, transitionType=%{public}d, "
        "linkMediumType=%{public}d, reason=%{public}d",
        channelId, channelType, eventData.transitionType, eventData.linkMediumType, eventData.reason);
    int32_t ret = GetClientSessionCb()->OnEvent(
        channelId, channelType, EVENT_TYPE_MULTIPATH, (const void *)&eventData, sizeof(MultipathEvent));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "handle on event error, ret=%{public}d", ret);
        return;
    }
    TRANS_LOGI(TRANS_SDK, "handle on event success");
}

SOFTBUS_VISIBILITY bool TransMultipathIsWiredRoute(int32_t routeType)
{
    return routeType == WIFI_USB;
}

int32_t TransMultipathSelectCtrlChannelOnNode(const SessionInfo *node, int32_t *channelId, int32_t *channelType)
{
    if (node == NULL || channelId == NULL || channelType == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (!node->enableMultipath) {
        *channelId = node->channelId;
        *channelType = node->channelType;
        return SOFTBUS_OK;
    }
    bool usbInPrimary = (node->routeType == WIFI_USB);
    bool usbInReserve = (node->routeTypeReserve == WIFI_USB);
    if (usbInPrimary && node->ctrlWiredStatus == CTRL_CH_STATUS_UP) {
        *channelId = node->channelId;
        *channelType = node->channelType;
        return SOFTBUS_OK;
    }
    if (usbInReserve && node->ctrlWiredStatus == CTRL_CH_STATUS_UP) {
        *channelId = node->channelIdReserve;
        *channelType = node->channelTypeReserve;
        return SOFTBUS_OK;
    }
    // no USB path or USB unavailable -> take wireless path (the non-USB slot)
    if (node->ctrlWirelessStatus == CTRL_CH_STATUS_UP) {
        if (usbInPrimary) {
            *channelId = node->channelIdReserve;
            *channelType = node->channelTypeReserve;
        } else {
            *channelId = node->channelId;
            *channelType = node->channelType;
        }
        return SOFTBUS_OK;
    }
    return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
}

static void TransMultipathMarkDownByRoute(SessionInfo *node, int32_t routeType)
{
    if (TransMultipathIsWiredRoute(routeType)) {
        node->ctrlWiredStatus = CTRL_CH_STATUS_DOWN;
    } else {
        node->ctrlWirelessStatus = CTRL_CH_STATUS_DOWN;
    }
}

int32_t TransMultipathMarkDownOnNode(SessionInfo *node, int32_t channelId)
{
    if (node == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (node->channelId == channelId) {
        TransMultipathMarkDownByRoute(node, node->routeType);
        return SOFTBUS_OK;
    }
    if (node->channelIdReserve == channelId) {
        TransMultipathMarkDownByRoute(node, node->routeTypeReserve);
        return SOFTBUS_OK;
    }
    return SOFTBUS_INVALID_PARAM;
}

int32_t TransMultipathClearChannelSlotOnNode(SessionInfo *node, int32_t channelId)
{
    if (node == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (node->channelId == channelId) {
        node->channelId = INVALID_CHANNEL_ID;
        node->channelType = CHANNEL_TYPE_UNDEFINED;
        node->routeType = INVALID_ROUTE_TYPE;
        return SOFTBUS_OK;
    }
    if (node->channelIdReserve == channelId) {
        node->channelIdReserve = INVALID_CHANNEL_ID;
        node->channelTypeReserve = CHANNEL_TYPE_UNDEFINED;
        node->routeTypeReserve = INVALID_ROUTE_TYPE;
        return SOFTBUS_OK;
    }
    return SOFTBUS_INVALID_PARAM;
}

int32_t TransMultipathGetOtherCtrlChannelOnNode(
    const SessionInfo *node, int32_t failChannelId, int32_t *altChannelId, int32_t *altChannelType)
{
    if (node == NULL || altChannelId == NULL || altChannelType == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    bool failIsPrimary = (node->channelId == failChannelId);
    bool failIsReserve = (node->channelIdReserve == failChannelId);
    if (!failIsPrimary && !failIsReserve) {
        return SOFTBUS_INVALID_PARAM;
    }
    int32_t otherId = failIsPrimary ? node->channelIdReserve : node->channelId;
    int32_t otherType = failIsPrimary ? node->channelTypeReserve : node->channelType;
    int32_t otherRoute = failIsPrimary ? node->routeTypeReserve : node->routeType;
    CtrlChannelStatus otherStatus =
        TransMultipathIsWiredRoute(otherRoute) ? node->ctrlWiredStatus : node->ctrlWirelessStatus;
    if (otherStatus != CTRL_CH_STATUS_UP) {
        return SOFTBUS_TRANS_INVALID_CHANNEL_TYPE;
    }
    *altChannelId = otherId;
    *altChannelType = otherType;
    return SOFTBUS_OK;
}

SOFTBUS_VISIBILITY int32_t TransMultipathMarkUpOnNode(SessionInfo *node, int32_t routeType)
{
    if (node == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    if (TransMultipathIsWiredRoute(routeType)) {
        node->ctrlWiredStatus = CTRL_CH_STATUS_UP;
    } else {
        node->ctrlWirelessStatus = CTRL_CH_STATUS_UP;
    }
    return SOFTBUS_OK;
}

int32_t TransMultipathSelectCtrlChannel(int32_t sessionId, int32_t *channelId, int32_t *channelType)
{
    if (channelId == NULL || channelType == NULL) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found, ret=%{public}d", sessionId, ret);
        return ret;
    }
    ret = TransMultipathSelectCtrlChannelOnNode(sessionNode, channelId, channelType);
    UnlockClientSessionServerList();
    return ret;
}

int32_t TransMultipathGetOtherCtrlChannel(
    int32_t sessionId, int32_t failChannelId, int32_t *altChannelId, int32_t *altChannelType)
{
    if (altChannelId == NULL || altChannelType == NULL) {
        TRANS_LOGE(TRANS_SDK, "Invalid param");
        return SOFTBUS_INVALID_PARAM;
    }

    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found, ret=%{public}d", sessionId, ret);
        return ret;
    }
    ret = TransMultipathGetOtherCtrlChannelOnNode(sessionNode, failChannelId, altChannelId, altChannelType);
    UnlockClientSessionServerList();
    return ret;
}

int32_t TransMultipathMarkDown(int32_t sessionId, int32_t channelId)
{
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }

    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found, ret=%{public}d", sessionId, ret);
        return ret;
    }
    ret = TransMultipathMarkDownOnNode(sessionNode, channelId);
    UnlockClientSessionServerList();
    return ret;
}

bool TransMultipathIsOtherUp(int32_t sessionId, int32_t channelId)
{
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        return false;
    }
    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        return false;
    }
    if (!sessionNode->enableMultipath) {
        UnlockClientSessionServerList();
        return false;
    }
    bool isPrimary = (sessionNode->channelId == channelId);
    bool isReserve = (sessionNode->channelIdReserve == channelId);
    if (!isPrimary && !isReserve) {
        UnlockClientSessionServerList();
        return false;
    }
    int32_t otherRoute = isPrimary ? sessionNode->routeTypeReserve : sessionNode->routeType;
    CtrlChannelStatus otherStatus =
        TransMultipathIsWiredRoute(otherRoute) ? sessionNode->ctrlWiredStatus : sessionNode->ctrlWirelessStatus;
    UnlockClientSessionServerList();
    return otherStatus == CTRL_CH_STATUS_UP;
}

SOFTBUS_VISIBILITY bool TransMultipathHandleChannelClose(int32_t sessionId, int32_t channelId,
    int32_t *surviveChannelId, int32_t *surviveChannelType, int32_t *deadRouteType)
{
    if (surviveChannelId == NULL || surviveChannelType == NULL || deadRouteType == NULL) {
        return false;
    }
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        return false;
    }
    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        return false;
    }
    if (!sessionNode->enableMultipath) {
        UnlockClientSessionServerList();
        return false;
    }
    bool isPrimary = (sessionNode->channelId == channelId);
    bool isReserve = (sessionNode->channelIdReserve == channelId);
    if (!isPrimary && !isReserve) {
        UnlockClientSessionServerList();
        return false;
    }
    *deadRouteType = isPrimary ? sessionNode->routeType : sessionNode->routeTypeReserve;
    (void)TransMultipathMarkDownOnNode(sessionNode, channelId);
    int32_t otherRoute = isPrimary ? sessionNode->routeTypeReserve : sessionNode->routeType;
    CtrlChannelStatus otherStatus =
        TransMultipathIsWiredRoute(otherRoute) ? sessionNode->ctrlWiredStatus : sessionNode->ctrlWirelessStatus;
    if (otherStatus != CTRL_CH_STATUS_UP) {
        UnlockClientSessionServerList();
        return false;
    }
    *surviveChannelId = isPrimary ? sessionNode->channelIdReserve : sessionNode->channelId;
    *surviveChannelType = isPrimary ? sessionNode->channelTypeReserve : sessionNode->channelType;
    (void)TransMultipathClearChannelSlotOnNode(sessionNode, channelId);
    UnlockClientSessionServerList();
    TRANS_LOGI(TRANS_SDK, "multipath one channel closed, session preserved, sessionId=%{public}d, "
        "deadChannelId=%{public}d, surviveChannelId=%{public}d",
        sessionId, channelId, *surviveChannelId);
    return true;
}

int32_t TransMultipathCloseDeadChannel(int32_t sessionId, int32_t channelId, int32_t channelType, bool async)
{
    int32_t ret = LockClientSessionServerList();
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed, ret=%{public}d", ret);
        return ret;
    }
    ClientSessionServer *serverNode = NULL;
    SessionInfo *sessionNode = NULL;
    ret = GetSessionById(sessionId, &serverNode, &sessionNode);
    if (ret != SOFTBUS_OK) {
        UnlockClientSessionServerList();
        TRANS_LOGE(TRANS_SDK, "socketFd=%{public}d not found, ret=%{public}d", sessionId, ret);
        return SOFTBUS_TRANS_SESSION_INFO_NOT_FOUND;
    }
    int32_t socketId = sessionNode->sessionId;
    (void)TransMultipathClearChannelSlotOnNode(sessionNode, channelId);
    if (async) {
        g_ctrlAsyncCloseCnt++;
    }
    UnlockClientSessionServerList();
    TRANS_LOGI(TRANS_SDK, "close dead channel, sessionId=%{public}d, channelId=%{public}d, async=%{public}d",
        sessionId, channelId, async);
    return ClientTransCloseChannel(channelId, channelType, socketId, async);
}

uint32_t TransMultipathGetAsyncCloseCount(void)
{
    return g_ctrlAsyncCloseCnt;
}
