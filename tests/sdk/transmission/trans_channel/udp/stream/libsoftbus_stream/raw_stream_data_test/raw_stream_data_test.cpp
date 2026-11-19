/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <securec.h>

#include "i_stream.h"
#include "softbus_adapter_mem.h"
#include "softbus_error_code.h"
#include "stream_adaptor.h"

#define private public
#include "raw_stream_data.cpp"
#include "raw_stream_data.h"
#undef private

using namespace std;
using namespace testing::ext;
using namespace Communication;
using namespace SoftBus;

namespace OHOS {
#define DEVICE_ID "DEVICE_ID"
#define TEST_RAW_BUFLEN 4
#define TEST_RAW_PAYLOAD "DATA"

class RawStreamDataTest : public testing::Test {
public:
    RawStreamDataTest() { }
    ~RawStreamDataTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override { }
    void TearDown() override { }
};

void RawStreamDataTest::SetUpTestCase(void) { }

void RawStreamDataTest::TearDownTestCase(void) { }

/*
 * @tc.name: SetTimeStamp001
 * @tc.desc: SetTimeStamp and GetTimeStamp with zero timestamp
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, SetTimeStamp001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rRawStreamData = std::make_shared<RawStreamData>();
    uint32_t timestamp = 0;
    rRawStreamData->SetTimeStamp(timestamp);
    uint32_t tmp = rRawStreamData->GetTimeStamp();
    EXPECT_EQ(0, static_cast<int32_t>(tmp));
}

/*
 * @tc.name: GetExtBuffer001
 * @tc.desc: GetExtBuffer, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetExtBuffer001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rRawStreamData = std::make_shared<RawStreamData>();

    EXPECT_EQ(nullptr, rRawStreamData->GetExtBuffer());
}

/*
 * @tc.name: GetExtBufferLen001
 * @tc.desc: GetExtBufferLen001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetExtBufferLen001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rRawStreamData = std::make_shared<RawStreamData>();

    EXPECT_EQ(0, rRawStreamData->GetExtBufferLen());
}

/*
 * @tc.name: GetSeqNum001
 * @tc.desc: GetSeqNum001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetSeqNum001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rRawStreamData = std::make_shared<RawStreamData>();

    EXPECT_EQ(0, rRawStreamData->GetSeqNum());
}
/*
 * @tc.name: GetStreamId001
 * @tc.desc: GetStreamId001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetStreamId001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rRawStreamData = std::make_shared<RawStreamData>();
    EXPECT_EQ(0, rRawStreamData->GetStreamId());
}

/*
 * @tc.name: GetStreamFrameInfo001
 * @tc.desc: GetStreamFrameInfo001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetStreamFrameInfo001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rRawStreamData = std::make_shared<RawStreamData>();

    EXPECT_NE(nullptr, rRawStreamData->GetStreamFrameInfo());
}

/*
 * @tc.name: MakeRawStream001
 * @tc.desc: MakeRawStream with Communication::SoftBus::StreamData and Communication::SoftBus::StreamFrameInfo returns
 * non-null stream
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, MakeRawStream001, TestSize.Level1)
{
    Communication::SoftBus::StreamData *data = reinterpret_cast<Communication::SoftBus::StreamData *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamData)));
    ASSERT_TRUE(data != nullptr);
    Communication::SoftBus::StreamFrameInfo *info = reinterpret_cast<Communication::SoftBus::StreamFrameInfo *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamFrameInfo)));
    ASSERT_TRUE(info != nullptr);
    std::shared_ptr<IStream> iStream;
    std::unique_ptr<IStream> tmpIStream = iStream->MakeRawStream(*data, *info);
    EXPECT_TRUE(tmpIStream != nullptr);
    SoftBusFree(data);
    SoftBusFree(info);
}

/*
 * @tc.name: MakeRawStreamTest002
 * @tc.desc: MakeRawStream returns nullptr when scene is UNKNOWN_SCENE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, MakeRawStreamTest002, TestSize.Level1)
{
    Communication::SoftBus::StreamFrameInfo *info = reinterpret_cast<Communication::SoftBus::StreamFrameInfo *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamFrameInfo)));
    ASSERT_TRUE(info != nullptr);
    std::shared_ptr<IStream> iStream;
    int32_t scene = UNKNOWN_SCENE;
    ssize_t bufLen = 64;
    auto data = std::make_unique<char[]>(bufLen);
    std::unique_ptr<IStream> tmpIStream = iStream->MakeRawStream(data.get(), bufLen, *info, scene);
    EXPECT_TRUE(tmpIStream == nullptr);
    SoftBusFree(info);
}

/*
 * @tc.name: MakeRawStreamTest003
 * @tc.desc: MakeRawStream returns nullptr when bufLen exceeds MAX_STREAM_LEN with COMPATIBLE_SCENE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, MakeRawStreamTest003, TestSize.Level1)
{
    Communication::SoftBus::StreamFrameInfo *info = reinterpret_cast<Communication::SoftBus::StreamFrameInfo *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamFrameInfo)));
    ASSERT_TRUE(info != nullptr);
    std::shared_ptr<IStream> iStream;
    int32_t scene = COMPATIBLE_SCENE;
    ssize_t bufLen = MAX_STREAM_LEN + 1;
    auto data = std::make_unique<char[]>(bufLen);
    std::unique_ptr<IStream> tmpIStream = iStream->MakeRawStream(data.get(), bufLen, *info, scene);
    EXPECT_TRUE(tmpIStream == nullptr);
    SoftBusFree(info);
}

/*
 * @tc.name: InitStreamData001
 * @tc.desc: InitStreamData with valid buffers and lengths returns SOFTBUS_OK
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, InitStreamData001, TestSize.Level1)
{
    ssize_t bufLen = 22;
    auto buffer = std::make_unique<char[]>(bufLen + RawStreamData::FRAME_HEADER_LEN);
    ssize_t extLen = 33;
    auto extBuffer = std::make_unique<char[]>(extLen + RawStreamData::FRAME_HEADER_LEN);
    std::shared_ptr<RawStreamData> rawStreamData = std::make_shared<RawStreamData>();
    int32_t ret = rawStreamData->InitStreamData(std::move(buffer), bufLen, std::move(extBuffer), extLen);
    EXPECT_EQ(SOFTBUS_OK, ret);
}

/*
 * @tc.name: GetBuffer001
 * @tc.desc: GetBuffer001, use the wrong parameter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetBuffer001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rawStreamData = std::make_shared<RawStreamData>();
    EXPECT_EQ(nullptr, rawStreamData->GetBuffer());
}

/*
 * @tc.name: GetBufferLen001
 * @tc.desc: GetBufferLen returns zero on uninitialized RawStreamData
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, GetBufferLen001, TestSize.Level1)
{
    std::shared_ptr<RawStreamData> rawStreamData = std::make_shared<RawStreamData>();
    EXPECT_EQ(0, rawStreamData->GetBufferLen());
}

/*
 * @tc.name: InsertBufferLength001
 * @tc.desc: InsertBufferLength writes length bytes to output buffer
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, InsertBufferLength001, TestSize.Level1)
{
    ssize_t bufLen = 1;
    auto buffer = std::make_unique<char[]>(bufLen + RawStreamData::FRAME_HEADER_LEN);
    ASSERT_TRUE(buffer != nullptr);
    int32_t num = 3;
    int32_t length = 1;
    RawStreamData::InsertBufferLength(num, length, reinterpret_cast<uint8_t *>(buffer.get()));
    EXPECT_EQ(3, static_cast<int32_t>(buffer[0]));
}

/*
 * @tc.name: MakeRawStreamTest004
 * @tc.desc: MakeRawStream returns nullptr when bufLen is not positive with COMPATIBLE_SCENE
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, MakeRawStreamTest004, TestSize.Level1)
{
    Communication::SoftBus::StreamFrameInfo *info = reinterpret_cast<Communication::SoftBus::StreamFrameInfo *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamFrameInfo)));
    ASSERT_TRUE(info != nullptr);
    char buf[TEST_RAW_BUFLEN] = { 'D', 'A', 'T', 'A' };
    std::unique_ptr<IStream> tmpIStream = IStream::MakeRawStream(buf, 0, *info, COMPATIBLE_SCENE);
    EXPECT_TRUE(tmpIStream == nullptr);
    tmpIStream = IStream::MakeRawStream(buf, -1, *info, COMPATIBLE_SCENE);
    EXPECT_TRUE(tmpIStream == nullptr);
    SoftBusFree(info);
}

/*
 * @tc.name: MakeRawStreamTest005
 * @tc.desc: MakeRawStream with COMPATIBLE_SCENE returns a stream
 *           holding the payload without the frame header
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, MakeRawStreamTest005, TestSize.Level1)
{
    Communication::SoftBus::StreamFrameInfo *info = reinterpret_cast<Communication::SoftBus::StreamFrameInfo *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamFrameInfo)));
    ASSERT_TRUE(info != nullptr);
    char buf[TEST_RAW_BUFLEN] = { 'D', 'A', 'T', 'A' };
    std::unique_ptr<IStream> tmpIStream = IStream::MakeRawStream(buf, TEST_RAW_BUFLEN, *info, COMPATIBLE_SCENE);
    ASSERT_TRUE(tmpIStream != nullptr);
    EXPECT_EQ(TEST_RAW_BUFLEN, tmpIStream->GetBufferLen());
    std::unique_ptr<char[]> streamBuf = tmpIStream->GetBuffer();
    ASSERT_TRUE(streamBuf != nullptr);
    EXPECT_EQ(0, memcmp(streamBuf.get(), TEST_RAW_PAYLOAD, TEST_RAW_BUFLEN));
    SoftBusFree(info);
}

/*
 * @tc.name: MakeRawStreamTest006
 * @tc.desc: MakeRawStream with SOFTBUS_SCENE returns a stream with the
 *           frame header carrying the payload length
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, MakeRawStreamTest006, TestSize.Level1)
{
    Communication::SoftBus::StreamFrameInfo *info = reinterpret_cast<Communication::SoftBus::StreamFrameInfo *>(
        SoftBusCalloc(sizeof(Communication::SoftBus::StreamFrameInfo)));
    ASSERT_TRUE(info != nullptr);
    char buf[TEST_RAW_BUFLEN] = { 'D', 'A', 'T', 'A' };
    std::unique_ptr<IStream> tmpIStream = IStream::MakeRawStream(buf, TEST_RAW_BUFLEN, *info, SOFTBUS_SCENE);
    ASSERT_TRUE(tmpIStream != nullptr);
    EXPECT_EQ(TEST_RAW_BUFLEN + RawStreamData::FRAME_HEADER_LEN, tmpIStream->GetBufferLen());
    std::unique_ptr<char[]> streamBuf = tmpIStream->GetBuffer();
    ASSERT_TRUE(streamBuf != nullptr);
    EXPECT_EQ(0, memcmp(streamBuf.get() + RawStreamData::FRAME_HEADER_LEN, TEST_RAW_PAYLOAD, TEST_RAW_BUFLEN));
    SoftBusFree(info);
}

/*
 * @tc.name: InsertBufferLength002
 * @tc.desc: InsertBufferLength with null output or negative length
 *           does not write anything
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RawStreamDataTest, InsertBufferLength002, TestSize.Level1)
{
    auto buffer = std::make_unique<char[]>(TEST_RAW_BUFLEN + RawStreamData::FRAME_HEADER_LEN);
    ASSERT_TRUE(buffer != nullptr);
    (void)memset_s(buffer.get(), TEST_RAW_BUFLEN + RawStreamData::FRAME_HEADER_LEN, 0,
        TEST_RAW_BUFLEN + RawStreamData::FRAME_HEADER_LEN);
    RawStreamData::InsertBufferLength(TEST_RAW_BUFLEN, RawStreamData::FRAME_HEADER_LEN, nullptr);
    RawStreamData::InsertBufferLength(TEST_RAW_BUFLEN, -1, reinterpret_cast<uint8_t *>(buffer.get()));
    EXPECT_EQ(0, static_cast<int32_t>(buffer[0]));
}
} // namespace OHOS
