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

#include "lnn_event_monitor_impl.h"

#include <functional>
#include <list>
#include <mutex>
#include <unordered_map>

#include "bus_center_event.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "lnn_async_callback_utils.h"
#include "lnn_log.h"
#include "lnn_ohos_account.h"
#include "lnn_ohos_account_adapter.h"
#include "long_wrapper.h"
#include "softbus_adapter_json.h"
#include "power_mgr_client.h"
#include "softbus_error_code.h"
#include "lnn_init_monitor.h"

static const int32_t DELAY_LEN = 1000;
static const int32_t RETRY_MAX = 20;

#define COMMON_EVENT_USER_SLEEP_STATE_CHANGED "COMMON_EVENT_USER_SLEEP_STATE_CHANGED"

#ifdef DSOFTBUS_FEATURE_MULTI_FOREGROUND_USER
enum MultiScreenId {
    SCREEN_ID_CENTER = 0,
    SCREEN_ID_CO_DRIVER = 4,
    SCREEN_ID_ADDITIONAL = 6,
};

struct LnnMultiScreenStateNode {
    int64_t dispalyId;
    SoftBusMultiScreenState state;
};

static std::mutex g_multiScreenLock;
static std::list<LnnMultiScreenStateNode> g_multiScreenStateList;

static bool UpdateMultiScreenState(int64_t screenId, SoftBusMultiScreenState state)
{
    std::lock_guard<std::mutex> lock(g_multiScreenLock);
    for (auto &item : g_multiScreenStateList) {
        if (item.dispalyId == screenId) {
            if (item.state == state) {
                return false;
            }
            item.state = state;
            return true;
        }
    }
    LnnMultiScreenStateNode node = { .dispalyId = screenId, .state = state };
    g_multiScreenStateList.push_back(node);
    return true;
}

static bool IsValidScreenId(int64_t screenId)
{
    return screenId == SCREEN_ID_CENTER || screenId == SCREEN_ID_CO_DRIVER || screenId == SCREEN_ID_ADDITIONAL;
}

static void NotifyMultiScreenStateChanged(int32_t stateType, int64_t screenId, int32_t userId)
{
    if (stateType != SOFTBUS_MULTI_SCREEN_ON && stateType != SOFTBUS_MULTI_SCREEN_OFF) {
        return;
    }
    if (!IsValidScreenId(screenId)) {
        return;
    }
    bool changed = UpdateMultiScreenState(screenId, (SoftBusMultiScreenState)stateType);
    if (changed) {
        LnnNotifyMultiScreenStateChangeEvent((SoftBusMultiScreenState)stateType, screenId, userId);
    }
}

static void ParseAllScreenStates(const std::string &allStates)
{
    JsonObj *jsonArray = JSON_ParseArray(allStates.c_str(), static_cast<uint32_t>(allStates.length()));
    if (jsonArray == nullptr) {
        LNN_LOGE(LNN_EVENT, "parse allScreenStates fail or not array");
        return;
    }
    int32_t size = JSON_GetArraySize(jsonArray);
    for (int32_t i = 0; i < size; i++) {
        JsonObj *item = JSON_GetArrayItem(jsonArray, i);
        if (item == nullptr) {
            continue;
        }
        int64_t screenId = -1;
        int32_t userId = -1;
        int32_t stateType = -1;
        JSON_GetInt64FromOject(item, "screenId", &screenId);
        JSON_GetInt32FromOject(item, "userId", &userId);
        JSON_GetInt32FromOject(item, "stateType", &stateType);
        JSON_Delete(item);
        NotifyMultiScreenStateChanged(stateType, screenId, userId);
    }
    JSON_Delete(jsonArray);
}
#endif

namespace OHOS {
namespace EventFwk {

using EventHandler = std::function<void(const CommonEventData &)>;

static void HandleScreenOff(const CommonEventData &data)
{
    (void)data;
    LnnNotifyScreenStateChangeEvent(SOFTBUS_SCREEN_OFF);
}

static void HandleScreenOn(const CommonEventData &data)
{
    (void)data;
    LnnNotifyScreenStateChangeEvent(SOFTBUS_SCREEN_ON);
}

static void HandleUserUnlocked(const CommonEventData &data)
{
    (void)data;
    LnnNotifyScreenLockStateChangeEvent(SOFTBUS_USER_UNLOCK);
}

static void HandleScreenUnlocked(const CommonEventData &data)
{
    (void)data;
    LnnNotifyScreenLockStateChangeEvent(SOFTBUS_SCREEN_UNLOCK);
}

static void HandleDataShareReady(const CommonEventData &data)
{
    (void)data;
    LnnNotifyDataShareStateChangeEvent(SOFTBUS_DATA_SHARE_READY);
}

static void HandleTimeChanged(const CommonEventData &data)
{
    (void)data;
    LnnNotifySysTimeChangeEvent();
}

static void HandleBootCompleted(const CommonEventData &data)
{
    (void)data;
    LnnNotifyDeviceRiskStateChangeEvent();
}

static void HandleDistributedAccountChange(const CommonEventData &data)
{
    const AAFwk::WantParams &wantParams = data.GetWant().GetParams();
    int32_t eventUserId = wantParams.GetIntParam("userId", -1);
    std::string action = data.GetWant().GetAction();
    if (action == CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN) {
        LNN_LOGI(LNN_EVENT, "login eventUserId=%{public}d", eventUserId);
        LnnUpdateOhosAccount(UPDATE_ACCOUNT_ONLY);
        if (LnnIsSameAccountGroupDeviceByUserId(eventUserId)) {
            LnnNotifyAccountStateChangeEvent(SOFTBUS_ACCOUNT_LOG_IN, eventUserId);
        } else {
            LNN_LOGI(LNN_EVENT, "login but no same account group, skip LOG_IN, wait for hichain onGroupCreated");
        }
        return;
    }
    if (action != CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT &&
        action != CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF) {
        return;
    }
#ifdef DSOFTBUS_FEATURE_MULTI_FOREGROUND_USER
    LNN_LOGI(LNN_EVENT, "logout eventUserId=%{public}d", eventUserId);
    LnnNotifyAccountStateChangeEvent(SOFTBUS_ACCOUNT_LOG_OUT, eventUserId);
#else
    int32_t activeUserId = JudgeDeviceTypeAndGetOsAccountIds();
    LNN_LOGI(LNN_EVENT, "logout activeUserId=%{public}d, eventUserId=%{public}d", activeUserId, eventUserId);
    if (eventUserId == activeUserId) {
        LnnNotifyAccountStateChangeEvent(SOFTBUS_ACCOUNT_LOG_OUT, eventUserId);
    }
#endif
}

#ifdef DSOFTBUS_FEATURE_MULTI_FOREGROUND_USER
static const char *COMMON_EVENT_MULTI_SCREEN_CHANGE = "usual.event.ivi.DISPLAY_STATE_CHANGED";

static void HandleSubProfileSwitched(const CommonEventData &data)
{
    const AAFwk::WantParams &wantParams = data.GetWant().GetParams();
    int32_t userId = wantParams.GetIntParam("userId", -1);
    if (userId < 0) {
        LNN_LOGE(LNN_EVENT, "SWITCHED event missing userId, skip check, please check event params");
        return;
    }
    LNN_LOGI(LNN_EVENT, "SWITCHED event for userId=%{public}d", userId);
    LnnNotifyAccountSwitchCheckEvent(userId);
}

static void HandleMultiScreenStateChange(const CommonEventData &data)
{
    auto want = data.GetWant();
    int64_t screenId = want.GetLongParam("screenId", -1);
    int32_t userId = want.GetIntParam("userId", -1);
    int32_t stateType = want.GetIntParam("stateType", -1);
    std::string allStates = want.GetStringParam("allScreenStates");
    if (allStates.empty()) {
        NotifyMultiScreenStateChanged(stateType, screenId, userId);
        return;
    }
    ParseAllScreenStates(allStates);
}

#endif

static void HandleUserSwitched(const CommonEventData &data)
{
    (void)data;
    LnnUpdateConstraintMapForCurrentAccount();
    LnnNotifyUserSwitchEvent(SOFTBUS_USER_SWITCHED);
}

static void HandleUserSleepStateChange(const CommonEventData &data)
{
    bool isSleep = data.GetWant().GetBoolParam("isSleep", false);
    if (isSleep) {
        LnnNotifyUserSleepStateEvent(SOFTBUS_USER_SLEEP);
    } else {
        LnnNotifyUserSleepStateEvent(SOFTBUS_USER_WAKE);
    }
}

class CommonEventMonitor : public CommonEventSubscriber {
public:
    explicit CommonEventMonitor(const CommonEventSubscribeInfo &subscriberInfo);
    virtual ~CommonEventMonitor() {}
    virtual void OnReceiveEvent(const CommonEventData &data);
private:
    std::unordered_map<std::string, EventHandler> eventHandlers_;
};

CommonEventMonitor::CommonEventMonitor(const CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
    eventHandlers_[CommonEventSupport::COMMON_EVENT_SCREEN_OFF] = HandleScreenOff;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_SCREEN_ON] = HandleScreenOn;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_USER_UNLOCKED] = HandleUserUnlocked;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED] = HandleScreenUnlocked;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY] = HandleDataShareReady;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_TIME_CHANGED] = HandleTimeChanged;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED] = HandleBootCompleted;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT] = HandleDistributedAccountChange;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF] = HandleDistributedAccountChange;
    eventHandlers_[CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN] = HandleDistributedAccountChange;
#ifdef DSOFTBUS_FEATURE_MULTI_FOREGROUND_USER
    eventHandlers_[CommonEventSupport::COMMON_EVENT_OS_ACCOUNT_SUB_PROFILE_SWITCHED] = HandleSubProfileSwitched;
    eventHandlers_[COMMON_EVENT_MULTI_SCREEN_CHANGE] = HandleMultiScreenStateChange;
#endif
    eventHandlers_[CommonEventSupport::COMMON_EVENT_USER_SWITCHED] = HandleUserSwitched;
    eventHandlers_[COMMON_EVENT_USER_SLEEP_STATE_CHANGED] = HandleUserSleepStateChange;
}

void CommonEventMonitor::OnReceiveEvent(const CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    LNN_LOGI(LNN_EVENT, "notify common event=%{public}s", action.c_str());
    auto it = eventHandlers_.find(action);
    if (it != eventHandlers_.end()) {
        it->second(data);
    }
}

class SubscribeEvent {
public:
    int32_t SubscribeCommonEvent();
private:
    std::shared_ptr<CommonEventMonitor> subscriber_ = nullptr;
};

int32_t SubscribeEvent::SubscribeCommonEvent()
{
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOUT);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGOFF);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DISTRIBUTED_ACCOUNT_LOGIN);
#ifdef DSOFTBUS_FEATURE_MULTI_FOREGROUND_USER
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_OS_ACCOUNT_SUB_PROFILE_SWITCHED);
    matchingSkills.AddEvent(COMMON_EVENT_MULTI_SCREEN_CHANGE);
#endif
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_DATA_SHARE_READY);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_TIME_CHANGED);
    matchingSkills.AddEvent(CommonEventSupport::COMMON_EVENT_BOOT_COMPLETED);
    matchingSkills.AddEvent(COMMON_EVENT_USER_SLEEP_STATE_CHANGED);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    subscriber_ = std::make_shared<CommonEventMonitor>(subscriberInfo);
    if (!CommonEventManager::SubscribeCommonEvent(subscriber_)) {
        LNN_LOGE(LNN_EVENT, "subscribe common event err");
        return SOFTBUS_NETWORK_SUBSCRIBE_COMMON_EVENT_FAILED;
    }
    return SOFTBUS_OK;
}
} // namespace EventFwk
} // namespace OHOS

bool LnnQueryLocalScreenStatusOnce(bool notify)
{
    bool isScreenOn = OHOS::PowerMgr::PowerMgrClient::GetInstance().IsScreenOn(true);
    LNN_LOGI(LNN_EVENT, "query screen status is %{public}s", isScreenOn ? "on" : "off");
    if (notify) {
        SoftBusScreenState screenState = isScreenOn ? SOFTBUS_SCREEN_ON : SOFTBUS_SCREEN_OFF;
        LnnNotifyScreenStateChangeEvent(screenState);
    }
    return isScreenOn;
}

#ifdef DSOFTBUS_FEATURE_MULTI_FOREGROUND_USER
bool LnnIsAllMultiScreenOff(void)
{
    std::lock_guard<std::mutex> lock(g_multiScreenLock);
    bool allOff = true;
    for (const auto &item : g_multiScreenStateList) {
        if (item.state != SOFTBUS_MULTI_SCREEN_OFF) {
            allOff = false;
            LNN_LOGI(LNN_EVENT, "screen not off, screenId=%{public}" PRId64, item.dispalyId);
            break;
        }
    }
    LNN_LOGI(LNN_EVENT, "all multi screen off=%{public}d, listSize=%{public}zu", allOff, g_multiScreenStateList.size());
    return allOff;
}

SoftBusScreenState LnnQueryCenterScreenStatus(void)
{
    std::lock_guard<std::mutex> lock(g_multiScreenLock);
    for (const auto &item : g_multiScreenStateList) {
        if (item.dispalyId == static_cast<int64_t>(SCREEN_ID_CENTER)) {
            return (item.state == SOFTBUS_MULTI_SCREEN_ON) ? SOFTBUS_SCREEN_ON : SOFTBUS_SCREEN_OFF;
        }
    }
    LNN_LOGW(LNN_EVENT, "center screen state not found, return UNKNOWN");
    return SOFTBUS_SCREEN_UNKNOWN;
}
#else
bool LnnIsAllMultiScreenOff(void)
{
    return false;
}

SoftBusScreenState LnnQueryCenterScreenStatus(void)
{
    return SOFTBUS_SCREEN_UNKNOWN;
}
#endif

int32_t LnnSubscribeCommonEvent(void)
{
    OHOS::EventFwk::SubscribeEvent *subscriberPtr = new OHOS::EventFwk::SubscribeEvent();
    if (subscriberPtr == nullptr) {
        LNN_LOGE(LNN_EVENT, "SubscribeEvent init fail");
        return SOFTBUS_MEM_ERR;
    }
    if (subscriberPtr->SubscribeCommonEvent() != SOFTBUS_OK) {
        delete subscriberPtr;
        LNN_LOGE(LNN_EVENT, "subscribe common event fail");
        return SOFTBUS_NETWORK_SUBSCRIBE_COMMON_EVENT_FAILED;
    }
    LNN_LOGI(LNN_EVENT, "subscribe common event success");
    LnnUpdateOhosAccount(UPDATE_HEARTBEAT);
    LnnUpdateConstraintMapForCurrentAccount();
    if (!LnnIsDefaultOhosAccount()) {
        LnnNotifyAccountStateChangeEvent(SOFTBUS_ACCOUNT_LOG_IN, JudgeDeviceTypeAndGetOsAccountIds());
    }
    (void)LnnQueryLocalScreenStatusOnce(true);
    delete subscriberPtr;
    return SOFTBUS_OK;
}

int32_t LnnInitCommonEventMonitorImpl(void)
{
    int32_t ret = LnnInitModuleNotifyWithRetryAsync(INIT_DEPS_SCREEN_STATUS, LnnSubscribeCommonEvent, RETRY_MAX,
        DELAY_LEN, false);
    if (ret != SOFTBUS_OK) {
        LNN_LOGE(LNN_INIT, "LnnAsyncCallbackHelper fail");
    }
    return ret;
}
