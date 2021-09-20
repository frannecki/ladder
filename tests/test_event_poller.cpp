#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <EventPoller.h>

class EventPollerTest : public testing::Test {
  static void SetUpTestSuite() {
    poller = new ladder::EventPoller();
  }

  static ladder::EventPoller* poller;
};
ladder::EventPoller* EventPollerTest::poller = nullptr;

TEST(TestEventPoller, test_Wakeup) {
  ;
}

TEST(TestEventPoller, test2) {
  ;
}
