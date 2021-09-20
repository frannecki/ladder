#include <gtest/gtest.h>

#include <Buffer.h>

#include "proto/tests.pb.h"

#define private public
#include <ProtobufCodec.h>

class ProtobufCodecTest : public testing::Test {
protected:
  void SetUp() override {
    codec = new ladder::ProtobufCodec;
  }
  
  void TearDown() override {
    if(codec) {
      delete codec;
      codec = nullptr;
    }
  }

  ladder::ProtobufCodec* codec;
};

TEST_F(ProtobufCodecTest, test_message) {
  ladder::TestMessage1 message;
  message.set_key("arbitrary_key");
  message.set_value("arbitrary_value");
  message.set_success(false);
  message.set_validate(true);
  message.set_id(9);

  std::string buf;
  codec->ComposeMessage(&message, buf);

  ladder::Buffer buffer;
  buffer.Write(buf);
  google::protobuf::Message* msg = nullptr;
  int length = codec->ParseMessage(msg, &buffer);

  ASSERT_GT(length, ladder::ProtobufCodec::kMinMessageLength);
  ASSERT_TRUE(msg != nullptr);

  ladder::TestMessage1 *msg1 = dynamic_cast<ladder::TestMessage1*>(msg);
  EXPECT_EQ(msg1->key(), "arbitrary_key");
  EXPECT_EQ(msg1->value(), "arbitrary_value");
  EXPECT_FALSE(msg1->success());
  EXPECT_TRUE(msg1->validate());
  EXPECT_EQ(msg1->id(), 9);
}
