#include <net/reactor/reactor_pool.hpp>

#include "net_test.hpp"

class ReactorPoolTest : public testing::Test {};

TEST_F(ReactorPoolTest, Run) {
  net::ReactorPool reactor_pool(4);
  int total_num = 10000;
  std::atomic<int> num = 0;
  reactor_pool.Start();
  for (int i = 0; i < total_num; ++i) {
    reactor_pool.SubmitTask([&num] { ++num; });
  }
  reactor_pool.Stop();
  EXPECT_EQ(num, total_num);
}
