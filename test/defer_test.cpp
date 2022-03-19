#include <net/defer.hpp>

#include "net_test.hpp"

class DeferTest : public testing::Test {};

TEST_F(DeferTest, Defer) {
  bool defer_called = false;
  {
    defer(defer_called = true);
    EXPECT_FALSE(defer_called);
  }
  EXPECT_TRUE(defer_called);
}

TEST_F(DeferTest, DeferOrder) {
  int counter = 0;
  int a = 0, b = 0, c = 0;
  {
    defer(a = ++counter);
    defer(b = ++counter);
    defer(c = ++counter);
  }
  ASSERT_EQ(a, 3);
  ASSERT_EQ(b, 2);
  ASSERT_EQ(c, 1);
}
