#include <iostream>
#include <functional>

#include <Timer.h>
#include <EventLoop.h>
#include <Logger.h>

static int tick_ = 0;

void PrintTick(int *tick) {
  std::cout << "Current tick is " << *tick << std::endl;
  *tick += 1;
}

int main(int argc, char** argv) {
  ladder::Logger::create();
  ladder::EventLoop loop;
  ladder::Timer *timer = new ladder::Timer(ladder::EventLoopPtr(&loop));
  timer->SetTimerEventCallback(std::bind(PrintTick, &tick_));
  timer->SetInterval(1000000);
  loop.StartLoop();
  return 0;
}
