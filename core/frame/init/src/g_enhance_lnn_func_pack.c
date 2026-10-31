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
#include "g_enhance_lnn_func_pack.h"

#include "auth_log.h"
#include "g_enhance_lnn_func.h"
#include "lnn_heartbeat_medium_mgr.h"
#include "lnn_heartbeat_utils_struct.h"
#include "softbus_adapter_mem.h"
#include "softbus_error_code.h"
#include "softbus_init_common.h"
#include "softbus_def.h"

#define VIRTUAL_DEFAULT_SCORE 60

#ifdef ENABLE_FEATURE_LNN_BLE
int32_t InitBleHeartbeat(const LnnHeartbeatMediumMgrCb *callback)
{
    (void)callback;

    LNN_LOGI(LNN_INIT, "ble heartbeat stub impl init");
    return SOFTBUS_OK;
}

int32_t BleHeartbeatOnceBegin(const LnnHeartbeatSendBeginData *custData)
{
    (void)custData;

    LNN_LOGI(LNN_HEART_BEAT, "ble heartbeat stub impl beat once");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t BleHeartbeatOnceEnd(const LnnHeartbeatSendEndData *custData)
{
    (void)custData;

    LNN_LOGI(LNN_HEART_BEAT, "ble heartbeat stub impl beat end");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t SetBleMediumParam(const LnnHeartbeatMediumParam *param)
{
    (void)param;

    LNN_LOGI(LNN_HEART_BEAT, "ble heartbeat stub impl set medium param");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t UpdateBleSendInfo(LnnHeartbeatUpdateInfoType type)
{
    (void)type;

    LNN_LOGI(LNN_HEART_BEAT, "ble heartbeat stub impl update send info");
    return SOFTBUS_NOT_IMPLEMENT;
}

int32_t StopBleHeartbeat(void)
{
    LNN_LOGI(LNN_HEART_BEAT, "ble heartbeat stub impl beat stop");
    return SOFTBUS_NOT_IMPLEMENT;
}

void DeinitBleHeartbeat(void)
{
    LNN_LOGI(LNN_INIT, "ble heartbeat stub impl deinit");
    return;
}

LnnHeartbeatMediumMgr g_bleMgr = {
    .supportType = HEARTBEAT_TYPE_BLE_V0 | HEARTBEAT_TYPE_BLE_V1,
    .init = InitBleHeartbeat,
    .onSendOneHbBegin = BleHeartbeatOnceBegin,
    .onSendOneHbEnd = BleHeartbeatOnceEnd,
    .onSetMediumParam = SetBleMediumParam,
    .onUpdateSendInfo = UpdateBleSendInfo,
    .onStopHbByType = StopBleHeartbeat,
    .deinit = DeinitBleHeartbeat,
};
#endif

SOFTBUS_VISIBILITY int32_t AuthMetaOpenConnPacked(const AuthConnInfo *info, uint32_t requestId, const AuthConnCallback *callback)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaOpenConn) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaOpenConn(info, requestId, callback);
}

SOFTBUS_VISIBILITY int32_t AuthMetaPostTransDataPacked(int64_t authId, const AuthTransData *dataInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaPostTransData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaPostTransData(authId, dataInfo);
}

SOFTBUS_VISIBILITY void AuthMetaCloseConnPacked(int64_t authId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaCloseConn) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authMetaCloseConn(authId);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetPreferConnInfoPacked(const char *networkId, AuthConnInfo *connInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetPreferConnInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetPreferConnInfo(networkId, connInfo);
}

SOFTBUS_VISIBILITY int64_t AuthMetaGetIdByConnInfoPacked(const AuthConnInfo *connInfo, bool isServer)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetIdByConnInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetIdByConnInfo(connInfo, isServer);
}

SOFTBUS_VISIBILITY int64_t AuthMetaGetIdByUuidPacked(const char *uuid, AuthLinkType type, bool isServer)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetIdByUuid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetIdByUuid(uuid, type, isServer);
}

SOFTBUS_VISIBILITY int64_t AuthMetaGetIdByIpPacked(const char *ip)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetIdByIp) != SOFTBUS_OK) {
        return AUTH_INVALID_ID;
    }
    return pfnLnnEnhanceFuncList->authMetaGetIdByIp(ip);
}

SOFTBUS_VISIBILITY int32_t AuthMetaEncryptPacked(int64_t authId, const uint8_t *inData, uint32_t inLen,
    uint8_t *outData, uint32_t *outLen)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaEncrypt) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaEncrypt(authId, inData, inLen, outData, outLen);
}


SOFTBUS_VISIBILITY int32_t AuthMetaDecryptPacked(int64_t authId, const uint8_t *inData, uint32_t inLen,
    uint8_t *outData, uint32_t *outLen)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaDecrypt) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaDecrypt(authId, inData, inLen, outData, outLen);
}

SOFTBUS_VISIBILITY int32_t AuthMetaSetP2pMacPacked(int64_t authId, const char *p2pMac)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaSetP2pMac) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaSetP2pMac(authId, p2pMac);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetConnInfoPacked(int64_t authId, AuthConnInfo *connInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetConnInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetConnInfo(authId, connInfo);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetDeviceUuidPacked(int64_t authId, char *uuid, uint16_t size)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetDeviceUuid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetDeviceUuid(authId, uuid, size);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetServerSidePacked(int64_t authId, bool *isServer)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetServerSide) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetServerSide(authId, isServer);
}

SOFTBUS_VISIBILITY void AuthMetaCheckMetaExistPacked(const AuthConnInfo *connInfo, bool *isExist)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaCheckMetaExist) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authMetaCheckMetaExist(connInfo, isExist);
}

SOFTBUS_VISIBILITY int32_t CustomizedSecurityProtocolInitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->customizedSecurityProtocolInit) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->customizedSecurityProtocolInit();
}

SOFTBUS_VISIBILITY void CustomizedSecurityProtocolDeinitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->customizedSecurityProtocolDeinit) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->customizedSecurityProtocolDeinit();
}

SOFTBUS_VISIBILITY void AuthMetaDeinitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaDeinit) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authMetaDeinit();
}

SOFTBUS_VISIBILITY void DelAuthMetaManagerByPidPacked(const char *pkgName, int32_t pid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->delAuthMetaManagerByPid) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->delAuthMetaManagerByPid(pkgName, pid);
}

SOFTBUS_VISIBILITY void ClearMetaNodeRequestByPidPacked(const char *pkgName, int32_t pid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->clearMetaNodeRequestByPid) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->clearMetaNodeRequestByPid(pkgName, pid);
}

SOFTBUS_VISIBILITY void LnnClearAuthExchangeUdidPacked(const char *networkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnClearAuthExchangeUdid) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnClearAuthExchangeUdid(networkId);
}

SOFTBUS_VISIBILITY int32_t AuthInsertDeviceKeyPacked(const NodeInfo *deviceInfo, const AuthDeviceKeyInfo *deviceKey,
    AuthLinkType type)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authInsertDeviceKey) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authInsertDeviceKey(deviceInfo, deviceKey, type);
}

SOFTBUS_VISIBILITY void AuthUpdateKeyIndexPacked(const char *udidHash, int32_t keyType, int64_t index, bool isServer)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authUpdateKeyIndex) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authUpdateKeyIndex(udidHash, keyType, index, isServer);
}

SOFTBUS_VISIBILITY int32_t LnnGenerateLocalPtkPacked(char *udid, char *uuid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGenerateLocalPtk) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnGenerateLocalPtk(udid, uuid);
}

SOFTBUS_VISIBILITY bool CalcHKDFPacked(const uint8_t *ikm, uint32_t ikmLen, uint8_t *out, uint32_t outLen)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->calcHKDF) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->calcHKDF(ikm, ikmLen, out, outLen);
}

SOFTBUS_VISIBILITY void AuthUpdateCreateTimePacked(const char *udidHash, int32_t keyType, bool isServer)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authUpdateCreateTime) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authUpdateCreateTime(udidHash, keyType, isServer);
}

SOFTBUS_VISIBILITY int32_t AuthFindNormalizeKeyByServerSidePacked(const char *udidHash, bool isServer, AuthDeviceKeyInfo *deviceKey)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authFindNormalizeKeyByServerSide) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authFindNormalizeKeyByServerSide(udidHash, isServer, deviceKey);
}

SOFTBUS_VISIBILITY bool IsSupportUDIDAbatementPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isSupportUDIDAbatement) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isSupportUDIDAbatement();
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetConnIdByInfoPacked(const AuthConnInfo *connInfo, uint32_t *connectionId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetConnIdByInfo) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->authMetaGetConnIdByInfo(connInfo, connectionId);
}

SOFTBUS_VISIBILITY int32_t LnnGetMetaPtkPacked(uint32_t connId, char *metaPtk, uint32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetMetaPtk) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnGetMetaPtk(connId, metaPtk, len);
}

SOFTBUS_VISIBILITY bool PackCipherKeySyncMsgPacked(void *json)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->packCipherKeySyncMsg) != SOFTBUS_OK) {
        return true;
    }
    return pfnLnnEnhanceFuncList->packCipherKeySyncMsg(json);
}

SOFTBUS_VISIBILITY void ProcessCipherKeySyncInfoPacked(const void *json, const char *networkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->processCipherKeySyncInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->processCipherKeySyncInfo(json, networkId);
}

SOFTBUS_VISIBILITY void FreeSoftbusChainPacked(SoftbusCertChain *softbusCertChain)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->freeSoftbusChain) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->freeSoftbusChain(softbusCertChain);
}

SOFTBUS_VISIBILITY int32_t InitSoftbusChainPacked(SoftbusCertChain *softbusCertChain)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->initSoftbusChain) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->initSoftbusChain(softbusCertChain);
}

SOFTBUS_VISIBILITY int32_t LnnSyncTrustedRelationShipPacked(const char *pkgName, const char *msg, uint32_t msgLen)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSyncTrustedRelationShip) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnSyncTrustedRelationShip(pkgName, msg, msgLen);
}

SOFTBUS_VISIBILITY int32_t LnnProcessPushMsgPacked(const uint8_t *data, uint32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnProcessPushMsg) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnProcessPushMsg(data, len);
}

SOFTBUS_VISIBILITY void LnnCoapConnectPacked(const char *ip)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnCoapConnect) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnCoapConnect(ip);
}

SOFTBUS_VISIBILITY void LnnDestroyCoapConnectListPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDestroyCoapConnectList) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDestroyCoapConnectList();
}

SOFTBUS_VISIBILITY int32_t LnnInitQosPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitQos) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitQos();
}

SOFTBUS_VISIBILITY void LnnDeinitQosPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitQos) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitQos();
}

SOFTBUS_VISIBILITY int32_t LnnSyncBleOfflineMsgPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSyncBleOfflineMsg) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnSyncBleOfflineMsg();
}

SOFTBUS_VISIBILITY void LnnBleHbRegDataLevelChangeCbPacked(const IDataLevelChangeCallback *callback)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnBleHbRegDataLevelChangeCb) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnBleHbRegDataLevelChangeCb(callback);
}

SOFTBUS_VISIBILITY void LnnBleHbUnregDataLevelChangeCbPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnBleHbUnregDataLevelChangeCb) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnBleHbUnregDataLevelChangeCb();
}

SOFTBUS_VISIBILITY int32_t DecryptUserIdPacked(NodeInfo *deviceInfo, uint8_t *advUserId, uint32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->decryptUserId) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->decryptUserId(deviceInfo, advUserId, len);
}

SOFTBUS_VISIBILITY bool IsCloudSyncEnabledPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isCloudSyncEnabled) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isCloudSyncEnabled();
}

SOFTBUS_VISIBILITY int32_t AuthFindDeviceKeyPacked(const char *udidHash, int32_t keyType, AuthDeviceKeyInfo *deviceKey)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authFindDeviceKey) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authFindDeviceKey(udidHash, keyType, deviceKey);
}

SOFTBUS_VISIBILITY int32_t AuthFindLatestNormalizeKeyPacked(const char *udidHash, AuthDeviceKeyInfo *deviceKey, bool clearOldKey)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authFindLatestNormalizeKey) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authFindLatestNormalizeKey(udidHash, deviceKey, clearOldKey);
}

SOFTBUS_SYMBOL_EXPORT bool IsCipherManagerFindKeyPk(const char *udid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isCipherManagerFindKey) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isCipherManagerFindKey(udid);
}

SOFTBUS_VISIBILITY int32_t LnnAddRemoteChannelCodePacked(const char *udid, int32_t channelCode)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnAddRemoteChannelCode) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnAddRemoteChannelCode(udid, channelCode);
}

SOFTBUS_VISIBILITY int32_t LnnRegistBleHeartbeatMediumMgrPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRegistBleHeartbeatMediumMgr) != SOFTBUS_OK) {
    #ifdef ENABLE_FEATURE_LNN_BLE
        return LnnRegistHeartbeatMediumMgr(&g_bleMgr);
    #else
        return SOFTBUS_OK;
    #endif
    }
    return pfnLnnEnhanceFuncList->lnnRegistBleHeartbeatMediumMgr();
}

SOFTBUS_VISIBILITY int32_t LnnRegisterBleLpDeviceMediumMgrPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRegisterBleLpDeviceMediumMgr) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnRegisterBleLpDeviceMediumMgr();
}

SOFTBUS_VISIBILITY int32_t LnnRegisterSleHeartbeatMediumMgrPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRegisterSleHeartbeatMediumMgr) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnRegisterSleHeartbeatMediumMgr();
}

SOFTBUS_VISIBILITY bool HaveConcurrencyPreLinkReqIdByReuseConnReqIdPacked(uint32_t connReqId, bool isCheckPreLink)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->haveConcurrencyPreLinkReqIdByReuseConnReqId) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->haveConcurrencyPreLinkReqIdByReuseConnReqId(connReqId, isCheckPreLink);
}

SOFTBUS_VISIBILITY bool HaveConcurrencyPreLinkNodeByLaneReqIdPacked(uint32_t laneReqId, bool isCheckPreLink)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return false;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->haveConcurrencyPreLinkNodeByLaneReqId) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->haveConcurrencyPreLinkNodeByLaneReqId(laneReqId, isCheckPreLink);
}

SOFTBUS_VISIBILITY int32_t GetConcurrencyLaneReqIdByConnReqIdPacked(uint32_t connReqId, uint32_t *laneReqId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->getConcurrencyLaneReqIdByConnReqId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->getConcurrencyLaneReqIdByConnReqId(connReqId, laneReqId);
}

SOFTBUS_VISIBILITY void LnnFreePreLinkPacked(void *para)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnFreePreLink) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnFreePreLink(para);
}

SOFTBUS_VISIBILITY int32_t LnnRequestCheckOnlineStatusPacked(const char *networkId, uint64_t timeout)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRequestCheckOnlineStatus) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRequestCheckOnlineStatus(networkId, timeout);
}

SOFTBUS_VISIBILITY int32_t LnnSyncPtkPacked(const char *networkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSyncPtk) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnSyncPtk(networkId);
}

SOFTBUS_VISIBILITY int32_t GetConcurrencyLaneReqIdByActionIdPacked(uint32_t actionId, uint32_t *laneReqId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->getConcurrencyLaneReqIdByActionId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->getConcurrencyLaneReqIdByActionId(actionId, laneReqId);
}

SOFTBUS_VISIBILITY int32_t UpdateConcurrencyReuseLaneReqIdByActionIdPacked(uint32_t actionId, uint32_t reuseLaneReqId, uint32_t connReqId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->updateConcurrencyReuseLaneReqIdByActionId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->updateConcurrencyReuseLaneReqIdByActionId(actionId, reuseLaneReqId, connReqId);
}

SOFTBUS_VISIBILITY int32_t UpdateConcurrencyReuseLaneReqIdByUdidPacked(
    const char *udidHash, uint32_t udidHashLen, uint32_t reuseLaneReqId, uint32_t connReqId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->updateConcurrencyReuseLaneReqIdByUdid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->updateConcurrencyReuseLaneReqIdByUdid(
        udidHash, udidHashLen, reuseLaneReqId, connReqId);
}

SOFTBUS_VISIBILITY int32_t LnnAddLocalVapInfoPacked(LnnVapType type, const LnnVapAttr *vapAttr)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnAddLocalVapInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnAddLocalVapInfo(type, vapAttr);
}

SOFTBUS_VISIBILITY int32_t LnnDeleteLocalVapInfoPacked(LnnVapType type)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeleteLocalVapInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnDeleteLocalVapInfo(type);
}

SOFTBUS_VISIBILITY void DisablePowerControlPacked(const WifiDirectLinkInfo *wifiDirectInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->disablePowerControl) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->disablePowerControl(wifiDirectInfo);
}

SOFTBUS_VISIBILITY int32_t EnablePowerControlPacked(const WifiDirectLinkInfo *wifiDirectInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->enablePowerControl) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->enablePowerControl(wifiDirectInfo);
}

SOFTBUS_VISIBILITY int32_t LnnInitScorePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitScore) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitScore();
}

SOFTBUS_VISIBILITY int32_t LnnStartScoringPacked(int32_t interval)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnStartScoring) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnStartScoring(interval);
}

SOFTBUS_VISIBILITY int32_t LnnInitVapInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitVapInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnInitVapInfo();
}

SOFTBUS_VISIBILITY void LnnDeinitScorePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitScore) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitScore();
}

SOFTBUS_VISIBILITY void LnnDeinitVapInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitVapInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitVapInfo();
}

SOFTBUS_VISIBILITY int32_t LnnGetWlanLinkedInfoPacked(LnnWlanLinkedInfo *info)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetWlanLinkedInfo) != SOFTBUS_OK) {
        return SOFTBUS_LANE_SELECT_FAIL;
    }
    return pfnLnnEnhanceFuncList->lnnGetWlanLinkedInfo(info);
}

SOFTBUS_VISIBILITY int32_t LnnGetCurrChannelScorePacked(int32_t channelId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetCurrChannelScore) != SOFTBUS_OK) {
        return VIRTUAL_DEFAULT_SCORE;
    }
    return pfnLnnEnhanceFuncList->lnnGetCurrChannelScore(channelId);
}

SOFTBUS_VISIBILITY bool IsPowerControlEnabledPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isPowerControlEnabled) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isPowerControlEnabled();
}

SOFTBUS_VISIBILITY int32_t LnnStartTimeSyncImplPacked(const char *targetNetworkId, TimeSyncAccuracy accuracy,
    TimeSyncPeriod period, const TimeSyncImplCallback *callback)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnStartTimeSyncImpl) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnStartTimeSyncImpl(targetNetworkId, accuracy, period, callback);
}

SOFTBUS_VISIBILITY int32_t LnnStopTimeSyncImplPacked(const char *targetNetworkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnStopTimeSyncImpl) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnStopTimeSyncImpl(targetNetworkId);
}

SOFTBUS_VISIBILITY int32_t LnnTimeSyncImplInitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL ||
        LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnTimeSyncImplInit) != SOFTBUS_OK) {
        LNN_LOGE(LNN_INIT, "LnnTimeSyncImplInit get fail");
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnTimeSyncImplInit();
}

SOFTBUS_VISIBILITY int32_t LnnTimeChangeNotifyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL ||
        LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnTimeChangeNotify) != SOFTBUS_OK) {
        LNN_LOGE(LNN_INIT, "LnnTimeChangeNotify get fail");
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnTimeChangeNotify();
}

SOFTBUS_VISIBILITY void LnnTimeSyncImplDeinitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnTimeSyncImplDeinit) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnTimeSyncImplDeinit();
}

SOFTBUS_VISIBILITY void SendDeviceStateToMlpsPacked(void *para)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->sendDeviceStateToMlps) != SOFTBUS_OK) {
        SoftBusFree(para);
        return;
    }
    return pfnLnnEnhanceFuncList->sendDeviceStateToMlps(para);
}

SOFTBUS_VISIBILITY int32_t LnnRetrieveDeviceInfoByNetworkIdPacked(const char *networkId, NodeInfo *info)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRetrieveDeviceInfoByNetworkId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRetrieveDeviceInfoByNetworkId(networkId, info);
}

SOFTBUS_VISIBILITY void SetLpKeepAliveStatePacked(void *para)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->setLpKeepAliveState) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->setLpKeepAliveState(para);
}

SOFTBUS_VISIBILITY int32_t LnnSetRemoteBroadcastCipherInfoPacked(const char *value, const char *udid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSetRemoteBroadcastCipherInfo) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnSetRemoteBroadcastCipherInfo(value, udid);
}

SOFTBUS_VISIBILITY int32_t LnnGetLocalCacheNodeInfoPacked(NodeInfo *info)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetLocalCacheNodeInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetLocalCacheNodeInfo(info);
}

SOFTBUS_VISIBILITY void LnnDeleteDeviceInfoPacked(const char *udid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeleteDeviceInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeleteDeviceInfo(udid);
}

SOFTBUS_VISIBILITY int32_t LnnUnPackCloudSyncDeviceInfoPacked(cJSON *json, NodeInfo *cloudSyncInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUnPackCloudSyncDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnUnPackCloudSyncDeviceInfo(json, cloudSyncInfo);
}

SOFTBUS_VISIBILITY int32_t LnnPackCloudSyncDeviceInfoPacked(cJSON *json, const NodeInfo *cloudSyncInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnPackCloudSyncDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnPackCloudSyncDeviceInfo(json, cloudSyncInfo);
}

SOFTBUS_VISIBILITY int32_t LnnGetLocalBroadcastCipherInfoPacked(CloudSyncInfo *info)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetLocalBroadcastCipherInfo) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnGetLocalBroadcastCipherInfo(info);
}

SOFTBUS_VISIBILITY int32_t LnnPackCloudSyncAckSeqPacked(cJSON *json, char *peerudid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnPackCloudSyncAckSeq) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnPackCloudSyncAckSeq(json, peerudid);
}

SOFTBUS_VISIBILITY int32_t LnnInitCipherKeyManagerPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitCipherKeyManager) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitCipherKeyManager();
}

SOFTBUS_VISIBILITY int32_t LnnSendNotTrustedInfoPacked(const NotTrustedDelayInfo *info, uint32_t num,
    LnnSyncInfoMsgComplete complete)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSendNotTrustedInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSendNotTrustedInfo(info, num, complete);
}

SOFTBUS_VISIBILITY void RegisterOOBEMonitorPacked(void *para)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->registerOOBEMonitor) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->registerOOBEMonitor(para);
}

SOFTBUS_VISIBILITY int32_t LnnLinkFinderInitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLinkFinderInit) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnLinkFinderInit();
}

SOFTBUS_VISIBILITY int32_t LnnInitFastOfflinePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitFastOffline) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitFastOffline();
}

SOFTBUS_VISIBILITY void LnnDeinitFastOfflinePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitFastOffline) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitFastOffline();
}

SOFTBUS_VISIBILITY int32_t LnnDeviceCloudConvergenceInitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeviceCloudConvergenceInit) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnDeviceCloudConvergenceInit();
}

SOFTBUS_VISIBILITY int32_t LnnRemoveLinkFinderInfoPacked(const char *networkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRemoveLinkFinderInfo) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnRemoveLinkFinderInfo(networkId);
}

SOFTBUS_VISIBILITY int32_t LnnUpdateLastAccLoginTimestampByUdidPacked(uint64_t timestamp, const char *udid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUpdateLastAccLoginTimestampByUdid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnUpdateLastAccLoginTimestampByUdid(timestamp, udid);
}

SOFTBUS_VISIBILITY int32_t LnnRetrieveDeviceInfoByUdidPacked(const char *udid, NodeInfo *deviceInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRetrieveDeviceInfoByUdid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRetrieveDeviceInfoByUdid(udid, deviceInfo);
}

SOFTBUS_VISIBILITY int32_t LnnInitBroadcastLinkKeyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitBroadcastLinkKey) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnInitBroadcastLinkKey();
}

SOFTBUS_VISIBILITY int32_t LnnInitPtkPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitPtk) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitPtk();
}

SOFTBUS_VISIBILITY void LnnDeinitBroadcastLinkKeyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitBroadcastLinkKey) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitBroadcastLinkKey();
}

SOFTBUS_VISIBILITY void LnnDeinitPtkPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitPtk) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitPtk();
}

SOFTBUS_VISIBILITY void LnnIpAddrChangeEventHandlerPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnIpAddrChangeEventHandler) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnIpAddrChangeEventHandler();
}

SOFTBUS_VISIBILITY void LnnInitOOBEStateMonitorImplPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitOOBEStateMonitorImpl) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnInitOOBEStateMonitorImpl();
}

SOFTBUS_VISIBILITY void EhLoginEventHandlerPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->ehLoginEventHandler) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->ehLoginEventHandler();
}

SOFTBUS_VISIBILITY int32_t LnnInitMetaNodeExtLedgerPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitMetaNodeExtLedger) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitMetaNodeExtLedger();
}

SOFTBUS_VISIBILITY void LnnDeinitMetaNodeExtLedgerPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitMetaNodeExtLedger) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitMetaNodeExtLedger();
}

SOFTBUS_VISIBILITY bool IsSupportLpFeaturePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isSupportLpFeature) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isSupportLpFeature();
}

SOFTBUS_VISIBILITY bool LnnIsSupportLpSparkFeaturePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return false;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnIsSupportLpSparkFeature) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->lnnIsSupportLpSparkFeature();
}

SOFTBUS_VISIBILITY bool LnnIsFeatureSupportDetailPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return false;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isFeatureSupportDetail) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isFeatureSupportDetail();
}

SOFTBUS_VISIBILITY void AuthLoadDeviceKeyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authLoadDeviceKey) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authLoadDeviceKey();
}

SOFTBUS_VISIBILITY void UpdateLocalDeviceInfoToMlpsPacked(const NodeInfo *localInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->updateLocalDeviceInfoToMlps) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->updateLocalDeviceInfoToMlps(localInfo);
}

SOFTBUS_VISIBILITY int32_t LnnLoadLocalDeviceInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadLocalDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnLoadLocalDeviceInfo();
}

SOFTBUS_VISIBILITY void LnnLoadPtkInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadPtkInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnLoadPtkInfo();
}

SOFTBUS_VISIBILITY int32_t LnnLoadRemoteDeviceInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadRemoteDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnLoadRemoteDeviceInfo();
}

SOFTBUS_VISIBILITY void LoadBleBroadcastKeyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->loadBleBroadcastKey) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->loadBleBroadcastKey();
}

SOFTBUS_VISIBILITY void LnnClearPtkListPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnClearPtkList) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnClearPtkList();
}

SOFTBUS_VISIBILITY void ClearDeviceInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->clearDeviceInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->clearDeviceInfo();
}

SOFTBUS_VISIBILITY int32_t GenerateNewLocalCipherKeyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->generateNewLocalCipherKey) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->generateNewLocalCipherKey();
}

SOFTBUS_VISIBILITY int32_t LnnRetrieveDeviceInfoPacked(const char *udid, NodeInfo *deviceInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRetrieveDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRetrieveDeviceInfo(udid, deviceInfo);
}

SOFTBUS_VISIBILITY int32_t LnnSaveRemoteDeviceInfoPacked(const NodeInfo *deviceInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSaveRemoteDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSaveRemoteDeviceInfo(deviceInfo);
}

SOFTBUS_VISIBILITY int32_t LnnInsertLinkFinderInfoPacked(const char *networkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInsertLinkFinderInfo) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInsertLinkFinderInfo(networkId);
}

SOFTBUS_VISIBILITY int32_t LnnUpdateRemoteDeviceInfoPacked(const NodeInfo *deviceInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUpdateRemoteDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnUpdateRemoteDeviceInfo(deviceInfo);
}

SOFTBUS_VISIBILITY int32_t LnnSaveLocalDeviceInfoPacked(const NodeInfo *deviceInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSaveLocalDeviceInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSaveLocalDeviceInfo(deviceInfo);
}

SOFTBUS_VISIBILITY int32_t LnnGetAccountIdFromLocalCachePacked(int64_t *buf)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetAccountIdFromLocalCache) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetAccountIdFromLocalCache(buf);
}

SOFTBUS_VISIBILITY int32_t LnnGetLocalDevInfoPacked(NodeInfo *deviceInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetLocalDevInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetLocalDevInfo(deviceInfo);
}

SOFTBUS_VISIBILITY int32_t LnnGetLocalBroadcastCipherKeyPacked(BroadcastCipherKey *broadcastKey)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetLocalBroadcastCipherKey) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnGetLocalBroadcastCipherKey(broadcastKey);
}

SOFTBUS_VISIBILITY int32_t LnnLoadLocalBroadcastCipherKeyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadLocalBroadcastCipherKey) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnLoadLocalBroadcastCipherKey();
}

SOFTBUS_VISIBILITY int32_t LnnUpdateLocalBroadcastCipherKeyPacked(BroadcastCipherKey *broadcastKey)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUpdateLocalBroadcastCipherKey) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnUpdateLocalBroadcastCipherKey(broadcastKey);
}

SOFTBUS_VISIBILITY int32_t HbBuildUserIdCheckSumPacked(const int32_t *userIdArray, int32_t num, uint8_t *custData, int32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->hbBuildUserIdCheckSum) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->hbBuildUserIdCheckSum(userIdArray, num, custData, len);
}

SOFTBUS_VISIBILITY void LnnUpdateAuthExchangeUdidPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUpdateAuthExchangeUdid) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnUpdateAuthExchangeUdid();
}

SOFTBUS_VISIBILITY void LnnCoapConnectInitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnCoapConnectInit) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnCoapConnectInit();
}

SOFTBUS_VISIBILITY int32_t LnnInitMetaNodePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitMetaNode) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitMetaNode();
}

SOFTBUS_VISIBILITY int32_t InitActionBleConcurrencyPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->initActionBleConcurrency) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->initActionBleConcurrency();
}

SOFTBUS_VISIBILITY int32_t InitActionStateAdapterPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->initActionStateAdapter) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->initActionStateAdapter();
}

SOFTBUS_VISIBILITY int32_t LnnLoadLocalDeviceAccountIdInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadLocalDeviceAccountIdInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnLoadLocalDeviceAccountIdInfo();
}

SOFTBUS_VISIBILITY void LnnDeinitMetaNodePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitMetaNode) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitMetaNode();
}

SOFTBUS_VISIBILITY void LnnCoapConnectDeinitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnCoapConnectDeinit) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnCoapConnectDeinit();
}

SOFTBUS_VISIBILITY int32_t LnnGetOOBEStatePacked(SoftBusOOBEState *state)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetOOBEState) != SOFTBUS_OK) {
        if (state == NULL) {
            return SOFTBUS_INVALID_PARAM;
        }

        *state = SOFTBUS_OOBE_END;
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnGetOOBEState(state);
}

SOFTBUS_VISIBILITY void LnnReportLaneIdStatsInfoPacked(const LaneIdStatsInfo *statsList, uint32_t listSize)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnReportLaneIdStatsInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnReportLaneIdStatsInfo(statsList, listSize);
}

SOFTBUS_VISIBILITY int32_t LnnRequestQosOptimizationPacked(const uint64_t *laneIdList, uint32_t listSize, int32_t *result,
    uint32_t resultSize)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRequestQosOptimization) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRequestQosOptimization(laneIdList, listSize, result, resultSize);
}

SOFTBUS_VISIBILITY void LnnCancelQosOptimizationPacked(const uint64_t *laneIdList, uint32_t listSize)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnCancelQosOptimization) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnCancelQosOptimization(laneIdList, listSize);
}

SOFTBUS_VISIBILITY void LnnReportRippleDataPacked(uint64_t laneId, const LnnRippleData *data)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnReportRippleData) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnReportRippleData(laneId, data);
}

SOFTBUS_VISIBILITY int32_t LnnGetUdidByBrMacPacked(const char *brMac, char *udid, uint32_t udidLen)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetUdidByBrMac) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetUdidByBrMac(brMac, udid, udidLen);
}

SOFTBUS_VISIBILITY void AuthRemoveDeviceKeyByUdidPacked(const char *udidOrHash)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authRemoveDeviceKeyByUdid) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->authRemoveDeviceKeyByUdid(udidOrHash);
}

SOFTBUS_VISIBILITY int32_t LnnGetRecommendChannelPacked(const char *udid, int32_t *preferChannel)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetRecommendChannel) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetRecommendChannel(udid, preferChannel);
}

SOFTBUS_VISIBILITY int32_t LnnGetLocalPtkByUuidPacked(const char *uuid, char *localPtk, uint32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetLocalPtkByUuid) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnGetLocalPtkByUuid(uuid, localPtk, len);
}

SOFTBUS_VISIBILITY int32_t LnnInitSleRangePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitSLeRange) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitSLeRange();
}

SOFTBUS_VISIBILITY void LnnDeinitSleRangePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitSleRange) != SOFTBUS_OK) {
        return;
    }
    pfnLnnEnhanceFuncList->lnnDeinitSleRange();
}

SOFTBUS_VISIBILITY int32_t LnnStartRangePacked(const RangeConfig *config)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnStartRange) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnStartRange(config);
}

SOFTBUS_VISIBILITY int32_t LnnStopRangePacked(const RangeConfig *config)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnStopRange) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnStopRange(config);
}

SOFTBUS_VISIBILITY void LnnRegSleRangeCbPacked(const ISleRangeInnerCallback *callback)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRegSleRangeCb) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnRegSleRangeCb(callback);
}

SOFTBUS_VISIBILITY void LnnUnregSleRangeCbPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUnregSleRangeCb) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnUnregSleRangeCb();
}

SOFTBUS_VISIBILITY void SleRangeDeathCallbackPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->sleRangeDeathCallback) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->sleRangeDeathCallback();
}

SOFTBUS_VISIBILITY bool IsSupportLowLatencyPacked(const TransReqInfo *reqInfo, const LaneLinkInfo *laneLinkInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isSupportLowLatency) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "Is support low latency packed, func pointer is NULL");
        return false;
    }
    return pfnLnnEnhanceFuncList->isSupportLowLatency(reqInfo, laneLinkInfo);
}

SOFTBUS_VISIBILITY int32_t LnnRetrieveDeviceDataPacked(LnnDataType dataType, char **data, uint32_t *dataLen)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL ||
        LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRetrieveDeviceData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRetrieveDeviceData(dataType, data, dataLen);
}

SOFTBUS_VISIBILITY int32_t LnnSaveDeviceDataPacked(const char *data, LnnDataType dataType)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSaveDeviceData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSaveDeviceData(data, dataType);
}

SOFTBUS_VISIBILITY int32_t LnnVirtualLinkInitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_OK;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnVirtualLinkInit) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "init packed, func pointer is NULL");
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnVirtualLinkInit();
}

SOFTBUS_VISIBILITY void LnnVirtualLinkDeinitPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnVirtualLinkDeinit) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "Deinit packed, func pointer is NULL");
        return;
    }
    pfnLnnEnhanceFuncList->lnnVirtualLinkDeinit();
}

SOFTBUS_VISIBILITY int32_t LnnRegisterPushListenerPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnRegisterPushListener) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnRegisterPushListener();
}

SOFTBUS_VISIBILITY int32_t DcTriggerVirtualLinkPacked(const char *peerNetworkId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->dcTriggerVirtualLink) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "trigger virtual link, func pointer is NULL");
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->dcTriggerVirtualLink(peerNetworkId);
}

SOFTBUS_VISIBILITY int32_t LnnGetLocalChannelInfoPacked(VapChannelInfo *channelInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetLocalChannelInfo) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "lnn get local channel info, func pointer is NULL");
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetLocalChannelInfo(channelInfo);
}

SOFTBUS_VISIBILITY int32_t LnnSetLocalChannelInfoPacked(LnnVapType type, int32_t channelId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSetLocalChannelInfo) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "lnn set local channel info, func pointer is NULL");
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSetLocalChannelInfo(type, channelId);
}

SOFTBUS_VISIBILITY void TriggerSparkGroupBuildPacked(uint32_t delayTime)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->triggerSparkGroupBuild) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->triggerSparkGroupBuild(delayTime);
}

SOFTBUS_VISIBILITY void TriggerSparkGroupClearPacked(uint32_t state, uint32_t delayTime)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->triggerSparkGroupClear) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->triggerSparkGroupClear(state, delayTime);
}

SOFTBUS_VISIBILITY void TriggerSparkGroupJoinAgainPacked(const char *udid, uint32_t delayTime)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->triggerSparkGroupJoinAgain) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->triggerSparkGroupJoinAgain(udid, delayTime);
}

SOFTBUS_VISIBILITY int32_t InitControlPlanePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->initControlPlane) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->initControlPlane();
}

SOFTBUS_VISIBILITY void DeinitControlPlanePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->deinitControlPlane) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->deinitControlPlane();
}

SOFTBUS_VISIBILITY int32_t QueryControlPlaneNodeValidPacked(const char *deviceId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->queryControlPlaneNodeValid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->queryControlPlaneNodeValid(deviceId);
}

SOFTBUS_VISIBILITY int32_t LnnDumpControlLaneGroupInfoPacked(int32_t fd)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDumpControlLaneGroupInfo) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnDumpControlLaneGroupInfo(fd);
}

SOFTBUS_VISIBILITY bool IsSparkGroupEnabledPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return false;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isSparkGroupEnabled) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isSparkGroupEnabled();
}

SOFTBUS_VISIBILITY bool IsDeviceHasRiskFactorPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL ||
        LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isDeviceHasRiskFactor) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isDeviceHasRiskFactor();
}

SOFTBUS_VISIBILITY void LnnDeinitLocalUserInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitLocalUserInfo) != SOFTBUS_OK) {
        return;
    }
    return pfnLnnEnhanceFuncList->lnnDeinitLocalUserInfo();
}

SOFTBUS_VISIBILITY int32_t LnnPackCloudSyncUserInfoPacked(cJSON *json, const UserInfo *userInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnPackCloudSyncUserInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnPackCloudSyncUserInfo(json, userInfo);
}

SOFTBUS_VISIBILITY int32_t LnnUnPackCloudSyncUserInfoPacked(cJSON *json, UserInfo *userInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnUnPackCloudSyncUserInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnUnPackCloudSyncUserInfo(json, userInfo);
}

SOFTBUS_VISIBILITY bool LnnCheckUserExistsByAccountIdPacked(int64_t accountId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnCheckUserExistsByAccountId) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->lnnCheckUserExistsByAccountId(accountId);
}

SOFTBUS_VISIBILITY int32_t LnnSaveLocalUserInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSaveLocalUserInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSaveLocalUserInfo();
}

SOFTBUS_VISIBILITY int32_t LnnLoadLocalUserInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadLocalUserInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnLoadLocalUserInfo();
}

SOFTBUS_VISIBILITY int32_t LnnSaveRemoteUserInfoPacked(const char *udid, const UserInfo *userInfo)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSaveRemoteUserInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSaveRemoteUserInfo(udid, userInfo);
}

SOFTBUS_VISIBILITY int32_t LnnLoadRemoteUserInfoPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnLoadRemoteUserInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnLoadRemoteUserInfo();
}

SOFTBUS_VISIBILITY int32_t LnnAsyncSaveDeviceDataPacked(const char *data, LnnDataType dataType)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnAsyncSaveDeviceData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnAsyncSaveDeviceData(data, dataType);
}

SOFTBUS_VISIBILITY int32_t LnnDeleteDeviceDataPacked(LnnDataType dataType)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeleteDeviceData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnDeleteDeviceData(dataType);
}

SOFTBUS_VISIBILITY void CheckNeedCloudSyncOfflinePacked(DiscoveryType type)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->checkNeedCloudSyncOffline) != SOFTBUS_OK) {
        return;
    }
    pfnLnnEnhanceFuncList->checkNeedCloudSyncOffline(type);
}

SOFTBUS_VISIBILITY int32_t LnnInitDecisionCenterV2Packed(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_OK;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitDecisionCenterV2) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "init packed, func pointer is NULL");
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitDecisionCenterV2();
}

SOFTBUS_VISIBILITY void LnnDeinitDecisionCenterV2Packed(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnDeinitDecisionCenterV2) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "deinit packed, func pointer is NULL");
        return;
    }
    pfnLnnEnhanceFuncList->lnnDeinitDecisionCenterV2();
}

SOFTBUS_VISIBILITY void SdMgrDeathCallbackPacked(const char *pkgName)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->sdMgrDeathCallback) != SOFTBUS_OK) {
        LNN_LOGI(LNN_LANE, "sd death callback packed, func pointer is NULL");
        return;
    }
    pfnLnnEnhanceFuncList->sdMgrDeathCallback(pkgName);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetIpByMetaNodeIdPacked(const char *metaNodeId, char *ip, int32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetIpByMetaNodeId) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "func pointer is NULL");
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetIpByMetaNodeId(metaNodeId, ip, len);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetLocalIpByMetaNodeIdPacked(const char *metaNodeId, char *localIp, int32_t len)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetLocalIpByMetaNodeId) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "func pointer is NULL");
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetLocalIpByMetaNodeId(metaNodeId, localIp, len);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetConnectionTypeByMetaNodeIdPacked(const char *metaNodeId, NetworkConnectionType *connectionType)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetConnectionTypeByMetaNodeId) != SOFTBUS_OK) {
        LNN_LOGE(LNN_LANE, "func pointer is NULL");
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetConnectionTypeByMetaNodeId(metaNodeId, connectionType);
}

SOFTBUS_VISIBILITY bool IsSupportMcuFeaturePacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return false;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->isSupportMcuFeature) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->isSupportMcuFeature();
}

SOFTBUS_VISIBILITY void LnnSendDeviceStateToMcuPacked(void *para)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        SoftBusFree(para);
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSendDeviceStateToMcu) != SOFTBUS_OK) {
        SoftBusFree(para);
        return;
    }
    return pfnLnnEnhanceFuncList->lnnSendDeviceStateToMcu(para);
}

SOFTBUS_VISIBILITY int32_t LnnInitMcuPacked(void)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnInitMcu) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnLnnEnhanceFuncList->lnnInitMcu();
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetPidByAuthIdPacked(int64_t authId, int32_t *pid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaGetPidByAuthId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaGetPidByAuthId(authId, pid);
}

SOFTBUS_VISIBILITY int32_t LnnGetAllRemoteDevInfoPacked(NodeInfo **info, int32_t *nums)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetAllRemoteDevInfo) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetAllRemoteDevInfo(info, nums);
}

SOFTBUS_VISIBILITY int32_t LnnSendAgentDataPacked(const char *udid, const char *data, uint32_t length,
    LnnEventExtra *extra, bool isAckMsg)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnSendAgentData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnSendAgentData(udid, data, length, extra, isAckMsg);
}

SOFTBUS_VISIBILITY bool FarfieldParseModuleTypePacked(const uint8_t *data, uint32_t dataLen, uint32_t *moduleType)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return false;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->farfieldParseModuleType) != SOFTBUS_OK) {
        return false;
    }
    return pfnLnnEnhanceFuncList->farfieldParseModuleType(data, dataLen, moduleType);
}

SOFTBUS_VISIBILITY int32_t PostLnnCloudEventPacked(LnnCloudMsgType event, LnnCloudHandler handler,
    const void *obj, uint32_t size, uint64_t delayMs)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->postLnnCloudEvent) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->postLnnCloudEvent(event, handler, obj, size, delayMs);
}

SOFTBUS_VISIBILITY int32_t RemoveLnnCloudEventPacked(LnnCloudMsgType event, LnnCloudRemoveCompareFunc func, void *param)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->removeLnnCloudEvent) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->removeLnnCloudEvent(event, func, param);
}

SOFTBUS_VISIBILITY int32_t PerceptionBuildAdvDataPacked(const PerceptionType *type, const PerceptionAdvParam *params,
    uint32_t paramCnt, BroadcastPacket *packet)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionBuildAdvData) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return funcList->perceptionBuildAdvData(type, params, paramCnt, packet);
}

SOFTBUS_VISIBILITY int32_t PerceptionBuildScanFilterPacked(BcScanFilter **filter, uint8_t *filterNum)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionBuildScanFilter) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return funcList->perceptionBuildScanFilter(filter, filterNum);
}

SOFTBUS_VISIBILITY int32_t PerceptionGetDeviceInfoListPacked(PerceptionType type, PerceptionDeviceInfo **list, uint32_t *count)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionGetDeviceInfoList) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return funcList->perceptionGetDeviceInfoList(type, list, count);
}

SOFTBUS_VISIBILITY int32_t PerceptionSyncStatePacked(PerceptionState state)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionSyncState) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return funcList->perceptionSyncState(state);
}

SOFTBUS_VISIBILITY int32_t PerceptionSyncCyclePacked(PerceptionType type, uint32_t cycle)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionSyncCycle) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return funcList->perceptionSyncCycle(type, cycle);
}

SOFTBUS_VISIBILITY int32_t PerceptionEnhanceInitPacked(void)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionEnhanceInit) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return funcList->perceptionEnhanceInit();
}

SOFTBUS_VISIBILITY void PerceptionEnhanceDeinitPacked(void)
{
    LnnEnhanceFuncList *funcList = LnnEnhanceFuncListGet();
    if (funcList == NULL || LnnCheckFuncPointer((void *)funcList->perceptionEnhanceDeinit) != SOFTBUS_OK) {
        return;
    }
    funcList->perceptionEnhanceDeinit();
}

SOFTBUS_VISIBILITY int32_t AuthMetaOpenConnWithOtherOsTypePacked(const AuthConnInfo *info, const char *networkId,
    uint32_t requestId, const AuthConnCallback *callback)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->authMetaOpenConnWithOtherOsType) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->authMetaOpenConnWithOtherOsType(info, networkId, requestId, callback);
}

SOFTBUS_VISIBILITY int32_t LnnGetAllDevicesUdidPacked(bool isCloud, char (**udidList)[UDID_BUF_LEN], uint32_t *count)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->lnnGetAllDevicesUdid) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->lnnGetAllDevicesUdid(isCloud, udidList, count);
}

SOFTBUS_VISIBILITY int32_t DcWifiDirectExclusiveCancelByDeviceIdPacked(const char *deviceId)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->dcWifiDirectExclusiveCancelByDeviceId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->dcWifiDirectExclusiveCancelByDeviceId(deviceId);
}

SOFTBUS_VISIBILITY void DcWifiDirectExclusiveClearByPidPacked(int32_t pid)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->dcWifiDirectExclusiveClearByPid) != SOFTBUS_OK) {
        return;
    }
    pfnLnnEnhanceFuncList->dcWifiDirectExclusiveClearByPid(pid);
}

SOFTBUS_VISIBILITY int32_t DcWifiDirectExclusiveCancelBySessionNamePacked(const char *sessionName)
{
    LnnEnhanceFuncList *pfnLnnEnhanceFuncList = LnnEnhanceFuncListGet();
    if (pfnLnnEnhanceFuncList == NULL) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    if (LnnCheckFuncPointer((void *)pfnLnnEnhanceFuncList->dcWifiDirectExclusiveCancelBySessionName) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnLnnEnhanceFuncList->dcWifiDirectExclusiveCancelBySessionName(sessionName);
}
