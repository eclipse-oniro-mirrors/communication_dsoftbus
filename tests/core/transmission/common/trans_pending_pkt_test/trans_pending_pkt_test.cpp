/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include <securec.h>
#include <atomic>
#include <chrono>
#include <climits>
#include <thread>

#include "trans_pending_pkt.c"

using namespace testing::ext;
namespace OHOS {
class TransPendingPktTest : public testing::Test {
public:
    TransPendingPktTest()
    {}
    ~TransPendingPktTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override
    {}
    void TearDown() override
    {}
};

void TransPendingPktTest::SetUpTestCase(void)
{}

void TransPendingPktTest::TearDownTestCase(void)
{}

/*
 * @tc.name: PendingInit001
 * @tc.desc: PendingInit001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, PendingInit001, TestSize.Level1)
{
    int32_t type = -88;
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = 999;
    ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = PENDING_TYPE_DIRECT;
    ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: PendingDeinit001
 * @tc.desc: PendingDeinit001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, PendingDeinit001, TestSize.Level1)
{
    int32_t type = -21;
    PendingDeinit(type);

    type = 999;
    PendingDeinit(type);

    type = PENDING_TYPE_DIRECT;
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    PendingDeinit(type);
}

/*
 * @tc.name: CreatePendingItem001
 * @tc.desc: CreatePendingItem001, use the wrong parameter
 * @tc.desc: ReleasePendingItem, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, CreatePendingItem001, TestSize.Level1)
{
    int32_t seqNum = 1111;
    int32_t channelId = 222;
    PendingPktInfo *item = nullptr;
    item = CreatePendingItem(channelId, seqNum);
    EXPECT_TRUE(item != nullptr);

    ReleasePendingItem(item);
    ReleasePendingItem(nullptr);
}

/*
 * @tc.name: ProcPendingPacket001
 * @tc.desc: ProcPendingPacket001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, ProcPendingPacket001, TestSize.Level1)
{
    int32_t seqNum = 1111;
    int32_t channelId = 222;
    int32_t type = PENDING_TYPE_BUTT + 1;
    int32_t ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    type = PENDING_TYPE_PROXY - 1;
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = PENDING_TYPE_PROXY + 1;
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_PENDINGLIST_NOT_FOUND, ret);
}

/*
 * @tc.name: SetPendingPacket001
 * @tc.desc: SetPendingPacket001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, SetPendingPacket001, TestSize.Level1)
{
    int32_t channelId = 1111;
    int32_t seqNum = 222;
    int32_t type = PENDING_TYPE_BUTT + 1;
    int32_t ret = SetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    type = PENDING_TYPE_PROXY - 1;
    ret = SetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = PENDING_TYPE_DIRECT;
    channelId = -1;
    ret = SetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_PENDINGLIST_NOT_FOUND, ret);
}

/*
 * @tc.name: DelPendingPacket001
 * @tc.desc: DelPendingPacket001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, DelPendingPacket001, TestSize.Level1)
{
    int32_t channelId = 1111;
    int32_t type = PENDING_TYPE_BUTT + 1;
    int32_t ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    type = PENDING_TYPE_PROXY - 1;
    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = PENDING_TYPE_PROXY + 1;
    PendingDeinit(type);
    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_PENDINGLIST_NOT_FOUND, ret);
}

/*
 * @tc.name: ProcPendingPacket002
 * @tc.desc: ProcPendingPacket, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, ProcPendingPacket002, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t seqNum = 0;
    int32_t type = 1;

    int32_t ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_PENDINGLIST_NOT_FOUND, ret);

    type = -1;
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = 1;
    ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
    PendingDeinit(type);

    ret = PendingInit(PENDING_TYPE_UDP);
    EXPECT_EQ(SOFTBUS_OK, ret);
    type = PENDING_TYPE_UDP;
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
    PendingDeinit(type);
}

/*
 * @tc.name: TimeBefore001
 * @tc.desc: TimeBefore001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, TimeBefore001, TestSize.Level1)
{
    SoftBusSysTime outtime;
    SoftBusSysTime now;
    SoftBusGetTime(&now);
    outtime.sec = now.sec + MSG_TIMEOUT_S;
    outtime.usec = now.usec;
    bool res = TimeBefore(&outtime);
    EXPECT_TRUE(res);
}

/*
 * @tc.name: ProcPendingPacket002
 * @tc.desc: ProcPendingPacket, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, SetPendingPacket002, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t seqNum = 0;
    int32_t type = 1;

    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ProcPendingPacket(channelId, seqNum, type);
    ret = SetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_NODE_NOT_FOUND, ret);

    PendingDeinit(type);
}

/*
 * @tc.name: DelPendingPacket002
 * @tc.desc: DelPendingPacket002, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, DelPendingPacket002, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t type = 1;
    int32_t seqNum = 0;

    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);
    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: PendingPacketTestAll001
 * @tc.desc: PendingPacketTestAll001, use the wrong parameter
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, PendingPacketTestAll001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t type = PENDING_TYPE_UDP;
    int32_t seqNum = 0;

    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_NOT_FIND, ret);

    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_CHANNEL_ALREADY_PENDING, ret);

    PendingPktInfo *pkgInfo = GetPendingPacket(channelId, seqNum, PENDING_TYPE_BUTT + 1);
    EXPECT_EQ(pkgInfo, nullptr);
    pkgInfo = GetPendingPacket(channelId, seqNum, type);
    EXPECT_NE(pkgInfo, nullptr);

    ret = ProcPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TIMOUT, ret);

    ret = SetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_NODE_NOT_FOUND, ret);

    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: DelPendingPacketbyChannelId001
 * @tc.desc: DelPendingPacketbyChannelId, use the illegal type and uninitialized list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, DelPendingPacketbyChannelId001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t seqNum = 0;

    int32_t type = PENDING_TYPE_BUTT + 1;
    DelPendingPacketbyChannelId(channelId, seqNum, type);
    type = PENDING_TYPE_PROXY - 1;
    DelPendingPacketbyChannelId(channelId, seqNum, type);

    type = PENDING_TYPE_PROXY;
    PendingDeinit(type);
    DelPendingPacketbyChannelId(channelId, seqNum, type);

    type = PENDING_TYPE_DIRECT;
    PendingDeinit(type);
    DelPendingPacketbyChannelId(channelId, seqNum, type);
}

/*
 * @tc.name: DelPendingPacketbyChannelId002
 * @tc.desc: DelPendingPacketbyChannelId, delete items matched by channelId and seqNum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, DelPendingPacketbyChannelId002, TestSize.Level1)
{
    int32_t type = PENDING_TYPE_PROXY;
    PendingDeinit(type);
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = AddPendingPacket(1, 100, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(2, 100, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(1, 200, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(3, g_pendingList[type]->cnt);

    DelPendingPacketbyChannelId(1, 100, type);
    EXPECT_EQ(2, g_pendingList[type]->cnt);
    DelPendingPacketbyChannelId(2, 100, type);
    EXPECT_EQ(1, g_pendingList[type]->cnt);
    DelPendingPacketbyChannelId(1, 100, type);
    EXPECT_EQ(1, g_pendingList[type]->cnt);
    DelPendingPacketbyChannelId(1, 200, type);
    EXPECT_EQ(0, g_pendingList[type]->cnt);
    DelPendingPacketbyChannelId(1, 200, type);
    EXPECT_EQ(0, g_pendingList[type]->cnt);

    PendingDeinit(type);
}

/*
 * @tc.name: DelPendingPacketbyChannelId003
 * @tc.desc: DelPendingPacketbyChannelId, use the boundary channelId and seqNum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, DelPendingPacketbyChannelId003, TestSize.Level1)
{
    int32_t type = PENDING_TYPE_PROXY;
    PendingDeinit(type);
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = AddPendingPacket(INT32_MAX, INT32_MAX, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(INT32_MIN, INT32_MIN, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(2, g_pendingList[type]->cnt);

    PendingPktInfo *item = GetPendingPacket(INT32_MAX, INT32_MIN, type);
    EXPECT_EQ(nullptr, item);
    item = GetPendingPacket(INT32_MAX, INT32_MAX, type);
    EXPECT_NE(nullptr, item);

    DelPendingPacketbyChannelId(INT32_MAX, INT32_MAX, type);
    EXPECT_EQ(1, g_pendingList[type]->cnt);
    DelPendingPacketbyChannelId(INT32_MIN, INT32_MIN, type);
    EXPECT_EQ(0, g_pendingList[type]->cnt);

    PendingDeinit(type);
}

/*
 * @tc.name: AddPendingPacket001
 * @tc.desc: AddPendingPacket, use the illegal type and uninitialized list
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, AddPendingPacket001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t seqNum = 0;

    int32_t type = PENDING_TYPE_BUTT + 1;
    int32_t ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);
    type = PENDING_TYPE_PROXY - 1;
    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_INVALID_PARAM, ret);

    type = PENDING_TYPE_PROXY;
    PendingDeinit(type);
    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_TRANS_TDC_PENDINGLIST_NOT_FOUND, ret);
}

/*
 * @tc.name: GetPendingPacket001
 * @tc.desc: GetPendingPacket, use the uninitialized list and unmatched seqNum
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, GetPendingPacket001, TestSize.Level1)
{
    int32_t channelId = 1;
    int32_t seqNum = 0;
    int32_t type = PENDING_TYPE_PROXY;

    PendingDeinit(type);
    PendingPktInfo *item = GetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(nullptr, item);

    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    item = GetPendingPacket(channelId, seqNum + 1, type);
    EXPECT_EQ(nullptr, item);
    item = GetPendingPacket(channelId + 1, seqNum, type);
    EXPECT_EQ(nullptr, item);
    item = GetPendingPacket(channelId, seqNum, type);
    EXPECT_NE(nullptr, item);

    PendingDeinit(type);
}

/*
 * @tc.name: PendingPacketStateTransition001
 * @tc.desc: Proc returns immediately when the item status is FINISHED or CANCELED
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, PendingPacketStateTransition001, TestSize.Level1)
{
    int32_t channelId = 30;
    int32_t type = PENDING_TYPE_UDP;
    PendingDeinit(type);
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);

    ret = AddPendingPacket(channelId, 700, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = SetPendingPacket(channelId, 700, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ProcPendingPacket(channelId, 700, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    EXPECT_EQ(0, g_pendingList[type]->cnt);

    ret = AddPendingPacket(channelId, 701, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = ProcPendingPacket(channelId, 701, type);
    EXPECT_EQ(SOFTBUS_TIMOUT, ret);
    EXPECT_EQ(0, g_pendingList[type]->cnt);

    PendingDeinit(type);
}

/*
 * @tc.name: PendingPacketWakeUpBySet001
 * @tc.desc: ProcPendingPacket is woken up by SetPendingPacket in another thread
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, PendingPacketWakeUpBySet001, TestSize.Level1)
{
    int32_t channelId = 10;
    int32_t seqNum = 500;
    int32_t type = PENDING_TYPE_UDP;
    PendingDeinit(type);
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_OK, ret);

    std::atomic<int32_t> procRet(SOFTBUS_INVALID_PARAM);
    std::thread procThread([&]() { procRet = ProcPendingPacket(channelId, seqNum, type); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ret = SetPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    procThread.join();
    EXPECT_EQ(SOFTBUS_OK, procRet.load());
    EXPECT_EQ(0, g_pendingList[type]->cnt);

    PendingDeinit(type);
}

/*
 * @tc.name: PendingPacketWakeUpByDel001
 * @tc.desc: ProcPendingPacket is woken up by DelPendingPacket in another thread
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(TransPendingPktTest, PendingPacketWakeUpByDel001, TestSize.Level1)
{
    int32_t channelId = 20;
    int32_t seqNum = 600;
    int32_t type = PENDING_TYPE_UDP;
    PendingDeinit(type);
    int32_t ret = PendingInit(type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    ret = AddPendingPacket(channelId, seqNum, type);
    EXPECT_EQ(SOFTBUS_OK, ret);

    std::atomic<int32_t> procRet(SOFTBUS_OK);
    std::thread procThread([&]() { procRet = ProcPendingPacket(channelId, seqNum, type); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ret = DelPendingPacket(channelId, type);
    EXPECT_EQ(SOFTBUS_OK, ret);
    procThread.join();
    EXPECT_EQ(SOFTBUS_TIMOUT, procRet.load());
    EXPECT_EQ(0, g_pendingList[type]->cnt);

    PendingDeinit(type);
}
} // OHOS
