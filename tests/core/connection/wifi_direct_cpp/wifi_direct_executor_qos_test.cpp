#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <memory>
#include <string>

#include "wifi_direct_executor.h"
#include "wifi_direct_scheduler.h"
#include "conn_log.h"
#include "softbus_error_code.h"

using namespace testing::ext;
using namespace OHOS::SoftBus;

namespace {
std::atomic<int> g_setThreadQosCallCount{0};
std::atomic<int> g_setThreadQosReturnVal{0};
std::atomic<int> g_setThreadQosLastLevel{-1};

void ResetQosMock()
{
    g_setThreadQosCallCount.store(0);
    g_setThreadQosReturnVal.store(0);
    g_setThreadQosLastLevel.store(-1);
}
}

namespace OHOS::QOS {
int SetThreadQos(enum QosLevel level)
{
    g_setThreadQosCallCount.fetch_add(1);
    g_setThreadQosLastLevel.store(static_cast<int>(level));
    return g_setThreadQosReturnVal.load();
}

int SetQosForOtherThread(enum QosLevel level, int tid) { return 0; }
int ResetThreadQos() { return 0; }
int ResetQosForOtherThread(int tid) { return 0; }
int GetThreadQos(enum QosLevel &level) { return 0; }
int GetQosForOtherThread(enum QosLevel &level, int tid) { return 0; }
int AddThreadToProcRtg(int tid) { return 0; }
int AddThreadsToRrocRtg(int tid[5], int size) { return 0; }
int RemoveThreadFromProcRtg(int tid) { return 0; }
int RemoveThreadsFromProcRtg(int tid[5], int size) { return 0; }
}

namespace OHOS::SoftBus {

class StubProcessor : public WifiDirectProcessor {
public:
    explicit StubProcessor(const std::string &remoteUuid) : WifiDirectProcessor(remoteUuid) {}
    void Run() override
    {
        throw ProcessorTerminate();
    }
    bool CanAcceptNegotiateDataAtState(WifiDirectCommand &command) override { return false; }
    void HandleCommandAfterTerminate(WifiDirectCommand &command) override {}
    std::string GetProcessorName() const override { return "StubProcessor"; }
    std::string GetState() const override { return "stub"; }
};

class StubScheduler : public WifiDirectScheduler {
public:
    bool ProcessNextCommand(WifiDirectExecutor *executor, std::shared_ptr<WifiDirectProcessor> &processor) override
    {
        return false;
    }
};

class WifiDirectExecutorQosTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override { ResetQosMock(); }
    void TearDown() override {}
};

HWTEST_F(WifiDirectExecutorQosTest, Run_QosSetSuccess_CallSetThreadQosAndContinue, TestSize.Level0)
{
    g_setThreadQosReturnVal.store(0);

    StubScheduler scheduler;
    std::shared_ptr<WifiDirectProcessor> processor = std::make_shared<StubProcessor>("testDeviceId");
    bool active = true;
    WifiDirectExecutor executor("testDeviceId", scheduler, processor, active);

    executor.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_GE(g_setThreadQosCallCount.load(), 1);
    EXPECT_EQ(g_setThreadQosLastLevel.load(), static_cast<int>(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE));
}

HWTEST_F(WifiDirectExecutorQosTest, Run_QosSetFail_LogErrorAndContinue, TestSize.Level0)
{
    g_setThreadQosReturnVal.store(-1);

    StubScheduler scheduler;
    std::shared_ptr<WifiDirectProcessor> processor = std::make_shared<StubProcessor>("testDeviceIdFail");
    bool active = true;
    WifiDirectExecutor executor("testDeviceIdFail", scheduler, processor, active);

    executor.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_GE(g_setThreadQosCallCount.load(), 1);
    EXPECT_EQ(g_setThreadQosLastLevel.load(), static_cast<int>(OHOS::QOS::QosLevel::QOS_USER_INTERACTIVE));
}
}
