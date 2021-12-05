#include <map>

#include "ladtest/ladtest.h"

#include <ArgumentParser.h>

#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

using namespace ladder;

class ArgumentParserTest : public ladder::IArgumentParser {
 public:
  void InitOptions() override {
    this->fields_options_.insert(std::pair<std::string, bool>("ip", true));
    this->fields_options_.insert(std::pair<std::string, bool>("port", true));
    this->fields_options_.insert(std::pair<std::string, bool>("name", false));
    this->fields_options_.insert(
        std::pair<std::string, bool>("support", false));

    this->fields_enabled_.insert(std::pair<std::string, bool>("ipv6", false));
    this->fields_enabled_.insert(
        std::pair<std::string, bool>("daemon", true));
  }
};

TEST(TestArgParser, test_normal) {
  const char* argv[] = {"--ip",   "127.0.0.1", "--name",  "frannecki_test",
                        "--port", "8067",      "--daemon"};
  ArgumentParserTest parser;
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
  EXPECT_FALSE(parser.Init(4, const_cast<char**>(argv)));
}

TEST(TestArgParser, test_no_such_option) {
  const char* argv[] = {"--ip",   "127.0.0.1", "--name",   "frannecki_test",
                        "--port", "8067",      "--daemon", "--author"};
  ArgumentParserTest parser;
  EXPECT_FALSE(parser.Init(4, const_cast<char**>(argv)));
}


#ifdef _MSC_VER
int main(int argc, char** argv) {
#ifdef _MSC_VER
    WSADATA wsa_data;
    int ret = 0;
    if ((ret = WSAStartup(MAKEWORD(2, 2), &wsa_data)) != 0) {
        perror("WSAStartup failed");
        return -1;
    }
#endif
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
