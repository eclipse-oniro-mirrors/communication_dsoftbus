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
#include "g_enhance_auth_func_pack.h"

#include "g_enhance_auth_func.h"
#include "softbus_init_common.h"
#include "softbus_def.h"

SOFTBUS_VISIBILITY int32_t AuthMetaGetConnInfoBySidePacked(const char *networkId, bool isClient, AuthConnInfo *connInfo)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetConnInfoBySide) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetConnInfoBySide(networkId, isClient, connInfo);
}

SOFTBUS_VISIBILITY int32_t AuthMetaInitPacked(const AuthTransCallback *callback)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaInit) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnAuthEnhanceFuncList->authMetaInit(callback);
}

SOFTBUS_VISIBILITY void AuthUpdateNormalizeKeyIndexPacked(const char *udidHash, int64_t index,
    AuthLinkType type, SessionKey *normalizedKey, bool isServer)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authUpdateNormalizeKeyIndex) != SOFTBUS_OK) {
        return;
    }
    return pfnAuthEnhanceFuncList->authUpdateNormalizeKeyIndex(udidHash, index, type, normalizedKey, isServer);
}

SOFTBUS_VISIBILITY int32_t GenerateCertificatePacked(SoftbusCertChain *softbusCertChain, const AuthSessionInfo *info)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->generateCertificate) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->generateCertificate(softbusCertChain, info);
}

SOFTBUS_VISIBILITY bool IsNeedUDIDAbatementPacked(const AuthSessionInfo *info)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->isNeedUDIDAbatement) != SOFTBUS_OK) {
        return false;
    }
    return pfnAuthEnhanceFuncList->isNeedUDIDAbatement(info);
}

SOFTBUS_VISIBILITY int32_t VerifyCertificatePacked(SoftbusCertChain *softbusCertChain, const NodeInfo *nodeInfo,
    const AuthSessionInfo *info)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->verifyCertificate) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->verifyCertificate(softbusCertChain, nodeInfo, info);
}

SOFTBUS_VISIBILITY void AuthMetaNotifyDataReceivedPacked(uint32_t connectionId, const SocketPktHead *pktHead, const uint8_t *data)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaNotifyDataReceived) != SOFTBUS_OK) {
        return;
    }
    return pfnAuthEnhanceFuncList->authMetaNotifyDataReceived(connectionId, pktHead, data);
}

SOFTBUS_VISIBILITY void AuthClearDeviceKeyPacked(void)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authClearDeviceKey) != SOFTBUS_OK) {
        return;
    }
    return pfnAuthEnhanceFuncList->authClearDeviceKey();
}

SOFTBUS_VISIBILITY void DelAuthMetaManagerByConnectionIdPacked(uint32_t connectionId)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->delAuthMetaManagerByConnectionId) != SOFTBUS_OK) {
        return;
    }
    return pfnAuthEnhanceFuncList->delAuthMetaManagerByConnectionId(connectionId);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetOsTypeByMetaNodeIdPacked(const char *metaNodeId, int32_t *osType)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetOsTypeByMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetOsTypeByMetaNodeId(metaNodeId, osType);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetMetaTypeByMetaNodeIdPacked(const char *metaNodeId, int32_t *metaType)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetMetaTypeByMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetMetaTypeByMetaNodeId(metaNodeId, metaType);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetMetaNodeIdByIpPacked(const char *ip, char *metaNodeId, int32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetMetaNodeIdByIp) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetMetaNodeIdByIp(ip, metaNodeId, len);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetDeviceIdByMetaNodeIdPacked(const char *metaNodeId, char *deviceId, uint32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetDeviceIdByMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetDeviceIdByMetaNodeId(metaNodeId, deviceId, len);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetP2pMacByMetaNodeIdPacked(const char *metaNodeId, char *p2pMacAddr, int32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetP2pMacByMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetP2pMacByMetaNodeId(metaNodeId, p2pMacAddr, len);
}

SOFTBUS_VISIBILITY bool AuthMetaGetMetaValueByMetaNodeIdPacked(const char *metaNodeId)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetMetaValueByMetaNodeId) != SOFTBUS_OK) {
        return false;
    }
    return pfnAuthEnhanceFuncList->authMetaGetMetaValueByMetaNodeId(metaNodeId);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetFeatureSDKByMetaNodeIdPacked(const char *metaNodeId, uint64_t *featureSDK)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetFeatureSDKByMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetFeatureSDKByMetaNodeId(metaNodeId, featureSDK);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetLocalUuidByPeerMetaNodeIdPacked(const char *peerMetaNodeId, char *localUuid, uint32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetLocalUuidByPeerMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetLocalUuidByPeerMetaNodeId(peerMetaNodeId, localUuid, len);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetAuthHandleByPeerMetaNodeIdPacked(const char *peerMetaNodeId, AuthHandle *authHandle)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetAuthHandleByPeerMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetAuthHandleByPeerMetaNodeId(peerMetaNodeId, authHandle);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetPeerMetaNodeIdByPeerAuthIdPacked(int64_t peerAuthId, char *peerMetaNodeId,
    uint32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetPeerMetaNodeIdByPeerAuthId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetPeerMetaNodeIdByPeerAuthId(peerAuthId, peerMetaNodeId, len);
}

SOFTBUS_VISIBILITY bool AuthMetaIsSupportConcurrentByConnectionIdPacked(uint32_t connectionId)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaIsSupportConcurrentByConnectionId) != SOFTBUS_OK) {
        return false;
    }
    return pfnAuthEnhanceFuncList->authMetaIsSupportConcurrentByConnectionId(connectionId);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetLocalMetaNodeIdByPeerMetaNodeIdPacked(
    const char *peerMetaNodeId, char *localMetaNodeId, uint32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetLocalMetaNodeIdByPeerMetaNodeId) !=
        SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetLocalMetaNodeIdByPeerMetaNodeId(peerMetaNodeId, localMetaNodeId, len);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetPeerUdidByMetaNodeIdPacked(const char *metaNodeId, char *peerUdid, uint32_t len)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetPeerUdidByMetaNodeId) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetPeerUdidByMetaNodeId(metaNodeId, peerUdid, len);
}

SOFTBUS_VISIBILITY bool AuthMetaIsSupportConcurrentByRemoteIpPacked(const char *ip)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaIsSupportConcurrentByRemoteIp) != SOFTBUS_OK) {
        return false;
    }
    return pfnAuthEnhanceFuncList->authMetaIsSupportConcurrentByRemoteIp(ip);
}

SOFTBUS_VISIBILITY bool AuthMetaIsSupportConcurrentByMetaNodeIdPacked(const char *metaNodeId)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaIsSupportConcurrentByMetaNodeId) != SOFTBUS_OK) {
        return false;
    }
    return pfnAuthEnhanceFuncList->authMetaIsSupportConcurrentByMetaNodeId(metaNodeId);
}

SOFTBUS_VISIBILITY int32_t AuthMetaGetNetworkIdsPacked(const char *udid, char **networkIds, uint32_t *networkIdCount)
{
    AuthEnhanceFuncList *pfnAuthEnhanceFuncList = AuthEnhanceFuncListGet();
    if (AuthCheckFuncPointer((void *)pfnAuthEnhanceFuncList->authMetaGetNetworkIds) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnAuthEnhanceFuncList->authMetaGetNetworkIds(udid, networkIds, networkIdCount);
}
