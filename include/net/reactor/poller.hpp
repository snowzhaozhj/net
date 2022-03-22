#ifndef NET_INCLUDE_NET_REACTOR_POLLER_HPP_
#define NET_INCLUDE_NET_REACTOR_POLLER_HPP_

#include "net/socket.hpp"
#include "net/reactor/channel.hpp"

namespace net {

namespace detail {

inline void EpollCtl(int epfd, int op, Channel *channel) {
  struct epoll_event event{};
  bzero(&event, sizeof(event));
  event.events = channel->GetEvents();
  event.data.ptr = channel;
  if (::epoll_ctl(epfd, op, channel->GetFd(), &event) < 0) {
    LOG_ERROR("epoll_ctl() failed");
  }
}

} // namespace net::detail

class Poller {
 public:
  static constexpr int kInitEventVecSize = 64;

  Poller()
      : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
        event_vec_(kInitEventVecSize) {}
  ~Poller() {
    net::Close(epoll_fd_);
  }

  void Poll(int timeout_ms) {
    int num_event = ::epoll_wait(epoll_fd_, event_vec_.data(), event_vec_.size(), timeout_ms);
    if (num_event < 0) {
      LOG_ERROR("epoll_wait() failed");
    }
    for (int i = 0; i < num_event; ++i) {
      auto channel = static_cast<Channel *>(event_vec_[i].data.ptr);
      channel->SetREvents(event_vec_[i].events);
      channel->HandleEvents();
    }
    if (num_event == event_vec_.size()) {
      event_vec_.resize(num_event * 2);
    }
  }
  void UpdateChannel(Channel *channel) const {
    if (channel->GetState() == Channel::State::Add) {
      detail::EpollCtl(epoll_fd_, EPOLL_CTL_ADD, channel);
      channel->SetState(Channel::State::Mod);
    } else {
      if (channel->IsNoneEvent()) {
        detail::EpollCtl(epoll_fd_, EPOLL_CTL_DEL, channel);
        channel->SetState(Channel::State::Add);
      } else {
        detail::EpollCtl(epoll_fd_, EPOLL_CTL_MOD, channel);
      }
    }
  }
  void RemoveChannel(Channel *channel) const {
    if (channel->GetState() == Channel::State::Mod) {
      detail::EpollCtl(epoll_fd_, EPOLL_CTL_DEL, channel);
      channel->SetState(Channel::State::Add);
    }
  }
 private:
  int epoll_fd_;
  std::vector<struct epoll_event> event_vec_;
};

} // namespace net

#endif //NET_INCLUDE_NET_REACTOR_POLLER_HPP_
