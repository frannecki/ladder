#include <unistd.h>

#include <EventLoopThread.h>
#include <Logger.h>

using namespace ladder;

int main(int argc, char** argv) {
  Logger::create();
  EventLoopThread thread;
  sleep(1);
  thread.Stop();
  usleep(50000);
  return 0;
}
