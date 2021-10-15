#include <map>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <ArgumentParser.h>

class ArgumentParserTest : public ladder::IArgumentParser {
 public:
  void DelegateToFake() {
    ON_CALL(*this, InitOptions).WillByDefault([this]() {
      this->fields_options_.insert(std::pair<std::string, bool>("ip", true));
      this->fields_options_.insert(std::pair<std::string, bool>("port", true));
      this->fields_options_.insert(std::pair<std::string, bool>("name", false));
      this->fields_options_.insert(
          std::pair<std::string, bool>("support", false));

      this->fields_enabled_.insert(std::pair<std::string, bool>("ipv6", false));
      this->fields_enabled_.insert(
          std::pair<std::string, bool>("daemon", true));
    });
  }

 private:
  MOCK_METHOD(void, InitOptions, ());
};

TEST(TestArgParser, test_normal) {
  const char* argv[] = {"--ip",   "127.0.0.1", "--name",  "frannecki_test",
                        "--port", "8067",      "--daemon"};
  ArgumentParserTest parser;
  parser.DelegateToFake();
  EXPECT_TRUE(parser.Init(7, const_cast<char**>(argv)));

  EXPECT_FALSE(parser.GetBoolArg("ipv6"));
  EXPECT_TRUE(parser.GetBoolArg("daemon"));

  std::string result;
  parser.GetStringArg("ip", result);
  EXPECT_EQ(result, "127.0.0.1");
  parser.GetStringArg("name", result);
  EXPECT_EQ(result, "frannecki_test");
  parser.GetStringArg("port", result);
  EXPECT_EQ(result, "8067");
  EXPECT_FALSE(parser.GetStringArg("support", result));
}

TEST(TestArgParser, test_required) {
  const char* argv[] = {"frannecki_test", "--port", "8067", "--daemon"};
  ArgumentParserTest parser;
  parser.DelegateToFake();
  EXPECT_FALSE(parser.Init(4, const_cast<char**>(argv)));
}

TEST(TestArgParser, test_no_such_option) {
  const char* argv[] = {"--ip",   "127.0.0.1", "--name",   "frannecki_test",
                        "--port", "8067",      "--daemon", "--author"};
  ArgumentParserTest parser;
  parser.DelegateToFake();
  EXPECT_FALSE(parser.Init(4, const_cast<char**>(argv)));
}
