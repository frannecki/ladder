#include <iostream>
#include <chrono>

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#include "ladtest.h"

namespace ladder {

UnitTest* UnitTest::instance() { return UnitTest::create(); }

void UnitTest::release() {
  if (instance_) {
    delete instance_;
    instance_ = nullptr;
  }
}

int UnitTest::Run() {
  std::cout << "[==========] Running " << num_tests_ << " tests from "
            << test_suites_.size() << " test suites.\n";
  int n_passed = 0;
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (auto iter = test_suites_.begin(); iter != test_suites_.end(); ++iter) {
    int n = iter->second->Run();
    if (n >= 0) n_passed += n;
    else {
      // assertion failure
      std::cout << "[  FAILED  ] Assertion failed. Tests listed below:\n";
      iter->second->ListFailedTests();
      return -1;
    }
  }
  std::chrono::steady_clock::time_point end =
      std::chrono::steady_clock::now();
  uint64_t elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  std::cout << "[==========] " << num_tests_ << " tests from "
            << test_suites_.size() << " test suites ran. (" << elapsed << " us in total)\n";
  std::cout << "[  PASSED  ] " << n_passed << " tests.\n";
  if (n_passed < num_tests_) {
    std::cout << "[  FAILED  ] " << num_tests_ - n_passed
              << " tests, listed below:\n";
    for (auto iter = test_suites_.begin(); iter != test_suites_.end(); ++iter) {
      iter->second->ListFailedTests();
    }
  }
  return 0;
}

void UnitTest::RegisterTest(TestInfo* info) {
  if (!info) return;
  if (test_ids_.find(info->test_id_) != test_ids_.end()) {
    info->setup_ = info->teardown_ = nullptr;
  } else {
    test_ids_.insert(info->test_id_);
  }

  if (test_suites_.find(info->test_suite_name_) == test_suites_.end()) {
    test_suites_.insert(std::pair<std::string, TestSuite*>(
        info->test_suite_name_, new TestSuite(info->test_suite_name_)));
  }
  auto iter = test_suites_.find(info->test_suite_name_);
  iter->second->RegisterTest(info);

  num_tests_ += 1;
}

UnitTest::UnitTest() : num_tests_(0) {}

UnitTest::~UnitTest() {
  for (auto iter = test_suites_.begin(); iter != test_suites_.end(); ++iter) {
    delete iter->second;
    iter->second = nullptr;
  }
}

UnitTest* UnitTest::create() {
  static UnitTest* instance = new UnitTest;
  return instance_ = instance;
}

UnitTest* UnitTest::instance_ = nullptr;

TestSuite::TestSuite(const std::string& suite_name) : suite_name_(suite_name) {}

TestSuite::~TestSuite() {
  for (size_t i = 0; i < tests_.size(); ++i) {
    delete tests_[i];
  }
}

int TestSuite::Run() {
  std::cout << "[----------] " << tests_.size() << " tests from " << suite_name_ << "\n";
  int n_passed = 0;
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  size_t cur = 0;
  for (; cur < tests_.size(); ++cur) {
    tests_[cur]->Run();
    if (tests_[cur]->test_object_->passed() == kPassStatus::kPassed) {
      n_passed += 1;
    } else if (tests_[cur]->test_object_->passed() ==
               kPassStatus::kAssertionFailed) {
      ++cur;
      break;
    }
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  uint64_t elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  std::cout << "[----------] " << cur << " tests from " << suite_name_
            << " (" << elapsed << " us in total)"
            << "\n";

  // return -1 on assertion failure
  return cur < tests_.size() ? -1 : n_passed;
}

void TestSuite::ListFailedTests() {
  for (size_t i = 0; i < tests_.size(); ++i) {
    if (tests_[i]->test_object_->passed() != kPassStatus::kPassed) {
      std::cout << "[  FAILED  ] " << tests_[i]->test_suite_name_ << "."
                << tests_[i]->test_name_ << ".\n ";
    }
  }
}

void TestSuite::RegisterTest(TestInfo* info) { tests_.push_back(info); }

Test::~Test() {};

void Test::SetUp() {}

void Test::TearDown() {};

void Test::Run() {
  SetUp();
  TestBody();
  TearDown();
}

TestInfo::TestInfo(const char* test_suite_name, const char* test_name,
                   const void* test_id, SetUpOrTestDownTestSuiteFp setup,
                   SetUpOrTestDownTestSuiteFp teardown, Test* test_object)
    : test_suite_name_(std::string(test_suite_name)),
      test_name_(std::string(test_name)),
      test_id_(test_id),
      setup_(setup),
      teardown_(teardown),
      test_object_(test_object) {}

TestInfo* RegisterTest(const char* teset_suite_name, const char* test_name,
                  const void* test_id, SetUpOrTestDownTestSuiteFp setup,
                  SetUpOrTestDownTestSuiteFp teardown, Test* test_object) {
  TestInfo* test_info = new TestInfo(teset_suite_name, test_name, test_id,
                                     setup, teardown, test_object);
  UnitTest::instance()->RegisterTest(test_info);
  return test_info;
}

TestInfo::~TestInfo() {
  if (teardown_)
    teardown_();  // TODO: assign to the last test of the same test suite
  delete test_object_;
  test_object_ = nullptr;
}

void TestInfo::Run() {
  if (setup_) setup_();
  std::cout << "[ RUN      ] "
            << test_suite_name_ << "." << test_name_ << "\n";
  
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  test_object_->Run();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  uint64_t elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  std::cout << ((test_object_->passed() == kPassStatus::kPassed)
                    ? "[       OK ] "
                    : "[  FAILED  ] ")
            << test_suite_name_ << "." << test_name_ << " (" << elapsed
            << " us)\n";
}

} // namespace ladder

int main(int argc, char** argv) {
  int ret = RUN_ALL_TESTS();
  ::ladder::UnitTest::release();
#if defined(_MSC_VER) && defined(_DEBUG)
  _CrtDumpMemoryLeaks();
#endif
  return ret;
}
