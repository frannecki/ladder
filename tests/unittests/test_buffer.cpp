#include <string>

#include <gtest/gtest.h>

#define private public
#include <Buffer.h>

TEST(TestBuffer, test_Write) {
  ladder::Buffer buffer;
  std::string str = "areyouokbro?";
  buffer.Write(str);
  EXPECT_EQ(buffer.buffer_.size(), str.size());
  EXPECT_EQ(buffer.ReadableBytes(), str.size());
  EXPECT_EQ(buffer.read_index_, 0);
  EXPECT_EQ(buffer.write_index_, str.size());
  EXPECT_EQ(buffer.ReadAll(), str);
}

TEST(TestBuffer, test_Write_char) {
  ladder::Buffer buffer;
  const char* str = "areyouokbro?";
  buffer.Write(str, strlen(str));
  EXPECT_EQ(buffer.buffer_.size(), strlen(str));
  EXPECT_EQ(buffer.ReadAll(), std::string(str, strlen(str)));
}

TEST(TestBuffer, test_Read) {
  ladder::Buffer buffer;
  std::string str = "areyouokbro?howareyou?";
  buffer.Write(str);

  uint32_t read_size = 6;
  std::string read_result = "areyou";
  std::string remains = "okbro?howareyou?";

  EXPECT_EQ(buffer.Read(read_size), read_result);
  EXPECT_EQ(buffer.ReadableBytes(), str.size() - read_size);
  EXPECT_EQ(buffer.read_index_, read_size);
  EXPECT_EQ(buffer.write_index_, str.size());

  EXPECT_EQ(buffer.ReadAll(), remains);
  EXPECT_EQ(buffer.ReadableBytes(), 0);
  EXPECT_EQ(buffer.read_index_, 0);
  EXPECT_EQ(buffer.write_index_, 0);
}

TEST(TestBuffer, test_Peek) {
  ladder::Buffer buffer;
  std::string str = "areyouokbro?howareyou?";
  buffer.Write(str);

  uint32_t read_size = 6;
  std::string read_result = "areyou";
  std::string remains = "okbro?howareyou?";
  std::string result;

  buffer.Peek(read_size, result);

  EXPECT_EQ(result, read_result);
  EXPECT_EQ(buffer.ReadableBytes(), str.size());
  EXPECT_EQ(buffer.read_index_, 0);
  EXPECT_EQ(buffer.write_index_, str.size());
}

TEST(TestBuffer, test_uint32) {
  uint32_t i1 = 53, i2 = 646;
  ladder::Buffer buffer;
  buffer.WriteUInt32(i1);
  buffer.WriteUInt32(i2);
  EXPECT_EQ(buffer.ReadableBytes(), sizeof(uint32_t) * 2);

  EXPECT_EQ(buffer.ReadUInt32(), i1);
  EXPECT_EQ(buffer.ReadUInt32(), i2);
}
