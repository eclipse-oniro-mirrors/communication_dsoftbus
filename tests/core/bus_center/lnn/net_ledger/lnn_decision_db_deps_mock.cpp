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

#include <securec.h>

#include "lnn_decision_db_deps_mock.h"
#include "softbus_common.h"
#include "softbus_error_code.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
void *g_decisionDbDepsInterface;
DecisionDbDepsInterfaceMock::DecisionDbDepsInterfaceMock()
{
    g_decisionDbDepsInterface = reinterpret_cast<void *>(this);
}

DecisionDbDepsInterfaceMock::~DecisionDbDepsInterfaceMock()
{
    g_decisionDbDepsInterface = nullptr;
}

static DecisionDbDepsInterfaceMock *GetDecisionDbDepsInterface()
{
    return reinterpret_cast<DecisionDbDepsInterfaceMock *>(g_decisionDbDepsInterface);
}

int32_t DecisionDbDepsInterfaceMock::DecisionDbAsyncCallbackHelper(
    SoftBusLooper *looper, LnnAsyncCallbackFunc callback, void *para)
{
    if (callback != nullptr) {
        callback(para);
        return SOFTBUS_OK;
    }
    return SOFTBUS_INVALID_PARAM;
}

extern "C" {
int32_t LnnGenerateKeyByHuks(struct HksBlob *keyAlias)
{
    return GetDecisionDbDepsInterface()->LnnGenerateKeyByHuks(keyAlias);
}

int32_t LnnDeleteKeyByHuks(struct HksBlob *keyAlias)
{
    return GetDecisionDbDepsInterface()->LnnDeleteKeyByHuks(keyAlias);
}

int32_t LnnEncryptDataByHuks(const struct HksBlob *keyAlias, const struct HksBlob *inData, struct HksBlob *outData)
{
    return GetDecisionDbDepsInterface()->LnnEncryptDataByHuks(keyAlias, inData, outData);
}

int32_t LnnDecryptDataByHuks(const struct HksBlob *keyAlias, const struct HksBlob *inData, struct HksBlob *outData)
{
    return GetDecisionDbDepsInterface()->LnnDecryptDataByHuks(keyAlias, inData, outData);
}

int32_t LnnGenerateRandomByHuks(uint8_t *randomKey, uint32_t len)
{
    return GetDecisionDbDepsInterface()->LnnGenerateRandomByHuks(randomKey, len);
}

int32_t OpenDatabase(DbContext **ctx)
{
    return GetDecisionDbDepsInterface()->OpenDatabase(ctx);
}

int32_t CloseDatabase(DbContext *ctx)
{
    return GetDecisionDbDepsInterface()->CloseDatabase(ctx);
}

int32_t CreateTable(DbContext *ctx, TableNameID id)
{
    return GetDecisionDbDepsInterface()->CreateTable(ctx, id);
}

int32_t CheckTableExist(DbContext *ctx, TableNameID id, bool *isExist)
{
    return GetDecisionDbDepsInterface()->CheckTableExist(ctx, id, isExist);
}

int32_t RemoveRecordByKey(DbContext *ctx, TableNameID id, uint8_t *data)
{
    return GetDecisionDbDepsInterface()->RemoveRecordByKey(ctx, id, data);
}

int32_t GetRecordNumByKey(DbContext *ctx, TableNameID id, uint8_t *data)
{
    return GetDecisionDbDepsInterface()->GetRecordNumByKey(ctx, id, data);
}

int32_t EncryptedDb(DbContext *ctx, const uint8_t *password, uint32_t len)
{
    return GetDecisionDbDepsInterface()->EncryptedDb(ctx, password, len);
}

int32_t UpdateDbPassword(DbContext *ctx, const uint8_t *password, uint32_t len)
{
    return GetDecisionDbDepsInterface()->UpdateDbPassword(ctx, password, len);
}

int32_t QueryRecordByKey(DbContext *ctx, TableNameID id, uint8_t *data, uint8_t **replyInfo, int32_t infoNum)
{
    return GetDecisionDbDepsInterface()->QueryRecordByKey(ctx, id, data, replyInfo, infoNum);
}

int32_t LnnGetFullStoragePath(LnnFileId id, char *path, uint32_t len)
{
    return GetDecisionDbDepsInterface()->LnnGetFullStoragePath(id, path, len);
}

int32_t SoftBusReadFullFile(const char *fileName, char *readBuf, uint32_t maxLen)
{
    return GetDecisionDbDepsInterface()->SoftBusReadFullFile(fileName, readBuf, maxLen);
}

int32_t SoftBusWriteFile(const char *fileName, const char *writeBuf, uint32_t len)
{
    return GetDecisionDbDepsInterface()->SoftBusWriteFile(fileName, writeBuf, len);
}

int32_t SoftBusAccessFile(const char *pathName, int32_t mode)
{
    return GetDecisionDbDepsInterface()->SoftBusAccessFile(pathName, mode);
}

int32_t LnnAsyncCallbackHelper(SoftBusLooper *looper, LnnAsyncCallbackFunc callback, void *para)
{
    return GetDecisionDbDepsInterface()->LnnAsyncCallbackHelper(looper, callback, para);
}

int32_t LnnGetLocalByteInfo(InfoKey key, uint8_t *info, uint32_t len)
{
    return GetDecisionDbDepsInterface()->LnnGetLocalByteInfo(key, info, len);
}

int32_t ConvertBytesToHexString(char *outBuf, uint32_t outBufLen, const unsigned char *inBuf, uint32_t inLen)
{
    return GetDecisionDbDepsInterface()->ConvertBytesToHexString(outBuf, outBufLen, inBuf, inLen);
}

void LnnNotifyNetworkStateChanged(SoftBusNetworkState state)
{
    return GetDecisionDbDepsInterface()->LnnNotifyNetworkStateChanged(state);
}

TrustedReturnType AuthHasTrustedRelation(void)
{
    return GetDecisionDbDepsInterface()->AuthHasTrustedRelation();
}

bool IsEnableSoftBusHeartbeat(void)
{
    return GetDecisionDbDepsInterface()->IsEnableSoftBusHeartbeat();
}

void LnnNotifyHBRepeat(void)
{
    return GetDecisionDbDepsInterface()->LnnNotifyHBRepeat();
}

void LnnHbClearRecvList(void)
{
    return GetDecisionDbDepsInterface()->LnnHbClearRecvList();
}

int32_t LnnConvertHbTypeToId(LnnHeartbeatType type)
{
    return GetDecisionDbDepsInterface()->LnnConvertHbTypeToId(type);
}

bool LnnVisitHbTypeSet(VisitHbTypeCb callback, LnnHeartbeatType *typeSet, void *data)
{
    return GetDecisionDbDepsInterface()->LnnVisitHbTypeSet(callback, typeSet, data);
}

int32_t LnnCeEncryptDataByHuks(const struct HksBlob *keyAlias, const struct HksBlob *inData, struct HksBlob *outData)
{
    return GetDecisionDbDepsInterface()->LnnCeEncryptDataByHuks(keyAlias, inData, outData);
}

int32_t LnnCeDecryptDataByHuks(const struct HksBlob *keyAlias, const struct HksBlob *inData, struct HksBlob *outData)
{
    return GetDecisionDbDepsInterface()->LnnCeDecryptDataByHuks(keyAlias, inData, outData);
}

int64_t SoftBusGetRealTimeMs(void)
{
    return GetDecisionDbDepsInterface()->SoftBusGetRealTimeMs();
}

int32_t LnnGetLocalNum64Info(InfoKey key, int64_t *info)
{
    return GetDecisionDbDepsInterface()->LnnGetLocalNum64Info(key, info);
}

int32_t LnnSetLocalNum64Info(InfoKey key, int64_t info)
{
    return GetDecisionDbDepsInterface()->LnnSetLocalNum64Info(key, info);
}

int32_t LnnAsyncCallbackDelayHelper(
    SoftBusLooper *looper, LnnAsyncCallbackFunc callback, void *para, uint64_t delayMillis)
{
    return GetDecisionDbDepsInterface()->LnnAsyncCallbackDelayHelper(looper, callback, para, delayMillis);
}

SoftBusLooper *GetLooper(int32_t looper)
{
    return GetDecisionDbDepsInterface()->GetLooper(looper);
}

int32_t LnnSaveDeviceData(const char *data, LnnDataType dataType)
{
    return GetDecisionDbDepsInterface()->LnnSaveDeviceData(data, dataType);
}

int32_t LnnRetrieveDeviceData(LnnDataType dataType, char **data, uint32_t *dataLen)
{
    return GetDecisionDbDepsInterface()->LnnRetrieveDeviceData(dataType, data, dataLen);
}

int32_t LnnSaveLocalDeviceInfo(const NodeInfo *deviceInfo)
{
    return GetDecisionDbDepsInterface()->LnnSaveLocalDeviceInfo(deviceInfo);
}

int32_t LnnSaveRemoteDeviceInfo(const NodeInfo *deviceInfo)
{
    return GetDecisionDbDepsInterface()->LnnSaveRemoteDeviceInfo(deviceInfo);
}

int32_t LnnDeleteCeKeyByHuks(struct HksBlob *keyAlias)
{
    return GetDecisionDbDepsInterface()->LnnDeleteCeKeyByHuks(keyAlias);
}

int32_t LnnGenerateCeKeyByHuks(struct HksBlob *keyAlias)
{
    return GetDecisionDbDepsInterface()->LnnGenerateCeKeyByHuks(keyAlias);
}

int32_t LnnGetLocalDevInfo(NodeInfo *deviceInfo)
{
    return GetDecisionDbDepsInterface()->LnnGetLocalDevInfo(deviceInfo);
}

int32_t LnnGetAllRemoteDevInfo(NodeInfo **info, int32_t *nums)
{
    return GetDecisionDbDepsInterface()->LnnGetAllRemoteDevInfo(info, nums);
}

int32_t LnnGetLocalNumInfo(InfoKey key, int32_t *info)
{
    return GetDecisionDbDepsInterface()->LnnGetLocalNumInfo(key, info);
}
} // extern "C"
} // namespace OHOS
