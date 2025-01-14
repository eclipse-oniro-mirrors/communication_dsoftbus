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

#include "trans_session_account_adapter.h"

#include "os_account_manager.h"
#include "softbus_error_code.h"
#include "trans_log.h"

using namespace OHOS;

int32_t TransGetForegroundUserId(void)
{
    int32_t userId = INVALID_USER_ID;
    int32_t ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(userId);
    if (ret != 0) {
        userId = INVALID_USER_ID;
        TRANS_LOGE(TRANS_CTRL, "GetForegroundOsAccountLocalId failed ret=%{public}d.", ret);
    }
    return userId;
}