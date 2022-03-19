#ifndef NET_INCLUDE_NET_SERVER_ACCEPTOR_HPP_
#define NET_INCLUDE_NET_SERVER_ACCEPTOR_HPP_

#include "net/reactor/reactor.hpp"

namespace net {

/// 监听某个InetAddress, 通过调用Listen()接口将Acceptor注册到Reactor中
class Acceptor : noncopyable {
 public:
  using NewConnectionCallback = std::function<void(int conn_fd, const InetAddress &peer_addr)>;

  Acceptor(Reactor *reactor, const InetAddress &listen_addr)
      : reactor_(reactor),
        channel_(NewNonBlockTcpSocketFd()) {
    net::SetReuseAddr(channel_.GetFd(), true);
    net::Bind(channel_.GetFd(), listen_addr);
    channel_.SetReadCallback([this] { HandleRead(); });
  }

  void SetNewConnectionCallback(NewConnectionCallback &&cb) {
    new_connection_callback_ = std::move(cb);
  }

  /// 开启监听，并将Acceptor中的Channel注册到Reactor中
  /// @note 线程安全(可在另一个线程调用该函数)
  void Listen() {
    net::Listen(channel_.GetFd());
    channel_.EnableRead();
    reactor_->UpdateChannel(&channel_);
  }
 private:
  void HandleRead() {
    auto[conn_fd, peer_addr] = net::Accept(channel_.GetFd());
    if (conn_fd >= 0) {
      new_connection_callback_(conn_fd, peer_addr);
    } else {
      net::Close(conn_fd);
    }
  }

  Reactor *reactor_;
  Channel channel_;
  NewConnectionCallback new_connection_callback_;
};

} // namespace net

#endif //NET_INCLUDE_NET_SERVER_ACCEPTOR_HPP_
