/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef NAPI_SOFTBUS_BASE_ERROR_CODE_
#define NAPI_SOFTBUS_BASE_ERROR_CODE_

/* Error codes aligned with @ohos.distributed.softbusBase.d.ts @throws clauses. */

#define SOFTBUS_BASE_OK 0

/* 201: Permission denied, need ACCESS_SOFTBUS_SYS_HAP and DISTRIBUTED_DATASYNC. */
#define SOFTBUS_BASE_PERMISSION_ERR 201
/* 202: A non-system application calls a system API. */
#define SOFTBUS_BASE_PERMISSION_SYSTEMAPI_ERR 202
/* 401: Invalid argument (parameter check at the NAPI layer). */
#define SOFTBUS_BASE_INVALID_PARAM 401
/* 801: Capability not supported / feature not compiled. */
#define SOFTBUS_BASE_NOT_SUPPORT 801
/* 2000001: Internal error. An unexpected system error occurred. */
#define SOFTBUS_BASE_INTERNAL_ERR 2000001
/* 2000002: Caller error. The caller did not call the API in the specified order. */
#define SOFTBUS_BASE_CALLER_ERR 2000002
/* 2000003: Temporary error. The request failed due to a temporary error and can be retried. */
#define SOFTBUS_BASE_TEMP_ERR 2000003
/* 2006001: Underlying module error (BLE/broadcast subsystem). */
#define SOFTBUS_BASE_UNDERLYING_ERR 2006001

#endif /* NAPI_SOFTBUS_BASE_ERROR_CODE_ */
