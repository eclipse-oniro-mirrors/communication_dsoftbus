/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "auth_hichain.h"

#include "auth_identity_service_adapter.h"
#include "auth_log.h"
#include "auth_manager.h"
#include "lnn_async_callback_utils.h"

#define DELAY_REG_HICHAIN_TIME 5000
#define REG_HICHAIN_MAX_RETRY 10

static int32_t g_regRetryCount = 0;

static void RetryRegHichainListener(void *para)
{
    (void)para;
    g_regRetryCount++;
    if (g_regRetryCount > REG_HICHAIN_MAX_RETRY) {
        AUTH_LOGE(AUTH_INIT, "lite reg hichain listener reach max retry=%{public}d, please check deviceauth",
            REG_HICHAIN_MAX_RETRY);
        return;
    }
    if (RegTrustListenerOnHichainSaStart() != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_INIT, "lite reg trust listener failed, please wait deviceauth ready, retry after 5s, "
            "count=%{public}d", g_regRetryCount);
        (void)LnnAsyncCallbackDelayHelper(GetLooper(LOOP_TYPE_DEFAULT),
            RetryRegHichainListener, NULL, DELAY_REG_HICHAIN_TIME);
        return;
    }
    if (IdServiceRegCredMgr() != SOFTBUS_OK) {
        AUTH_LOGE(AUTH_INIT, "lite reg cred listener failed, please wait deviceauth ready, retry after 5s, "
            "count=%{public}d", g_regRetryCount);
        (void)LnnAsyncCallbackDelayHelper(GetLooper(LOOP_TYPE_DEFAULT),
            RetryRegHichainListener, NULL, DELAY_REG_HICHAIN_TIME);
        return;
    }
    AUTH_LOGI(AUTH_INIT, "lite reg hichain listener successful, count=%{public}d", g_regRetryCount);
}

int32_t RegHichainSaStatusListener(void)
{
    AUTH_LOGI(AUTH_INIT, "lite reg hichain sa listener enter");
    RetryRegHichainListener(NULL);
    return SOFTBUS_OK;
}

int32_t UnRegHichainSaStatusListener(void)
{
    IdServiceUnRegCredMgr();
    return SOFTBUS_OK;
}
