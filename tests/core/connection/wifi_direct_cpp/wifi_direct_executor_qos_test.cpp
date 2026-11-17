#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "wifi_direct_executor.h"
#include "wifi_direct_scheduler.h"
#include "conn_log.h"
#include "softbus_error_code.h"

using namespace testing::ext;
using namespace OHOS::SoftBus;

namespace {
std::atomic<int> g_requestAuthCallCount{0};
std::atomic<bool> g_processorRunCalled{false};

void ResetQosMock()
{
    g_requestAuthCallCount.store(0);
    g_processorRunCalled.store(false);
}
}

namespace OHOS::ConcurrentTask {
class ConcurrentTaskClient {
public:
    static ConcurrentTaskClient& GetInstance()
    {
        static ConcurrentTaskClient instance;
        return instance;
    }
    void RequestAuth(const std::unordered_map<std::string, std::string>& mapPayload)
    {
        g_requestAuthCallCount.fetch_add(1);
    }
private:
    ConcurrentTaskClient() = default;
    ~ConcurrentTaskClient() = default;
};
}

namespace OHOS::SoftBus {

class StubProcessor : public WifiDirectProcessor {
public:
    explicit StubProcessor(const std::string &remoteUuid) : WifiDirectProcessor(remoteUuid) {}
    void Run() override
    {
        g_processorRunCalled.store(true);
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

HWTEST_F(WifiDirectExecutorQosTest, Start_BoostQosTrue_RequestAuthCalledAndProcessorRun, TestSize.Level0)
{
    StubScheduler scheduler;
    std::shared_ptr<WifiDirectProcessor> processor = std::make_shared<StubProcessor>("testDeviceId");
    bool active = true;
    WifiDirectExecutor executor("testDeviceId", scheduler, processor, active);
    executor.SetBoostQos(true);

    executor.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_GE(g_requestAuthCallCount.load(), 1);
    EXPECT_TRUE(g_processorRunCalled.load());
}

HWTEST_F(WifiDirectExecutorQosTest, Start_BoostQosFalse_RequestAuthNotCalledAndProcessorRun, TestSize.Level0)
{
    StubScheduler scheduler;
    std::shared_ptr<WifiDirectProcessor> processor = std::make_shared<StubProcessor>("testDeviceIdFail");
    bool active = true;
    WifiDirectExecutor executor("testDeviceIdFail", scheduler, processor, active);
    executor.SetBoostQos(false);

    executor.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(g_requestAuthCallCount.load(), 0);
    EXPECT_TRUE(g_processorRunCalled.load());
}

HWTEST_F(WifiDirectExecutorQosTest, Start_BoostQosDefault_RequestAuthNotCalled, TestSize.Level0)
{
    StubScheduler scheduler;
    std::shared_ptr<WifiDirectProcessor> processor = std::make_shared<StubProcessor>("testDeviceIdDefault");
    bool active = true;
    WifiDirectExecutor executor("testDeviceIdDefault", scheduler, processor, active);

    executor.Start();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_EQ(g_requestAuthCallCount.load(), 0);
    EXPECT_TRUE(g_processorRunCalled.load());
}
}
