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

#ifndef AUTH_NEGOTIATE_CHANNEL_H
#define AUTH_NEGOTIATE_CHANNEL_H

#include <map>
#include <mutex>
#include <memory>

#include "auth_interface.h"
#include "softbus_common.h"

#include "wifi_direct_initiator.h"
#include "channel/negotiate_channel.h"

namespace OHOS::SoftBus {
struct AuthOpenEvent {
    int reason_;
    AuthHandle handle_;
};

class AuthNegotiateChannel : public NegotiateChannel {
public:
    struct OpenParam {
        AuthLinkType type;
        std::string remoteUuid;
        std::string remoteIp;
        int remotePort;
        ListenerModule module;
    };

    static void Init();
    static std::pair<int, ListenerModule> StartListening(AuthLinkType type, const std::string &localIp, int port);
    static void StopListening(AuthLinkType type, ListenerModule module);
    static int OpenConnection(const OpenParam &param, const std::shared_ptr<AuthNegotiateChannel> &channel);

    explicit AuthNegotiateChannel(const AuthHandle &handle);
    AuthNegotiateChannel(const AuthNegotiateChannel &channel) = default;
    ~AuthNegotiateChannel() override;

    bool operator==(const AuthNegotiateChannel &other) const;
    bool IsMeta() const;
    void SetClose();

    int SendMessage(const NegotiateMessage &msg) const override;
    std::string GetRemoteDeviceId() const override;

private:
    static void OnConnOpened(uint32_t requestId, AuthHandle authHandle);
    static void OnConnOpenFailed(uint32_t requestId, int32_t reason);

    class Initiator {
    public:
        Initiator()
        {
            WifiDirectInitiator::GetInstance().Add(AuthNegotiateChannel::Init);
        }
    };

    static inline Initiator initiator_;
    static inline std::recursive_mutex lock_;
    static inline std::map<uint32_t, std::string> requestIdToDeviceIdMap_;
    AuthHandle handle_;
    bool close_;
};
} // namespace OHOS::SoftBus
#endif