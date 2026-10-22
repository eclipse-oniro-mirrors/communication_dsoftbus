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
#include "far_field_proxy_dfx.h"

#include "conn_event.h"
#include "conn_log.h"
#include "securec.h"
#include "softbus_adapter_mem.h"
#include "softbus_adapter_thread.h"
#include "softbus_adapter_timer.h"
#include "softbus_common.h"
#include "softbus_conn_common.h"
#include "softbus_error_code.h"
#include "softbus_utils.h"

#define FAR_FIELD_DFX_RECORD_MAX 16

typedef struct {
    bool used;
    uint32_t requestId;
    char brMac[BT_MAC_LEN];
    uint64_t startTime;
} FarFieldDfxRecordEntry;

static FarFieldDfxRecordEntry g_dfxRecords[FAR_FIELD_DFX_RECORD_MAX];
static SoftBusMutex g_dfxLock;

static FarFieldDfxRecordEntry *FindDfxRecordUnsafe(const char *brMac)
{
    for (int32_t i = 0; i < FAR_FIELD_DFX_RECORD_MAX; i++) {
        if (g_dfxRecords[i].used && StrCmpIgnoreCase(g_dfxRecords[i].brMac, brMac) == 0) {
            return &g_dfxRecords[i];
        }
    }
    return NULL;
}

static FarFieldDfxRecordEntry *AllocDfxRecordUnsafe(void)
{
    for (int32_t i = 0; i < FAR_FIELD_DFX_RECORD_MAX; i++) {
        if (!g_dfxRecords[i].used) {
            return &g_dfxRecords[i];
        }
    }
    return NULL;
}

static void ReleaseDfxRecordUnsafe(FarFieldDfxRecordEntry *entry)
{
    entry->used = false;
    entry->requestId = 0;
    entry->startTime = 0;
}

int32_t FarFieldProxyDfxInit(void)
{
    int32_t ret = SoftBusMutexInit(&g_dfxLock, NULL);
    CONN_CHECK_AND_RETURN_RET_LOGE(ret == SOFTBUS_OK, ret, CONN_FARFIELD, "init dfx lock fail");
    (void)memset_s(&g_dfxRecords, sizeof(g_dfxRecords), 0, sizeof(g_dfxRecords));
    return SOFTBUS_OK;
}

void FarFieldDfxRecordConnectStart(uint32_t requestId, const char *brMac, bool isDirectly)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    int32_t ret = SoftBusMutexLock(&g_dfxLock);
    CONN_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, CONN_FARFIELD, "lock dfx fail");
    FarFieldDfxRecordEntry *entry = FindDfxRecordUnsafe(brMac);
    if (entry == NULL) {
        entry = AllocDfxRecordUnsafe();
    }
    if (entry == NULL) {
        CONN_LOGE(CONN_FARFIELD, "dfx record table full, reqId=%{public}u", requestId);
        SoftBusMutexUnlock(&g_dfxLock);
        return;
    }
    entry->used = true;
    entry->requestId = requestId;
    if (strcpy_s(entry->brMac, BT_MAC_LEN, brMac) != EOK) {
        CONN_LOGE(CONN_FARFIELD, "copy brMac fail");
        entry->used = false;
        SoftBusMutexUnlock(&g_dfxLock);
        return;
    }
    entry->startTime = SoftBusGetTimeMs();
    SoftBusMutexUnlock(&g_dfxLock);

    char anonymizeAddress[BT_MAC_LEN] = {0};
    ConvertAnonymizeMacAddress(anonymizeAddress, BT_MAC_LEN, brMac, BT_MAC_LEN);
    ConnEventExtra extra = {
        .result = EVENT_STAGE_RESULT_OK,
        .requestId = (int32_t)requestId,
        .peerBrMac = anonymizeAddress,
        .isDirectly = isDirectly ? 1 : 0,
    };
    CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_CONNECT_START, extra);
}

static uint64_t GetDfxStartTime(const char *brMac, bool consume)
{
    int32_t ret = SoftBusMutexLock(&g_dfxLock);
    if (ret != SOFTBUS_OK) {
        CONN_LOGE(CONN_FARFIELD, "lock dfx fail");
        return 0;
    }
    FarFieldDfxRecordEntry *entry = FindDfxRecordUnsafe(brMac);
    uint64_t startTime = 0;
    if (entry != NULL) {
        startTime = entry->startTime;
        if (consume) {
            ReleaseDfxRecordUnsafe(entry);
        }
    }
    SoftBusMutexUnlock(&g_dfxLock);
    return startTime;
}

void FarFieldDfxRecordConnectSuccess(uint32_t requestId, const char *brMac, bool isDirectly)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    uint64_t startTime = GetDfxStartTime(brMac, true);
    char anonymizeAddress[BT_MAC_LEN] = {0};
    ConvertAnonymizeMacAddress(anonymizeAddress, BT_MAC_LEN, brMac, BT_MAC_LEN);
    int32_t costTime = startTime > 0 ? (int32_t)(SoftBusGetTimeMs() - startTime) : 0;
    ConnEventExtra extra = {
        .result = EVENT_STAGE_RESULT_OK,
        .requestId = (int32_t)requestId,
        .peerBrMac = anonymizeAddress,
        .costTime = costTime,
    };
    if (isDirectly) {
        CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_CONNECT_END, extra);
    } else {
        CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_BR_PROXY_RECONNECT, extra);
    }
}

void FarFieldDfxRecordConnectFail(uint32_t requestId, const char *brMac, int32_t reason, bool isDirectly)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    uint64_t startTime = GetDfxStartTime(brMac, isDirectly);
    char anonymizeAddress[BT_MAC_LEN] = {0};
    ConvertAnonymizeMacAddress(anonymizeAddress, BT_MAC_LEN, brMac, BT_MAC_LEN);
    int32_t costTime = startTime > 0 ? (int32_t)(SoftBusGetTimeMs() - startTime) : 0;
    ConnEventExtra extra = {
        .result = EVENT_STAGE_RESULT_FAILED,
        .errcode = reason,
        .requestId = (int32_t)requestId,
        .peerBrMac = anonymizeAddress,
        .costTime = costTime,
    };
    CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_CONNECT_END, extra);
}

void FarFieldDfxRecordDisconnected(uint32_t requestId, const char *brMac, int32_t reason)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    char anonymizeAddress[BT_MAC_LEN] = {0};
    ConvertAnonymizeMacAddress(anonymizeAddress, BT_MAC_LEN, brMac, BT_MAC_LEN);
    ConnEventExtra extra = {
        .result = EVENT_STAGE_RESULT_FAILED,
        .errcode = reason,
        .requestId = (int32_t)requestId,
        .peerBrMac = anonymizeAddress,
    };
    CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_CONNECT_DISCONNECTED, extra);
}

void FarFieldDfxRecordClose(uint32_t requestId, const char *brMac, bool isClearReconnectEvent)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    char anonymizeAddress[BT_MAC_LEN] = {0};
    ConvertAnonymizeMacAddress(anonymizeAddress, BT_MAC_LEN, brMac, BT_MAC_LEN);
    ConnEventExtra extra = {
        .result = EVENT_STAGE_RESULT_OK,
        .requestId = (int32_t)requestId,
        .peerBrMac = anonymizeAddress,
        .brProxyIsClear = isClearReconnectEvent ? 1 : 0,
    };
    CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_CONNECT_DISCONNECTED, extra);
}

void FarFieldDfxRecordBrOpenSuccess(uint32_t requestId, const char *brMac, bool isSupportFarField)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    char anonymizeAddress[BT_MAC_LEN] = {0};
    ConvertAnonymizeMacAddress(anonymizeAddress, BT_MAC_LEN, brMac, BT_MAC_LEN);
    ConnEventExtra extra = {
        .result = EVENT_STAGE_RESULT_OK,
        .requestId = (int32_t)requestId,
        .peerBrMac = anonymizeAddress,
        .isSupportFarField = isSupportFarField ? 1 : 0,
    };
    CONN_EVENT(EVENT_SCENE_FAR_FIELD, EVENT_STAGE_CONNECT_END, extra);
}

void FarFieldDfxRemoveRecord(const char *brMac)
{
    CONN_CHECK_AND_RETURN_LOGE(brMac != NULL, CONN_FARFIELD, "brMac is NULL");
    int32_t ret = SoftBusMutexLock(&g_dfxLock);
    CONN_CHECK_AND_RETURN_LOGE(ret == SOFTBUS_OK, CONN_FARFIELD, "lock dfx fail");
    FarFieldDfxRecordEntry *entry = FindDfxRecordUnsafe(brMac);
    if (entry != NULL) {
        ReleaseDfxRecordUnsafe(entry);
    }
    SoftBusMutexUnlock(&g_dfxLock);
}
