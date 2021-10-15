#include <gtest/gtest.h>

#include <Buffer.h>

#include "proto/tests.pb.h"

#define private public
#define protected public
#include <codec/ProtobufCodec.h>

class ProtobufCodecTest : public testing::Test {
 protected:
  void SetUp() override { codec = new ladder::ProtobufCodec; }

  void TearDown() override {
    if (codec) {
      delete codec;
      codec = nullptr;
    }
  }

  ladder::Codec* codec;
};

TEST_F(ProtobufCodecTest, test_message) {
  ladder::TestMessage1 message;
  message.set_key("arbitrary_key");
  message.set_value("arbitrary_value");
  message.set_success(false);
  message.set_validate(true);
  message.set_id(9);

  std::string packet;
  codec->Encapsulate(&message, packet);

  ladder::Buffer buffer;
  buffer.Write(packet);

  std::string raw_data;
  ASSERT_TRUE(codec->Decapsulate(raw_data, &buffer));

  ASSERT_GT(raw_data.size(), ladder::ProtobufCodec::kMinMessageLength);

  void* msg = nullptr;
  ASSERT_TRUE(codec->ParseMessage(raw_data, msg));
  ASSERT_TRUE(msg != nullptr);

  ladder::TestMessage1* msg1 = reinterpret_cast<ladder::TestMessage1*>(msg);
  EXPECT_EQ(msg1->key(), "arbitrary_key");
  EXPECT_EQ(msg1->value(), "arbitrary_value");
  EXPECT_FALSE(msg1->success());
  EXPECT_TRUE(msg1->validate());
  EXPECT_EQ(msg1->id(), 9);
}
