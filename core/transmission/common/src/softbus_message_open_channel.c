/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "softbus_message_open_channel.h"

#include <securec.h>

#include "softbus_adapter_crypto.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_socket.h"
#include "softbus_errcode.h"
#include "softbus_json_utils.h"

#define BASE64KEY 45 // Base64 encrypt SessionKey length

static int32_t g_tdcPktHeadSeq = 1024;

NO_SANITIZE("cfi") char *PackError(int errCode, const char *errDesc)
{
    if (errDesc == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "[%s]invalid param", __func__);
        return NULL;
    }
    cJSON *json =  cJSON_CreateObject();
    if (json == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Cannot create cJSON object");
        return NULL;
    }
    if (!AddNumberToJsonObject(json, CODE, CODE_OPEN_CHANNEL) ||
        !AddNumberToJsonObject(json, ERR_CODE, errCode) ||
        !AddStringToJsonObject(json, ERR_DESC, errDesc)) {
        cJSON_Delete(json);
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "add to cJSON object failed");
        return NULL;
    }
    char *data = cJSON_PrintUnformatted(json);
    if (data == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "cJSON_PrintUnformatted failed");
    }
    cJSON_Delete(json);
    return data;
}

static int PackFirstData(const AppInfo *appInfo, cJSON *json)
{
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "begin to pack first data");
    uint8_t *encodeFastData = (uint8_t *)SoftBusMalloc(BASE64_FAST_DATA_LEN);
    if (encodeFastData == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "malloc encode fast data fail.");
        return SOFTBUS_ERR;
    }
    size_t fastDataSize = 0;
    uint32_t outLen;
    char *buf = TransTdcPackFastData(appInfo, &outLen);
    if (buf == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "failed to pack bytes.");
        SoftBusFree(encodeFastData);
        return SOFTBUS_ENCRYPT_ERR;
    }
    if (outLen != appInfo->fastTransDataSize + FAST_TDC_EXT_DATA_SIZE) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "pack bytes len error, len: %d", outLen);
        SoftBusFree(buf);
        SoftBusFree(encodeFastData);
        return SOFTBUS_ENCRYPT_ERR;
    }
    int32_t ret = SoftBusBase64Encode(encodeFastData, BASE64_FAST_DATA_LEN, &fastDataSize,
        (const unsigned char *)buf, outLen);
    if (ret != 0) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "mbedtls base64 encode failed.");
        SoftBusFree(encodeFastData);
        SoftBusFree(buf);
        return SOFTBUS_ERR;
    }
    if (!AddStringToJsonObject(json, FIRST_DATA, (char *)encodeFastData)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "add first data failed.");
        SoftBusFree(encodeFastData);
        SoftBusFree(buf);
        return SOFTBUS_ERR;
    }
    SoftBusFree(encodeFastData);
    SoftBusFree(buf);
    return SOFTBUS_OK;
}

char *PackRequest(const AppInfo *appInfo)
{
    if (appInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "[%s]invalid param.", __func__);
        return NULL;
    }

    cJSON *json =  cJSON_CreateObject();
    if (json == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Cannot create cJSON object");
        return NULL;
    }
    if (!AddNumber16ToJsonObject(json, FIRST_DATA_SIZE, appInfo->fastTransDataSize)) {
        cJSON_Delete(json);
        return NULL;
    }
    if (appInfo->fastTransDataSize > 0) {
        if (PackFirstData(appInfo, json) != SOFTBUS_OK) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "pack first data failed");
            cJSON_Delete(json);
            return NULL;
        }
    }
    unsigned char encodeSessionKey[BASE64KEY] = {0};
    size_t keyLen = 0;
    int32_t ret = SoftBusBase64Encode(encodeSessionKey, BASE64KEY, &keyLen, (unsigned char*)appInfo->sessionKey,
        SESSION_KEY_LENGTH);
    if (ret != 0) {
        cJSON_Delete(json);
        return NULL;
    }
    if (!AddNumberToJsonObject(json, CODE, CODE_OPEN_CHANNEL) ||
        !AddNumberToJsonObject(json, API_VERSION, appInfo->myData.apiVersion) ||
        !AddStringToJsonObject(json, BUS_NAME, appInfo->peerData.sessionName) ||
        !AddStringToJsonObject(json, GROUP_ID, appInfo->groupId) ||
        !AddNumberToJsonObject(json, UID, appInfo->myData.uid) ||
        !AddNumberToJsonObject(json, PID, appInfo->myData.pid) ||
        !AddStringToJsonObject(json, SESSION_KEY, (char*)encodeSessionKey) ||
        !AddNumberToJsonObject(json, MTU_SIZE, appInfo->myData.dataConfig)) {
        cJSON_Delete(json);
        return NULL;
    }
    char *authState = (char*)appInfo->myData.authState;
    if (appInfo->myData.apiVersion != API_V1) {
        if (!AddStringToJsonObject(json, PKG_NAME, appInfo->myData.pkgName) ||
            !AddStringToJsonObject(json, CLIENT_BUS_NAME, appInfo->myData.sessionName) ||
            !AddStringToJsonObject(json, AUTH_STATE, authState) ||
            !AddNumberToJsonObject(json, MSG_ROUTE_TYPE, appInfo->routeType)) {
            cJSON_Delete(json);
            return NULL;
        }
    }
    (void)AddNumberToJsonObject(json, BUSINESS_TYPE, appInfo->businessType);
    (void)AddNumberToJsonObject(json, AUTO_CLOSE_TIME, appInfo->autoCloseTime);
    (void)AddNumberToJsonObject(json, TRANS_FLAGS, TRANS_FLAG_HAS_CHANNEL_AUTH);
    (void)AddNumberToJsonObject(json, MY_HANDLE_ID, appInfo->myHandleId);
    (void)AddNumberToJsonObject(json, PEER_HANDLE_ID, appInfo->peerHandleId);
    char *data = cJSON_PrintUnformatted(json);
    if (data == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "cJSON_PrintUnformatted failed");
    }
    cJSON_Delete(json);
    return data;
}

static int UnpackFirstData(AppInfo *appInfo, const cJSON *json)
{
    if (!GetJsonObjectNumber16Item(json, FIRST_DATA_SIZE, &(appInfo->fastTransDataSize))) {
        appInfo->fastTransDataSize = 0;
    }
    SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_DBG, "fast data size=%d", appInfo->fastTransDataSize);
    if (appInfo->fastTransDataSize > 0 && appInfo->fastTransDataSize <= MAX_FAST_DATA_LEN) {
        uint8_t *encodeFastData = (uint8_t *)SoftBusMalloc(BASE64_FAST_DATA_LEN);
        if (encodeFastData == NULL) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "malloc encode fast data fail.");
            return SOFTBUS_ERR;
        }
        size_t fastDataSize = 0;
        if (!GetJsonObjectStringItem(json, FIRST_DATA, (char *)encodeFastData, BASE64_FAST_DATA_LEN)) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get fast data");
            SoftBusFree(encodeFastData);
            return SOFTBUS_ERR;
        }
        appInfo->fastTransData = (uint8_t *)SoftBusMalloc(appInfo->fastTransDataSize + FAST_TDC_EXT_DATA_SIZE);
        if (appInfo->fastTransData == NULL) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "malloc fast data fail.");
            return SOFTBUS_ERR;
        }
        int32_t ret = SoftBusBase64Decode((unsigned char *)appInfo->fastTransData, appInfo->fastTransDataSize +
            FAST_TDC_EXT_DATA_SIZE, &fastDataSize, encodeFastData, strlen((char*)encodeFastData));
        if (ret != 0) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "mbedtls decode failed.");
            SoftBusFree((void *)appInfo->fastTransData);
            SoftBusFree(encodeFastData);
            return SOFTBUS_ERR;
        }
        SoftBusFree(encodeFastData);
    }
    return SOFTBUS_OK;
}

int UnpackRequest(const cJSON *msg, AppInfo *appInfo)
{
    if (msg == NULL || appInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "[%s]invalid param", __func__);
        return SOFTBUS_ERR;
    }
    if (UnpackFirstData(appInfo, msg) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "unpack first data failed");
        SoftBusFree((void *)appInfo->fastTransData);
        return SOFTBUS_ERR;
    }
    int apiVersion = API_V1;
    (void)GetJsonObjectNumberItem(msg, API_VERSION, &apiVersion);
    char sessionKey[BASE64KEY] = {0};
    if (!GetJsonObjectStringItem(msg, BUS_NAME, (appInfo->myData.sessionName), SESSION_NAME_SIZE_MAX) ||
        !GetJsonObjectStringItem(msg, GROUP_ID, (appInfo->groupId), GROUP_ID_SIZE_MAX) ||
        !GetJsonObjectStringItem(msg, SESSION_KEY, sessionKey, sizeof(sessionKey))) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get BUS_NAME");
        return SOFTBUS_ERR;
    }
    if (!GetJsonObjectNumberItem(msg, MTU_SIZE, (int32_t *)&(appInfo->peerData.dataConfig))) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "peer dataconfig is null.");
    }
    appInfo->peerData.apiVersion = (ApiVersion)apiVersion;
    appInfo->peerData.uid = -1;
    appInfo->peerData.pid = -1;
    (void)GetJsonObjectNumberItem(msg, UID, &appInfo->peerData.uid);
    (void)GetJsonObjectNumberItem(msg, PID, &appInfo->peerData.pid);
    appInfo->myHandleId = -1;
    appInfo->peerHandleId = -1;
    if (!GetJsonObjectInt32Item(msg, MY_HANDLE_ID, &(appInfo->peerHandleId)) ||
        !GetJsonObjectInt32Item(msg, PEER_HANDLE_ID, &(appInfo->myHandleId))) {
            appInfo->myHandleId = -1;
            appInfo->peerHandleId = -1;
    }

    size_t len = 0;
    int32_t ret = SoftBusBase64Decode((unsigned char *)appInfo->sessionKey, SESSION_KEY_LENGTH,
        &len, (unsigned char *)sessionKey, strlen(sessionKey));
    (void)memset_s(sessionKey, sizeof(sessionKey), 0, sizeof(sessionKey));
    if (len != SESSION_KEY_LENGTH) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to decode sessionKey %d, len %zu", ret, len);
        return SOFTBUS_ERR;
    }
    if (apiVersion == API_V1) {
        return SOFTBUS_OK;
    }

    if (!GetJsonObjectStringItem(msg, PKG_NAME, (appInfo->peerData.pkgName), PKG_NAME_SIZE_MAX) ||
        !GetJsonObjectStringItem(msg, CLIENT_BUS_NAME, (appInfo->peerData.sessionName), SESSION_NAME_SIZE_MAX) ||
        !GetJsonObjectStringItem(msg, AUTH_STATE, (appInfo->peerData.authState), AUTH_STATE_SIZE_MAX)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get pkgName");
        return SOFTBUS_ERR;
    }
    int32_t routeType = WIFI_STA;
    if (GetJsonObjectNumberItem(msg, MSG_ROUTE_TYPE, &routeType) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get route type");
    }
    appInfo->routeType = (RouteType)routeType;

    if (!GetJsonObjectNumberItem(msg, BUSINESS_TYPE, (int*)&appInfo->businessType)) {
        appInfo->businessType = BUSINESS_TYPE_NOT_CARE;
    }
    int transFlag = TRANS_FLAG_HAS_CHANNEL_AUTH;
    (void)GetJsonObjectNumberItem(msg, AUTO_CLOSE_TIME, (int*)&appInfo->autoCloseTime);
    (void)GetJsonObjectNumberItem(msg, TRANS_FLAGS, &transFlag);

    return SOFTBUS_OK;
}

NO_SANITIZE("cfi") char *PackReply(const AppInfo *appInfo)
{
    if (appInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "[%s]invalid param", __func__);
        return NULL;
    }
    cJSON *json =  cJSON_CreateObject();
    if (json == NULL) {
        return NULL;
    }
    if (!AddNumberToJsonObject(json, CODE, CODE_OPEN_CHANNEL) ||
        !AddNumberToJsonObject(json, API_VERSION, appInfo->myData.apiVersion) ||
        !AddStringToJsonObject(json, DEVICE_ID, appInfo->myData.deviceId) ||
        !AddNumberToJsonObject(json, UID, appInfo->myData.uid) ||
        !AddNumberToJsonObject(json, PID, appInfo->myData.pid)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to add items");
        cJSON_Delete(json);
        return NULL;
    }
    if (appInfo->peerData.dataConfig != 0) {
        if (!AddNumberToJsonObject(json, MTU_SIZE, appInfo->myData.dataConfig)) {
            cJSON_Delete(json);
            return NULL;
        }
    }
    if (!AddNumber16ToJsonObject(json, FIRST_DATA_SIZE, appInfo->fastTransDataSize)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to add trans data size");
        cJSON_Delete(json);
        return NULL;
    }
    if (appInfo->myData.apiVersion != API_V1) {
        char *authState = (char *)appInfo->myData.authState;
        if (!AddStringToJsonObject(json, PKG_NAME, appInfo->myData.pkgName) ||
            !AddStringToJsonObject(json, AUTH_STATE, authState)) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to add pkgName or authState");
            cJSON_Delete(json);
            return NULL;
        }
    }
    (void)AddNumberToJsonObject(json, MY_HANDLE_ID, appInfo->myHandleId);
    (void)AddNumberToJsonObject(json, PEER_HANDLE_ID, appInfo->peerHandleId);
    char *data = cJSON_PrintUnformatted(json);
    if (data == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "cJSON_PrintUnformatted failed");
    }
    cJSON_Delete(json);
    return data;
}

int UnpackReply(const cJSON *msg, AppInfo *appInfo, uint16_t *fastDataSize)
{
    if (msg == NULL || appInfo == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "[%s]invalid param", __func__);
        return SOFTBUS_ERR;
    }

    char uuid[DEVICE_ID_SIZE_MAX] = {0};
    if (!GetJsonObjectStringItem(msg, DEVICE_ID, uuid, DEVICE_ID_SIZE_MAX)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get uuid");
        return SOFTBUS_ERR;
    }
    if (strcmp(uuid, appInfo->peerData.deviceId) != 0) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Invalid uuid");
        return SOFTBUS_ERR;
    }
    if (!GetJsonObjectNumber16Item(msg, FIRST_DATA_SIZE, fastDataSize)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get fast data size");
    }

    int apiVersion = API_V1;
    (void)GetJsonObjectNumberItem(msg, API_VERSION, &apiVersion);
    appInfo->peerData.apiVersion = (ApiVersion)apiVersion;
    appInfo->peerData.uid = -1;
    appInfo->peerData.pid = -1;
    (void)GetJsonObjectNumberItem(msg, UID, &appInfo->peerData.uid);
    (void)GetJsonObjectNumberItem(msg, PID, &appInfo->peerData.pid);
    if (!GetJsonObjectInt32Item(msg, MY_HANDLE_ID, &(appInfo->peerHandleId)) ||
        !GetJsonObjectInt32Item(msg, PEER_HANDLE_ID, &(appInfo->myHandleId))) {
            appInfo->myHandleId = -1;
            appInfo->peerHandleId = -1;
    }
    if (!GetJsonObjectNumberItem(msg, MTU_SIZE, (int32_t *)&(appInfo->peerData.dataConfig))) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "peer dataconfig is null.");
    }
    if (apiVersion != API_V1) {
        if (!GetJsonObjectStringItem(msg, PKG_NAME, (appInfo->peerData.pkgName), PKG_NAME_SIZE_MAX) ||
            !GetJsonObjectStringItem(msg, AUTH_STATE, (appInfo->peerData.authState), AUTH_STATE_SIZE_MAX)) {
            SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "Failed to get pkgName or authState");
            return SOFTBUS_ERR;
        }
    }
    return SOFTBUS_OK;
}

int UnpackReplyErrCode(const cJSON *msg, int32_t *errCode)
{
    if ((msg == NULL) && (errCode == NULL)) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "[%s]invalid param", __func__);
        return SOFTBUS_ERR;
    }

    if (!GetJsonObjectInt32Item(msg, ERR_CODE, errCode)) {
        return SOFTBUS_ERR;
    }

    return SOFTBUS_OK;
}

static int32_t TransTdcEncrypt(const char *sessionKey, const char *in, uint32_t inLen, char *out, uint32_t *outLen)
{
    AesGcmCipherKey cipherKey = {0};
    cipherKey.keyLen = SESSION_KEY_LENGTH;
    if (memcpy_s(cipherKey.key, SESSION_KEY_LENGTH, sessionKey, SESSION_KEY_LENGTH) != EOK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "memcpy key error.");
        return SOFTBUS_ERR;
    }
    int32_t ret = SoftBusEncryptData(&cipherKey, (unsigned char*)in, inLen, (unsigned char*)out, outLen);
    (void)memset_s(&cipherKey, sizeof(AesGcmCipherKey), 0, sizeof(AesGcmCipherKey));
    if (ret != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_AUTH, SOFTBUS_LOG_ERROR, "SoftBusEncryptData fail(=%d).", ret);
        return SOFTBUS_ENCRYPT_ERR;
    }
    return SOFTBUS_OK;
}

static void PackTcpFastDataPacketHead(TcpFastDataPacketHead *data)
{
    data->magicNumber = SoftBusHtoLl(data->magicNumber);
    data->seq = (int32_t)SoftBusHtoLl((uint32_t)data->seq);
    data->flags = SoftBusHtoLl(data->flags);
    data->dataLen = SoftBusHtoLl(data->dataLen);
}

char *TransTdcPackFastData(const AppInfo *appInfo, uint32_t *outLen)
{
#define MAGIC_NUMBER 0xBABEFACE
    uint32_t dataLen = appInfo->fastTransDataSize + OVERHEAD_LEN;
    char *buf = (char *)SoftBusMalloc(dataLen + FAST_DATA_HEAD_SIZE);
    if (buf == NULL) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "malloc failed.");
        return NULL;
    }
    TcpFastDataPacketHead pktHead = {
        .magicNumber = MAGIC_NUMBER,
        .seq = g_tdcPktHeadSeq++,
        .flags = (appInfo->businessType == BUSINESS_TYPE_BYTE) ? FLAG_BYTES : FLAG_MESSAGE,
        .dataLen = dataLen,
    };
    PackTcpFastDataPacketHead(&pktHead);
    if (memcpy_s(buf, FAST_DATA_HEAD_SIZE, &pktHead, sizeof(TcpFastDataPacketHead)) != EOK) {
        SoftBusFree(buf);
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "memcpy_s error");
        return NULL;
    }
    if (TransTdcEncrypt(appInfo->sessionKey, (const char *)appInfo->fastTransData,
        appInfo->fastTransDataSize, buf + FAST_DATA_HEAD_SIZE, &dataLen) != SOFTBUS_OK) {
        SoftBusLog(SOFTBUS_LOG_TRAN, SOFTBUS_LOG_ERROR, "encrypt error");
        SoftBusFree(buf);
        return NULL;
    }
    *outLen = dataLen + FAST_DATA_HEAD_SIZE;
    return buf;
}