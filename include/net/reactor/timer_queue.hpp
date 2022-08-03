#ifndef NET_INCLUDE_NET_REACTOR_TIMER_QUEUE_HPP_
#define NET_INCLUDE_NET_REACTOR_TIMER_QUEUE_HPP_

#include "net/reactor/channel.hpp"
#include "net/util/chrono.hpp"

#include <set>
#include <sys/timerfd.h>

namespace net {

namespace detail {

inline int CreateTimerFd() {
  int fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (fd < 0) {
    LOG_ERROR("timerfd_create() failed");
  }
  return fd;
}

inline void ReadTimerFd(int timer_fd) {
  uint64_t howmany;
  if (::read(timer_fd, &howmany, sizeof(howmany)) != sizeof(howmany)) {
    LOG_ERROR("read() failed");
  }
}

inline void SetTimerFd(int timer_fd, TimePoint expiration) {
  struct itimerspec new_val{};
  struct itimerspec old_val{};
  bzero(&new_val, sizeof(new_val));
  bzero(&old_val, sizeof(old_val));
  new_val.it_value = ToTimespec(expiration - GetNow());
  if (::timerfd_settime(timer_fd, 0, &new_val, &old_val) < 0) {
    LOG_ERROR("timerfd_settime() failed");
  }
}

} // namespace net::detail

/// 需要通过GetChannel接口将Channel注册到Poller上
/// @note 非线程安全，请确保只在一个线程内调用某个TimeQueue的接口
class TimerQueue {
 public:
  using Task = std::function<void()>;
  using TimerId = int;

  struct Timer {
    Timer(Task &&task, TimePoint expiration, Duration interval, TimerId timer_id)
        : task(std::move(task)),
          expiration(expiration),
          interval(interval),
          timer_id(timer_id) {}

    Task task;              ///< 过期时需要运行的任务
    TimePoint expiration;   ///< 过期时间
    Duration interval;      ///< 重复间隔
    TimerId timer_id;       ///< 在TimerQueue中的唯一标识
  };

  TimerQueue()
      : channel_(detail::CreateTimerFd()),
        id_(0) {
    channel_.SetReadCallback([this] { HandleRead(); });
    channel_.EnableRead();
  }
  ~TimerQueue() {
    ::close(channel_.GetFd());
    for (auto &[tp, timer]: timer_set_) {
      delete timer;
    }
  }

  Channel *GetChannel() { return &channel_; }

  TimerId AddTimer(TimePoint expiration, Duration interval, Task &&task) {
    auto timer = new Timer(std::move(task), expiration, interval, ++id_);
    bool earliest_changed = Insert(timer);
    if (earliest_changed) {
      detail::SetTimerFd(channel_.GetFd(), expiration + interval);
    }
    return timer->timer_id;
  }
  // 考虑到CancleTimer的需求应该不是很多，所以实现为O(N)级别的复杂度，如果要优化的话，可以考虑用空间换时间
  void CancleTimer(TimerId timer_id) {
    auto pos = std::find_if(timer_set_.begin(), timer_set_.end(), [timer_id](const Entry &entry) {
      auto timer = entry.second;
      return timer->timer_id == timer_id;
    });
    if (pos != timer_set_.end()) {
      delete (*pos).second;
      timer_set_.erase(pos);
    }
  }
 private:
  void HandleRead() {
    detail::ReadTimerFd(channel_.GetFd());
    auto sentinel = std::make_pair(GetNow(), reinterpret_cast<Timer *>(UINTPTR_MAX));
    auto expired_end = timer_set_.lower_bound(sentinel);
    std::set<Entry> renew_timer_set;
    // 调用所有过期的Timer的Task
    for (auto it = timer_set_.begin(); it != expired_end; ++it) {
      auto timer = it->second;
      timer->task();
      if (timer->interval != Duration(0)) { // 是需要重复的Timer
        timer->expiration += timer->interval;
        renew_timer_set.emplace(timer->expiration, timer);
      } else {
        delete timer;
      }
    }
    timer_set_.erase(timer_set_.begin(), expired_end);
    // TODO: 使用set_union可能比insert更好, 或者考虑把renew_timer从set换成其他可以线性插入的容器
    timer_set_.insert(renew_timer_set.begin(), renew_timer_set.end());
    auto next_expiration = timer_set_.empty() ?
                           TimePoint{} :
                           timer_set_.begin()->second->expiration;
    if (next_expiration != TimePoint{}) {
      detail::SetTimerFd(channel_.GetFd(), next_expiration);
    }
  }
  /// @return 插入该timer后，如果最早过期时间发生了变化则返回true，否则返回false
  bool Insert(Timer *timer) {
    bool earliest_changed = false;
    if (timer_set_.empty() || timer->expiration < timer_set_.begin()->second->expiration) {
      earliest_changed = true;
    }
    timer_set_.emplace(timer->expiration, timer);
    return earliest_changed;
  }

  using Entry = std::pair<TimePoint, Timer *>;

  Channel channel_;
  std::set<Entry> timer_set_;
  TimerId id_;                      ///< 用于给Timer分配Id
};

} // namespace net

#endif //NET_INCLUDE_NET_REACTOR_TIMER_QUEUE_HPP_
