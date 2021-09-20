#include <unistd.h>

#include <thread>

#include <Logger.h>
#include <EventPoller.h>

#define private public
#include <EventLoop.h>

using namespace ladder;

void WakeupCallback() {
  LOG_INFO("wakeup callback");
}

int main(int argc, char** argv) {
  Logger::create();
  EventLoop loop;  
  loop.SetWakeupCallback(WakeupCallback);
  std::thread th(&EventLoop::StartLoop, &loop);

  LOG_WARNING("First wakeup");
  loop.poller_->Wakeup();  

  usleep(40000);
  loop.poller_->Wakeup();
  
  th.join();
  return 0;
}
