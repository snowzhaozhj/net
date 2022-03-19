#ifndef NET_INCLUDE_NET_REACTOR_REACTOR_POOL_HPP_
#define NET_INCLUDE_NET_REACTOR_REACTOR_POOL_HPP_

#include "net/reactor/reactor.hpp"

namespace net {

class ReactorPool : noncopyable {
 public:
  explicit ReactorPool(int thread_num = 1)
      : thread_num_(thread_num),
        next_(0) {
  }

  /// 设置ReactorPool内部的线程数，请确保thread_num > 0
  void SetThreadNum(int thread_num) {
    NET_ASSERT(thread_num > 0);
    thread_num_ = thread_num;
  }

  /// @note 非线程安全
  void Start() {
    thread_vec_.reserve(thread_num_);
    reactor_vec_.reserve(thread_num_);
    for (int i = 0; i < thread_num_; ++i) {
      thread_vec_.emplace_back([this] {
        Reactor reactor;
        reactor_vec_.push_back(&reactor);
        reactor.Run();
      });
    }
    while (reactor_vec_.size() < thread_num_);  // 通过自旋，等待所有Reactor构造完成
  }

  /// 向ReactorPool中的所有Reactor提交一个Stop任务，并等待所有线程结束
  /// @note 线程安全
  void Stop() {
    for (auto reactor: reactor_vec_) {
      reactor->SubmitTask([reactor] { reactor->Stop(); });
    }
    for (auto &thread: thread_vec_) {
      thread.join();
    }
  }

  /// 向ReactorPool提交任务, 将会采用RoundRobin算法来给内部的Reactor分配任务
  /// @note 非线程安全
  bool SubmitTask(Reactor::Task &&task) {
    return GetNextReactor()->SubmitTask(std::move(task));
  }
 private:
  /// 使用RoundRobin算法获取下一个Reactor
  /// @note 非线程安全
  Reactor *GetNextReactor() {
    Reactor *reactor = reactor_vec_[next_];
    next_ = (next_ == thread_num_ - 1) ? 0 : (next_ + 1);
    return reactor;
  }

  std::vector<std::thread> thread_vec_;
  std::vector<Reactor *> reactor_vec_;
  int thread_num_;
  int next_;
};

} // namespace net

#endif //NET_INCLUDE_NET_REACTOR_REACTOR_POOL_HPP_
