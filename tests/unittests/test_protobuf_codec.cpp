#include "ladtest/ladtest.h"

#include <Buffer.h>

#include <codec/Codec.h>
#include <codec/ProtobufCodec.h>

#ifdef _MSC_VER
#include "proto/tests.pb.cc"
#else
#include "proto/tests.pb.h"
#endif

#ifdef _MSC_VER

class Connection;
using ConnectionPtr = std::shared_ptr<ladder::Connection>;

class DerivedProtobufCodec : public ladder::ProtobufCodec {
    void Send(const ConnectionPtr& conn, google::protobuf::Message* message) {}
};
#endif

using namespace ladder;

class ProtobufCodecTest : public Test {
 protected:
  void SetUp() override {
#ifdef _MSC_VER
    codec = new DerivedProtobufCodec;
#else
    codec = new ladder::ProtobufCodec;
#endif
  }
  
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
