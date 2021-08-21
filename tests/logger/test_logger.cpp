#include <Logger.h>
#include <unistd.h>

using namespace ladder;

int main() {
  Logger::create("./test_logger.log");
  while(1) {
    LOG_FATAL("This is a fatal log message.");
    LOG_INFO("Greeting: Are You OK?");
    ::usleep(100000);
  }
  Logger::release();
  return 0;
}
