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
#include "wifi_direct_executor.h"

#include <unordered_map>
#include <utility>
#include <unistd.h>

#include "wifi_direct_scheduler.h"
#include "data/link_manager.h"
#include "event/wifi_direct_event_dispatcher.h"
#include "conn_log.h"
#include "ffrt.h"
#include "concurrent_task_client.h"
#include "utils/wifi_direct_anonymous.h"
#include "utils/wifi_direct_utils.h"
#include "wifi_direct_scheduler_factory.h"
#include "command/negotiate_command.h"

namespace OHOS::SoftBus {
namespace {
constexpr auto FFRT_TASK_NAME = "wifi_direct_exec";
}
SOFTBUS_DEFAULT_USED_EXPORT
WifiDirectExecutor::WifiDirectExecutor(const std::string &remoteDeviceId, WifiDirectScheduler &scheduler,
                                       std::shared_ptr<WifiDirectProcessor> &processor, bool active)
    : remoteDeviceId_(remoteDeviceId), scheduler_(scheduler), processor_(processor), active_(active),
      boostQos_(false), started_(false)
{
    CONN_LOGI(CONN_WIFI_DIRECT, "remoteDeviceId=%{public}s, active=%{public}d",
              WifiDirectAnonymizeDeviceId(remoteDeviceId_).c_str(), active_);
}

SOFTBUS_DEFAULT_USED_EXPORT WifiDirectExecutor::~WifiDirectExecutor()
{
    CONN_LOGI(CONN_WIFI_DIRECT, "remoteDeviceId=%{public}s", WifiDirectAnonymizeDeviceId(remoteDeviceId_).c_str());
}

void WifiDirectExecutor::Start()
{
    if (started_) {
        CONN_LOGI(CONN_WIFI_DIRECT, "remoteDeviceId=%{public}s repeat start, ignore",
            WifiDirectAnonymizeDeviceId(remoteDeviceId_).c_str());
        return;
    }
    started_ = true;
    SubmitProcessorTask();
}

void WifiDirectExecutor::RequestQosAuth()
{
    std::unordered_map<std::string, std::string> payload;
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
}

void WifiDirectExecutor::SetBoostQos(bool boostQos)
{
    boostQos_ = boostQos;
}

void WifiDirectExecutor::SubmitProcessorTask()
{
    ffrt::task_attr attr;
    attr.name(FFRT_TASK_NAME);
    if (boostQos_) {
        RequestQosAuth();
        attr.qos(ffrt::qos_user_interactive);
    }
    ffrt::submit(
        [this] {
            Run(processor_);
        },
        attr);
}

void WifiDirectExecutor::Run(std::shared_ptr<WifiDirectProcessor> processor)
{
    processor_ = std::move(processor);
    do {
        if (IsActive()) {
            WifiDirectTrace::StartTrace(WifiDirectUtils::GetLocalUuid(), remoteDeviceId_);
        } else {
            WifiDirectTrace::StartTrace(remoteDeviceId_, WifiDirectUtils::GetLocalUuid());
        }

        processor_->BindExecutor(this);
        try {
            CONN_LOGI(CONN_WIFI_DIRECT, "processor run");
            processor_->Run();
        } catch (const std::exception& e) {
            if (std::string("std::exception") != e.what()) {
                CONN_LOGE(CONN_WIFI_DIRECT, "catch exception=%{public}s", e.what());
            }
            std::string src = typeid(e).name();
            std::string dst = typeid(ProcessorTerminate).name();
            if (WifiDirectUtils::CompareIgnoreCase(src, dst) == 0) {
                LinkManager::GetInstance().Dump();
                CONN_LOGI(CONN_WIFI_DIRECT, "processor terminate");
                ProcessUnHandleCommand();
            }
        }

        std::lock_guard lock(processorLock_);
        processor_ = nullptr;
        WifiDirectTrace::StopTrace();
    } while (scheduler_.ProcessNextCommand(this, processor_));

    CONN_LOGI(CONN_WIFI_DIRECT, "executor terminate");
}

SOFTBUS_DEFAULT_USED_EXPORT std::string WifiDirectExecutor::GetRemoteDeviceId()
{
    return remoteDeviceId_;
}

SOFTBUS_DEFAULT_USED_EXPORT void WifiDirectExecutor::SetRemoteDeviceId(const std::string &remoteDeviceId)
{
    remoteDeviceId_ = remoteDeviceId;
}

bool WifiDirectExecutor::IsActive() const
{
    return active_;
}

SOFTBUS_DEFAULT_USED_EXPORT void WifiDirectExecutor::SetActive(bool active)
{
    active_ = active;
}

SOFTBUS_DEFAULT_USED_EXPORT bool WifiDirectExecutor::CanAcceptNegotiateData(WifiDirectCommand &command)
{
    std::lock_guard lock(processorLock_);
    if (processor_ == nullptr) {
        return false;
    }
    return processor_->CanAcceptNegotiateData(command);
}

bool WifiDirectExecutor::HasAnyConnectCommand(WifiDirectLinkType linkType)
{
    bool found = false;
    GetSender().ProcessUnHandle([&](std::shared_ptr<WifiDirectEventBase> &content) {
        if (found || content == nullptr) {
            return;
        }
        if (content->getContentTypeid() == typeid(std::shared_ptr<ConnectCommand>).name()) {
            auto cw = std::static_pointer_cast<WifiDirectEventWrapper<std::shared_ptr<ConnectCommand>>>(content);
            if (cw->content_ != nullptr && cw->content_->GetLinkType() == linkType) {
                found = true;
            }
        }
    });
    if (found) {
        return true;
    }
    std::lock_guard lock(processorLock_);
    return processor_ != nullptr && processor_->HasActiveConnectCommand(linkType);
}

SOFTBUS_DEFAULT_USED_EXPORT WifiDirectEventDispatcher WifiDirectExecutor::WaitEvent()
{
    return receiver_.Wait();
}

SOFTBUS_DEFAULT_USED_EXPORT void WifiDirectExecutor::ProcessUnHandleCommand()
{
    CONN_LOGI(CONN_WIFI_DIRECT, "enter");
    WifiDirectSchedulerFactory::GetInstance().GetScheduler().RejectNegotiateData(*processor_);
    GetSender().ProcessUnHandle([this](std::shared_ptr<WifiDirectEventBase> &content) {
        if (content != nullptr && content->getContentTypeid() == typeid(std::shared_ptr<NegotiateCommand>).name()) {
            CONN_LOGI(CONN_WIFI_DIRECT, "type id is same");
            auto ncw =
                std::static_pointer_cast<WifiDirectEventWrapper<std::shared_ptr<NegotiateCommand>>>(content);
            processor_->HandleCommandAfterTerminate(*ncw->content_);
            return;
        }
    });
    GetSender().Clear();
}

void WifiDirectExecutor::Dump(std::list<std::shared_ptr<ProcessorSnapshot>> &snapshots)
{
    std::lock_guard lock(processorLock_);
    if (processor_ != nullptr) {
        snapshots.push_back(std::make_shared<ProcessorSnapshot>(
            processor_->GetRemoteDeviceId(), processor_->GetProcessorName(), processor_->GetState()));
    }
}
}
