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

#include "softbus_tcp_connect_manager_test_common.h"

using namespace testing::ext;

namespace OHOS {

/*
* @tc.name: testConnInitTcp001
* @tc.desc: Test ConnInitTcp with NULL callback
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NULL
* @tc.type: FUNC
* @tc.require: The ConnInitTcp operates normally.
*/
HWTEST_F(TcpManagerTest, testConnInitTcp001, TestSize.Level1)
{
    ConnectFuncInterface *ret = ConnInitTcp(nullptr);
    EXPECT_EQ(ret, nullptr);
}

/*
* @tc.name: testConnInitTcp002
* @tc.desc: Test ConnInitTcp with valid callback
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonNull
* @tc.type: FUNC
* @tc.require: The ConnInitTcp operates normally.
*/
HWTEST_F(TcpManagerTest, testConnInitTcp002, TestSize.Level1)
{
    ConnectCallback cb;
    cb.OnConnected = TcpOnConnected;
    cb.OnDataReceived = TcpDataReceived;
    cb.OnDisconnected = TcpOnDisConnect;
    ConnectFuncInterface *ret = ConnInitTcp(&cb);
    EXPECT_NE(ret, nullptr);
    EXPECT_NE(ret->ConnectDevice, nullptr);
    EXPECT_NE(ret->DisconnectDevice, nullptr);
    EXPECT_NE(ret->DisconnectDeviceNow, nullptr);
    EXPECT_NE(ret->PostBytes, nullptr);
    EXPECT_NE(ret->GetConnectionInfo, nullptr);
    EXPECT_NE(ret->StartLocalListening, nullptr);
    EXPECT_NE(ret->StopLocalListening, nullptr);
    EXPECT_NE(ret->CheckActiveConnection, nullptr);
    EXPECT_EQ(ret->UpdateConnection, nullptr);
    EXPECT_EQ(ret->PreventConnection, nullptr);
}

/*
* @tc.name: testConnInitTcp003
* @tc.desc: Test ConnInitTcp multiple times to verify re-init
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonNull
* @tc.type: FUNC
* @tc.require: The ConnInitTcp operates normally.
*/
HWTEST_F(TcpManagerTest, testConnInitTcp003, TestSize.Level1)
{
    ConnectCallback cb;
    cb.OnConnected = TcpOnConnected;
    cb.OnDataReceived = TcpDataReceived;
    cb.OnDisconnected = TcpOnDisConnect;
    ConnectFuncInterface *ret1 = ConnInitTcp(&cb);
    EXPECT_NE(ret1, nullptr);
    ConnectFuncInterface *ret2 = ConnInitTcp(&cb);
    EXPECT_NE(ret2, nullptr);
    EXPECT_EQ(ret1, ret2);
}

/*
* @tc.name: testCalTcpConnectionId001
* @tc.desc: Test CalTcpConnectionId with positive fd
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The CalTcpConnectionId operates normally.
*/
HWTEST_F(TcpManagerTest, testCalTcpConnectionId001, TestSize.Level1)
{
    uint32_t connId1 = CalTcpConnectionId(1);
    EXPECT_NE(connId1, 0);
    uint32_t connId2 = CalTcpConnectionId(2);
    EXPECT_NE(connId2, 0);
    EXPECT_NE(connId1, connId2);
}

/*
* @tc.name: testCalTcpConnectionId002
* @tc.desc: Test CalTcpConnectionId with zero fd
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The CalTcpConnectionId operates normally.
*/
HWTEST_F(TcpManagerTest, testCalTcpConnectionId002, TestSize.Level1)
{
    uint32_t connId = CalTcpConnectionId(0);
    EXPECT_NE(connId, 0);
}

/*
* @tc.name: testCalTcpConnectionId003
* @tc.desc: Test CalTcpConnectionId with large fd value
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The CalTcpConnectionId operates normally.
*/
HWTEST_F(TcpManagerTest, testCalTcpConnectionId003, TestSize.Level1)
{
    uint32_t connId = CalTcpConnectionId(65535);
    EXPECT_NE(connId, 0);
    uint32_t connId2 = CalTcpConnectionId(65534);
    EXPECT_NE(connId2, 0);
    EXPECT_NE(connId, connId2);
}

/*
* @tc.name: testCalTcpConnectionId004
* @tc.desc: Test CalTcpConnectionId sequential calls produce increasing seq
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The CalTcpConnectionId operates normally.
*/
HWTEST_F(TcpManagerTest, testCalTcpConnectionId004, TestSize.Level1)
{
    uint32_t connId1 = CalTcpConnectionId(10);
    uint32_t connId2 = CalTcpConnectionId(10);
    EXPECT_NE(connId1, connId2);
}

/*
* @tc.name: testCalTcpConnectionId005
* @tc.desc: Test CalTcpConnectionId with negative fd
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The CalTcpConnectionId operates normally.
*/
HWTEST_F(TcpManagerTest, testCalTcpConnectionId005, TestSize.Level1)
{
    uint32_t connId = CalTcpConnectionId(-1);
    EXPECT_NE(connId, 0);
}

/*
* @tc.name: testTcpConnSetKeepalive001
* @tc.desc: Test TcpConnSetKeepalive with invalid fd
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnSetKeepalive operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnSetKeepalive001, TestSize.Level1)
{
    int32_t ret = TcpConnSetKeepalive(-1, true);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
    ret = TcpConnSetKeepalive(-1, false);
    EXPECT_EQ(ret, SOFTBUS_INVALID_PARAM);
}

/*
* @tc.name: testTcpConnSetKeepalive002
* @tc.desc: Test TcpConnSetKeepalive with needKeepalive=true and valid fd
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnSetKeepalive operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnSetKeepalive002, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t fd = tcp->OpenServerSocket(&info);
    ASSERT_TRUE(fd > 0);
    int32_t ret = TcpConnSetKeepalive(fd, true);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ConnCloseSocket(fd);
}

/*
* @tc.name: testTcpConnSetKeepalive003
* @tc.desc: Test TcpConnSetKeepalive with needKeepalive=false and valid fd
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnSetKeepalive operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnSetKeepalive003, TestSize.Level1)
{
    const SocketInterface *tcp = GetTcpProtocol();
    ASSERT_NE(tcp, nullptr);
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t fd = tcp->OpenServerSocket(&info);
    ASSERT_TRUE(fd > 0);
    int32_t ret = TcpConnSetKeepalive(fd, false);
    EXPECT_EQ(ret, SOFTBUS_OK);
    ConnCloseSocket(fd);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg001
* @tc.desc: Test TcpConnectDevice with result OnConnectFailed null
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    ConnectResult result;
    result.OnConnectSuccessed = TcpOnConnectionSuccessed;
    result.OnConnectFailed = nullptr;
    int32_t ret = TcpConnectDevice(&option, requestId, &result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg002
* @tc.desc: Test TcpConnectDevice with result OnConnectSuccessed null
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    ConnectResult result;
    result.OnConnectSuccessed = nullptr;
    result.OnConnectFailed = TcpOnConnectionFailed;
    int32_t ret = TcpConnectDevice(&option, requestId, &result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg003
* @tc.desc: Test TcpConnectDevice with both result callbacks null
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg003, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    ConnectResult result = {};
    int32_t ret = TcpConnectDevice(&option, requestId, &result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg004
* @tc.desc: Test TcpConnectDevice with CONNECT_BR type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg004, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_BR;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg005
* @tc.desc: Test TcpConnectDevice with CONNECT_HML type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg005, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_HML;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceKeepAlive001
* @tc.desc: Test TcpConnectDevice with keepAlive=1
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally with keepAlive.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceKeepAlive001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.keepAlive = 1;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceKeepAlive002
* @tc.desc: Test TcpConnectDevice with keepAlive=0
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally without keepAlive.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceKeepAlive002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.keepAlive = 0;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceKeepAlive003
* @tc.desc: Test TcpConnectDevice with keepAlive=1 and AUTH_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally with keepAlive and AUTH_P2P.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceKeepAlive003, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = AUTH_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.keepAlive = 1;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceInvalidAddr001
* @tc.desc: Test TcpConnectDevice with invalid address
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceInvalidAddr001, TestSize.Level1)
{
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = 9999;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), "0.0.0.0");
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_TRUE(ret != SOFTBUS_OK);
}

/*
* @tc.name: testTcpConnectDeviceInvalidAddr002
* @tc.desc: Test TcpConnectDevice with unreachable address
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceInvalidAddr002, TestSize.Level1)
{
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = 9999;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), "192.0.2.1");
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_TRUE(ret != SOFTBUS_OK);
}

/*
* @tc.name: testTcpPostBytes001
* @tc.desc: Test TcpPostBytes with null data
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpPostBytes operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpPostBytes001, TestSize.Level1)
{
    int32_t ret = TcpPostBytes(1, nullptr, 10, 0, 0, 0, 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpPostBytes002
* @tc.desc: Test TcpPostBytes with len=0
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpPostBytes operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpPostBytes002, TestSize.Level1)
{
    char *data = static_cast<char *>(SoftBusCalloc(10));
    ASSERT_NE(data, nullptr);
    int32_t ret = TcpPostBytes(1, reinterpret_cast<uint8_t*>(data), 0, 0, 0, 0, 0);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    SoftBusFree(data);
}

/*
* @tc.name: testTcpPostBytes003
* @tc.desc: Test TcpPostBytes with invalid connectionId
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpPostBytes operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpPostBytes003, TestSize.Level1)
{
    char *data = static_cast<char*>(SoftBusCalloc(10));
    ASSERT_NE(data, nullptr);
    (void)memset_s(data, 10, 0x1, 10);
    int32_t ret = TcpPostBytes(0xFFFF,  reinterpret_cast<uint8_t*>(data), 10, 0, 0, 0, 0);
    EXPECT_TRUE(ret != SOFTBUS_OK);
}

/*
* @tc.name: testTcpPostBytes004
* @tc.desc: Test TcpPostBytes with valid connection after disconnect
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpPostBytes operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpPostBytes004, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);
    uint32_t savedConnId = g_connectionId;
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);

    char *data = static_cast<char *>(SoftBusCalloc(10));
    ASSERT_NE(data, nullptr);
    (void)memset_s(data, 10, 0x1, 10);
    int32_t ret = TcpPostBytes(savedConnId, reinterpret_cast<uint8_t*>(data), 10, 0, 0, 0, 0);
    EXPECT_TRUE(ret != SOFTBUS_OK);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpGetConnectionInfo001
* @tc.desc: Test TcpGetConnectionInfo with valid connectionId after connect
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpGetConnectionInfo operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpGetConnectionInfo001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    ConnectionInfo connInfo = {};
    int32_t ret = TcpGetConnectionInfo(g_connectionId, &connInfo);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(true, connInfo.isAvailable);
    EXPECT_EQ(false, connInfo.isServer);
    EXPECT_EQ(CONNECT_TCP, connInfo.type);
    EXPECT_EQ(port, connInfo.socketInfo.port);
    EXPECT_EQ(PROXY, connInfo.socketInfo.moduleId);

    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpGetConnectionInfo002
* @tc.desc: Test TcpGetConnectionInfo after disconnect
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpGetConnectionInfo operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpGetConnectionInfo002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);
    uint32_t savedConnId = g_connectionId;
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);

    ConnectionInfo connInfo = {};
    int32_t ret = TcpGetConnectionInfo(savedConnId, &connInfo);
    EXPECT_EQ(SOFTBUS_TCPCONNECTION_SOCKET_ERR, ret);
    EXPECT_EQ(false, connInfo.isAvailable);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpGetConnectionInfo003
* @tc.desc: Test TcpGetConnectionInfo with zero connectionId
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpGetConnectionInfo operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpGetConnectionInfo003, TestSize.Level1)
{
    ConnectionInfo connInfo = {};
    int32_t ret = TcpGetConnectionInfo(0, &connInfo);
    EXPECT_EQ(SOFTBUS_TCPCONNECTION_SOCKET_ERR, ret);
    EXPECT_EQ(false, connInfo.isAvailable);
}

/*
* @tc.name: testTcpStartListening001
* @tc.desc: Test TcpStartListening with CONNECT_HML type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_HML;
    info.socketOption.port = port;
    info.socketOption.moduleId = PROXY;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening002
* @tc.desc: Test TcpStartListening with CONNECT_P2P type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = PROXY;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening003
* @tc.desc: Test TcpStartListening with CONNECT_HML type and AUTH module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening003, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_HML;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening004
* @tc.desc: Test TcpStartListening with CONNECT_HML type and AUTH_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening004, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_HML;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening005
* @tc.desc: Test TcpStartListening with CONNECT_HML type and DIRECT_CHANNEL_SERVER_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening005, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_HML;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening006
* @tc.desc: Test TcpStartListening with CONNECT_HML type and DIRECT_CHANNEL_CLIENT module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening006, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_HML;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening007
* @tc.desc: Test TcpStartListening with CONNECT_HML type and DIRECT_CHANNEL_SERVER_WIFI module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening007, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_HML;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening008
* @tc.desc: Test TcpStartListening with CONNECT_P2P type and AUTH module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening008, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening009
* @tc.desc: Test TcpStartListening with CONNECT_P2P type and AUTH_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening009, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening010
* @tc.desc: Test TcpStartListening with CONNECT_P2P type and DIRECT_CHANNEL_SERVER_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening010, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening011
* @tc.desc: Test TcpStartListening with CONNECT_P2P type and DIRECT_CHANNEL_CLIENT module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening011, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpStartListening012
* @tc.desc: Test TcpStartListening with CONNECT_P2P type and DIRECT_CHANNEL_SERVER_WIFI module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpStartListening operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpStartListening012, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_P2P;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);
    int32_t ret = TcpStartListening(&info);
    EXPECT_EQ(port, ret);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpDisconnectDeviceNow002
* @tc.desc: Test TcpDisconnectDeviceNow with actual matching connections
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpDisconnectDeviceNow operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpDisconnectDeviceNow002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    int32_t ret = TcpDisconnectDeviceNow(&option);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpDisconnectDeviceNow003
* @tc.desc: Test TcpDisconnectDeviceNow with no matching connections
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpDisconnectDeviceNow operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpDisconnectDeviceNow003, TestSize.Level1)
{
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = 9999;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), "192.168.1.1");
    int32_t ret = TcpDisconnectDeviceNow(&option);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
* @tc.name: testTcpDisconnectDeviceNow004
* @tc.desc: Test TcpDisconnectDeviceNow with different protocol
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpDisconnectDeviceNow operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpDisconnectDeviceNow004, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    ConnectOption diffOption = {};
    diffOption.type = CONNECT_TCP;
    diffOption.socketOption.port = port;
    diffOption.socketOption.moduleId = PROXY;
    diffOption.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(diffOption.socketOption.addr, sizeof(diffOption.socketOption.addr), "192.168.1.1");
    int32_t ret = TcpDisconnectDeviceNow(&diffOption);
    EXPECT_EQ(SOFTBUS_OK, ret);

    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceAuth001
* @tc.desc: Test TcpConnectDevice with CONNECT_TCP type and AUTH module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceAuth001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = AUTH;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceAuthP2P001
* @tc.desc: Test TcpConnectDevice with CONNECT_TCP type and AUTH_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceAuthP2P001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = AUTH_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceDirectChannel001
* @tc.desc: Test TcpConnectDevice with CONNECT_TCP type and DIRECT_CHANNEL_SERVER_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceDirectChannel001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceDirectChannel002
* @tc.desc: Test TcpConnectDevice with CONNECT_TCP type and DIRECT_CHANNEL_CLIENT module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceDirectChannel002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceDirectChannel003
* @tc.desc: Test TcpConnectDevice with CONNECT_TCP type and DIRECT_CHANNEL_SERVER_WIFI module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceDirectChannel003, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceMultiple001
* @tc.desc: Test multiple connect and disconnect cycles
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceMultiple001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    for (int32_t i = 0; i < 3; i++) {
        g_connectionId = 0;
        EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
        sleep(1);
        EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
        sleep(1);
    }
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6KeepAlive001
* @tc.desc: Test connect with IPv6 and keepAlive=1
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally with IPv6 and keepAlive.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6KeepAlive001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.keepAlive = 1;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6KeepAlive002
* @tc.desc: Test connect with IPv6 and keepAlive=0
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally with IPv6.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6KeepAlive002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.keepAlive = 0;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6Auth001
* @tc.desc: Test connect with IPv6 and AUTH module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6Auth001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = AUTH;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6AuthP2P001
* @tc.desc: Test connect with IPv6 and AUTH_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6AuthP2P001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = AUTH_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6DirectChannel001
* @tc.desc: Test connect with IPv6 and DIRECT_CHANNEL_SERVER_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6DirectChannel001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6DirectChannel002
* @tc.desc: Test connect with IPv6 and DIRECT_CHANNEL_CLIENT module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6DirectChannel002, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_CLIENT;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceIpv6DirectChannel003
* @tc.desc: Test connect with IPv6 and DIRECT_CHANNEL_SERVER_WIFI module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceIpv6DirectChannel003, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = DIRECT_CHANNEL_SERVER_WIFI;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpGetConnectionInfoIpv6001
* @tc.desc: Test TcpGetConnectionInfo with valid IPv6 connection
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpGetConnectionInfo operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpGetConnectionInfoIpv6001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    ConnectionInfo connInfo = {};
    int32_t ret = TcpGetConnectionInfo(g_connectionId, &connInfo);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(true, connInfo.isAvailable);
    EXPECT_EQ(false, connInfo.isServer);
    EXPECT_EQ(CONNECT_TCP, connInfo.type);

    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpDisconnectDeviceNowIpv6001
* @tc.desc: Test TcpDisconnectDeviceNow with IPv6 connection
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpDisconnectDeviceNow operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpDisconnectDeviceNowIpv6001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    int32_t ret = TcpDisconnectDeviceNow(&option);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceCheckArg006
* @tc.desc: Test TcpConnectDevice with CONNECT_SLE type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg006, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_SLE;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg007
* @tc.desc: Test TcpConnectDevice with CONNECT_P2P_REUSE type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg007, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_P2P_REUSE;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg008
* @tc.desc: Test TcpConnectDevice with CONNECT_BLE_DIRECT type
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg008, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_BLE_DIRECT;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg009
* @tc.desc: Test TcpConnectDevice with null option
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg009, TestSize.Level1)
{
    uint32_t requestId = 1;
    int32_t ret = TcpConnectDevice(nullptr, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpConnectDeviceCheckArg010
* @tc.desc: Test TcpConnectDevice with null result
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpConnectDeviceCheckArg operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceCheckArg010, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);
    int32_t ret = TcpConnectDevice(&option, requestId, nullptr);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
}

/*
* @tc.name: testTcpPostBytes005
* @tc.desc: Test TcpPostBytes with valid connection
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpPostBytes operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpPostBytes005, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    char *data = static_cast<char *>(SoftBusCalloc(10));
    ASSERT_NE(data, nullptr);
    (void)memset_s(data, 10, 0x1, 10);
    int32_t ret = TcpPostBytes(g_connectionId, reinterpret_cast<uint8_t*>(data), 10, 0, 0, 0, 0);
    EXPECT_TRUE(ret == SOFTBUS_OK || ret == SOFTBUS_TCPCONNECTION_SOCKET_ERR);

    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpPostBytes006
* @tc.desc: Test TcpPostBytes with large data length
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: NonZero
* @tc.type: FUNC
* @tc.require: The TcpPostBytes operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpPostBytes006, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    EXPECT_EQ(SOFTBUS_OK, TcpConnectDevice(&option, requestId, &g_result));
    sleep(1);

    uint32_t largeLen = 1024;
    char *data = static_cast<char *>(SoftBusCalloc(largeLen));
    ASSERT_NE(data, nullptr);
    (void)memset_s(data, largeLen, 0x1, largeLen);
    int32_t ret = TcpPostBytes(g_connectionId, reinterpret_cast<uint8_t*>(data), largeLen, 0, 0, 0, 0);
    EXPECT_TRUE(ret == SOFTBUS_OK || ret == SOFTBUS_TCPCONNECTION_SOCKET_ERR);

    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceLargeRequestId001
* @tc.desc: Test TcpConnectDevice with large requestId
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceLargeRequestId001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ip);

    uint32_t requestId = UINT32_MAX;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = PROXY;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ip);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}

/*
* @tc.name: testTcpConnectDeviceKeepAliveIpv6AuthP2P001
* @tc.desc: Test connect with IPv6, keepAlive=1, AUTH_P2P module
* @tc.in: Test module, Test number, Test Levels.
* @tc.out: Zero
* @tc.type: FUNC
* @tc.require: The TcpConnectDevice operates normally.
*/
HWTEST_F(TcpManagerTest, testTcpConnectDeviceKeepAliveIpv6AuthP2P001, TestSize.Level1)
{
    int32_t port = CLIENTPORT;
    LocalListenerInfo info = {};
    info.type = CONNECT_TCP;
    info.socketOption.port = port;
    info.socketOption.moduleId = AUTH_P2P;
    info.socketOption.protocol = LNN_PROTOCOL_IP;
    (void)strcpy_s(info.socketOption.addr, sizeof(info.socketOption.addr), Ipv6);

    uint32_t requestId = 1;
    ConnectOption option = {};
    option.type = CONNECT_TCP;
    option.socketOption.port = port;
    option.socketOption.moduleId = AUTH_P2P;
    option.socketOption.protocol = LNN_PROTOCOL_IP;
    option.socketOption.keepAlive = 1;
    (void)strcpy_s(option.socketOption.addr, sizeof(option.socketOption.addr), Ipv6);

    EXPECT_EQ(port, TcpStartListening(&info));
    int32_t ret = TcpConnectDevice(&option, requestId, &g_result);
    EXPECT_EQ(SOFTBUS_OK, ret);
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpDisconnectDevice(g_connectionId));
    sleep(1);
    EXPECT_EQ(SOFTBUS_OK, TcpStopListening(&info));
}
} // namespace OHOS
