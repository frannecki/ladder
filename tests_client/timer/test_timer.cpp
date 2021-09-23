#include <iostream>
#include <functional>

#include <Timer.h>
#include <EventLoop.h>
#include <Logger.h>

static int tick_ = 0;
static int tick1_ = 0;

void PrintTick(int *tick) {
  std::cout << "Current tick is " << *tick << std::endl;
  *tick += 1;
}

void PrintTick1(int *tick) {
  std::cout << "Current tick1 is " << *tick << std::endl;
  *tick += 1;
}

int main(int argc, char** argv) {
  ladder::Logger::create();
  ladder::EventLoop loop;
  ladder::Timer *timer = new ladder::Timer(ladder::EventLoopPtr(&loop));
  ladder::Timer *timer1 = new ladder::Timer(ladder::EventLoopPtr(&loop));
  timer->SetTimerEventCallback(std::bind(PrintTick, &tick_));
  timer1->SetTimerEventCallback(std::bind(PrintTick1, &tick1_));
  timer->SetInterval(1000000);        // triggered once
  timer1->SetInterval(1000000, true); // triggered periodically
  loop.StartLoop();
  return 0;
}