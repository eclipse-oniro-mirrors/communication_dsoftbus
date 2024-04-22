/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "link_manager.h"

#include "conn_log.h"

#include "utils/wifi_direct_anonymous.h"
#include "wifi_direct_manager.h"

namespace OHOS::SoftBus {
int LinkManager::AllocateLinkId()
{
    std::lock_guard lock(lock_);
    if (currentLinkId_ < 0) {
        currentLinkId_ = 0;
    }
    auto newId = currentLinkId_++;
    while (GetLinkById(newId) != nullptr) {
        newId = currentLinkId_++;
    }
    return newId;
}

std::shared_ptr<InnerLink> LinkManager::GetLinkById(int linkId)
{
    std::lock_guard lock(lock_);
    for (const auto &link : links_) {
        if (link.second->IsContainId(linkId)) {
            return link.second;
        }
    }
    return nullptr;
}

void LinkManager::ForEach(const Checker &checker)
{
    std::lock_guard lock(lock_);
    for (auto &[key, link] : links_) {
        if (checker(*link)) {
            break;
        }
    }
}

bool LinkManager::ProcessIfPresent(InnerLink::LinkType type, const std::string &remoteDeviceId, const Handler &handler)
{
    std::lock_guard lock(lock_);
    auto iterator = links_.find({ type, remoteDeviceId });
    if (iterator == links_.end()) {
        CONN_LOGE(CONN_WIFI_DIRECT, "type=%{public}d remoteDeviceId=%{public}s not found", static_cast<int>(type),
            WifiDirectAnonymizeDeviceId(remoteDeviceId).c_str());
        return false;
    }

    handler(*iterator->second);
    return true;
}

bool LinkManager::ProcessIfAbsent(InnerLink::LinkType type, const std::string &remoteDeviceId, const Handler &handler)
{
    std::lock_guard lock(lock_);
    auto iterator = links_.find({ type, remoteDeviceId });
    if (iterator != links_.end()) {
        CONN_LOGE(CONN_WIFI_DIRECT, "type=%{public}d remoteDeviceId=%{public}s already exist", static_cast<int>(type),
            WifiDirectAnonymizeDeviceId(remoteDeviceId).c_str());
        return false;
    }

    auto link = std::make_shared<InnerLink>(type, remoteDeviceId);
    links_.insert({
        { type, remoteDeviceId },
        link
    });
    handler(*link);
    return true;
}

bool LinkManager::ProcessIfPresent(const std::string &remoteMac, const Handler &handler)
{
    std::lock_guard lock(lock_);
    auto iterator = std::find_if(links_.begin(), links_.end(), [&remoteMac](const auto &link) {
        return link.second->GetRemoteBaseMac() == remoteMac;
    });
    if (iterator == links_.end()) {
        CONN_LOGE(CONN_WIFI_DIRECT, "remoteMac=%{public}s not found", WifiDirectAnonymizeMac(remoteMac).c_str());
        return false;
    }

    handler(*iterator->second);
    return true;
}

bool LinkManager::ProcessIfAbsent(const std::string &remoteMac, const Handler &handler)
{
    std::lock_guard lock(lock_);
    auto iterator = std::find_if(links_.begin(), links_.end(), [&remoteMac](const auto &link) {
        return link.second->GetRemoteBaseMac() == remoteMac;
    });
    if (iterator != links_.end()) {
        CONN_LOGE(CONN_WIFI_DIRECT, "remoteMac=%{public}s already exist", WifiDirectAnonymizeMac(remoteMac).c_str());
        return false;
    }

    auto link = std::make_shared<InnerLink>(remoteMac);
    handler(*link);
    auto type = link->GetLinkType();
    auto remoteDeviceId = link->GetRemoteDeviceId();
    links_.insert({
        { type, remoteDeviceId },
        link
    });
    return true;
}

bool LinkManager::ProcessIfPresent(int linkId, const Handler &handler)
{
    std::lock_guard lock(lock_);
    auto iterator = std::find_if(links_.begin(), links_.end(), [&linkId](const auto &link) {
        return link.second->IsContainId(linkId);
    });
    if (iterator == links_.end()) {
        CONN_LOGE(CONN_WIFI_DIRECT, "link id=%{public}d not found", linkId);
        return false;
    }

    handler(*iterator->second);
    return true;
}

void LinkManager::RemoveLink(InnerLink::LinkType type, const std::string &remoteDeviceId)
{
    std::lock_guard lock(lock_);
    auto it = links_.find({ type, remoteDeviceId });
    if (it != links_.end()) {
        auto &link = it->second;
        if (link->GetState() == InnerLink::LinkState::CONNECTED) {
            GetWifiDirectManager()->notifyOffline(link->GetRemoteBaseMac().c_str(), link->GetRemoteIpv4().c_str(),
                link->GetRemoteDeviceId().c_str(), link->GetLocalIpv4().c_str());
        }
        links_.erase(it);
    }
}

void LinkManager::RemoveLinks(InnerLink::LinkType type)
{
    std::lock_guard lock(lock_);
    auto it = links_.begin();
    while (it != links_.end()) {
        if (it->first.first == type) {
            if (it->second->GetState() == InnerLink::LinkState::CONNECTED) {
                GetWifiDirectManager()->notifyOffline(it->second->GetRemoteBaseMac().c_str(),
                    it->second->GetRemoteIpv4().c_str(), it->second->GetRemoteDeviceId().c_str(),
                    it->second->GetLocalIpv4().c_str());
            }

            it = links_.erase(it);
        } else {
            it++;
        }
    }
}

std::shared_ptr<InnerLink> LinkManager::GetReuseLink(
    WifiDirectConnectType connectType, const std::string &remoteDeviceId)
{
    InnerLink::LinkType linkType { InnerLink::LinkType::P2P };
    if (connectType == WIFI_DIRECT_CONNECT_TYPE_AUTH_NEGO_HML ||
        connectType == WIFI_DIRECT_CONNECT_TYPE_BLE_TRIGGER_HML ||
        connectType == WIFI_DIRECT_CONNECT_TYPE_AUTH_TRIGGER_HML) {
        linkType = InnerLink::LinkType::HML;
    }

    std::lock_guard lock(lock_);
    auto iterator = links_.find({ linkType, remoteDeviceId });
    if (iterator == links_.end() || iterator->second->GetState() != InnerLink::LinkState::CONNECTED) {
        return nullptr;
    }
    return iterator->second;
}

void LinkManager::Dump() const
{
    std::lock_guard lock(lock_);
    for (const auto &[key, value] : links_) {
        value->Dump();
    }
    if (links_.empty()) {
        CONN_LOGI(CONN_WIFI_DIRECT, "no inner link");
    }
}
} // namespace OHOS::SoftBus