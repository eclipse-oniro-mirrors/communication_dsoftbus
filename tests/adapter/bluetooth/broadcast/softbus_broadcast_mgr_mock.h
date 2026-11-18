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

#ifndef SOFTBUS_BROADCAST_MGR_MOCK_H
#define SOFTBUS_BROADCAST_MGR_MOCK_H

#include "softbus_broadcast_manager.h"

#ifdef __cplusplus
extern "C"{
#endif

bool CompareSameFilter(BcScanFilter* src, BcScanFilter* dst);

int32_t PerformSetBroadcastingParam(int32_t bcId, SoftbusBroadcastParam *softbusBcParam);

int32_t EnableBroadcasting(int32_t bcId);

int32_t DisableBroadcasting(int32_t bcId);

int32_t QueryBroadcastStatus(int32_t bcId, int32_t *status);

#ifdef __cplusplus
}

// Configurable stub medium interface used to drive error/edge branches of
// softbus_broadcast_mgr.c in unit tests.
#define STUB_BC_ID_BASE 100
#define STUB_SCAN_ID 200
#define STUB_SLE_SCAN_ID 300

struct StubMediumConfig {
    int32_t registerBroadcasterRet;
    int32_t unRegisterBroadcasterRet;
    int32_t registerScanListenerRet;
    int32_t startBcRet;
    int32_t stopBcRet;
    int32_t setBcDataRet;
    int32_t setBcParamRet;
    int32_t enableBcRet;
    int32_t disableBcRet;
    int32_t startScanRet;
    int32_t stopScanRet;
    int32_t setScanParamsRet;
    int32_t setScanReportChannelRet;
    bool setAdvFilterParamRet;
    bool invokeStartBcCb;
    bool invokeStopBcCb;
    bool invokeEnableBcCb;
    bool invokeDisableBcCb;
};

extern StubMediumConfig g_stubMediumConfig;
extern SoftbusBroadcastMediumInterface g_stubMediumInterface;
extern SoftbusBroadcastMediumInterface g_stubSleMediumInterface;
extern const SoftbusBroadcastCallback *g_stubBcCallback;
extern const SoftbusScanCallback *g_stubScanCallback;

void ResetStubMedium(void);
void ResetStubMediumInterfaceFuncs(void);
void ActionOfRegisterStubMediumInterface(void);
void SetStubCallocFailCountdown(int32_t countdown);
void SetStubCondInitFailCountdown(int32_t countdown);
void SetStubGetTimeFail(bool fail);
int32_t InvokeStubBcMgrDump(int32_t fd);
#endif

#endif /* SOFTBUS_BROADCAST_MGR_MOCK_H */