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
#ifndef DUMMY_NEGOTIATE_CHANNEL_H
#define DUMMY_NEGOTIATE_CHANNEL_H

#include "channel/negotiate_channel.h"

namespace OHOS::SoftBus {
class DummyNegotiateChannel : public NegotiateChannel {
public:
    ~DummyNegotiateChannel() override;
    int SendMessage(const NegotiateMessage &msg) const override;
    std::string GetRemoteDeviceId() const override;
    NegotiateChannelType GetType() const override
    {
        return NegotiateChannelType::DUMMY_CHANNEL;
    }
};
} // namespace OHOS::SoftBus
#endif