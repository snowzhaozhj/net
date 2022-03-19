#ifndef NET_INCLUDE_NET_UTIL_THREAD_POOL_HPP_
#define NET_INCLUDE_NET_UTIL_THREAD_POOL_HPP_

#include "net/containers/mpmc_queue.hpp"

#include <condition_variable>

namespace net {

// TODO: 暂不提供绑定CPU功能

/// 请确保线程数大于0
class ThreadPool {
 public:
  using Task = std::function<void()>;

  explicit ThreadPool(int thread_num = 1)
      : thread_num_(thread_num),
        stopped_(false) {
  }

  void SetThreadNum(int thread_num) { thread_num_ = thread_num; }

  void Start() {
    for (int i = 0; i < thread_num_; ++i) {
      thread_vec_.emplace_back([this] { ThreadMain(); });
    }
  }

  void Stop() {
    stopped_.store(true, std::memory_order_release);
    cv_.notify_all();
    for (auto &thread: thread_vec_) {
      thread.join();
    }
  }

  bool SubmitTask(Task &&task) {
    int ret = task_queue_.enqueue(std::move(task));
    cv_.notify_one();
    return ret;
  }

 private:
  /// 线程池中每个线程将会运行的函数
  void ThreadMain() {
    while (true) {
      Task task;
      if (task_queue_.try_dequeue(task)) {
        task();
      } else {
        if (stopped_.load(std::memory_order_acquire)) break;
        std::unique_lock<std::mutex> ul(mutex_);
        cv_.wait(ul);
      }
    }
  }

  std::vector<std::thread> thread_vec_;
  int thread_num_;
  std::atomic<bool> stopped_;
  containers::MPMCQueue<Task> task_queue_;

  std::mutex mutex_;
  std::condition_variable cv_;
};

} // namespace net

#endif //NET_INCLUDE_NET_UTIL_THREAD_POOL_HPP_
