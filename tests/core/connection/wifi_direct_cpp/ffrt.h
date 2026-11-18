/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FFRT_STUB_H
#define FFRT_STUB_H

#include <functional>

namespace ffrt {
enum qos {
    qos_inherit = -1,
    qos_background = 0,
    qos_utility = 1,
    qos_default = 2,
    qos_user_initiated = 3,
    qos_deadline_request = 4,
    qos_user_interactive = 5,
};

class task_attr {
public:
    task_attr() = default;
    ~task_attr() = default;
    task_attr& name(const char *name) { return *this; }
    task_attr& qos(enum qos level) { return *this; }
};

inline void submit(std::function<void()> &&func, const task_attr &attr = {}) { func(); }
inline void submit(const std::function<void()> &func, const task_attr &attr = {}) { func(); }
} // namespace ffrt

#endif // FFRT_STUB_H
