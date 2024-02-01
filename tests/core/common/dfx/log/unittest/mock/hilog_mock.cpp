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

#include "hilog_mock.h"

namespace {
constexpr int ARG_COUNT = 1;
}

int HilogMock::HiLogPrint(LogType type, LogLevel level, unsigned int domain, const char *tag, const char *fmt, ...)
{
    return 0;
}

HilogMock::HilogMock()
{
    mock.store(this);
}
HilogMock::~HilogMock()
{
    mock.store(nullptr);
}

int HiLogPrint(LogType type, LogLevel level, unsigned int domain, const char *tag, const char *fmt, ...)
{
    char *args;
    va_list va_args;
    va_start(va_args, fmt);
    for (int i = 0; i < ARG_COUNT; ++i) {
        args = va_arg(va_args, char *);
    }
    va_end(va_args);
    return HilogMock::GetMock()->HiLogPrint(type, level, domain, tag, fmt, args);
}

#ifdef HILOG_RAWFORMAT
int HilogMock::HiLogPrintDict(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const FmtId *fmtid, const char *fmt, ...)
{
    return 0;
}

int HiLogPrintDict(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const FmtId *fmtid, const char *fmt, ...)
{
    (void)fmtid;
    char *args;
    va_list va_args;
    va_start(va_args, fmt);
    for (int i = 0; i < ARG_COUNT; ++i) {
        args = va_arg(va_args, char *);
    }
    va_end(va_args);
    return HilogMock::GetMock()->HiLogPrint(type, level, domain, tag, fmt, args);
}
#endif // HILOG_RAWFORMAT