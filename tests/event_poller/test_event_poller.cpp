#ifndef _MSC_VER
#include <unistd.h>

#include <thread>

#include <EventPoller.h>
#include <Logging.h>

#define private public
#include <EventLoop.h>

using namespace ladder;

void WakeupCallback() { LOG_INFO("wakeup callback"); }

int main(int argc, char** argv) {
  Logger::create();
  EventLoop loop;
  loop.set_wakeup_callback(WakeupCallback);
  std::thread th(&EventLoop::StartLoop, &loop);

  LOG_WARNING("First wakeup");
  loop.poller_->Wakeup();

  usleep(40000);
  loop.poller_->Wakeup();

  th.join();
  return 0;
}
#endif
