#include "softbus_conn_ble_client.h"
#include "softbus_conn_ble_server.h"
#include "softbus_error_code.h"

static volatile int g_bleStubUnique0 = 0;
static volatile int g_bleStubUnique1 = 0;
static volatile int g_bleStubUnique2 = 0;
static volatile int g_bleStubUnique3 = 0;
static volatile int g_bleStubUnique4 = 0;
static volatile int g_bleStubUnique5 = 0;
static volatile int g_bleStubUnique6 = 0;
static volatile int g_bleStubUnique7 = 0;
static volatile int g_bleStubUnique8 = 0;
static volatile int g_bleStubUnique9 = 0;
static volatile int g_bleStubUnique10 = 0;

int32_t ConnGattClientConnect(ConnBleConnection *connection)
{
    (void)connection;
    g_bleStubUnique0 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattClientDisconnect(ConnBleConnection *connection, bool grace, bool refreshGatt)
{
    (void)connection;
    (void)grace;
    (void)refreshGatt;
    g_bleStubUnique1 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattClientSend(ConnBleConnection *connection, const uint8_t *data, uint32_t dataLen, int32_t module)
{
    (void)connection;
    (void)data;
    (void)dataLen;
    (void)module;
    g_bleStubUnique2 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattClientUpdatePriority(ConnBleConnection *connection, ConnectBlePriority priority)
{
    (void)connection;
    (void)priority;
    g_bleStubUnique3 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattInitClientModule(SoftBusLooper *looper, const ConnBleClientEventListener *listener)
{
    (void)looper;
    (void)listener;
    g_bleStubUnique4 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattServerStartService(void)
{
    g_bleStubUnique5 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattServerStopService(void)
{
    g_bleStubUnique6 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattServerSend(ConnBleConnection *connection, const uint8_t *data, uint32_t dataLen, int32_t module)
{
    (void)connection;
    (void)data;
    (void)dataLen;
    (void)module;
    g_bleStubUnique7 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattServerDisconnect(ConnBleConnection *connection)
{
    (void)connection;
    g_bleStubUnique8 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattServerConnect(ConnBleConnection *connection)
{
    (void)connection;
    g_bleStubUnique9 = 1;
    return SOFTBUS_ERR;
}

int32_t ConnGattInitServerModule(SoftBusLooper *looper, const ConnBleServerEventListener *listener)
{
    (void)looper;
    (void)listener;
    g_bleStubUnique10 = 1;
    return SOFTBUS_ERR;
}
