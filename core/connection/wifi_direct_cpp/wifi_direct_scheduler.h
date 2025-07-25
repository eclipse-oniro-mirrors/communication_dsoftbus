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
#ifndef WIFI_DIRECT_SCHEDULER_H
#define WIFI_DIRECT_SCHEDULER_H

#include <list>
#include <map>
#include <mutex>
#include <memory>
#include <string>
#include <thread>
#include "command/connect_command.h"
#include "command/wifi_direct_command.h"
#include "dfx/processor_snapshot.h"
#include "event/wifi_direct_event_wrapper.h"
#include "utils/wifi_direct_anonymous.h"
#include "utils/wifi_direct_utils.h"
#include "wifi_direct_executor.h"
#include "wifi_direct_executor_factory.h"
#include "wifi_direct_types.h"
#include "wifi_direct_executor_manager.h"

namespace OHOS::SoftBus {
class WifiDirectSchedulerFactory;
class WifiDirectScheduler {
public:
    virtual ~WifiDirectScheduler() = default;

    int ConnectDevice(const WifiDirectConnectInfo &info, const WifiDirectConnectCallback &callback,
                      bool markRetried = false);
    int ConnectDevice(const std::shared_ptr<ConnectCommand> &command, bool markRetried = false);
    int CancelConnectDevice(const WifiDirectConnectInfo &info);
    int DisconnectDevice(WifiDirectDisconnectInfo &info, WifiDirectDisconnectCallback &callback);
    int ForceDisconnectDevice(WifiDirectForceDisconnectInfo &info, WifiDirectDisconnectCallback &callback);

    template<typename Command>
    void ProcessNegotiateData(const std::string &remoteDeviceId, Command &command)
    {
        CONN_CHECK_AND_RETURN_LOGE(!remoteDeviceId.empty(), CONN_WIFI_DIRECT, "remote device id is empty");
        auto aDeviceId = WifiDirectAnonymizeDeviceId(remoteDeviceId);
        CONN_LOGD(CONN_WIFI_DIRECT, "remoteDeviceId=%{public}s", aDeviceId.c_str());
        std::lock_guard lock(executorLock_);
        auto executor = executorManager_.Find(remoteDeviceId);
        if (executor != nullptr) {
            if (executor->CanAcceptNegotiateData(command)) {
                CONN_LOGI(CONN_WIFI_DIRECT, "send command to executor=%{public}s, commandId=%{public}d",
                          aDeviceId.c_str(), command.GetId());
                executor->SendEvent(std::make_shared<Command>(command));
                return;
            }
            std::lock_guard commandLock(commandLock_);
            CONN_LOGI(CONN_WIFI_DIRECT, "push command to list, commandId=%{public}u", command.GetId());
            commandList_.push_back(std::make_shared<Command>(command));
            return;
        }

        if (executorManager_.Size() == MAX_EXECUTOR) {
            CONN_LOGI(CONN_WIFI_DIRECT, "push command to list, commandId=%{public}u", command.GetId());
            std::lock_guard commandLock(commandLock_);
            commandList_.push_back(std::make_shared<Command>(command));
            return;
        }

        auto processor = command.GetProcessor();
        if (processor == nullptr) {
            CONN_LOGE(CONN_WIFI_DIRECT, "get processor failed");
            return;
        }
        executor = WifiDirectExecutorFactory::GetInstance().NewExecutor(remoteDeviceId, *this, processor, false);
        if (executor == nullptr) {
            return;
        }
        executorManager_.Insert(remoteDeviceId, executor);
        CONN_LOGI(CONN_WIFI_DIRECT, "send command to executor=%{public}s, commandId=%{public}d",
                  aDeviceId.c_str(), command.GetId());
        executor->SendEvent(std::make_shared<Command>(command));
    }

    template<typename Event>
    void ProcessEvent(const std::string &remoteDeviceId, const Event &event)
    {
        std::lock_guard lock(executorLock_);
        auto executor = executorManager_.Find(remoteDeviceId);
        if (executor == nullptr) {
            CONN_LOGE(CONN_WIFI_DIRECT, "executor not exist, remoteDeviceId=%{public}s",
                      WifiDirectAnonymizeDeviceId(remoteDeviceId).c_str());
            return;
        }

        CONN_LOGI(CONN_WIFI_DIRECT, "send event to executor=%{public}s",
                  WifiDirectAnonymizeDeviceId(remoteDeviceId).c_str());
        auto content = std::make_shared<Event>(event);
        executor->SendEvent(content);
    }

    template<typename Command>
    void QueueCommandFront(Command &command)
    {
        std::lock_guard commandLock(commandLock_);
        CONN_LOGI(CONN_WIFI_DIRECT, "push data to list front");
        commandList_.push_front(std::make_shared<Command>(command));
    }

    template<typename Command>
    void QueueCommandBack(Command &command)
    {
        std::lock_guard commandLock(commandLock_);
        CONN_LOGI(CONN_WIFI_DIRECT, "push data to list back");
        commandList_.push_back(std::make_shared<Command>(command));
    }

    template<typename Command>
    void FetchAndDispatchCommand(const std::string &remoteDeviceId, CommandType type)
    {
        std::lock_guard executorLock(executorLock_);
        auto executor = executorManager_.Find(remoteDeviceId);
        if (executor == nullptr) {
            return;
        }

        std::lock_guard commandLock(commandLock_);
        for (auto cit = commandList_.begin(); cit != commandList_.end(); cit++) {
            auto &command = *cit;
            if (command->GetRemoteDeviceId() != remoteDeviceId) {
                continue;
            }
            if (command->GetType() == type) {
                auto cmd = std::static_pointer_cast<Command>(command);
                CONN_LOGI(CONN_WIFI_DIRECT, "type=%{public}d, commandId=%{public}u",
                          static_cast<int>(command->GetType()), command->GetId());
                executor->SendEvent(cmd);
                commandList_.erase(cit);
                return;
            }
        }
    }

    void RejectNegotiateData(WifiDirectProcessor &processor)
    {
        std::lock_guard executorLock(executorLock_);
        processor.SetRejectNegotiateData();
    }

    virtual bool ProcessNextCommand(WifiDirectExecutor *executor, std::shared_ptr<WifiDirectProcessor> &processor);

    bool CheckExecutorRunning(const std::string &remoteDeviceId)
    {
        std::lock_guard lock(executorLock_);
        return executorManager_.Find(remoteDeviceId) != nullptr;
    }

    void Dump(std::list<std::shared_ptr<ProcessorSnapshot>> &snapshots);

protected:
    int ScheduleActiveCommand(const std::shared_ptr<WifiDirectCommand> &command,
                              std::shared_ptr<WifiDirectExecutor> &executor);
    static void DumpNegotiateChannel(const WifiDirectNegotiateChannel &channel);

    static constexpr int MAX_EXECUTOR = 8;
    std::recursive_mutex executorLock_;
    WifiDirectExecutorManager executorManager_;
    std::recursive_mutex commandLock_;
    std::list<std::shared_ptr<WifiDirectCommand>> commandList_;

private:
    friend WifiDirectSchedulerFactory;
    static WifiDirectScheduler& GetInstance();
};
}
#endif
