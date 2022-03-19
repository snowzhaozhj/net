#include <net/reactor/reactor.hpp>

#include "net_test.hpp"

using namespace std::chrono_literals;

class ReactorTest : public testing::Test {
 public:
  ReactorTest() : reactor_(new net::Reactor) {}
  ~ReactorTest() override { delete reactor_; }
  net::Reactor *reactor_;
};

TEST_F(ReactorTest, GetCurrent) {
  EXPECT_EQ(net::Reactor::GetCurrent(), reactor_);
}

TEST_F(ReactorTest, SubmitTask) {
  int num = 0;
  std::vector<std::thread> threads;
  // 在其他线程添加若干任务
  for (int i = 0; i < 4; ++i) {
    threads.emplace_back([this, &num] {
      for (int j = 0; j < 1000; ++j) {
        reactor_->SubmitTask([&num] { ++num; });
      }
    });
  }
  // 在当前线程添加任务
  for (int i = 0; i < 100; ++i) {
    reactor_->SubmitTask([&num] { ++num; });
  }
  // 添加一个Stop任务
  std::thread t([this] {
    std::this_thread::sleep_for(1s); // 假定1s已经足够Reactor完成所有其他任务
    reactor_->SubmitTask([this] { reactor_->Stop(); });
  });
  reactor_->Run();
  EXPECT_EQ(num, 4100);
  for (auto &thread: threads) {
    thread.join();
  }
  t.join();
}

TEST_F(ReactorTest, Timer) {
  int num = 0;
  std::thread t([this, &num] {
    std::this_thread::sleep_for(100ms);
    EXPECT_EQ(num, 0);
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(num, 5);
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(num, 11);
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(num, 15);
    reactor_->SubmitTask([this] { reactor_->Stop(); });
  });
  reactor_->AddTimerAt(net::GetNow() + 300ms, [&num] { num += 1; });
  reactor_->AddTimerAfter(500ms, [&num] { num += 2; });
  reactor_->AddTimerEvery(300ms, [&num] { num += 4; });
  reactor_->Run();
  EXPECT_EQ(num, 15);
  t.join();
}
