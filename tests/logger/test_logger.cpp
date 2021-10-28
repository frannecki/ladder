#ifdef __unix__
#include <unistd.h>
#endif
#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <Logging.h>

using namespace ladder;

int main() {
  // Logger::create("./test_logger.log", kLogError);
  while (1) {
    LOG_FATAL("This is a fatal log message.");
    LOG_INFO("Greeting: Are You OK?");
#ifdef __unix__
    ::usleep(100000);
#endif
#ifdef _MSC_VER
    ::Sleep(100);
#endif
    LOGF_FATAL("This is a fatal log message. %03d %s %.6f", 53, "areyouok?",
               0.65);
    LOGF_INFO("Greeting: Are You OK? %u", 98);
  }
  Logger::release();
  return 0;
}
