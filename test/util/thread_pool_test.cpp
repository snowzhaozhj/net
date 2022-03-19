#include <net/util/thread_pool.hpp>

#include "net_test.hpp"

class ThreadPoolTest : public testing::Test {};

TEST_F(ThreadPoolTest, Run) {
  net::ThreadPool thread_pool(4);
  int total_num = 10000;
  std::atomic<int> num = 0;
  thread_pool.Start();
  for (int i = 0; i < total_num; ++i) {
    thread_pool.SubmitTask([&num] { ++num; });
  }
  thread_pool.Stop();
  EXPECT_EQ(num, total_num);
}
