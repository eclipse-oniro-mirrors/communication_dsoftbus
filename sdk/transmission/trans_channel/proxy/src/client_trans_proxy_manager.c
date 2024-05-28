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

#include "client_trans_proxy_manager.h"

#include <securec.h>
#include <unistd.h>

#include "anonymizer.h"
#include "client_trans_proxy_file_manager.h"
#include "client_trans_tcp_direct_message.h"
#include "softbus_adapter_crypto.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_socket.h"
#include "softbus_def.h"
#include "softbus_errcode.h"
#include "softbus_feature_config.h"
#include "softbus_utils.h"
#include "trans_log.h"
#include "trans_pending_pkt.h"
#include "trans_server_proxy.h"

#define SLICE_LEN (4 * 1024)
#define PROXY_ACK_SIZE 4

static IClientSessionCallBack g_sessionCb;
static uint32_t g_proxyMaxByteBufSize;
static uint32_t g_proxyMaxMessageBufSize;

static SoftBusList *g_proxyChannelInfoList = NULL;
static SoftBusList *g_channelSliceProcessorList = NULL;

typedef struct {
    int32_t priority;
    int32_t sliceNum;
    int32_t sliceSeq;
    int32_t reserved;
} SliceHead;

typedef struct {
    int32_t magicNumber;
    int32_t seq;
    int32_t flags;
    int32_t dataLen;
} PacketHead;

typedef struct {
    uint8_t *inData;
    uint32_t inLen;
    uint8_t *outData;
    uint32_t outLen;
} ClientProxyDataInfo;

void ClientPackSliceHead(SliceHead *data)
{
    data->priority = (int32_t)SoftBusHtoLl((uint32_t)data->priority);
    data->sliceNum = (int32_t)SoftBusHtoLl((uint32_t)data->sliceNum);
    data->sliceSeq = (int32_t)SoftBusHtoLl((uint32_t)data->sliceSeq);
    data->reserved = (int32_t)SoftBusHtoLl((uint32_t)data->reserved);
}

void ClientUnPackSliceHead(SliceHead *data)
{
    data->priority = (int32_t)SoftBusLtoHl((uint32_t)data->priority);
    data->sliceNum = (int32_t)SoftBusLtoHl((uint32_t)data->sliceNum);
    data->sliceSeq = (int32_t)SoftBusLtoHl((uint32_t)data->sliceSeq);
    data->reserved = (int32_t)SoftBusLtoHl((uint32_t)data->reserved);
}

void ClientPackPacketHead(PacketHead *data)
{
    data->magicNumber = (int32_t)SoftBusHtoLl((uint32_t)data->magicNumber);
    data->seq = (int32_t)SoftBusHtoLl((uint32_t)data->seq);
    data->flags = (int32_t)SoftBusHtoLl((uint32_t)data->flags);
    data->dataLen = (int32_t)SoftBusHtoLl((uint32_t)data->dataLen);
}

void ClientUnPackPacketHead(PacketHead *data)
{
    data->magicNumber = (int32_t)SoftBusLtoHl((uint32_t)data->magicNumber);
    data->seq = (int32_t)SoftBusLtoHl((uint32_t)data->seq);
    data->flags = (int32_t)SoftBusLtoHl((uint32_t)data->flags);
    data->dataLen = (int32_t)SoftBusLtoHl((uint32_t)data->dataLen);
}

static void ClientTransProxySliceTimerProc(void);

static int32_t ClientTransProxyListInit()
{
    g_proxyChannelInfoList = CreateSoftBusList();
    if (g_proxyChannelInfoList == NULL) {
        return SOFTBUS_ERR;
    }
    g_channelSliceProcessorList = CreateSoftBusList();
    if (g_channelSliceProcessorList == NULL) {
        DestroySoftBusList(g_proxyChannelInfoList);
        return SOFTBUS_ERR;
    }
    if (RegisterTimeoutCallback(SOFTBUS_PROXYSLICE_TIMER_FUN, ClientTransProxySliceTimerProc) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_INIT, "register timeout fail");
        DestroySoftBusList(g_proxyChannelInfoList);
        DestroySoftBusList(g_channelSliceProcessorList);
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static void ClientTransProxyListDeinit(void)
{
    if (g_proxyChannelInfoList != NULL) {
        DestroySoftBusList(g_proxyChannelInfoList);
        g_proxyChannelInfoList = NULL;
    }
    if (g_channelSliceProcessorList != NULL) {
        DestroySoftBusList(g_channelSliceProcessorList);
        g_channelSliceProcessorList = NULL;
    }
}

int32_t ClientTransProxyInit(const IClientSessionCallBack *cb)
{
    if (cb == NULL) {
        TRANS_LOGE(TRANS_INIT, "param is null!");
        return SOFTBUS_INVALID_PARAM;
    }

    g_sessionCb = *cb;
    if (ClinetTransProxyFileManagerInit() != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_INIT, "ClinetTransProxyFileManagerInit init fail!");
        return SOFTBUS_NO_INIT;
    }
    if (ClientTransProxyListInit() != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_INIT, "ClinetTransProxyListInit init fail!");
        return SOFTBUS_NO_INIT;
    }

    if (PendingInit(PENDING_TYPE_PROXY) == SOFTBUS_ERR) {
        TRANS_LOGE(TRANS_INIT, "trans proxy pending init failed.");
        return SOFTBUS_NO_INIT;
    }

    if (SoftbusGetConfig(SOFTBUS_INT_MAX_BYTES_NEW_LENGTH, (unsigned char *)&g_proxyMaxByteBufSize,
                         sizeof(g_proxyMaxByteBufSize)) != SOFTBUS_OK) {
        TRANS_LOGW(TRANS_INIT, "get auth proxy channel max bytes length fail");
    }
    if (SoftbusGetConfig(SOFTBUS_INT_MAX_MESSAGE_NEW_LENGTH, (unsigned char *)&g_proxyMaxMessageBufSize,
                         sizeof(g_proxyMaxMessageBufSize)) != SOFTBUS_OK) {
        TRANS_LOGW(TRANS_INIT, "get auth proxy channel max message length fail");
    }
    TRANS_LOGI(TRANS_INIT, "proxy auth byteSize=%{public}u, messageSize=%{public}u",
        g_proxyMaxByteBufSize, g_proxyMaxMessageBufSize);
    return SOFTBUS_OK;
}

void ClientTransProxyDeinit(void)
{
    ClinetTransProxyFileManagerDeinit();
    PendingDeinit(PENDING_TYPE_PROXY);
    ClientTransProxyListDeinit();
}

int32_t ClientTransProxyGetInfoByChannelId(int32_t channelId, ProxyChannelInfoDetail *info)
{
    if (info == NULL) {
        TRANS_LOGE(TRANS_SDK, "param invalid.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_proxyChannelInfoList->lock) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    ClientProxyChannelInfo *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &(g_proxyChannelInfoList->list), ClientProxyChannelInfo, node) {
        if (item->channelId == channelId) {
            (void)memcpy_s(info, sizeof(ProxyChannelInfoDetail), &item->detail, sizeof(ProxyChannelInfoDetail));
            item->detail.sequence++;
            (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
            return SOFTBUS_OK;
        }
    }

    (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
    TRANS_LOGE(TRANS_SDK, "can not find proxy channel by channelId=%{public}d", channelId);
    return SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND;
}

int32_t ClientTransProxyGetLinkTypeByChannelId(int32_t channelId, int32_t *linkType)
{
    if (linkType == NULL) {
        TRANS_LOGE(TRANS_SDK, "param invalid.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_proxyChannelInfoList->lock) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed");
        return SOFTBUS_LOCK_ERR;
    }
    ClientProxyChannelInfo *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &(g_proxyChannelInfoList->list), ClientProxyChannelInfo, node) {
        if (item->channelId == channelId) {
            *linkType = item->detail.linkType;
            (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
            return SOFTBUS_OK;
        }
    }

    (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
    TRANS_LOGE(TRANS_SDK, "can not find proxy channelId=%{public}d", channelId);
    return SOFTBUS_NOT_FIND;
}

int32_t ClientTransProxyAddChannelInfo(ClientProxyChannelInfo *info)
{
    if (info == NULL) {
        TRANS_LOGE(TRANS_SDK, "param invalid.");
        return SOFTBUS_INVALID_PARAM;
    }
    if (SoftBusMutexLock(&g_proxyChannelInfoList->lock) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    ClientProxyChannelInfo *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &(g_proxyChannelInfoList->list), ClientProxyChannelInfo, node) {
        if (item->channelId == info->channelId) {
            TRANS_LOGE(TRANS_SDK, "client is existed. channelId=%{public}d", item->channelId);
            (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
            return SOFTBUS_ERR;
        }
    }

    ListAdd(&g_proxyChannelInfoList->list, &info->node);
    TRANS_LOGI(TRANS_SDK, "add channelId=%{public}d", info->channelId);
    (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
    return SOFTBUS_OK;
}

int32_t ClientTransProxyDelChannelInfo(int32_t channelId)
{
    if (SoftBusMutexLock(&g_proxyChannelInfoList->lock) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "lock failed");
        return SOFTBUS_LOCK_ERR;
    }

    ClientProxyChannelInfo *item = NULL;
    LIST_FOR_EACH_ENTRY(item, &(g_proxyChannelInfoList->list), ClientProxyChannelInfo, node) {
        if (item->channelId == channelId) {
            ListDelete(&item->node);
            TRANS_LOGI(TRANS_SDK, "delete channelId=%{public}d", channelId);
            SoftBusFree(item);
            DelPendingPacket(channelId, PENDING_TYPE_PROXY);
            (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
            return SOFTBUS_OK;
        }
    }

    (void)SoftBusMutexUnlock(&g_proxyChannelInfoList->lock);
    TRANS_LOGE(TRANS_SDK, "can not find proxy channel by channelId=%{public}d", channelId);
    return SOFTBUS_TRANS_PROXY_CHANNEL_NOT_FOUND;
}

static ClientProxyChannelInfo *ClientTransProxyCreateChannelInfo(const ChannelInfo *channel)
{
    ClientProxyChannelInfo *info = (ClientProxyChannelInfo *)SoftBusCalloc(sizeof(ClientProxyChannelInfo));
    if (info == NULL) {
        TRANS_LOGE(TRANS_SDK, "info is null");
        return NULL;
    }
    if (memcpy_s(info->detail.sessionKey, SESSION_KEY_LENGTH, channel->sessionKey, SESSION_KEY_LENGTH) != EOK) {
        SoftBusFree(info);
        TRANS_LOGE(TRANS_SDK, "sessionKey memcpy fail");
        return NULL;
    }
    info->channelId = channel->channelId;
    info->detail.isEncrypted = channel->isEncrypt;
    info->detail.sequence = 0;
    info->detail.linkType = channel->linkType;
    return info;
}

int32_t ClientTransProxyOnChannelOpened(const char *sessionName, const ChannelInfo *channel)
{
    if (sessionName == NULL || channel == NULL) {
        TRANS_LOGW(TRANS_SDK, "invalid param.");
        return SOFTBUS_INVALID_PARAM;
    }
    int ret = ClientTransProxyAddChannelInfo(ClientTransProxyCreateChannelInfo(channel));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "ClientTransProxyAddChannelInfo fail");
        return ret;
    }

    ret = g_sessionCb.OnSessionOpened(sessionName, channel, TYPE_MESSAGE);
    if (ret != SOFTBUS_OK) {
        (void)ClientTransProxyDelChannelInfo(channel->channelId);
        char *tmpName = NULL;
        Anonymize(sessionName, &tmpName);
        TRANS_LOGE(TRANS_SDK, "notify session open fail, sessionName=%{public}s.", tmpName);
        AnonymizeFree(tmpName);
        return ret;
    }
    return SOFTBUS_OK;
}

int32_t TransProxyDelSliceProcessorByChannelId(int32_t channelId);

int32_t ClientTransProxyOnChannelClosed(int32_t channelId, ShutdownReason reason)
{
    (void)ClientTransProxyDelChannelInfo(channelId);
    (void)TransProxyDelSliceProcessorByChannelId(channelId);

    int ret = g_sessionCb.OnSessionClosed(channelId, CHANNEL_TYPE_PROXY, reason);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "notify session closed errCode=%{public}d, channelId=%{public}d.", ret, channelId);
        return ret;
    }
    return SOFTBUS_OK;
}

int32_t ClientTransProxyOnChannelOpenFailed(int32_t channelId, int32_t errCode)
{
    int ret = g_sessionCb.OnSessionOpenFailed(channelId, CHANNEL_TYPE_PROXY, errCode);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "notify session openfail errCode=%{public}d, channelId=%{public}d.", errCode, channelId);
        return ret;
    }

    return SOFTBUS_OK;
}

int32_t ClientTransProxySessionDataLenCheck(uint32_t dataLen, SessionPktType type)
{
    switch (type) {
        case TRANS_SESSION_MESSAGE:
        case TRANS_SESSION_ASYNC_MESSAGE: {
            if (dataLen > g_proxyMaxMessageBufSize) {
                return SOFTBUS_TRANS_INVALID_DATA_LENGTH;
            }
            break;
        }
        case TRANS_SESSION_BYTES: {
            if (dataLen > g_proxyMaxByteBufSize) {
                return SOFTBUS_TRANS_INVALID_DATA_LENGTH;
            }
            break;
        }
        default: {
            return SOFTBUS_OK;
        }
    }
    return SOFTBUS_OK;
}

static int32_t ClientTransProxyDecryptPacketData(int32_t channelId, int32_t seq, ClientProxyDataInfo *dataInfo)
{
    ProxyChannelInfoDetail info;
    int32_t ret = ClientTransProxyGetInfoByChannelId(channelId, &info);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "get channel Info by channelId=%{public}d failed, ret=%{public}d", channelId, ret);
        return ret;
    }
    AesGcmCipherKey cipherKey = { 0 };
    cipherKey.keyLen = SESSION_KEY_LENGTH;
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, info.sessionKey, SESSION_KEY_LENGTH) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy key error.");
        return SOFTBUS_MEM_ERR;
    }
    ret = SoftBusDecryptDataWithSeq(
        &cipherKey, dataInfo->inData, dataInfo->inLen, dataInfo->outData, &(dataInfo->outLen), seq);
    (void)memset_s(&cipherKey, sizeof(AesGcmCipherKey), 0, sizeof(AesGcmCipherKey));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "trans proxy Decrypt Data fail. ret=%{public}d ", ret);
        return SOFTBUS_DECRYPT_ERR;
    }

    return SOFTBUS_OK;
}

static int32_t ClientTransProxyCheckSliceHead(const SliceHead *head)
{
    if (head == NULL) {
        TRANS_LOGW(TRANS_SDK, "invalid param.");
        return SOFTBUS_ERR;
    }
    if (head->priority < 0 || head->priority >= PROXY_CHANNEL_PRORITY_BUTT) {
        TRANS_LOGE(TRANS_SDK, "invalid index=%{public}d", head->priority);
        return SOFTBUS_ERR;
    }

    if (head->sliceNum != 1 && head->sliceSeq >= head->sliceNum) {
        TRANS_LOGE(TRANS_SDK, "sliceNum=%{public}d, sliceSeq=%{public}d", head->sliceNum, head->sliceSeq);
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

int32_t TransProxyPackAndSendData(
    int32_t channelId, const void *data, uint32_t len, ProxyChannelInfoDetail *info, SessionPktType pktType);

static void ClientTransProxySendSessionAck(int32_t channelId, int32_t seq)
{
    unsigned char ack[PROXY_ACK_SIZE] = { 0 };
    if (memcpy_s(ack, PROXY_ACK_SIZE, &seq, sizeof(int32_t)) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy seq err");
        return;
    }

    ProxyChannelInfoDetail info;
    if (ClientTransProxyGetInfoByChannelId(channelId, &info) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "get proxy info err, channelId=%{public}d", channelId);
        return;
    }
    info.sequence = seq;
    if (TransProxyPackAndSendData(channelId, ack, PROXY_ACK_SIZE, &info, TRANS_SESSION_ACK) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "send ack err, seq=%{public}d", seq);
    }
}

static int32_t ClientTransProxyProcSendMsgAck(int32_t channelId, const char *data, int32_t len)
{
    if (len != PROXY_ACK_SIZE) {
        return SOFTBUS_TRANS_INVALID_DATA_LENGTH;
    }
    if (data == NULL) {
        return SOFTBUS_ERR;
    }
    int32_t seq = *(int32_t *)data;
    TRANS_LOGI(TRANS_SDK, "ClientTransProxyProcSendMsgAck. channelId=%{public}d, seq=%{public}d", channelId, seq);
    return SetPendingPacket(channelId, seq, PENDING_TYPE_PROXY);
}

static int32_t ClientTransProxyNotifySession(
    int32_t channelId, SessionPktType flags, int32_t seq, const char *data, uint32_t len)
{
    switch (flags) {
        case TRANS_SESSION_MESSAGE:
            ClientTransProxySendSessionAck(channelId, seq);
            return g_sessionCb.OnDataReceived(channelId, CHANNEL_TYPE_PROXY, data, len, flags);
        case TRANS_SESSION_ACK:
            return (int32_t)(ClientTransProxyProcSendMsgAck(channelId, data, len));
        case TRANS_SESSION_BYTES:
        case TRANS_SESSION_FILE_FIRST_FRAME:
        case TRANS_SESSION_FILE_ONGOINE_FRAME:
        case TRANS_SESSION_FILE_LAST_FRAME:
        case TRANS_SESSION_FILE_ONLYONE_FRAME:
        case TRANS_SESSION_FILE_ALLFILE_SENT:
        case TRANS_SESSION_FILE_CRC_CHECK_FRAME:
        case TRANS_SESSION_FILE_RESULT_FRAME:
        case TRANS_SESSION_FILE_ACK_REQUEST_SENT:
        case TRANS_SESSION_FILE_ACK_RESPONSE_SENT:
        case TRANS_SESSION_ASYNC_MESSAGE:
            return g_sessionCb.OnDataReceived(channelId, CHANNEL_TYPE_PROXY, data, len, flags);
        default:
            TRANS_LOGE(TRANS_SDK, "invalid flags=%{public}d", flags);
            return SOFTBUS_INVALID_PARAM;
    }
}

static int32_t ClientTransProxyProcessSessionData(int32_t channelId, const PacketHead *dataHead, const char *data)
{
    ClientProxyDataInfo dataInfo = { 0 };
    uint32_t outLen = 0;
    int32_t ret = SOFTBUS_ERR;

    if (dataHead->dataLen <= OVERHEAD_LEN) {
        TRANS_LOGE(TRANS_SDK, "invalid data head dataLen=%{public}d", dataHead->dataLen);
        return SOFTBUS_TRANS_INVALID_DATA_LENGTH;
    }

    outLen = dataHead->dataLen - OVERHEAD_LEN;
    dataInfo.outData = (unsigned char *)SoftBusCalloc(outLen);
    if (dataInfo.outData == NULL) {
        TRANS_LOGE(TRANS_SDK, "malloc fail when process session out data.");
        return SOFTBUS_MALLOC_ERR;
    }
    dataInfo.inData = (unsigned char *)data;
    dataInfo.inLen = dataHead->dataLen;
    dataInfo.outLen = outLen;

    ret = ClientTransProxyDecryptPacketData(channelId, dataHead->seq, &dataInfo);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "decrypt err");
        SoftBusFree(dataInfo.outData);
        return SOFTBUS_DECRYPT_ERR;
    }

    if (ClientTransProxySessionDataLenCheck(dataInfo.outLen, (SessionPktType)(dataHead->flags)) != SOFTBUS_OK) {
        TRANS_LOGE(
            TRANS_SDK, "data len is too large outlen=%{public}d, flags=%{public}d", dataInfo.outLen, dataHead->flags);
        SoftBusFree(dataInfo.outData);
        return SOFTBUS_ERR;
    }

    TRANS_LOGD(TRANS_SDK, "ProcessData debug: outlen=%{public}d", dataInfo.outLen);
    if (ClientTransProxyNotifySession(channelId, (SessionPktType)dataHead->flags, dataHead->seq,
                                      (const char *)dataInfo.outData, dataInfo.outLen) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "process data err");
        SoftBusFree(dataInfo.outData);
        return SOFTBUS_ERR;
    }
    SoftBusFree(dataInfo.outData);
    return SOFTBUS_OK;
}

static int32_t ClientTransProxyNoSubPacketProc(int32_t channelId, const char *data, uint32_t len)
{
    PacketHead head;
    if (memcpy_s(&head, sizeof(PacketHead), data, sizeof(PacketHead)) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy packetHead failed");
        return SOFTBUS_MEM_ERR;
    }
    ClientUnPackPacketHead(&head);
    if ((uint32_t)head.magicNumber != MAGIC_NUMBER) {
        TRANS_LOGE(TRANS_SDK, "invalid magicNumber=%{public}x", head.magicNumber);
        return SOFTBUS_ERR;
    }
    if (head.dataLen <= 0) {
        TRANS_LOGE(TRANS_SDK, "invalid dataLen=%{public}d", head.dataLen);
        return SOFTBUS_ERR;
    }
    TRANS_LOGD(TRANS_SDK, "NoSubPacketProc dataLen=%{public}d, inputLen=%{public}d", head.dataLen, len);
    if (head.dataLen + sizeof(PacketHead) != len) {
        TRANS_LOGE(TRANS_SDK, "dataLen error");
        return SOFTBUS_ERR;
    }
    int32_t ret = ClientTransProxyProcessSessionData(channelId, &head, data + sizeof(PacketHead));
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "process data err");
        return SOFTBUS_ERR;
    }
    return SOFTBUS_OK;
}

static ChannelSliceProcessor *ClientTransProxyGetChannelSliceProcessor(int32_t channelId)
{
    ChannelSliceProcessor *processor = NULL;
    LIST_FOR_EACH_ENTRY(processor, &g_channelSliceProcessorList->list, ChannelSliceProcessor, head) {
        if (processor->channelId == channelId) {
            return processor;
        }
    }

    ChannelSliceProcessor *node = (ChannelSliceProcessor *)SoftBusCalloc(sizeof(ChannelSliceProcessor));
    if (node == NULL) {
        TRANS_LOGE(TRANS_SDK, "calloc err");
        return NULL;
    }
    node->channelId = channelId;
    ListInit(&(node->head));
    ListAdd(&(g_channelSliceProcessorList->list), &(node->head));
    g_channelSliceProcessorList->cnt++;
    TRANS_LOGI(TRANS_SDK, "add new node, channelId=%{public}d", channelId);
    return node;
}

static void ClientTransProxyClearProcessor(SliceProcessor *processor)
{
    if (processor->data != NULL) {
        TRANS_LOGE(TRANS_SDK, "slice processor data not null");
        SoftBusFree(processor->data);
        processor->data = NULL;
    }
    processor->active = false;
    processor->bufLen = 0;
    processor->dataLen = 0;
    processor->expectedSeq = 0;
    processor->sliceNumber = 0;
    processor->timeout = 0;
}

int32_t TransProxyDelSliceProcessorByChannelId(int32_t channelId)
{
    ChannelSliceProcessor *node = NULL;
    ChannelSliceProcessor *next = NULL;

    if (g_channelSliceProcessorList == NULL) {
        TRANS_LOGE(TRANS_INIT, "not init");
        return SOFTBUS_NO_INIT;
    }
    if (SoftBusMutexLock(&g_channelSliceProcessorList->lock) != 0) {
        TRANS_LOGE(TRANS_SDK, "lock err");
        return SOFTBUS_LOCK_ERR;
    }
    LIST_FOR_EACH_ENTRY_SAFE(node, next, &g_channelSliceProcessorList->list, ChannelSliceProcessor, head) {
        if (node->channelId == channelId) {
            for (int i = PROXY_CHANNEL_PRORITY_MESSAGE; i < PROXY_CHANNEL_PRORITY_BUTT; i++) {
                ClientTransProxyClearProcessor(&(node->processor[i]));
            }
            ListDelete(&(node->head));
            TRANS_LOGI(TRANS_SDK, "delete channelId=%{public}d", channelId);
            SoftBusFree(node);
            g_channelSliceProcessorList->cnt--;
            (void)SoftBusMutexUnlock(&g_channelSliceProcessorList->lock);
            return SOFTBUS_OK;
        }
    }
    (void)SoftBusMutexUnlock(&g_channelSliceProcessorList->lock);
    return SOFTBUS_OK;
}

static int32_t ClientTransProxySliceProcessChkPkgIsValid(
    const SliceProcessor *processor, const SliceHead *head, const char *data, uint32_t len)
{
    (void)data;
    if (head->sliceNum != processor->sliceNumber || head->sliceSeq != processor->expectedSeq) {
        TRANS_LOGE(TRANS_SDK, "unmatched normal slice received");
        return SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_NO_INVALID;
    }
    if ((int32_t)len + processor->dataLen > processor->bufLen) {
        TRANS_LOGE(TRANS_SDK, "data len invalid");
        return SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_EXCEED_LENGTH;
    }
    if (processor->data == NULL) {
        TRANS_LOGE(TRANS_SDK, "data NULL");
        return SOFTBUS_TRANS_PROXY_ASSEMBLE_PACK_DATA_NULL;
    }
    return SOFTBUS_OK;
}

static int32_t ClientTransProxyFirstSliceProcess(
    SliceProcessor *processor, const SliceHead *head, const char *data, uint32_t len)
{
    ClientTransProxyClearProcessor(processor);

    uint32_t maxDataLen =
        (head->priority == PROXY_CHANNEL_PRORITY_MESSAGE) ? g_proxyMaxMessageBufSize : g_proxyMaxByteBufSize;
    uint32_t maxLen = maxDataLen + sizeof(PacketHead) + OVERHEAD_LEN;
    processor->data = (char *)SoftBusCalloc(maxLen);
    if (processor->data == NULL) {
        TRANS_LOGE(TRANS_SDK, "malloc fail when proc first slice package");
        return SOFTBUS_MALLOC_ERR;
    }
    processor->bufLen = (int32_t)maxLen;
    if (memcpy_s(processor->data, maxLen, data, len) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy fail when proc first slice package");
        SoftBusFree(processor->data);
        processor->data = NULL;
        return SOFTBUS_MEM_ERR;
    }
    processor->sliceNumber = head->sliceNum;
    processor->expectedSeq = 1;
    processor->dataLen = (int32_t)len;
    processor->active = true;
    processor->timeout = 0;

    TRANS_LOGI(TRANS_SDK, "FirstSliceProcess ok");
    return SOFTBUS_OK;
}

static int32_t ClientTransProxyLastSliceProcess(
    SliceProcessor *processor, const SliceHead *head, const char *data, uint32_t len, int32_t channelId)
{
    int32_t ret = ClientTransProxySliceProcessChkPkgIsValid(processor, head, data, len);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    if (memcpy_s(processor->data + processor->dataLen, (uint32_t)(processor->bufLen - processor->dataLen), data, len) !=
        EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy fail when proc last slice");
        return SOFTBUS_MEM_ERR;
    }
    processor->expectedSeq++;
    processor->dataLen += (int32_t)len;

    ret = ClientTransProxyNoSubPacketProc(channelId, processor->data, (uint32_t)processor->dataLen);
    if (ret != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "process packets err");
        return ret;
    }
    ClientTransProxyClearProcessor(processor);
    TRANS_LOGI(TRANS_SDK, "LastSliceProcess ok");
    return ret;
}

static int32_t ClientTransProxyNormalSliceProcess(
    SliceProcessor *processor, const SliceHead *head, const char *data, uint32_t len)
{
    int32_t ret = ClientTransProxySliceProcessChkPkgIsValid(processor, head, data, len);
    if (ret != SOFTBUS_OK) {
        return ret;
    }
    if (memcpy_s(processor->data + processor->dataLen,
        (uint32_t)(processor->bufLen - processor->dataLen), data, len) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy fail when proc normal slice");
        return SOFTBUS_MEM_ERR;
    }
    processor->expectedSeq++;
    processor->dataLen += (int32_t)len;
    processor->timeout = 0;
    TRANS_LOGI(TRANS_SDK, "NormalSliceProcess ok");
    return ret;
}

static int ClientTransProxySubPacketProc(int32_t channelId, const SliceHead *head, const char *data, uint32_t len)
{
    if (g_channelSliceProcessorList == NULL) {
        TRANS_LOGE(TRANS_SDK, "TransProxySubPacketProc not init");
        return SOFTBUS_NO_INIT;
    }
    if (SoftBusMutexLock(&g_channelSliceProcessorList->lock) != 0) {
        TRANS_LOGE(TRANS_SDK, "lock err");
        return SOFTBUS_ERR;
    }

    ChannelSliceProcessor *channelProcessor = ClientTransProxyGetChannelSliceProcessor(channelId);
    if (channelProcessor == NULL) {
        SoftBusMutexUnlock(&g_channelSliceProcessorList->lock);
        return SOFTBUS_ERR;
    }

    int ret;
    int32_t index = head->priority;
    SliceProcessor *processor = &(channelProcessor->processor[index]);
    if (head->sliceSeq == 0) {
        ret = ClientTransProxyFirstSliceProcess(processor, head, data, len);
    } else if (head->sliceNum == head->sliceSeq + 1) {
        ret = ClientTransProxyLastSliceProcess(processor, head, data, len, channelId);
    } else {
        ret = ClientTransProxyNormalSliceProcess(processor, head, data, len);
    }

    SoftBusMutexUnlock(&g_channelSliceProcessorList->lock);
    TRANS_LOGI(TRANS_SDK, "Proxy SubPacket Proc end");
    if (ret != SOFTBUS_OK) {
        ClientTransProxyClearProcessor(processor);
    }
    return ret;
}

static int32_t ClientTransProxySliceProc(int32_t channelId, const char *data, uint32_t len)
{
    if (data == NULL || len <= sizeof(SliceHead)) {
        TRANS_LOGE(TRANS_SDK, "data null or len error. len=%{public}d", len);
        return SOFTBUS_ERR;
    }

    SliceHead headSlice = *(SliceHead *)data;
    ClientUnPackSliceHead(&headSlice);
    if (ClientTransProxyCheckSliceHead(&headSlice)) {
        TRANS_LOGE(TRANS_SDK, "invalid slihead");
        return SOFTBUS_TRANS_PROXY_INVALID_SLICE_HEAD;
    }

    uint32_t dataLen = len - sizeof(SliceHead);
    if (headSlice.sliceNum == 1) { // no sub packets
        TRANS_LOGI(TRANS_SDK, "no sub packets proc, channelId=%{public}d", channelId);
        return ClientTransProxyNoSubPacketProc(channelId, data + sizeof(SliceHead), dataLen);
    } else {
        TRANS_LOGI(TRANS_SDK, "sub packets proc sliceNum=%{public}d", headSlice.sliceNum);
        return ClientTransProxySubPacketProc(channelId, &headSlice, data + sizeof(SliceHead), dataLen);
    }
}

static void ClientTransProxySliceTimerProc(void)
{
#define SLICE_PACKET_TIMEOUT 10 // 10s
    ChannelSliceProcessor *removeNode = NULL;
    ChannelSliceProcessor *nextNode = NULL;

    if (g_channelSliceProcessorList == NULL || g_channelSliceProcessorList->cnt == 0) {
        return;
    }
    if (SoftBusMutexLock(&g_channelSliceProcessorList->lock) != 0) {
        TRANS_LOGE(TRANS_SDK, "TransProxySliceTimerProc lock mutex fail!");
        return;
    }

    LIST_FOR_EACH_ENTRY_SAFE(removeNode, nextNode, &g_channelSliceProcessorList->list, ChannelSliceProcessor, head) {
        for (int i = PROXY_CHANNEL_PRORITY_MESSAGE; i < PROXY_CHANNEL_PRORITY_BUTT; i++) {
            if (removeNode->processor[i].active == true) {
                removeNode->processor[i].timeout++;
                if (removeNode->processor[i].timeout >= SLICE_PACKET_TIMEOUT) {
                    TRANS_LOGE(TRANS_SDK, "timeout=%{public}d", removeNode->processor[i].timeout);
                    ClientTransProxyClearProcessor(&removeNode->processor[i]);
                }
            }
        }
    }
    (void)SoftBusMutexUnlock(&g_channelSliceProcessorList->lock);
    return;
}

int32_t ClientTransProxyOnDataReceived(int32_t channelId, const void *data, uint32_t len, SessionPktType type)
{
    (void)type;
    if (data == NULL) {
        TRANS_LOGE(TRANS_SDK, "ClientTransProxyOnDataReceived data null. channelId=%{public}d", channelId);
        return SOFTBUS_INVALID_PARAM;
    }

    ProxyChannelInfoDetail info;
    if (ClientTransProxyGetInfoByChannelId(channelId, &info) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    if (!info.isEncrypted) {
        return g_sessionCb.OnDataReceived(channelId, CHANNEL_TYPE_PROXY, data, len, TRANS_SESSION_BYTES);
    }

    return ClientTransProxySliceProc(channelId, (char *)data, len);
}

void ClientTransProxyCloseChannel(int32_t channelId)
{
    (void)ClientTransProxyDelChannelInfo(channelId);
    (void)TransProxyDelSliceProcessorByChannelId(channelId);
    TRANS_LOGI(TRANS_SDK, "TransCloseProxyChannel, channelId=%{public}d", channelId);
    if (ServerIpcCloseChannel(NULL, channelId, CHANNEL_TYPE_PROXY) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "server close err. channelId=%{public}d", channelId);
    }
}

static int32_t ClientTransProxyEncryptWithSeq(const char *sessionKey, int32_t seqNum, const char *in,
    uint32_t inLen, char *out, uint32_t *outLen)
{
    AesGcmCipherKey cipherKey = {0};
    cipherKey.keyLen = SESSION_KEY_LENGTH;
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, sessionKey, SESSION_KEY_LENGTH) != EOK) {
        TRANS_LOGE(TRANS_SDK, "memcpy key error.");
        return SOFTBUS_MEM_ERR;
    }

    int ret = SoftBusEncryptDataWithSeq(&cipherKey, (unsigned char*)in, inLen, (unsigned char*)out, outLen, seqNum);
    (void)memset_s(cipherKey.key, SESSION_KEY_LENGTH, 0, SESSION_KEY_LENGTH);

    if (ret != SOFTBUS_OK || *outLen != inLen + OVERHEAD_LEN) {
        TRANS_LOGE(TRANS_SDK, "encrypt error, ret=%{public}d", ret);
        return SOFTBUS_ENCRYPT_ERR;
    }
    return SOFTBUS_OK;
}

static int32_t ClientTransProxyPackBytes(
    int32_t channelId, ClientProxyDataInfo *dataInfo, int seq, char *sessionKey, SessionPktType flag)
{
#define MAGIC_NUMBER 0xBABEFACE
    if (dataInfo == NULL) {
        return SOFTBUS_ERR;
    }
    dataInfo->outLen = dataInfo->inLen + OVERHEAD_LEN + sizeof(PacketHead);
    dataInfo->outData = (uint8_t *)SoftBusCalloc(dataInfo->outLen);
    if (dataInfo->outData == NULL) {
        TRANS_LOGE(TRANS_SDK, "calloc error");
        return SOFTBUS_MEM_ERR;
    }

    uint32_t outLen = 0;
    if (ClientTransProxyEncryptWithSeq(sessionKey, seq, (const char *)dataInfo->inData, dataInfo->inLen,
                                       (char *)dataInfo->outData + sizeof(PacketHead), &outLen) != SOFTBUS_OK) {
        SoftBusFree(dataInfo->outData);
        TRANS_LOGE(TRANS_SDK, "ClientTransProxyEncryptWithSeq channelId=%{public}d", channelId);
        return SOFTBUS_TRANS_PROXY_SESS_ENCRYPT_ERR;
    }
    PacketHead *pktHead = (PacketHead *)dataInfo->outData;
    pktHead->magicNumber = MAGIC_NUMBER;
    pktHead->seq = seq;
    pktHead->flags = flag;
    pktHead->dataLen = (int32_t)((int32_t)dataInfo->outLen - sizeof(PacketHead));
    ClientPackPacketHead(pktHead);

    return SOFTBUS_OK;
}

static int32_t SessionPktTypeToProxyIndex(SessionPktType packetType)
{
    switch (packetType) {
        case TRANS_SESSION_MESSAGE:
        case TRANS_SESSION_ASYNC_MESSAGE:
        case TRANS_SESSION_ACK:
            return PROXY_CHANNEL_PRORITY_MESSAGE;
        case TRANS_SESSION_BYTES:
            return PROXY_CHANNEL_PRORITY_BYTES;
        default:
            return PROXY_CHANNEL_PRORITY_FILE;
    }
}

int32_t TransProxyPackAndSendData(
    int32_t channelId, const void *data, uint32_t len, ProxyChannelInfoDetail *info, SessionPktType pktType)
{
    if (data == NULL || info == NULL) {
        return SOFTBUS_INVALID_PARAM;
    }
    ClientProxyDataInfo dataInfo = { (uint8_t *)data, len, (uint8_t *)data, len };
    if (ClientTransProxyPackBytes(channelId, &dataInfo, info->sequence, info->sessionKey, pktType) != SOFTBUS_OK) {
        TRANS_LOGE(TRANS_SDK, "ClientTransProxyPackBytes error, channelId=%{public}d", channelId);
        return SOFTBUS_ERR;
    }

    uint32_t sliceNum = (dataInfo.outLen + (uint32_t)(SLICE_LEN - 1)) / (uint32_t)SLICE_LEN;
    for (uint32_t i = 0; i < sliceNum; i++) {
        uint32_t dataLen = (i == (sliceNum - 1U)) ? (dataInfo.outLen - i * SLICE_LEN) : SLICE_LEN;
        int32_t offset = (int32_t)(i * SLICE_LEN);

        uint8_t *sliceData = (uint8_t *)SoftBusCalloc(dataLen + sizeof(SliceHead));
        if (sliceData == NULL) {
            TRANS_LOGE(TRANS_SDK, "malloc slice data error, channelId=%{public}d", channelId);
            return SOFTBUS_MALLOC_ERR;
        }
        SliceHead *slicehead = (SliceHead *)sliceData;
        slicehead->priority = SessionPktTypeToProxyIndex(pktType);
        if (sliceNum > INT32_MAX) {
            TRANS_LOGE(TRANS_FILE, "Data overflow");
            return SOFTBUS_INVALID_NUM;
        }
        slicehead->sliceNum = (int32_t)sliceNum;
        slicehead->sliceSeq = i;
        ClientPackSliceHead(slicehead);
        if (memcpy_s(sliceData + sizeof(SliceHead), dataLen, dataInfo.outData + offset, dataLen) != EOK) {
            TRANS_LOGE(TRANS_SDK, "memcpy_s error, channelId=%{public}d", channelId);
            return SOFTBUS_MEM_ERR;
        }

        int ret = ServerIpcSendMessage(channelId, CHANNEL_TYPE_PROXY, sliceData, dataLen + sizeof(SliceHead), pktType);
        if (ret != SOFTBUS_OK) {
            TRANS_LOGE(TRANS_SDK, "ServerIpcSendMessage error, channelId=%{public}d, ret=%{public}d", channelId, ret);
            return ret;
        }

        SoftBusFree(sliceData);
    }
    SoftBusFree(dataInfo.outData);

    TRANS_LOGI(TRANS_SDK, "TransProxyPackAndSendData success, channelId=%{public}d", channelId);
    return SOFTBUS_OK;
}

int32_t TransProxyChannelSendBytes(int32_t channelId, const void *data, uint32_t len)
{
    ProxyChannelInfoDetail info;
    if (ClientTransProxyGetInfoByChannelId(channelId, &info) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    int ret = SOFTBUS_ERR;
    if (!info.isEncrypted) {
        ret = ServerIpcSendMessage(channelId, CHANNEL_TYPE_PROXY, data, len, TRANS_SESSION_BYTES);
        TRANS_LOGI(TRANS_SDK, "send bytes: channelId=%{public}d, ret=%{public}d", channelId, ret);
        return ret;
    }

    return TransProxyPackAndSendData(channelId, data, len, &info, TRANS_SESSION_BYTES);
}

int32_t TransProxyChannelSendMessage(int32_t channelId, const void *data, uint32_t len)
{
    ProxyChannelInfoDetail info;
    if (ClientTransProxyGetInfoByChannelId(channelId, &info) != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    int ret = SOFTBUS_ERR;
    if (!info.isEncrypted) {
        // auth channel only can send bytes
        ret = ServerIpcSendMessage(channelId, CHANNEL_TYPE_PROXY, data, len, TRANS_SESSION_BYTES);
        TRANS_LOGI(TRANS_SDK, "send msg: channelId=%{public}d, ret=%{public}d", channelId, ret);
        return ret;
    }

    ret = TransProxyPackAndSendData(channelId, data, len, &info, TRANS_SESSION_MESSAGE);
    if (ret != SOFTBUS_OK) {
        return SOFTBUS_ERR;
    }
    return ProcPendingPacket(channelId, info.sequence, PENDING_TYPE_PROXY);
}
