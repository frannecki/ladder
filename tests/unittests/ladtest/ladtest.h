#ifndef LADTEST_H
#define LADTEST_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef _MSC_VER
#ifdef _CONSOLE
#define LADDER_API
#elif defined(_WINDLL)
#define LADDER_API __declspec(dllexport)
#else
#define LADDER_API __declspec(dllimport)
#endif
#else
#define LADDER_API
#endif

#define TEST(test_suite_name, test_name)                    \
  LADTEST_TEST_(test_suite_name, test_name, ::ladder::Test, \
                ::ladder::GetTypeId<::ladder::Test>())

#define TEST_F(test_fixture, test_name)                \
  LADTEST_TEST_(test_fixture, test_name, test_fixture, \
                ::ladder::GetTypeId<test_fixture>())

#define LADTEST_TEST_CLASS_NAME_(test_suite_name, test_name) \
  ladtest_##test_suite_name##_##test_name

#define LADTEST_TEST_(test_suite_name, test_name, parent_class,                \
                      parent_class_id)                                         \
  class LADTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                   \
      : public parent_class {                                                  \
   public:                                                                     \
    LADTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() {}                  \
                                                                               \
   private:                                                                    \
    void TestBody() override;                                                  \
    static TestInfo* test_info_;                                               \
  };                                                                           \
                                                                               \
  TestInfo* LADTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::test_info_ = \
      ::ladder::RegisterTest(                                                  \
          #test_suite_name, #test_name, (parent_class_id),                     \
          ::ladder::TestSuiteApiResolver<parent_class>::GetSetUpTestSuite(),   \
          ::ladder::TestSuiteApiResolver<                                      \
              parent_class>::GetTearDownTestSuite(),                           \
          new LADTEST_TEST_CLASS_NAME_(test_suite_name, test_name)());         \
  void LADTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::TestBody()


/// Assertions.
#define ASSERT_EQ(val1, val2)                                               \
  {                                                                         \
    if (val1 != val2) {                                                     \
      passed_ = kPassStatus::kAssertionFailed;                              \
      BinaryAssertionFailure(#val1, #val2, __FILE__, __LINE__, val1, val2); \
      return;                                                               \
    }                                                                       \
  }
#define ASSERT_GT(val1, val2)                  \
  {                                            \
    if (val1 <= val2) {                        \
      passed_ = kPassStatus::kAssertionFailed; \
      return;                                  \
    }                                          \
  }
#define ASSERT_LT(val1, val2)                  \
  {                                            \
    if (val1 >= val2) {                        \
      passed_ = kPassStatus::kAssertionFailed; \
      return;                                  \
    }                                          \
  }
#define ASSERT_GE(val1, val2)                  \
  {                                            \
    if (val1 < val2) {                         \
      passed_ = kPassStatus::kAssertionFailed; \
      return;                                  \
    }                                          \
  }
#define ASSERT_LE(val1, val2)                  \
  {                                            \
    if (val1 > val2) {                         \
      passed_ = kPassStatus::kAssertionFailed; \
      return;                                  \
    }                                          \
  }
#define ASSERT_TRUE(expr)                      \
  {                                            \
    if (!(expr)) {                               \
      passed_ = kPassStatus::kAssertionFailed; \
      return;                                  \
    }                                          \
  }
#define ASSERT_FALSE(expr)                     \
  {                                            \
    if (expr) {                                \
      passed_ = kPassStatus::kAssertionFailed; \
      return;                                  \
    }                                          \
  }


#define EXPECT_EQ(val1, val2)                                               \
  {                                                                         \
    if (val1 != val2) {                                                     \
      passed_ = kPassStatus::kFailed;                                       \
      BinaryAssertionFailure(#val1, #val2, __FILE__, __LINE__, val1, val2); \
      return;                                                               \
    }                                                                       \
  }
#define EXPECT_GT(val1, val2)         \
  {                                   \
    if (val1 <= val2) {               \
      passed_ = kPassStatus::kFailed; \
      return;                         \
    }                                 \
  }
#define EXPECT_LT(val1, val2)         \
  {                                   \
    if (val1 >= val2) {               \
      passed_ = kPassStatus::kFailed; \
      return;                         \
    }                                 \
  }
#define EXPECT_GE(val1, val2)         \
  {                                   \
    if (val1 < val2) {                \
      passed_ = kPassStatus::kFailed; \
      return;                         \
    }                                 \
  }
#define EXPECT_LE(val1, val2)         \
  {                                   \
    if (val1 > val2) {                \
      passed_ = kPassStatus::kFailed; \
      return;                         \
    }                                 \
  }
#define EXPECT_TRUE(expr)             \
  {                                   \
    if (!(expr)) {                      \
      passed_ = kPassStatus::kFailed; \
      return;                         \
    }                                 \
  }
#define EXPECT_FALSE(expr)            \
  {                                   \
    if (expr) {                       \
      passed_ = kPassStatus::kFailed; \
      return;                         \
    }                                 \
  }



namespace ladder {

using SetUpOrTestDownTestSuiteFp = void (*)();

struct TestInfo;
class TestSuite;
class LADDER_API UnitTest {
 public:
  UnitTest(const UnitTest&) = delete;
  UnitTest& operator=(const UnitTest&) = delete;
  static UnitTest* instance();
  static void release();
  int Run();
  void RegisterTest(TestInfo* info);

 private:
  UnitTest();
  ~UnitTest();
  
  static UnitTest* create();
  static UnitTest* instance_;

  int num_tests_;
  std::set<const void*> test_ids_;
  std::map<std::string, TestSuite*> test_suites_;
};

enum class kPassStatus { kPassed, kFailed, kAssertionFailed };

class Test {

 public:
  Test() : passed_(kPassStatus::kPassed) {}
  virtual ~Test();
  static void SetUpTestSuite() {}
  static void TearDownTestSuite() {}
  void Run();
  enum kPassStatus passed() { return passed_; }

 protected:
  virtual void SetUp();
  virtual void TearDown();
  enum kPassStatus passed_;

 private:
  virtual void TestBody() = 0;
};

struct TestInfo {
  std::string test_suite_name_;
  std::string test_name_;
  const void* test_id_;
  SetUpOrTestDownTestSuiteFp setup_;
  SetUpOrTestDownTestSuiteFp teardown_;
  Test* test_object_;

  TestInfo(const char* test_suite_name, const char* test_name,
           const void* test_id, SetUpOrTestDownTestSuiteFp setup,
           SetUpOrTestDownTestSuiteFp teardown, Test* test_object);

  ~TestInfo();

  void Run();
};

class TestSuite {
 public:
  TestSuite(const std::string& suite_name);
  TestSuite(const TestSuite&) = delete;
  TestSuite& operator=(const TestSuite&) = delete;
   ~TestSuite();
  int Run();
  void ListFailedTests();
  void RegisterTest(TestInfo* info);

 private:
  std::string suite_name_;
  std::vector<TestInfo*> tests_;
};


template <typename T>
class TestType {
 public:
  static char id;
};

template <typename T>
char TestType<T>::id = 0;

template <typename T>
const void* GetTypeId() {
  return &TestType<T>::id;
}

inline SetUpOrTestDownTestSuiteFp GetNotDefaultOrNull(
    SetUpOrTestDownTestSuiteFp a, SetUpOrTestDownTestSuiteFp b) {
  return a == b ? nullptr : a;
}

template <typename T>
class TestSuiteApiResolver {
 public:
  
  static SetUpOrTestDownTestSuiteFp GetSetUpTestSuite() {
    return GetNotDefaultOrNull(&T::SetUpTestSuite, &Test::SetUpTestSuite);
  }

  static SetUpOrTestDownTestSuiteFp GetTearDownTestSuite() {
    return GetNotDefaultOrNull(&T::TearDownTestSuite, &Test::TearDownTestSuite);
  }
};


TestInfo* RegisterTest(const char* teset_suite_name, const char* test_name,
                       const void* test_id, SetUpOrTestDownTestSuiteFp setup,
                       SetUpOrTestDownTestSuiteFp teardown, Test* test_object);

template <typename T1, typename T2>
void BinaryAssertionFailure(const char* expr1, const char* expr2,
                            const char* file, int line, const T1& value1,
                            const T2& value2) {
  std::cerr << "Binary assertion for the following expressions failed at "
            << file << ":" << line << "\n";
  std::cerr << "\t" << expr1 << ", which is:\n";
  std::cerr << "\t\t" << value1 << "\n";
  std::cerr << "\t" << expr2 << ", which is:\n";
  std::cerr << "\t\t" << value2 << "\n";
}

} // namespace ladder

inline int RUN_ALL_TESTS() { return ::ladder::UnitTest::instance()->Run(); }

#endif
