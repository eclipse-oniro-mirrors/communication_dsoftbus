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

/**
 * @addtogroup SoftBus
 * @{
 *
 * @brief Provides high-speed, secure communication between devices.
 *
 * This module implements unified distributed communication capability management between nearby devices, and provides
 * link-independent device discovery and transmission interfaces to support service publishing and data transmission.
 *
 * @since 6.0
 * @version 1.0
 */
/** @} */

/**
 * @file softbus_service_type.h
 *
 * @brief Declares structs and constants for service discovery.
 *
 * @since 6.0
 * @version 1.0
 */

#ifndef SOFTBUS_SERVICE_DISCOVERY_TYPE_H
#define SOFTBUS_SERVICE_DISCOVERY_TYPE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Indicate the maximum length of service type string.
 *
 * @since 6.0
 * @version 1.0
 */
#define DISC_SERVICE_TYPE_MAX_LEN 16

/**
 * @brief Indicate the maximum length of service name string.
 *
 * @since 6.0
 * @version 1.0
 */
#define DISC_SERVICE_NAME_MAX_LEN 64

/**
 * @brief Indicate the maximum length of service display name string.
 *
 * @since 6.0
 * @version 1.0
 */
#define DISC_SERVICE_DISPLAYNAME_MAX_LEN 128

/**
 * @brief Indicates the maximum length of the customData data string in <b>ServiceInfo</b>.
 *
 * @since 6.0
 * @version 1.0
 */
#define DISC_SERVICE_CUSTOMDATA_MAX_LEN 512

/**
 * @brief Indicates the maximum length of the customData data string in <b>DiscoveryInfo</b>.
 *
 * @since 6.0
 * @version 1.0
 */
#define DISC_SERVICE_REQUEST_CUSTOMDATA_MAX_LEN 10

/**
 * @brief Enumerates different media used for publishing services.
 *
 * @since 6.0
 * @version 1.0
 */
typedef enum {
    SERVICE_MEDIUM_TYPE_AUTO,        // All supported media will be called automatically
    SERVICE_MEDIUM_TYPE_BLE,         // Bluetooth
    SERVICE_MEDIUM_TYPE_BLE_TRIGGER, // Bluetooth triggered
    SERVICE_MEDIUM_TYPE_MDNS,        // mDNS
    SERVICE_MEDIUM_TYPE_BUTT,
} ServiceMediumType;

/**
 * @brief Enumerates the modes in which services are published and discovered.
 *
 * @since 6.0
 * @version 1.0
 */
typedef enum  {
    /* Passive */
    SERVICE_DISCOVER_MODE_PASSIVE = 0x15,
    /* Active */
    SERVICE_DISCOVER_MODE_ACTIVE = 0x25
} ServiceDiscoverMode;

/**
 * @brief Enumerates frequencies for Bluetooth Scanner.
 *
 * @since 6.0
 * @version 1.0
 */
typedef enum {
    FREQ_LOW,                   // Scan duty cycle is 2%
    FREQ_MID,                   // Scan duty cycle is 10%
    FREQ_HIGH,                  // Scan duty cycle is 25%
    FREQ_SUPER_HIGH,            // Scan duty cycle is 50%
    FREQ_ALMOST_EXTREMELY_HIGH, // Scan duty cycle is 75%
    FREQ_EXTREMELY_HIGH,        // Scan duty cycle is 100%
    FREQ_VALUE_BUTT
} EnableFreq;

/**
 * @brief Defines the service information returned by <b>IServiceDiscoveryCb</b>.
 *
 * @since 6.0
 * @version 1.0
 */
typedef struct {
    /* Service ID */
    int64_t serviceId;
    /* Service type: must be c-string format, end with '\0', 1-15 characters in length.*/
    char serviceType[DISC_SERVICE_TYPE_MAX_LEN];
    /* Service name to be registered: must be c-string format, end with '\0', 0-64 characters in length */
    char serviceName[DISC_SERVICE_NAME_MAX_LEN];
    /* Service name to be displayed: must be c-string format, end with '\0', 7-128 bytes in length */
    char serviceDisplayName[DISC_SERVICE_DISPLAYNAME_MAX_LEN];
    /* Custom data for service: must be c-string format, end with '\0', 0-512 bytes in length */
    unsigned char customData[DISC_SERVICE_CUSTOMDATA_MAX_LEN];
    /* Length of custom data */
    unsigned int dataLen;
} ServiceInfo;

/**
 * @brief Defines the service publishing and discovery parameters.
 *
 * @since 6.0
 * @version 1.0
 */
typedef struct {
    /* The media on which to register the service */
    ServiceMediumType media;
    /* The publishing and discovery mode */
    ServiceDiscoverMode mode;
    /* The publishing frequency */
    EnableFreq freq;
} ServiceDiscoveryParam;

/**
 * @brief Defines the service discovery information.
 *
 * @since 6.0
 * @version 1.0
 */
typedef struct {
    /* Local service ID generated by device manager */
    int64_t localServiceId;
    /* The service type must be a short text string, 1-15 characters in length */
    char serviceType[DISC_SERVICE_TYPE_MAX_LEN];
    /* Request Custom data for service discovery, 0-9 bytes in length */
    unsigned char reqCustomData[DISC_SERVICE_REQUEST_CUSTOMDATA_MAX_LEN];
    /* Length of request custom data */
    unsigned int dataLen;
} DiscoveryInfo;

#ifdef __cplusplus
}
#endif

#endif
