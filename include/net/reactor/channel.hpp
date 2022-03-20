#ifndef NET_INCLUDE_NET_REACTOR_CHANNEL_HPP_
#define NET_INCLUDE_NET_REACTOR_CHANNEL_HPP_

#include <functional>
#include <sys/epoll.h>

namespace net {

class Channel {
  constexpr static int kNoneEvent = 0;
  constexpr static int kReadEvent = EPOLLIN | EPOLLPRI;
  constexpr static int kWriteEvent = EPOLLOUT;
 public:
  using EventCallback = std::function<void()>;
  enum class State {
    Add,
    Mod,
  };

  explicit Channel(int fd) : fd_(fd), events_(0), revents_(0), state_(State::Add) {}

  [[nodiscard]] int GetFd() const { return fd_; }

  void EnableRead() { events_ |= kReadEvent; }
  void EnableWrite() { events_ |= kWriteEvent; }
  void DisableRead() { events_ &= ~kReadEvent; }
  void DisableWrite() { events_ &= ~kWriteEvent; }
  void DisableAll() { events_ = kNoneEvent; }

  [[nodiscard]] bool WriteEnabled() const { return events_ & kWriteEvent; }
  [[nodiscard]] int GetEvents() const { return events_; }
  [[nodiscard]] bool IsNoneEvent() const { return events_ == kNoneEvent; }
  void SetREvents(int revents) { revents_ = revents; }

  void SetReadCallback(const EventCallback &cb) { read_callback_ = cb; }
  void SetWriteCallback(const EventCallback &cb) { write_callback_ = cb; }
  void SetCloseCallback(const EventCallback &cb) { close_callback_ = cb; }
  void SetErrorCallback(const EventCallback &cb) { error_callback_ = cb; }

  void SetState(State state) { state_ = state; }
  [[nodiscard]] State GetState() const { return state_; }

  void HandleEvents() {
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      if (read_callback_) {
        read_callback_();
      }
    }
    if (revents_ & EPOLLOUT) {
      if (write_callback_) {
        write_callback_();
      }
    }
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
      if (close_callback_) {
        close_callback_();
      }
    }
    if (revents_ & EPOLLERR) {
      if (error_callback_) {
        error_callback_();
      }
    }
  }

 private:
  int fd_;

  int events_;
  int revents_;

  EventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;

  State state_;
};

} // namespace net

#endif //NET_INCLUDE_NET_REACTOR_CHANNEL_HPP_
