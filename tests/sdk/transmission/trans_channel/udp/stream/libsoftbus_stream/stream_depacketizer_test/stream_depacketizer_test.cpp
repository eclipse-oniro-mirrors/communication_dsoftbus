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

#include "common_inner.h"
#include "i_stream.h"
#include "softbus_adapter_crypto.h"
#include "softbus_adapter_mem.h"
#include "softbus_error_code.h"
#include "stream_common.h"

#define private   public
#define protected public
#include "stream_depacketizer.cpp"
#include "stream_depacketizer.h"
#undef protected
#undef private

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <securec.h>

using namespace testing::ext;
namespace OHOS {
#define TEST_DEPACK_BUFFER_SIZE 64
#define TEST_DEPACK_CHECKSUM 8
#define TEST_DEPACK_TLV_TOTAL_LEN (TEST_DEPACK_CHECKSUM + 4)
#define TEST_DEPACK_SMALL_DATA_LEN 10
#define TEST_DEPACK_LARGE_DATA_LEN 100
#define TEST_DEPACK_PAYLOAD_LEN 4
#define TEST_DEPACK_PAYLOAD "DATA"

class StreamDepacketizerTest : public testing::Test {
public:
    StreamDepacketizerTest() { }
    ~StreamDepacketizerTest() { }
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override { }
    void TearDown() override { }
};

void StreamDepacketizerTest::SetUpTestCase(void) { }

void StreamDepacketizerTest::TearDownTestCase(void) { }

/*
 * @tc.name: GetDefaultValues001
 * @tc.desc: stream depacketizer getter methods return zero or nullptr on uninitialized state
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, GetDefaultValues001, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);

    int32_t ret = streamDepacketizer->GetHeaderDataLen();
    EXPECT_EQ(0, ret);

    EXPECT_EQ(nullptr, streamDepacketizer->GetUserExt());
    ret = streamDepacketizer->GetUserExtSize();
    EXPECT_EQ(0, ret);

    EXPECT_EQ(nullptr, streamDepacketizer->GetData());
    ret = streamDepacketizer->GetDataLength();
    EXPECT_EQ(0, ret);
}

/*
 * @tc.name: DepacketizeHeader001
 * @tc.desc: DepacketizeHeader with COMMON_VIDEO_STREAM does not crash
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeHeader001, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    char header[Communication::SoftBus::MAX_STREAM_LEN - OVERHEAD_LEN + 1] = { 0 };
    EXPECT_NO_FATAL_FAILURE(streamDepacketizer->DepacketizeHeader(header));
    int32_t ret = streamDepacketizer->GetHeaderDataLen();
    EXPECT_EQ(0, ret);
}

/*
 * @tc.name: DepacketizeHeader002
 * @tc.desc: DepacketizeHeader with COMMON_AUDIO_STREAM does not crash
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeHeader002, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->streamType_ = Communication::SoftBus::COMMON_AUDIO_STREAM;
    char header[Communication::SoftBus::MAX_STREAM_LEN - OVERHEAD_LEN + 1] = { 0 };
    EXPECT_NO_FATAL_FAILURE(streamDepacketizer->DepacketizeHeader(header));
    int32_t ret = streamDepacketizer->GetHeaderDataLen();
    EXPECT_EQ(0, ret);
}

/*
 * @tc.name: DepacketizeBuffer001
 * @tc.desc: DepacketizeBuffer with zeroed buffer does not crash
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer001, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    char buffer[Communication::SoftBus::MAX_STREAM_LEN - OVERHEAD_LEN + 1] = { 0 };
    EXPECT_NO_FATAL_FAILURE(streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer)));
    int32_t ret = streamDepacketizer->GetDataLength();
    EXPECT_EQ(0, ret);
}

/*
 * @tc.name: DepacketizeBuffer002
 * @tc.desc: DepacketizeBuffer with ext flag set and zero checksum
 *           returns directly without copying data
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer002, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->header_.commonHeader_.extFlag = 1;
    char buffer[TEST_DEPACK_BUFFER_SIZE] = { 0 };
    streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer));
    EXPECT_EQ(0, streamDepacketizer->GetDataLength());
    EXPECT_EQ(nullptr, streamDepacketizer->GetData());
}

/*
 * @tc.name: DepacketizeBuffer003
 * @tc.desc: DepacketizeBuffer with ext flag set and data len smaller
 *           than the tlv total len returns without copying data
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer003, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->header_.commonHeader_.extFlag = 1;
    streamDepacketizer->header_.commonHeader_.dataLen = TEST_DEPACK_SMALL_DATA_LEN;
    streamDepacketizer->tlvs_.checkSum_ = TEST_DEPACK_CHECKSUM;
    char buffer[TEST_DEPACK_BUFFER_SIZE] = { 0 };
    streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer));
    EXPECT_EQ(nullptr, streamDepacketizer->GetData());
}

/*
 * @tc.name: DepacketizeBuffer004
 * @tc.desc: DepacketizeBuffer with data len exceeding MAX_STREAM_LEN
 *           returns without copying data
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer004, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->header_.commonHeader_.dataLen = Communication::SoftBus::MAX_STREAM_LEN + 1;
    char buffer[TEST_DEPACK_BUFFER_SIZE] = { 0 };
    streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer));
    EXPECT_EQ(nullptr, streamDepacketizer->GetData());
}

/*
 * @tc.name: DepacketizeBuffer005
 * @tc.desc: DepacketizeBuffer with buffer size smaller than data len
 *           returns without copying data
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer005, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->header_.commonHeader_.dataLen = TEST_DEPACK_LARGE_DATA_LEN;
    char buffer[TEST_DEPACK_BUFFER_SIZE] = { 0 };
    streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer));
    EXPECT_EQ(nullptr, streamDepacketizer->GetData());
}

/*
 * @tc.name: DepacketizeBuffer006
 * @tc.desc: DepacketizeBuffer with valid data len copies the payload
 *           from the buffer start
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer006, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->header_.commonHeader_.dataLen = TEST_DEPACK_PAYLOAD_LEN;
    char buffer[TEST_DEPACK_BUFFER_SIZE] = { 0 };
    (void)memcpy_s(buffer, sizeof(buffer), TEST_DEPACK_PAYLOAD, strlen(TEST_DEPACK_PAYLOAD));
    streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer));
    EXPECT_EQ(TEST_DEPACK_PAYLOAD_LEN, streamDepacketizer->GetDataLength());
    EXPECT_EQ(0, memcmp(streamDepacketizer->GetData().get(), TEST_DEPACK_PAYLOAD, TEST_DEPACK_PAYLOAD_LEN));
}

/*
 * @tc.name: DepacketizeBuffer007
 * @tc.desc: DepacketizeBuffer with ext flag set and valid data len copies
 *           the payload after the tlv total len
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StreamDepacketizerTest, DepacketizeBuffer007, TestSize.Level1)
{
    int32_t streamType = Communication::SoftBus::COMMON_VIDEO_STREAM;
    std::shared_ptr<Communication::SoftBus::StreamDepacketizer> streamDepacketizer =
        std::make_shared<Communication::SoftBus::StreamDepacketizer>(streamType);
    streamDepacketizer->header_.commonHeader_.extFlag = 1;
    streamDepacketizer->header_.commonHeader_.dataLen = TEST_DEPACK_TLV_TOTAL_LEN + TEST_DEPACK_PAYLOAD_LEN;
    streamDepacketizer->tlvs_.checkSum_ = TEST_DEPACK_CHECKSUM;
    char buffer[TEST_DEPACK_BUFFER_SIZE] = { 0 };
    (void)memcpy_s(buffer + TEST_DEPACK_TLV_TOTAL_LEN, TEST_DEPACK_BUFFER_SIZE - TEST_DEPACK_TLV_TOTAL_LEN,
        TEST_DEPACK_PAYLOAD, strlen(TEST_DEPACK_PAYLOAD));
    streamDepacketizer->DepacketizeBuffer(buffer, sizeof(buffer));
    EXPECT_EQ(TEST_DEPACK_PAYLOAD_LEN, streamDepacketizer->GetDataLength());
    EXPECT_EQ(0, memcmp(streamDepacketizer->GetData().get(), TEST_DEPACK_PAYLOAD, TEST_DEPACK_PAYLOAD_LEN));
}
} // namespace OHOS
