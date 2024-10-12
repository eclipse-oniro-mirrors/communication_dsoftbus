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

#ifndef TRANS_LANE_PENDING_CTL_H
#define TRANS_LANE_PENDING_CTL_H

#include <stdint.h>

#include "lnn_lane_interface.h"
#include "session.h"
#include "softbus_conn_interface.h"
#include "softbus_trans_def.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
    PARA_ACTION = 1,
    PARA_BUTT,
}ParaType;

typedef struct {
    uint32_t actionId;
}ActionAddr;

typedef struct {
    ParaType tyoe;
    union {
        ActionAddr action;
    };
    bool enable160M;
}LinkPara;
typedef struct {
    ListNode node;
    int32_t errCode;
    uint32_t laneReqId;
    int32_t channelId;
    char *sessionName;
    LinkPara linkPara;
    LaneConnInfo connInfo;
    bool bSucc;
    bool isFinished;
} TransAuthWithParaNode;

int32_t TransReqLanePendingInit(void);
void TransReqLanePendingDeinit(void);
int32_t TransAsyncReqLanePendingInit(void);
void TransAsyncReqLanePendingDeinit(void);
int32_t TransFreeLanePendingInit(void);
void TransFreeLanePendingDeinit(void);

int32_t TransGetConnectOptByConnInfo(const LaneConnInfo *info, ConnectOption *connOpt);
int32_t TransGetLaneInfo(const SessionParam *param, LaneConnInfo *connInfo, uint32_t *laneHandle);
int32_t TransAsyncGetLaneInfo(
    const SessionParam *param, uint32_t *laneHandle, uint32_t callingTokenId, int64_t timeStart);
int32_t TransGetLaneInfoByOption(const LaneRequestOption *requestOption, LaneConnInfo *connInfo, uint32_t *laneHandle);
int32_t TransGetLaneInfoByQos(const LaneAllocInfo *allocInfo, LaneConnInfo *connInfo, uint32_t *laneHandle);
bool TransGetAuthTypeByNetWorkId(const char *peerNetWorkId);
int32_t TransCancelLaneItemCondByLaneHandle(uint32_t laneHandle, bool bSucc, bool isAsync, int32_t errCode);
int32_t TransFreeLaneByLaneHandle(uint32_t laneHandle, bool isAsync);
int32_t TransAuthWithParaReqLanePendingInit(void);
void TransAuthWithParaReqLanePendingDeinit(void);
int32_t TransAuthWithParaAddLaneReqToList(uint32_t laneReqId, const char *sessionName,
    const LinkPara *linkPara, int32_t channelId);
int32_t TransAuthWithParaDelLaneReqById(uint32_t laneReqId);
int32_t TransAuthWithParaGetLaneReqByLaneReqId(uint32_t laneReqId, TransAuthWithParaNode *paraNode);
int32_t TransUpdateAuthWithParaLaneConnInfo(uint32_t laneHandle, bool bSucc, const LaneConnInfo *connInfo,
    int32_t errCode);

int32_t TransDeleteLaneReqItemByLaneHandle(uint32_t laneHandle, bool isAsync);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
