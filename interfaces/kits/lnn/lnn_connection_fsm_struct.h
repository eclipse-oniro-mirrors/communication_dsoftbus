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


#ifndef LNN_CONNECTION_FSM_STRUCT_H
#define LNN_CONNECTION_FSM_STRUCT_H

#include <stdint.h>

#include "auth_interface_struct.h"
#include "common_list.h"
#include "lnn_net_builder_struct.h"
#include "lnn_node_info_struct.h"
#include "lnn_state_machine_struct.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"
#include "legacy/softbus_hisysevt_bus_center.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define LNN_CONNECTION_FSM_NAME_LEN 32

#define LNN_CONN_INFO_FLAG_JOIN_REQUEST 0x01
#define LNN_CONN_INFO_FLAG_JOIN_AUTO 0x02
#define LNN_CONN_INFO_FLAG_JOIN_PASSIVE 0x04
#define LNN_CONN_INFO_FLAG_LEAVE_REQUEST 0x08
#define LNN_CONN_INFO_FLAG_LEAVE_AUTO 0x10
#define LNN_CONN_INFO_FLAG_LEAVE_PASSIVE 0x20
#define LNN_CONN_INFO_FLAG_INITIATE_ONLINE 0x40
#define LNN_CONN_INFO_FLAG_ONLINE 0x80

#define LNN_CONN_INFO_FLAG_JOIN_ACTIVE (LNN_CONN_INFO_FLAG_JOIN_REQUEST | LNN_CONN_INFO_FLAG_JOIN_AUTO)
#define LNN_CONN_INFO_FLAG_JOIN (LNN_CONN_INFO_FLAG_JOIN_ACTIVE | LNN_CONN_INFO_FLAG_JOIN_PASSIVE)

#define LNN_CONN_INFO_FLAG_LEAVE_ACTIVE (LNN_CONN_INFO_FLAG_LEAVE_REQUEST | LNN_CONN_INFO_FLAG_LEAVE_AUTO)
#define LNN_CONN_INFO_FLAG_LEAVE (LNN_CONN_INFO_FLAG_LEAVE_ACTIVE | LNN_CONN_INFO_FLAG_LEAVE_PASSIVE)

typedef struct {
    /* clean invalid addr type */
    ConnectionAddrType addrType;
    /* record newer connection networkId */
    char networkId[NETWORK_ID_BUF_LEN];
} LnnInvalidCleanInfo;

typedef struct {
    char peerNetworkId[NETWORK_ID_BUF_LEN];
    uint32_t requestId;
    uint32_t flag;
    SoftBusVersion version;
    AuthHandle authHandle;
    LnnDfxDeviceInfoReport infoReport;
    ConnectionAddr addr;
    LnnInvalidCleanInfo *cleanInfo;
    NodeInfo *nodeInfo;
    NodeInfo *dupInfo;
} LnnConntionInfo;

struct tagLnnConnectionFsm;

typedef void (*LnnConnectionFsmStopCallback)(struct tagLnnConnectionFsm *connFsm);

typedef struct tagLnnConnectionFsm {
    char fsmName[LNN_CONNECTION_FSM_NAME_LEN];
    char pkgName[PKG_NAME_SIZE_MAX];
    bool isDead;
    bool isNeedConnect;
    bool isSession;
    uint16_t id;

    LnnConnectionFsmStopCallback stopCallback;
    ListNode node;
    LnnStatisticData statisticData;
    FsmStateMachine fsm;
    LnnConntionInfo connInfo;
} LnnConnectionFsm;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* LNN_CONNECTION_FSM_STRUCT_H */