#ifndef NET_INCLUDE_NET_REACTOR_WAKER_HPP_
#define NET_INCLUDE_NET_REACTOR_WAKER_HPP_

#include "net/reactor/channel.hpp"

#include <sys/eventfd.h>

namespace net {

namespace detail {

inline int CreateEventFd() {
  int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (fd < 0) {
    LOG_ERROR("eventfd() failed");
  }
  return fd;
}

} // namespace net::detail

/// 需要通过GetChannel接口将Channel注册到Poller上
class Waker {
 public:
  Waker() : channel_(detail::CreateEventFd()) {
    channel_.SetReadCallback([this] { HandleRead(); });
    channel_.EnableRead();
  }
  ~Waker() {
    ::close(channel_.GetFd());
  }

  Channel *GetChannel() { return &channel_; }

  void WakeUp() const {
    eventfd_t value = 1;
    if (::eventfd_write(channel_.GetFd(), value) < 0) {
      LOG_ERROR("eventfd_write() failed");
    }
  }

 private:
  void HandleRead() {
    eventfd_t value;
    if (::eventfd_read(channel_.GetFd(), &value) < 0) {
      LOG_ERROR("eventfd_read() failed");
    }
  }

  Channel channel_;
};

} // namespace net

#endif //NET_INCLUDE_NET_REACTOR_WAKER_HPP_
