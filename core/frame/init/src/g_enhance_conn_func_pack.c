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
#include "g_enhance_conn_func_pack.h"

#include "g_enhance_conn_func.h"
#include "softbus_error_code.h"
#include "softbus_init_common.h"
#include "softbus_def.h"

SOFTBUS_VISIBILITY int32_t ConnCoapStartServerListenPacked(void)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connCoapStartServerListen) != SOFTBUS_OK) {
        return SOFTBUS_FUNC_NOT_SUPPORT;
    }
    return pfnConnEnhanceFuncList->connCoapStartServerListen();
}

SOFTBUS_VISIBILITY void ConnCoapStopServerListenPacked(void)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connCoapStopServerListen) != SOFTBUS_OK) {
        return;
    }
    return pfnConnEnhanceFuncList->connCoapStopServerListen();
}

SOFTBUS_VISIBILITY void SoftbusBleConflictNotifyDisconnectPacked(const char *addr, const char *udid)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->softbusBleConflictNotifyDisconnect) != SOFTBUS_OK) {
        return;
    }
    return pfnConnEnhanceFuncList->softbusBleConflictNotifyDisconnect(addr, udid);
}

SOFTBUS_VISIBILITY
void SoftbusBleConflictNotifyDateReceivePacked(int32_t underlayerHandle, const uint8_t *data, uint32_t dataLen)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->softbusBleConflictNotifyDateReceive) != SOFTBUS_OK) {
        return;
    }
    return pfnConnEnhanceFuncList->softbusBleConflictNotifyDateReceive(underlayerHandle, data, dataLen);
}

SOFTBUS_VISIBILITY
void SoftbusBleConflictNotifyConnectResultPacked(uint32_t requestId, int32_t underlayerHandle, bool status)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->softbusBleConflictNotifyConnectResult) != SOFTBUS_OK) {
        return;
    }
    return pfnConnEnhanceFuncList->softbusBleConflictNotifyConnectResult(requestId, underlayerHandle, status);
}

SOFTBUS_VISIBILITY void SoftbusBleConflictRegisterListenerPacked(SoftBusBleConflictListener *listener)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->softbusBleConflictRegisterListener) != SOFTBUS_OK) {
        return;
    }
    return pfnConnEnhanceFuncList->softbusBleConflictRegisterListener(listener);
}

SOFTBUS_VISIBILITY int32_t ConnExtInitPacked(void)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connExtInit) != SOFTBUS_OK) {
        return SOFTBUS_OK;
    }
    return pfnConnEnhanceFuncList->connExtInit();
}

SOFTBUS_VISIBILITY bool ConnBleDirectIsEnablePacked(BleProtocolType protocol)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connBleDirectIsEnable) != SOFTBUS_OK) {
        return false;
    }
    return pfnConnEnhanceFuncList->connBleDirectIsEnable(protocol);
}

SOFTBUS_VISIBILITY int32_t ConnBleDirectConnectDevicePacked(const ConnectOption *option, uint32_t reqId, const ConnectResult* result)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connBleDirectConnectDevice) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnConnEnhanceFuncList->connBleDirectConnectDevice(option, reqId, result);
}

SOFTBUS_VISIBILITY ConnectFuncInterface *ConnSleInitPacked(const ConnectCallback *callback)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connSleInit) != SOFTBUS_OK) {
        return NULL;
    }
    return pfnConnEnhanceFuncList->connSleInit(callback);
}

SOFTBUS_VISIBILITY int32_t ConnDirectConnectDevicePacked(const ConnectOption *option, uint32_t reqId, const ConnectResult* result)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connDirectConnectDevice) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnConnEnhanceFuncList->connDirectConnectDevice(option, reqId, result);
}

SOFTBUS_VISIBILITY int32_t ConnPagingConnectInitPacked(void)
{
    ConnEnhanceFuncList *pfnConnEnhanceFuncList = ConnEnhanceFuncListGet();
    if (ConnCheckFuncPointer((void *)pfnConnEnhanceFuncList->connPagingConnectInit) != SOFTBUS_OK) {
        return SOFTBUS_NOT_IMPLEMENT;
    }
    return pfnConnEnhanceFuncList->connPagingConnectInit();
}