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

#include "bluetooth_mock.h"
#include "c_header/ohos_bt_def.h"
#include "c_header/ohos_bt_gatt_server.h"
#include "conn_log.h"
#include "softbus_adapter_ble_gatt_server.h"
#include "softbus_error_code.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "assert_helper.h"

using namespace testing::ext;
using ::testing::Return;

#define MOCK_GATT_SERVER_HANDLE     0
#define MOCK_GATT_SERVICE_HANDLE    1
#define MOCK_GATT_CHARA_HANDLE      2
#define MOCK_GATT_DESCRIPTOR_HANDLE 3
#define MOCK_GATT_INCLUDE_SERVICE_HANDLE 4
#define SOFTBUS_TEST_MTU_SIZE 512

namespace OHOS {

class BtUuidRecordCtx : public StRecordCtx {
public:
    explicit BtUuidRecordCtx(const char *identifier);
    ~BtUuidRecordCtx();
    bool Update(int32_t id, int32_t st, SoftBusBtUuid *param);
    testing::AssertionResult Expect(int32_t id, int32_t st, SoftBusBtUuid *param);

private:
    SoftBusBtUuid uuid;
    void Reset();
};

class BtGattRecordCtx : public BtUuidRecordCtx {
public:
    explicit BtGattRecordCtx(const char *identifier);
    bool Update(int32_t id, int32_t st, int32_t handle, SoftBusBtUuid *param);
    testing::AssertionResult Expect(int32_t id, int32_t st, int32_t handle, SoftBusBtUuid *param);

private:
    int32_t handle;
};

class AdapterBleGattServerTest : public testing::Test {
public:
    static BtGattServerCallbacks *gattServerCallback;
    static BtUuidRecordCtx serviceAddCtx;
    static BtGattRecordCtx characteristicAddCtx;
    static BtGattRecordCtx descriptorAddCtx;
    static StRecordCtx serviceStartCtx;
    static StRecordCtx serviceStopCtx;
    static StRecordCtx serviceDeleteCtx;
    static BtAddrRecordCtx connectServerCtx;
    static BtAddrRecordCtx disconnectServerCtx;
    static SoftBusGattReadRequest requestReadCtx;
    static SoftBusGattWriteRequest requestWriteCtx;
    static StRecordCtx responseConfirmationCtx;
    static StRecordCtx notifySentCtx;
    static StRecordCtx mtuChangeCtx;
    static StRecordCtx isConcernedAttrHandleCtx;
};

static SoftBusGattsCallback *GetStubGattsCallback();
static testing::AssertionResult ExpectGattReadRequest(SoftBusGattReadRequest actual, SoftBusGattReadRequest want);
static testing::AssertionResult ExpectGattWriteRequest(SoftBusGattWriteRequest actual, SoftBusGattWriteRequest want);

int32_t ActionBleGattsRegisterCallbacks(BtGattServerCallbacks *func)
{
    AdapterBleGattServerTest::gattServerCallback = func;
    return OHOS_BT_STATUS_SUCCESS;
}

// 回绕到注册通知中
int32_t ActionBleGattsRegister(BtUuid appUuid)
{
    AdapterBleGattServerTest::gattServerCallback->registerServerCb(0, MOCK_GATT_SERVER_HANDLE, &appUuid);
    return OHOS_BT_STATUS_SUCCESS;
}

// 回绕到注册通知中
int32_t ActionBleGattsRegisterErrNull(BtUuid appUuid)
{
    appUuid.uuid = nullptr;
    AdapterBleGattServerTest::gattServerCallback->registerServerCb(0, MOCK_GATT_SERVER_HANDLE, &appUuid);
    return OHOS_BT_STATUS_SUCCESS;
}

// 回绕到注册通知中
int32_t ActionBleGattsRegisterErrLen(BtUuid appUuid)
{
    appUuid.uuid = (char *)"abc";
    AdapterBleGattServerTest::gattServerCallback->registerServerCb(0, MOCK_GATT_SERVER_HANDLE, &appUuid);
    return OHOS_BT_STATUS_SUCCESS;
}

// 回绕到注册通知中
int32_t ActionBleGattsRegisterErrStatus(BtUuid appUuid)
{
    AdapterBleGattServerTest::gattServerCallback->registerServerCb(-1, MOCK_GATT_SERVER_HANDLE, &appUuid);
    return OHOS_BT_STATUS_SUCCESS;
}

static void MockAll(MockBluetooth &mocker)
{
    EXPECT_CALL(mocker, BleGattsRegisterCallbacks).WillRepeatedly(ActionBleGattsRegisterCallbacks);
    EXPECT_CALL(mocker, BleGattsRegister).WillRepeatedly(ActionBleGattsRegister);
    EXPECT_CALL(mocker, BleGattsAddService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsUnRegister).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsAddCharacteristic).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsAddDescriptor).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsStartService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsStopService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsDeleteService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsConnect).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsDisconnect).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsSendResponse).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    EXPECT_CALL(mocker, BleGattsSendIndication).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusRegisterGattsCallbacks001
 * @tc.desc: test register gatt server callbacks
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusRegisterGattsCallbacks001, TestSize.Level3)
{
    InitSoftbusAdapterServer();
    MockBluetooth mocker;
    MockAll(mocker);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };

    ASSERT_EQ(SoftBusRegisterGattsCallbacks(nullptr, service), SOFTBUS_BLECONNECTION_REG_GATTS_CALLBACK_FAIL)
        << "nullptr gatts callback scenor";
    // 清空状态，允许重入
    SoftBusUnRegisterGattsCallbacks(service);
    EXPECT_CALL(mocker, BleGattsRegisterCallbacks).WillRepeatedly(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(
        SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_BLECONNECTION_REG_GATTS_CALLBACK_FAIL)
        << "BleGattsRegisterCallbacks fail scenor";

    EXPECT_CALL(mocker, BleGattsRegisterCallbacks).WillRepeatedly(ActionBleGattsRegisterCallbacks);
    EXPECT_CALL(mocker, BleGattsRegister).WillRepeatedly(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(
        SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_BLECONNECTION_REG_GATTS_CALLBACK_FAIL)
        << "BleGattsRegister fail scenor";

    EXPECT_CALL(mocker, BleGattsRegister).WillRepeatedly(ActionBleGattsRegister);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsUnRegister).WillRepeatedly(Return(SOFTBUS_INVALID_PARAM));
    SoftBusUnRegisterGattsCallbacks(service);

     // 清空状态
    EXPECT_CALL(mocker, BleGattsUnRegister).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusUnRegisterGattsCallbacks001
 * @tc.desc: test unregister gatt server callbacks
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusUnRegisterGattsCallbacks001, TestSize.Level3)
{
    InitSoftbusAdapterServer();
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    EXPECT_CALL(mocker, BleGattsRegister).WillOnce(ActionBleGattsRegisterErrNull);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);

    EXPECT_CALL(mocker, BleGattsRegister).WillRepeatedly(ActionBleGattsRegisterErrLen);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);

    EXPECT_CALL(mocker, BleGattsRegister).WillRepeatedly(ActionBleGattsRegisterErrStatus);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);

    EXPECT_CALL(mocker, BleGattsRegister).WillRepeatedly(ActionBleGattsRegister);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsAddService001
 * @tc.desc: test add gatt service
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsAddService001, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service1 = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    ASSERT_EQ(SoftBusGattsAddService(service1, true, 1), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);

    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service1), SOFTBUS_OK);

    // 注册service
    SoftBusBtUuid service = {
        .uuidLen = 0,
        .uuid = nullptr,
    };
    ASSERT_EQ(SoftBusGattsAddService(service, true, 1), SOFTBUS_INVALID_PARAM);

    EXPECT_CALL(mocker, BleGattsAddService).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    service.uuid = (char *)serviceUuid;
    service.uuidLen = strlen(serviceUuid);
    ASSERT_EQ(SoftBusGattsAddService(service, true, 1), SOFTBUS_CONN_BLE_UNDERLAY_SERVER_ADD_SERVICE_ERR);

    EXPECT_CALL(mocker, BleGattsAddService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsAddService(service, true, 1), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsAddCharacteristic001
 * @tc.desc: test add gatt characteristic
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsAddCharacteristic001, TestSize.Level3)
{
    SoftBusBtUuid characteristic = {
        .uuidLen = 0,
        .uuid = nullptr,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    int32_t properties = SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_READ | SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP |
        SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_WRITE | SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY |
        SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_INDICATE;
    int32_t permissions = SOFTBUS_GATT_PERMISSION_READ | SOFTBUS_GATT_PERMISSION_WRITE;
    ASSERT_EQ(SoftBusGattsAddCharacteristic(MOCK_GATT_SERVICE_HANDLE, characteristic, properties, permissions),
        SOFTBUS_INVALID_PARAM);

    const char *netCharacteristic = "00002B00-0000-1000-8000-00805F9B34FB";
    characteristic.uuid = (char *)netCharacteristic;
    characteristic.uuidLen = strlen(netCharacteristic);
    ASSERT_EQ(SoftBusGattsAddCharacteristic(MOCK_GATT_SERVICE_HANDLE, characteristic, properties, permissions),
        SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);

    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };

    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    EXPECT_CALL(mocker, BleGattsAddCharacteristic).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsAddCharacteristic(MOCK_GATT_SERVICE_HANDLE, characteristic, properties, permissions),
        SOFTBUS_CONN_BLE_UNDERLAY_CHARACTERISTIC_ADD_ERR);

    EXPECT_CALL(mocker, BleGattsAddCharacteristic).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(
        SoftBusGattsAddCharacteristic(MOCK_GATT_SERVICE_HANDLE, characteristic, properties, permissions), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsAddDescriptor001
 * @tc.desc: test add gatt descriptor
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsAddDescriptor001, TestSize.Level3)
{
    SoftBusBtUuid desciptor = {
        .uuidLen = 0,
        .uuid = nullptr,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    int32_t permissions = SOFTBUS_GATT_PERMISSION_READ | SOFTBUS_GATT_PERMISSION_WRITE;
    ASSERT_EQ(SoftBusGattsAddDescriptor(MOCK_GATT_SERVICE_HANDLE, desciptor, permissions), SOFTBUS_INVALID_PARAM);

    const char *connDesciptor = "00002902-0000-1000-8000-00805F9B34FB";
    desciptor.uuid = (char *)connDesciptor;
    desciptor.uuidLen = strlen(connDesciptor);
    ASSERT_EQ(SoftBusGattsAddDescriptor(MOCK_GATT_SERVICE_HANDLE,
        desciptor, permissions), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);

    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };

    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsAddDescriptor).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsAddDescriptor(MOCK_GATT_SERVICE_HANDLE, desciptor, permissions),
        SOFTBUS_CONN_BLE_UNDERLAY_DESCRIPTOR_ADD_ERR);
    EXPECT_CALL(mocker, BleGattsAddDescriptor).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsAddDescriptor(MOCK_GATT_SERVICE_HANDLE, desciptor, permissions), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsStartService001
 * @tc.desc: test start gatt service
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsStartService001, TestSize.Level3)
{
    ASSERT_EQ(SoftBusGattsStartService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsStartService).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsStartService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_CONN_BLE_UNDERLAY_SERVICE_START_ERR);

    EXPECT_CALL(mocker, BleGattsStartService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsStartService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsStopService001
 * @tc.desc: test stop gatt service
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsStopService001, TestSize.Level3)
{
    ASSERT_EQ(SoftBusGattsStopService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsStopService).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsStopService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_CONN_BLE_UNDERLAY_SERVICE_STOP_ERR);

    EXPECT_CALL(mocker, BleGattsStopService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsStopService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsDeleteService001
 * @tc.desc: test delete gatt service
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsDeleteService001, TestSize.Level3)
{
    ASSERT_EQ(SoftBusGattsDeleteService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsDeleteService).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsDeleteService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_CONN_BLE_UNDERLAY_SERVICE_DELETE_ERR);

    EXPECT_CALL(mocker, BleGattsDeleteService).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsDeleteService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

 /**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsConnect001
 * @tc.desc: test connect gatt connection
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsConnect001, TestSize.Level3)
{
    SoftBusBtAddr addr = {
        .addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
    };
    ASSERT_EQ(SoftBusGattsConnect(addr), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    EXPECT_CALL(mocker, BleGattsConnect).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsConnect(addr), SOFTBUS_CONN_BLE_UNDERLAY_SERVER_CONNECT_ERR);

    EXPECT_CALL(mocker, BleGattsConnect).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsConnect(addr), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsDisconnect001
 * @tc.desc: test disconnect gatt connection
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsDisconnect001, TestSize.Level3)
{
    int32_t connId = 1;
    SoftBusBtAddr addr = {
        .addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
    };
    ASSERT_EQ(SoftBusGattsDisconnect(addr, connId), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsDisconnect).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsDisconnect(addr, connId), SOFTBUS_CONN_BLE_UNDERLAY_SERVER_DISCONNECT_ERR);

    EXPECT_CALL(mocker, BleGattsDisconnect).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsDisconnect(addr, connId), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsSendResponse001
 * @tc.desc: test send gatt response
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsSendResponse001, TestSize.Level3)
{
    ASSERT_EQ(SoftBusGattsSendResponse(nullptr), SOFTBUS_INVALID_PARAM);
    SoftBusGattsResponse resp = { 0 };
    ASSERT_EQ(SoftBusGattsSendResponse(&resp), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsSendResponse).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsSendResponse(&resp), SOFTBUS_CONN_BLE_UNDERLAY_SERVER_SEND_RESPONSE_ERR);

    EXPECT_CALL(mocker, BleGattsSendResponse).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsSendResponse(&resp), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_SoftBusGattsSendNotify001
 * @tc.desc: test send gatt notify
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, SoftBusGattsSendNotify001, TestSize.Level3)
{
    ASSERT_EQ(SoftBusGattsSendNotify(nullptr), SOFTBUS_INVALID_PARAM);
    SoftBusGattsNotify notify = { 0 };
    ASSERT_EQ(SoftBusGattsSendNotify(&notify), SOFTBUS_CONN_BLE_CHECK_STATUS_ERR);
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    EXPECT_CALL(mocker, BleGattsSendIndication).Times(1).WillOnce(Return(OHOS_BT_STATUS_FAIL));
    ASSERT_EQ(SoftBusGattsSendNotify(&notify), SOFTBUS_CONN_BLE_UNDERLAY_SERVER_SEND_INDICATION_ERR);

    EXPECT_CALL(mocker, BleGattsSendIndication).WillRepeatedly(Return(OHOS_BT_STATUS_SUCCESS));
    ASSERT_EQ(SoftBusGattsSendNotify(&notify), SOFTBUS_OK);
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_BleConnectServerCallback001
 * @tc.desc: test gatt connect server callback
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, BleConnectServerCallback001, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    gattServerCallback->connectServerCb(1, MOCK_GATT_SERVER_HANDLE, nullptr);

    BdAddr bdAddr = {
        .addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
    };

    gattServerCallback->connectServerCb(1, 1,  &bdAddr);
    gattServerCallback->connectServerCb(1, MOCK_GATT_SERVER_HANDLE, &bdAddr);
    SoftBusBtAddr addr = {
        .addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
    };
    GetStubGattsCallback()->connectServerCallback(1, &addr);
    ASSERT_TRUE(connectServerCtx.Expect(1, &addr));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_BleDisconnectServerCallback001
 * @tc.desc: test gatt disconnect server callback
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, BleDisconnectServerCallback001, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    BdAddr bdAddr = {
        .addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
    };
    gattServerCallback->disconnectServerCb(1, 1,  &bdAddr);

    SoftBusBtAddr addr = {
        .addr = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
    };
    ASSERT_EQ(SoftBusGattsDisconnect(addr, 1), SOFTBUS_OK);
    gattServerCallback->disconnectServerCb(1, MOCK_GATT_SERVER_HANDLE, &bdAddr);

    GetStubGattsCallback()->disconnectServerCallback(1, &addr);
    ASSERT_TRUE(disconnectServerCtx.Expect(1, &addr));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_BleServiceAddCallback001
 * @tc.desc: test gatt server add callback
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, BleServiceAddCallback001, TestSize.Level3)
{
    // 注册service
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    gattServerCallback->serviceAddCb(
        OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, nullptr, MOCK_GATT_SERVICE_HANDLE);

    BtUuid btService = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    gattServerCallback->serviceAddCb(
        OHOS_BT_STATUS_SUCCESS, 1, &btService, MOCK_GATT_SERVICE_HANDLE);
    gattServerCallback->serviceAddCb(
        OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, &btService, MOCK_GATT_SERVICE_HANDLE);
    ASSERT_TRUE(serviceAddCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK, &service));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_BleDescriptorAddCallback001
 * @tc.desc: test descriptor add callback
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, BleDescriptorAddCallback001, TestSize.Level3)
{
    gattServerCallback->characteristicAddCb(OHOS_BT_STATUS_SUCCESS, SOFTBUS_INVALID_PARAM, nullptr,
        MOCK_GATT_SERVICE_HANDLE, MOCK_GATT_CHARA_HANDLE);

    // 注册service
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    gattServerCallback->includeServiceAddCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, MOCK_GATT_SERVICE_HANDLE,
        MOCK_GATT_INCLUDE_SERVICE_HANDLE);

    const char *netCharacteristic = "00002B00-0000-1000-8000-00805F9B34FB";
    BtUuid btCharacteristic = {
        .uuidLen = strlen(netCharacteristic),
        .uuid = (char *)netCharacteristic,
    };
    gattServerCallback->characteristicAddCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, &btCharacteristic,
        SOFTBUS_INVALID_PARAM, MOCK_GATT_CHARA_HANDLE);

    gattServerCallback->descriptorAddCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, nullptr,
        MOCK_GATT_SERVICE_HANDLE, MOCK_GATT_DESCRIPTOR_HANDLE);

    // 注册desciptor
    const char *connDesciptor = "00002902-0000-1000-8000-00805F9B34FB";
    BtUuid btDescriptor = {
        .uuidLen = strlen(connDesciptor),
        .uuid = (char *)connDesciptor,
    };
    gattServerCallback->descriptorAddCb(OHOS_BT_STATUS_SUCCESS, SOFTBUS_INVALID_PARAM, &btDescriptor,
        MOCK_GATT_SERVICE_HANDLE, MOCK_GATT_DESCRIPTOR_HANDLE);
    gattServerCallback->descriptorAddCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, &btDescriptor,
        SOFTBUS_INVALID_PARAM, MOCK_GATT_DESCRIPTOR_HANDLE);
    SoftBusBtUuid descriptor = {
        .uuidLen = strlen(connDesciptor),
        .uuid = (char *)connDesciptor,
    };
    ASSERT_FALSE(
        descriptorAddCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK, MOCK_GATT_DESCRIPTOR_HANDLE, &descriptor));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_BleRequestReadCallback001
 * @tc.desc: test ble request read callback
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, BleRequestReadCallback001, TestSize.Level3)
{
    gattServerCallback->serviceStartCb(OHOS_BT_STATUS_SUCCESS, SOFTBUS_INVALID_PARAM, MOCK_GATT_SERVICE_HANDLE);
    gattServerCallback->serviceStopCb(OHOS_BT_STATUS_SUCCESS, SOFTBUS_INVALID_PARAM, MOCK_GATT_SERVICE_HANDLE);
    gattServerCallback->serviceDeleteCb(OHOS_BT_STATUS_SUCCESS, SOFTBUS_INVALID_PARAM, MOCK_GATT_SERVICE_HANDLE);

    // 注册service
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    gattServerCallback->serviceStartCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, SOFTBUS_INVALID_PARAM);
    gattServerCallback->serviceStopCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, SOFTBUS_INVALID_PARAM);
    gattServerCallback->serviceDeleteCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, SOFTBUS_INVALID_PARAM);
    gattServerCallback->responseConfirmationCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE);
    gattServerCallback->indicationSentCb(1, OHOS_BT_STATUS_SUCCESS);

    // server建链
    BdAddr bdAddr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    gattServerCallback->connectServerCb(1, MOCK_GATT_SERVER_HANDLE, &bdAddr);
    BtReqReadCbPara btReadParam = {
        .connId = -1,
        .transId = 0,
        .bdAddr = &bdAddr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .isLong = false,
    };
    gattServerCallback->requestReadCb(btReadParam);
    gattServerCallback->indicationSentCb(1, OHOS_BT_STATUS_SUCCESS);
    SoftBusBtAddr addr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    SoftBusGattReadRequest readParam = {
        .connId = 1,
        .transId = 0,
        .btAddr = &addr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .isLong = false,
    };
    GetStubGattsCallback()->requestReadCallback(readParam);
    ASSERT_TRUE(ExpectGattReadRequest(requestReadCtx, readParam));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_BleRequestWriteCallback001
 * @tc.desc: test ble request write callback
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, BleRequestWriteCallback001, TestSize.Level3)
{
    // 注册service
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    // server建链
    BdAddr bdAddr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    gattServerCallback->connectServerCb(1, MOCK_GATT_SERVER_HANDLE, &bdAddr);
    SoftBusBtAddr addr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    GetStubGattsCallback()->connectServerCallback(1, &addr);
    ASSERT_TRUE(connectServerCtx.Expect(1, &addr));

    const char *valueExample = "hello gatt server, this is client";
    BtReqWriteCbPara btWriteParam = {
        .connId = -1,
        .transId = 0,
        .bdAddr = &bdAddr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .length = strlen(valueExample),
        .needRsp = true,
        .isPrep = false,
        .value = (unsigned char *)valueExample,
    };
    gattServerCallback->mtuChangeCb(-1, 0);
    gattServerCallback->mtuChangeCb(1, SOFTBUS_TEST_MTU_SIZE);
    gattServerCallback->requestWriteCb(btWriteParam);
    SoftBusGattWriteRequest writeParam = {
        .connId = 1,
        .transId = 0,
        .btAddr = &addr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .length = strlen(valueExample),
        .needRsp = true,
        .isPrep = false,
        .value = (unsigned char *)valueExample,
    };
    GetStubGattsCallback()->requestWriteCallback(writeParam);
    ASSERT_TRUE(ExpectGattWriteRequest(requestWriteCtx, writeParam));
    SoftBusUnRegisterGattsCallbacks(service);
}

/**
 * @tc.name: AdapterBleGattServerTest_GattServerLifeCycle001
 * @tc.desc: test gatt server complete life cyclel, from a real usage perspective, important
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, GattServerLifeCycle001, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    ASSERT_EQ(SoftBusGattsAddService(service, true, 8), SOFTBUS_OK);
    BtUuid btService = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    gattServerCallback->serviceAddCb(
        OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, &btService, MOCK_GATT_SERVICE_HANDLE);
    ASSERT_TRUE(serviceAddCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK, &service));

    // 注册charateristic
    const char *netCharacteristic = "00002B00-0000-1000-8000-00805F9B34FB";
    SoftBusBtUuid characteristic = {
        .uuidLen = strlen(netCharacteristic),
        .uuid = (char *)netCharacteristic,
    };
    int32_t properties = SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_READ | SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP |
        SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_WRITE | SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY |
        SOFTBUS_GATT_CHARACTER_PROPERTY_BIT_INDICATE;
    int32_t charaPermissions = SOFTBUS_GATT_PERMISSION_READ | SOFTBUS_GATT_PERMISSION_WRITE;
    ASSERT_EQ(SoftBusGattsAddCharacteristic(MOCK_GATT_SERVICE_HANDLE, characteristic, properties, charaPermissions),
        SOFTBUS_OK);
    BtUuid btCharacteristic = {
        .uuidLen = strlen(netCharacteristic),
        .uuid = (char *)netCharacteristic,
    };
    gattServerCallback->characteristicAddCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, &btCharacteristic,
        MOCK_GATT_SERVICE_HANDLE, MOCK_GATT_CHARA_HANDLE);
    ASSERT_TRUE(
        characteristicAddCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK, MOCK_GATT_CHARA_HANDLE, &characteristic));
}

/**
 * @tc.name: AdapterBleGattServerTest_GattServerLifeCycle002
 * @tc.desc: test gatt server complete life cyclel, from a real usage perspective, important
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, GattServerLifeCycle002, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);

    // 注册desciptor
    const char *connDesciptor = "00002902-0000-1000-8000-00805F9B34FB";
    int32_t descriptorPermissions = SOFTBUS_GATT_PERMISSION_READ | SOFTBUS_GATT_PERMISSION_WRITE;
    SoftBusBtUuid descriptor = {
        .uuidLen = strlen(connDesciptor),
        .uuid = (char *)connDesciptor,
    };
    ASSERT_EQ(SoftBusGattsAddDescriptor(MOCK_GATT_SERVICE_HANDLE, descriptor, descriptorPermissions), SOFTBUS_OK);
    BtUuid btDescriptor = {
        .uuidLen = strlen(connDesciptor),
        .uuid = (char *)connDesciptor,
    };
    gattServerCallback->descriptorAddCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, &btDescriptor,
        MOCK_GATT_SERVICE_HANDLE, MOCK_GATT_DESCRIPTOR_HANDLE);
    ASSERT_TRUE(
        descriptorAddCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK, MOCK_GATT_DESCRIPTOR_HANDLE, &descriptor));
    // 启动Listen
    ASSERT_EQ(SoftBusGattsStartService(MOCK_GATT_SERVICE_HANDLE), SOFTBUS_OK);
    gattServerCallback->serviceStartCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, MOCK_GATT_SERVICE_HANDLE);
    ASSERT_TRUE(serviceStartCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK));
}

/**
 * @tc.name: AdapterBleGattServerTest_GattServerLifeCycle003
 * @tc.desc: test gatt server complete life cyclel, from a real usage perspective, important
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, GattServerLifeCycle003, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    // server建链
    BdAddr bdAddr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    gattServerCallback->connectServerCb(1, MOCK_GATT_SERVER_HANDLE, &bdAddr);
    SoftBusBtAddr addr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    GetStubGattsCallback()->connectServerCallback(1, &addr);
    ASSERT_TRUE(connectServerCtx.Expect(1, &addr));
    // 读写数据，及响应回复
    const char *valueExample = "hello gatt server, this is client";
    BtReqWriteCbPara btWriteParam = {
        .connId = 1,
        .transId = 0,
        .bdAddr = &bdAddr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .length = strlen(valueExample),
        .needRsp = true,
        .isPrep = false,
        .value = (unsigned char *)valueExample,
    };
    gattServerCallback->requestWriteCb(btWriteParam);
    SoftBusGattWriteRequest writeParam = {
        .connId = 1,
        .transId = 0,
        .btAddr = &addr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .length = strlen(valueExample),
        .needRsp = true,
        .isPrep = false,
        .value = (unsigned char *)valueExample,
    };
    GetStubGattsCallback()->requestWriteCallback(writeParam);
    ASSERT_TRUE(ExpectGattWriteRequest(requestWriteCtx, writeParam));
    SoftBusGattsResponse resp = { 0 };
    ASSERT_EQ(SoftBusGattsSendResponse(&resp), SOFTBUS_OK);
}

/**
 * @tc.name: AdapterBleGattServerTest_GattServerLifeCycle004
 * @tc.desc: test gatt server complete life cyclel, from a real usage perspective, important
 * @tc.type: FUNC
 * @tc.require: NONE
 */
HWTEST_F(AdapterBleGattServerTest, GattServerLifeCycle004, TestSize.Level3)
{
    const char *serviceUuid = "11C8B310-80E4-4276-AFC0-F81590B2177F";
    SoftBusBtUuid service = {
        .uuidLen = strlen(serviceUuid),
        .uuid = (char *)serviceUuid,
    };
    MockBluetooth mocker;
    MockAll(mocker);
    ASSERT_EQ(SoftBusRegisterGattsCallbacks(GetStubGattsCallback(), service), SOFTBUS_OK);
    BdAddr bdAddr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    SoftBusBtAddr addr = {
        .addr = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 },
    };
    BtReqReadCbPara btReadParam = {
        .connId = 1,
        .transId = 0,
        .bdAddr = &bdAddr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .isLong = false,
    };
    gattServerCallback->requestReadCb(btReadParam);
    SoftBusGattReadRequest readParam = {
        .connId = 1,
        .transId = 0,
        .btAddr = &addr,
        .attrHandle = MOCK_GATT_CHARA_HANDLE,
        .offset = 0,
        .isLong = false,
    };
    GetStubGattsCallback()->requestReadCallback(readParam);
    ASSERT_TRUE(ExpectGattReadRequest(requestReadCtx, readParam));
    SoftBusGattsNotify notify = { 0 };
    ASSERT_EQ(SoftBusGattsSendNotify(&notify), SOFTBUS_OK);
    // server断链
    ASSERT_EQ(SoftBusGattsDisconnect(addr, 1), SOFTBUS_OK);
    gattServerCallback->disconnectServerCb(1, MOCK_GATT_SERVER_HANDLE, &bdAddr);

    // 停止GATT service
    ASSERT_EQ(SoftBusGattsStopService(MOCK_GATT_SERVER_HANDLE), SOFTBUS_OK);
    gattServerCallback->serviceStopCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, MOCK_GATT_SERVICE_HANDLE);
    ASSERT_TRUE(serviceStopCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK));

    // 删除GATT service
    ASSERT_EQ(SoftBusGattsDeleteService(MOCK_GATT_SERVER_HANDLE), SOFTBUS_OK);
    gattServerCallback->serviceDeleteCb(OHOS_BT_STATUS_SUCCESS, MOCK_GATT_SERVER_HANDLE, MOCK_GATT_SERVICE_HANDLE);
    ASSERT_TRUE(serviceDeleteCtx.Expect(MOCK_GATT_SERVICE_HANDLE, SOFTBUS_OK));
}

BtUuidRecordCtx::BtUuidRecordCtx(const char *identifier) : StRecordCtx(identifier)
{
    Reset();
}
BtUuidRecordCtx::~BtUuidRecordCtx()
{
    Reset();
}

void BtUuidRecordCtx::Reset()
{
    SoftBusFree(uuid.uuid);
    uuid.uuid = nullptr;
    uuid.uuidLen = 0;
}

bool BtUuidRecordCtx::Update(int32_t id, int32_t st, SoftBusBtUuid *param)
{
    if (!StRecordCtx::Update(id, st)) {
        return false;
    }
    uuid.uuid = (char *)SoftBusCalloc(param->uuidLen);
    if (uuid.uuid == nullptr) {
        return false;
    }
    if (memcpy_s(uuid.uuid, param->uuidLen, param->uuid, param->uuidLen) != EOK) {
        return false;
    }
    uuid.uuidLen = param->uuidLen;
    return true;
}

testing::AssertionResult BtUuidRecordCtx::Expect(int32_t id, int32_t st, SoftBusBtUuid *param)
{
    auto result = StRecordCtx::Expect(id, st);
    if (!result) {
        goto ClEANUP;
    }
    if (uuid.uuidLen != param->uuidLen || memcmp(uuid.uuid, param->uuid, uuid.uuidLen) != 0) {
        result = testing::AssertionFailure() << identifier << " is call by unexpectedly uuid";
        goto ClEANUP;
    }
    result = testing::AssertionSuccess();
ClEANUP:
    Reset();
    return result;
}

BtGattRecordCtx::BtGattRecordCtx(const char *identifier) : BtUuidRecordCtx(identifier)
{
    handle = -1;
}

bool BtGattRecordCtx::Update(int32_t id, int32_t st, int32_t handleParam, SoftBusBtUuid *param)
{
    if (!BtUuidRecordCtx::Update(id, st, param)) {
        return false;
    }
    this->handle = handleParam;
    return true;
}

testing::AssertionResult BtGattRecordCtx::Expect(int32_t id, int32_t st, int32_t handleParam, SoftBusBtUuid *param)
{
    auto result = BtUuidRecordCtx::Expect(id, st, param);
    if (!result) {
        goto ClEANUP;
    }
    if (this->handle != handleParam) {
        result = testing::AssertionFailure() << identifier << " is call by unexpectedly state,"
                                             << "want: " << handleParam << ", actual: " << this->handle;
        goto ClEANUP;
    }
    result = testing::AssertionSuccess();
ClEANUP:
    this->handle = -1;
    return result;
}

BtGattServerCallbacks *AdapterBleGattServerTest::gattServerCallback = nullptr;
BtUuidRecordCtx AdapterBleGattServerTest::serviceAddCtx("ServiceAddCallback");
BtGattRecordCtx AdapterBleGattServerTest::characteristicAddCtx("CharacteristicAddCallback");
BtGattRecordCtx AdapterBleGattServerTest::descriptorAddCtx("DescriptorAddCallback");
StRecordCtx AdapterBleGattServerTest::serviceStartCtx("ServiceStartCallback");
StRecordCtx AdapterBleGattServerTest::serviceStopCtx("ServiceStopCallback");
StRecordCtx AdapterBleGattServerTest::serviceDeleteCtx("ServiceDeleteCallback");
BtAddrRecordCtx AdapterBleGattServerTest::connectServerCtx("ConnectServerCallback");
BtAddrRecordCtx AdapterBleGattServerTest::disconnectServerCtx("DisconnectServerCallback");
SoftBusGattReadRequest AdapterBleGattServerTest::requestReadCtx = { 0 };
SoftBusGattWriteRequest AdapterBleGattServerTest::requestWriteCtx = { 0 };
StRecordCtx AdapterBleGattServerTest::responseConfirmationCtx("ResponseConfirmationCallback");
StRecordCtx AdapterBleGattServerTest::notifySentCtx("NotifySentCallback");
StRecordCtx AdapterBleGattServerTest::mtuChangeCtx("MtuChangeCallback");
StRecordCtx AdapterBleGattServerTest::isConcernedAttrHandleCtx("isConcernedAttrHandle");

static void StubServiceAddCallback(int32_t status, SoftBusBtUuid *uuid, int32_t srvcHandle)
{
    AdapterBleGattServerTest::serviceAddCtx.Update(srvcHandle, status, uuid);
}

static void StubCharacteristicAddCallback(
    int32_t status, SoftBusBtUuid *uuid, int32_t srvcHandle, int32_t characteristicHandle)
{
    AdapterBleGattServerTest::characteristicAddCtx.Update(srvcHandle, status, characteristicHandle, uuid);
}

static void StubDescriptorAddCallback(int32_t status, SoftBusBtUuid *uuid, int32_t srvcHandle, int32_t descriptorHandle)
{
    AdapterBleGattServerTest::descriptorAddCtx.Update(srvcHandle, status, descriptorHandle, uuid);
}

static void StubServiceStartCallback(int32_t status, int32_t srvcHandle)
{
    AdapterBleGattServerTest::serviceStartCtx.Update(srvcHandle, status);
}

static void StubServiceStopCallback(int32_t status, int32_t srvcHandle)
{
    AdapterBleGattServerTest::serviceStopCtx.Update(srvcHandle, status);
}

static void StubServiceDeleteCallback(int32_t status, int32_t srvcHandle)
{
    AdapterBleGattServerTest::serviceDeleteCtx.Update(srvcHandle, status);
}

static void StubConnectServerCallback(int32_t connId, const SoftBusBtAddr *btAddr)
{
    AdapterBleGattServerTest::connectServerCtx.Update(connId, btAddr);
}

static void StubDisconnectServerCallback(int32_t connId, const SoftBusBtAddr *btAddr)
{
    AdapterBleGattServerTest::disconnectServerCtx.Update(connId, btAddr);
}

static void StubRequestReadCallback(SoftBusGattReadRequest readCbPara)
{
    AdapterBleGattServerTest::requestReadCtx = readCbPara;
}

static void StubRequestWriteCallback(SoftBusGattWriteRequest writeCbPara)
{
    AdapterBleGattServerTest::requestWriteCtx = writeCbPara;
}

static void StubResponseConfirmationCallback(int32_t status, int32_t handle)
{
    AdapterBleGattServerTest::responseConfirmationCtx.Update(handle, status);
}
static void StubNotifySentCallback(int32_t connId, int32_t status)
{
    AdapterBleGattServerTest::notifySentCtx.Update(connId, status);
}
static void StubMtuChangeCallback(int32_t connId, int32_t mtu)
{
    AdapterBleGattServerTest::mtuChangeCtx.Update(connId, mtu);
}

static bool StubIsConcernedAttrHandle(int32_t srvcHandle, int32_t attrHandle)
{
    return AdapterBleGattServerTest::isConcernedAttrHandleCtx.Update(srvcHandle, attrHandle);
}

static SoftBusGattsCallback *GetStubGattsCallback()
{
    static SoftBusGattsCallback callbacks = {
        .serviceAddCallback = StubServiceAddCallback,
        .characteristicAddCallback = StubCharacteristicAddCallback,
        .descriptorAddCallback = StubDescriptorAddCallback,
        .serviceStartCallback = StubServiceStartCallback,
        .serviceStopCallback = StubServiceStopCallback,
        .serviceDeleteCallback = StubServiceDeleteCallback,
        .connectServerCallback = StubConnectServerCallback,
        .disconnectServerCallback = StubDisconnectServerCallback,
        .requestReadCallback = StubRequestReadCallback,
        .requestWriteCallback = StubRequestWriteCallback,
        .responseConfirmationCallback = StubResponseConfirmationCallback,
        .notifySentCallback = StubNotifySentCallback,
        .mtuChangeCallback = StubMtuChangeCallback,
        .isConcernedAttrHandle = StubIsConcernedAttrHandle,
    };
    return &callbacks;
}

static testing::AssertionResult ExpectGattWriteRequest(SoftBusGattWriteRequest actual, SoftBusGattWriteRequest want)
{
    if (want.connId != actual.connId || want.transId != actual.transId ||
        memcmp(want.btAddr->addr, actual.btAddr->addr, BT_ADDR_LEN) != 0 || want.attrHandle != actual.attrHandle ||
        want.offset != actual.offset || want.length != actual.length ||
        !(want.needRsp ? actual.needRsp : !actual.needRsp) || !(want.isPrep ? actual.isPrep : !actual.isPrep) ||
        memcmp(want.value, actual.value, want.length) != 0) {
        return testing::AssertionFailure() << "SoftBusGattWriteRequest is unexpected";
    }
    return testing::AssertionSuccess();
}

static testing::AssertionResult ExpectGattReadRequest(SoftBusGattReadRequest actual, SoftBusGattReadRequest want)
{
    if (want.connId != actual.connId || want.transId != actual.transId ||
        memcmp(want.btAddr->addr, actual.btAddr->addr, BT_ADDR_LEN) != 0 || want.attrHandle != actual.attrHandle ||
        want.offset != actual.offset || !(want.isLong ? actual.isLong : !actual.isLong)) {
        return testing::AssertionFailure() << "SoftBusGattReadRequest is unexpected";
    }
    return testing::AssertionSuccess();
}

} // namespace OHOS
