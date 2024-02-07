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

#include "trans_client_proxy_standard.h"

#include "message_parcel.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_server_ipc_interface_code.h"
#include "trans_log.h"

#define WRITE_PARCEL_WITH_RET(parcel, type, data, retval)                              \
    do {                                                                               \
        if (!(parcel).Write##type(data)) {                                             \
            TRANS_LOGE(TRANS_SVC, "write data failed.");                               \
            return (retval);                                                           \
        }                                                                              \
    } while (false)

namespace OHOS {
int32_t TransClientProxy::OnClientPermissonChange(const char *pkgName, int32_t state)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed!");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(state)) {
        TRANS_LOGE(TRANS_CTRL, "write permStateChangeType failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteCString(pkgName)) {
        TRANS_LOGE(TRANS_CTRL, "write pkgName failed");
        return SOFTBUS_ERR;
    }
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(CLIENT_ON_PERMISSION_CHANGE, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL,
            "DataSyncPermissionChange send request failed, ret=%{public}d", ret);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t TransClientProxy::MessageParcelWrite(MessageParcel &data, const char *sessionName, const ChannelInfo *channel)
{
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed.");
        return SOFTBUS_ERR;
    }
    WRITE_PARCEL_WITH_RET(data, CString, sessionName, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->channelId, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->channelType, SOFTBUS_ERR);
    
    if (channel->channelType == CHANNEL_TYPE_TCP_DIRECT) {
        WRITE_PARCEL_WITH_RET(data, FileDescriptor, channel->fd, SOFTBUS_ERR);
    }
    WRITE_PARCEL_WITH_RET(data, Bool, channel->isServer, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Bool, channel->isEnabled, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Bool, channel->isEncrypt, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->peerUid, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->peerPid, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, CString, channel->groupId, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Uint32, channel->keyLen, SOFTBUS_ERR);
    if (!data.WriteRawData(channel->sessionKey, channel->keyLen)) {
        TRANS_LOGE(TRANS_CTRL, "write sessionKey and keyLen failed.");
        return SOFTBUS_ERR;
    }
    WRITE_PARCEL_WITH_RET(data, CString, channel->peerSessionName, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, CString, channel->peerDeviceId, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->businessType, SOFTBUS_ERR);
    if (channel->channelType == CHANNEL_TYPE_UDP) {
        WRITE_PARCEL_WITH_RET(data, CString, channel->myIp, SOFTBUS_ERR);
        WRITE_PARCEL_WITH_RET(data, Int32, channel->streamType, SOFTBUS_ERR);
        WRITE_PARCEL_WITH_RET(data, Bool, channel->isUdpFile, SOFTBUS_ERR);
        
        if (!channel->isServer) {
            WRITE_PARCEL_WITH_RET(data, Int32, channel->peerPort, SOFTBUS_ERR);
            WRITE_PARCEL_WITH_RET(data, CString, channel->peerIp, SOFTBUS_ERR);
        }
    }
    WRITE_PARCEL_WITH_RET(data, Int32, channel->routeType, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->encrypt, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->algorithm, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Int32, channel->crc, SOFTBUS_ERR);
    WRITE_PARCEL_WITH_RET(data, Uint32, channel->dataConfig, SOFTBUS_ERR);

    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnChannelOpened(const char *sessionName, const ChannelInfo *channel)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }
    MessageParcel data;
    if (MessageParcelWrite(data, sessionName, channel) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "message parcel write failed.");
        return SOFTBUS_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CLIENT_ON_CHANNEL_OPENED, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelOpened send request failed, ret=%{public}d", ret);
        return SOFTBUS_ERR;
    }
    int32_t serverRet;
    if (!reply.ReadInt32(serverRet)) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelOpened read serverRet failed");
        return SOFTBUS_ERR;
    }
    return serverRet;
}

int32_t TransClientProxy::OnChannelOpenFailed(int32_t channelId, int32_t channelType, int32_t errCode)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed!");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelId)) {
        TRANS_LOGE(TRANS_CTRL, "write channel id failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelType)) {
        TRANS_LOGE(TRANS_CTRL, "write channel type failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(errCode)) {
        TRANS_LOGE(TRANS_CTRL, "write error code failed");
        return SOFTBUS_ERR;
    }
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    int32_t ret = remote->SendRequest(CLIENT_ON_CHANNEL_OPENFAILED, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelOpenFailed send request failed, ret=%{public}d", ret);
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnChannelLinkDown(const char *networkId, int32_t routeType)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }
    if (networkId == nullptr) {
        TRANS_LOGW(TRANS_CTRL, "invalid parameters");
        return SOFTBUS_ERR;
    }
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed!");
        return SOFTBUS_ERR;
    }
    if (!data.WriteCString(networkId)) {
        TRANS_LOGE(TRANS_CTRL, "write networkId failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(routeType)) {
        TRANS_LOGE(TRANS_CTRL, "write routeType failed");
        return SOFTBUS_ERR;
    }
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    int32_t ret = remote->SendRequest(CLIENT_ON_CHANNEL_LINKDOWN, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelLinkDwon send request failed, ret=%{public}d", ret);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnChannelClosed(int32_t channelId, int32_t channelType)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed!");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelId)) {
        TRANS_LOGE(TRANS_CTRL, "write channel id failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelType)) {
        TRANS_LOGE(TRANS_CTRL, "write channel type failed");
        return SOFTBUS_ERR;
    }
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CLIENT_ON_CHANNEL_CLOSED, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelClosed send request failed, ret=%{public}d", ret);
        return SOFTBUS_ERR;
    }
    int32_t serverRet;
    if (!reply.ReadInt32(serverRet)) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelClosed read serverRet failed");
        return SOFTBUS_ERR;
    }
    return serverRet;
}

int32_t TransClientProxy::OnChannelMsgReceived(int32_t channelId, int32_t channelType, const void *dataInfo,
    uint32_t len, int32_t type)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed!");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelId)) {
        TRANS_LOGE(TRANS_CTRL, "write channel id failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelType)) {
        TRANS_LOGE(TRANS_CTRL, "write channel type failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteUint32(len)) {
        TRANS_LOGE(TRANS_CTRL, "write data len failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteRawData(dataInfo, len)) {
        TRANS_LOGE(TRANS_CTRL, "write (dataInfo, len) failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(type)) {
        TRANS_LOGE(TRANS_CTRL, "write data type failed");
        return SOFTBUS_ERR;
    }

    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    TRANS_LOGD(TRANS_CTRL, "SendRequest start");
    int32_t ret = remote->SendRequest(CLIENT_ON_CHANNEL_MSGRECEIVED, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelMsgReceived send request failed, ret=%{public}d",
            ret);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnChannelQosEvent(int32_t channelId, int32_t channelType, int32_t eventId, int32_t tvCount,
    const QosTv *tvList)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TRANS_LOGE(TRANS_CTRL, "remote is nullptr");
        return SOFTBUS_ERR;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TRANS_LOGE(TRANS_CTRL, "write InterfaceToken failed!");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelId)) {
        TRANS_LOGE(TRANS_CTRL, "write channel id failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(channelType)) {
        TRANS_LOGE(TRANS_CTRL, "write channel type failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(eventId)) {
        TRANS_LOGE(TRANS_CTRL, "write channel type failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteInt32(tvCount)) {
        TRANS_LOGE(TRANS_CTRL, "write tv count failed");
        return SOFTBUS_ERR;
    }
    if (!data.WriteRawData(tvList, sizeof(QosTv) * tvCount)) {
        TRANS_LOGE(TRANS_CTRL, "write tv list failed");
        return SOFTBUS_ERR;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(CLIENT_ON_CHANNEL_QOSEVENT, data, reply, option);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelQosEvent send request failed, ret=%{public}d", ret);
        return SOFTBUS_ERR;
    }
    int32_t serverRet;
    if (!reply.ReadInt32(serverRet)) {
        TRANS_LOGE(TRANS_CTRL, "OnChannelQosEvent read serverRet failed");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

void TransClientProxy::OnDeviceFound(const DeviceInfo *device)
{
    (void)device;
}

void TransClientProxy::OnDiscoverFailed(int subscribeId, int failReason)
{
    (void)subscribeId;
    (void)failReason;
}

void TransClientProxy::OnDiscoverySuccess(int subscribeId)
{
    (void)subscribeId;
}

void TransClientProxy::OnPublishSuccess(int publishId)
{
    (void)publishId;
}

void TransClientProxy::OnPublishFail(int publishId, int reason)
{
    (void)publishId;
    (void)reason;
}

int32_t TransClientProxy::OnJoinLNNResult(void *addr, uint32_t addrTypeLen, const char *networkId, int retCode)
{
    (void)addr;
    (void)addrTypeLen;
    (void)networkId;
    (void)retCode;
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnJoinMetaNodeResult(void *addr, uint32_t addrTypeLen, void *metaInfo,
    uint32_t infoLen, int retCode)
{
    (void)addr;
    (void)addrTypeLen;
    (void)metaInfo;
    (void)infoLen;
    (void)retCode;
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnLeaveLNNResult(const char *networkId, int retCode)
{
    (void)networkId;
    (void)retCode;
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnLeaveMetaNodeResult(const char *networkId, int retCode)
{
    (void)networkId;
    (void)retCode;
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnNodeOnlineStateChanged(const char *pkgName, bool isOnline,
    void *info, uint32_t infoTypeLen)
{
    (void)pkgName;
    (void)isOnline;
    (void)info;
    (void)infoTypeLen;
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnNodeBasicInfoChanged(const char *pkgName, void *info,
    uint32_t infoTypeLen, int32_t type)
{
    (void)pkgName;
    (void)info;
    (void)infoTypeLen;
    (void)type;
    return SOFTBUS_OK;
}

int32_t TransClientProxy::OnTimeSyncResult(const void *info, uint32_t infoTypeLen, int32_t retCode)
{
    (void)info;
    (void)infoTypeLen;
    (void)retCode;
    return SOFTBUS_OK;
}

void TransClientProxy::OnPublishLNNResult(int32_t publishId, int32_t reason)
{
    (void)publishId;
    (void)reason;
}

void TransClientProxy::OnRefreshLNNResult(int32_t refreshId, int32_t reason)
{
    (void)refreshId;
    (void)reason;
}

void TransClientProxy::OnRefreshDeviceFound(const void *device, uint32_t deviceLen)
{
    (void)device;
    (void)deviceLen;
}
} // namespace OHOS