#ifndef NET_INCLUDE_NET_REACTOR_REACTOR_HPP_
#define NET_INCLUDE_NET_REACTOR_REACTOR_HPP_

#include "net/reactor/poller.hpp"
#include "net/reactor/waker.hpp"
#include "net/reactor/timer_queue.hpp"
#include "net/containers/mpmc_queue.hpp"

namespace net {

class Reactor : noncopyable {
  inline static thread_local Reactor *reactor_tls = nullptr;
 public:
  using Task = std::function<void()>;
  using TimerId = TimerQueue::TimerId;

  static constexpr int kMaxTaskOnce = 500;      ///< 每次调用HandleTasks允许处理的最大任务量
  static constexpr int kDefaultPollMs = 10000;  ///< 每次轮循的默认时间为10s

  static Reactor *GetCurrent() { return reactor_tls; }

  Reactor()
      : stopped_(false),
        is_polling_(false),
        thread_id_(std::this_thread::get_id()) {
    NET_ASSERT(reactor_tls == nullptr);
    reactor_tls = this;

    poller_.UpdateChannel(timer_queue_.GetChannel());
    poller_.UpdateChannel(waker_.GetChannel());
  }
  ~Reactor() {
    NET_ASSERT(stopped_ == true);
    reactor_tls = nullptr;
  }

  void Run() {
    stopped_ = false;
    while (!stopped_) {
      bool handled_many = HandleTasks();
      int poll_time = handled_many ? 0 : kDefaultPollMs;
      is_polling_ = true;
      poller_.Poll(poll_time);
      is_polling_ = false;
    }
    // 处理任务队列中剩余的任务
    Task task;
    while (task_queue_.try_dequeue(task)) {
      task();
    }
  }

  void Stop() {
    stopped_ = true;
    waker_.WakeUp();
  }

  /// 向当前Reactor的任务队列中添加一个任务
  /// @note 允许多个线程同时调用该接口
  bool SubmitTask(Task &&task) {
    bool ret = task_queue_.enqueue(std::move(task));
    if (!InCurrentReactorThread() || !is_polling_) {
      // 如果任务是从其他线程添加的，那么立即将当前Reactor从轮循中唤醒，去处理任务
      // 如果当前Reactor不处于轮循状态，来了新的任务，那么下次轮循的时候，可以快点结束，尽快来处理任务
      waker_.WakeUp();
    }
    return ret;
  }

  // 以下接口都不是线程安全的，要么在Reactor绑定线程中调用，要么通过SubmitTask接口间接在Reactor绑定线程中调用

  TimerId AddTimerAt(TimePoint tp, TimerQueue::Task &&task) {
    return timer_queue_.AddTimer(tp, Duration(0), std::move(task));
  }
  TimerId AddTimerAfter(Duration dur, TimerQueue::Task &&task) {
    return timer_queue_.AddTimer(GetNow() + dur, Duration(0), std::move(task));
  }
  TimerId AddTimerEvery(Duration dur, TimerQueue::Task &&task) {
    return timer_queue_.AddTimer(GetNow() + dur, dur, std::move(task));
  }
  void CancleTimer(TimerId timer_id) {
    timer_queue_.CancleTimer(timer_id);
  }

  // 根据https://stackoverflow.com/questions/7058737/is-epoll-thread-safe, epoll是线程安全的
  // 也就代表以下函数是线程安全的
  // 以下函数只建议在框架内部使用
  void UpdateChannel(Channel *channel) {
    poller_.UpdateChannel(channel);
  }
  void RemoveChannel(Channel *channel) {
    poller_.RemoveChannel(channel);
  }

 private:
  /// @return 如果是在处理了kMaxTaskOnce个任务量之后退出的则返回true，否则返回false
  bool HandleTasks() {
    int num = 0;
    Task task;
    while (task_queue_.try_dequeue(task)) {
      task();
      if (++num > kMaxTaskOnce) return true;
    }
    return false;
  }
  /// @return 如果是在当前Reactor绑定的thread中则返回true，否则返回false
  [[nodiscard]] bool InCurrentReactorThread() const {
    return std::this_thread::get_id() == thread_id_;
  }

  containers::MPMCQueue<Task> task_queue_;  // TODO: 使用MPSC队列性能可能会更好
  TimerQueue timer_queue_;
  Waker waker_;
  Poller poller_;
  bool stopped_;
  bool is_polling_;             // poller正在轮询中
  std::thread::id thread_id_;   // 当前Reactor所绑定到的线程的ID
};

} // namespace net

#endif //NET_INCLUDE_NET_REACTOR_REACTOR_HPP_
