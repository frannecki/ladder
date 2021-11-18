#ifdef __unix__
#include <unistd.h>
#elif defined(_MSC_VER)
#include <windows.h>
#endif

#include <EventLoopThread.h>
#include <Logging.h>

using namespace ladder;

int main(int argc, char** argv) {
  Logger::create();
  EventLoopThread thread;
#ifdef __unix__
  ::sleep(1);
#endif
#ifdef _MSC_VER
  ::Sleep(1000);
#endif
  thread.Stop();
#ifdef __unix__
  ::usleep(100000);
#endif
#ifdef _MSC_VER
  ::Sleep(50);
#endif
  return 0;
}
