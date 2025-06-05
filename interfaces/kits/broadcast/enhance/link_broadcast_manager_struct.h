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

#ifndef LINK_BROADCAST_MANAGER_STRUCT_H
#define LINK_BROADCAST_MANAGER_STRUCT_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define WIFI_MAC_SIZE      6
#define SLE_MAC_SIZE       6

typedef enum {
    BLE_GATT_MASTER = 0,
    BLE_GATT_SLAVE,
    BLE_COC_MASTER,
    BLE_COC_SLAVE,
} BleDirectRole;

typedef enum {
    SLE_SSAP_MASTER = 0,
    SLE_SSAP_SLAVE,
} SleDirectRole;

typedef enum {
    WIFI_HML = 0,
    WIFI_P2P_GO,
    WIFI_P2P_GC,
} WiFiDirectRole;

typedef struct {
    BleDirectRole role;
    uint8_t psm;
} BleDirectParam;

typedef struct {
    WiFiDirectRole role;
    bool channelFlag;
    bool isSupportActionChannel;
    bool isSupport160M;
    uint8_t channel;
    uint8_t mac[WIFI_MAC_SIZE];
    uint8_t psm;
} WiFiDirectParam;

typedef struct {
    int32_t resultCode;
} WiFiDirectRspParam;

typedef struct {
    SleDirectRole role;
    uint8_t mac[SLE_MAC_SIZE];
} SleDirectParam;

typedef enum {
    LINK_BLE_DIRECT = 0, /* Note: Cannot modify value */
    LINK_WIFI_DIRECT = 1,
    LINK_WIFI_DIRECT_RSP = 2,
    LINK_SLE_DIRECT = 3,
    LINK_TYPE_BUTT,
} LinkBroadcastType;

typedef struct {
    bool withSrcId;
    uint16_t challengeCode;
    LinkBroadcastType type;
    union {
        BleDirectParam bleDirect;
        WiFiDirectParam wifiDirect;
        WiFiDirectRspParam wifiDirectRsp;
        SleDirectParam sleDirect;
    };
} LinkBroadcastOption;

typedef void (*LinkBroadcastListener)(const char *networkId,
    const LinkBroadcastOption *option, const char *remoteBleMac);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif