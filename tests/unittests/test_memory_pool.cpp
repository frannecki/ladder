#include <gtest/gtest.h>

#include <MemoryPool.h>

class MemoryPoolTest : public testing::Test {
protected:
  // MemoryPoolImpl instance should only be
  // created once and released once
  static void SetUpTestSuite() {
    pool_ = ladder::MemoryPoolImpl::instance();
  }
  
  static void TearDownTestSuite() {
    if(pool_) {
      pool_ = nullptr;
      ladder::MemoryPoolImpl::release();
    }
  }

  static ladder::MemoryPoolImpl* pool_;
};

ladder::MemoryPoolImpl* MemoryPoolTest::pool_ = nullptr;

TEST_F(MemoryPoolTest, test_create) {
  ladder::MemoryPoolImpl* pool = ladder::MemoryPoolImpl::create();
  EXPECT_EQ(pool, pool_);
}

struct TestStruct {
  int a;
  char c;
  uint32_t b;

  TestStruct(int x = 0, char y = 0, uint32_t z = 0) : 
    a(x), c(y), b(z) {}
};

TEST_F(MemoryPoolTest, test_allocate) {
  int x = -3242;
  char y = 42;
  uint32_t z = 305;
  TestStruct* p = ladder::MemoryPool<TestStruct>::allocate(x, y, z);
  EXPECT_EQ(p->a, x);
  EXPECT_EQ(p->c, y);
  EXPECT_EQ(p->b, z);
}

TEST_F(MemoryPoolTest, test_allocate_n) {
  int num_allocate = 6;
  TestStruct* p = ladder::MemoryPool<TestStruct>::allocate_n(num_allocate);
  for(int i = 0; i < num_allocate; ++i) {
    EXPECT_EQ(p[i].a, 0);
    EXPECT_EQ(p[i].c, 0);
    EXPECT_EQ(p[i].b, 0);
  }
}
