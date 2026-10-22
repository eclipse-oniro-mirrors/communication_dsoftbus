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

#ifndef SOFTBUS_TCP_CONNECT_MANAGER_TEST_COMMON_H
#define SOFTBUS_TCP_CONNECT_MANAGER_TEST_COMMON_H

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <pthread.h>
#include <securec.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common_list.h"
#include "conn_log.h"
#include "softbus_adapter_mem.h"
#include "softbus_base_listener.h"
#include "softbus_conn_interface.h"
#include "softbus_conn_manager.h"
#include "softbus_def.h"
#include "softbus_error_code.h"
#include "softbus_feature_config.h"
#include "softbus_tcp_connect_manager.h"
#include "softbus_tcp_socket.h"
#include "softbus_utils.h"

#define CLIENTPORT 6666
#define SERVERPORT 6667
#define INVALID_FD (-100)
#define KEEPALIVE_IDLE 100
#define KEEPALIVE_IDLE_MAX 65535
#define KEEPALIVE_INTERVAL 2
#define KEEPALIVE_COUNT 5

namespace OHOS {
extern const char *Ip;
extern const char *Ipv6;
extern const char *g_data;

extern uint32_t g_connectionId;
extern ConnectFuncInterface *g_interface;
extern ConnectResult g_result;
extern ConnectCallback g_cb;
extern int32_t g_receivedDatalength;
extern int32_t g_connServerInit;

void TcpOnConnected(uint32_t connectionId, const ConnectionInfo *info);
void TcpOnDisConnect(uint32_t connectionId, const ConnectionInfo *info);
void TcpDataReceived(uint32_t connectionId, ConnModule moduleId, int64_t seq, char *data, int32_t length);
void TcpOnConnectionSuccessed(uint32_t requestId, uint32_t connectionId, const ConnectionInfo *info);
void TcpOnConnectionFailed(uint32_t requestId, int32_t reason);

class TcpManagerTest : public testing::Test {
public:
    TcpManagerTest()
    {}
    ~TcpManagerTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
} // namespace OHOS

#endif // SOFTBUS_TCP_CONNECT_MANAGER_TEST_COMMON_H
