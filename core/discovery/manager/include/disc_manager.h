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

#ifndef DISC_MANAGER_H
#define DISC_MANAGER_H

#include "disc_interface.h"
#include "disc_manager_struct.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Publish service to start publishing its own information to other devices.
 * @see {@link DiscUnPublishService}
 * @param[in] packageName Indicates the pointer to the package name,
 * and the name of the relevant package carrying its own capabilities.
 * @param[in] info ndicates the pointer to the published information,
 * which is used to publish the information body of its own information. For details, see {@link PublishInfo}.
 * @return <b>SOFTBUS_INVALID_PARAM</b> if any parameter is null or invalid.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM</b> Error in message medium during message check.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_NOT_INIT</b> Softbus found that management variables are not initialized.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE</b> The new publishing node corresponding to the discovery
 * information has not been created
 * @return <b>SOFTBUS_LOCK_ERR</b> Failed to lock.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM</b> The information to be published is already in the list.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_ITEM_NOT_CREATE</b> The new project node corresponding to the discovery
 * information was not created.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL</b> Internal function error.
 * @return <b>SOFTBUS_OK</b> Published self information successfully.
 */
int32_t DiscPublishService(const char *packageName, const PublishInfo *info, int32_t callingPid);

/**
 * @brief If the service is cancelled, the remote device cannot obtain its own information.
 * @see {@link DiscPublishService}
 * @param[in] packageName Indicates the pointer to the package name,
 * and the name of the relevant package carrying its own capabilities.
 * @param[in] publishId ID of the internal release information to be cancelled this time.
 * @return <b>SOFTBUS_INVALID_PARAM</b> if any parameter is null or invalid.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_NOT_INIT</b> Softbus found that management variables are not initialized.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INFO_NOT_DELETE</b> Delete the inode corresponding to the publication id
 * from the list.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL</b> Internal function error.
 * @return <b>SOFTBUS_OK</b> Unpublished service succeeded.
 */
int32_t DiscUnPublishService(const char *packageName, int32_t publishId, int32_t callingPid);

/**
 * @brief Start discovery, other devices can be discovered.
 * @see {@link DiscStopDiscovery}
 * @param[in] packageName Indicates the pointer to the package name,
 * and the name of the relevant package carrying its own capabilities.
 * @param[in] info Indicates a pointer to published information used
 * to discover the body of information for a specific capability. For more information, see {@link SubscribeInfo}.
 * @param[in] cb Indicates the pointer to the discovery callback, It is used to inform yourself,
 * Whether the discovery capability is successfully started after the discovery function is triggered.
 * For more information, see {@link IServerDiscInnerCallback}.
 * @return <b>SOFTBUS_INVALID_PARAM</b> if any parameter is null or invalid.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INVALID_MEDIUM</b> Error in message medium during message check.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_NOT_INIT</b> Softbus found that management variables are not initialized.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INFO_NOT_CREATE</b> The new publishing node corresponding to the discovery
 * information has not been created.
 * @return <b>SOFTBUS_LOCK_ERR</b> Failed to lock.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_DUPLICATE_PARAM</b> The information to be published is already in the list.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_ITEM_NOT_CREATE</b> The new project node corresponding to the discovery
 * information was not created.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL</b> Internal function error.
 * @return <b>SOFTBUS_OK</b> Passive discovery function successfully started.
 */
int32_t DiscStartDiscovery(const char *packageName, const SubscribeInfo *info, const IServerDiscInnerCallback *cb,
    int32_t callingPid);

/**
 * @brief Stop discovering, stop discovering other devices.
 * @see {@link DiscStartDiscovery}
 * @param[in] packageName Indicates the pointer to the package name,
 * and the name of the relevant package carrying its own capabilities.
 * @param[in] subscribeId ID to stop discovery this time.
 * @return <b>SOFTBUS_INVALID_PARAM</b> if any parameter is null or invalid.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_NOT_INIT</b> Softbus found that management variables are not initialized.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INFO_NOT_DELETE</b> Delete the inode corresponding to the publication id
 * from the list.
 * @return <b>SOFTBUS_DISCOVER_MANAGER_INNERFUNCTION_FAIL</b> Internal function error.
 * @return <b>SOFTBUS_OK</b> Passive stop discovery function stopped successfully
 */
int32_t DiscStopDiscovery(const char *packageName, int32_t subscribeId, int32_t callingPid);

int32_t DiscSetDisplayName(const char *pkgName, const char *nameData, uint32_t len);

int32_t DiscGetDisplayName(char *displayName, uint32_t length, uint32_t remainLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* DISC_MANAGER_H */