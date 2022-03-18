#ifndef NET_INCLUDE_NET_SOCKET_HPP_
#define NET_INCLUDE_NET_SOCKET_HPP_

#include "net/buffer.hpp"
#include "net/inet_address.hpp"

#include <unistd.h>
#include <sys/uio.h>

namespace net {

inline constexpr int kMaxSegmentSize = 65536;

inline void SetReuseAddr(int fd, bool on) {
  int optval = on ? 1 : 0;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
    LOG_ERROR("setsocketopt() failed");
  }
}

inline void Bind(int fd, const InetAddress &addr) {
  if (::bind(fd, SACast(&addr.GetAddr()), sizeof(struct sockaddr_in)) == -1) {
    LOG_ERROR("bind() failed");
  }
}

inline void Listen(int fd) {
  if (::listen(fd, SOMAXCONN) == -1) {
    LOG_ERROR("listen() failed");
  }
}

inline std::pair<int, InetAddress> Accept(int fd) {
  struct sockaddr_in peer_addr{};
  socklen_t addr_len = sizeof(struct sockaddr_in);
  int conn_fd = ::accept4(fd, SACast(&peer_addr),
                          &addr_len, SOCK_CLOEXEC);
  if (conn_fd == -1) {
    LOG_ERROR("accept4() failed");
  }
  return {conn_fd, InetAddress(peer_addr)};
}

inline std::pair<int, InetAddress> NonBlockAccept(int fd) {
  struct sockaddr_in peer_addr{};
  socklen_t addr_len = sizeof(struct sockaddr_in);
  int conn_fd = ::accept4(fd, SACast(&peer_addr),
                          &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (conn_fd == -1) {
    LOG_ERROR("accept4() failed");
  }
  return {conn_fd, InetAddress(peer_addr)};
}

inline int Connect(int fd, const InetAddress &addr) {
  int ret = connect(fd, SACast(&addr.GetAddr()), sizeof(struct sockaddr_in));
  if (ret == -1) {
    LOG_ERROR("connect() failed");
  }
  return ret;
}

inline ssize_t Read(int fd, const BufferPtr &buffer) {
  char extrabuf[kMaxSegmentSize];
  struct iovec vec[2];
  auto writable_bytes = buffer->WritableBytes();
  vec[0].iov_base = buffer->GetWritePtr();
  vec[0].iov_len = writable_bytes;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);
  ssize_t n = ::readv(fd, vec, 2);
  if (n < 0) {
    LOG_ERROR("readv() failed");
  } else if (n <= writable_bytes) {
    buffer->HasWritten(n);
  } else {
    buffer->HasWritten(writable_bytes);
    buffer->Append(extrabuf, n - writable_bytes);
  }
  return n;
}

inline ssize_t Write(int fd, const BufferPtr &buffer) {
  ssize_t n = ::write(fd, buffer->GetReadPtr(), buffer->ReadableBytes());
  if (n < 0) {
    LOG_ERROR("write() failed");
  }
  return n;
}

inline void ShutDown(int fd, int how) {
  if (::shutdown(fd, how) < 0) {
    LOG_ERROR("shutdown() failed");
  }
}

inline void Close(int fd) {
  if (::close(fd) < 0) {
    LOG_ERROR("close() failed");
  }
}

inline int NewTcpSocketFd() {
  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  if (fd == -1) {
    LOG_FATAL("socket() failed");
  }
  return fd;
}

inline int NewNonBlockTcpSocketFd() {
  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (fd == -1) {
    LOG_FATAL("socket() failed");
  }
  return fd;
}

/// IPv4 socket
class Socket {
 public:
  explicit Socket(int fd) : fd_(fd) {}

  [[nodiscard]] int GetFd() const { return fd_; }

  void SetReuseAddr(bool on) const {
    net::SetReuseAddr(fd_, on);
  }

  void Bind(const InetAddress &addr) const {
    net::Bind(fd_, addr);
  }

  void Listen() const {
    net::Listen(fd_);
  }

  [[nodiscard]] std::pair<Socket, InetAddress> Accept() const {
    auto[conn_fd, peer_addr] = net::Accept(fd_);
    return {Socket(conn_fd), peer_addr};
  }

  [[nodiscard]] std::pair<Socket, InetAddress> NonBlockAccept() const {
    auto[conn_fd, peer_addr] = net::NonBlockAccept(fd_);
    return {Socket(conn_fd), peer_addr};
  }

  int Connect(const InetAddress &addr) const {
    return net::Connect(fd_, addr);
  }

  ssize_t Read(const BufferPtr &buffer) const {
    return net::Read(fd_, buffer);
  }

  ssize_t Write(const BufferPtr &buffer) const {
    return net::Write(fd_, buffer);
  }

  void ShutDown(int how) const {
    net::ShutDown(fd_, how);
  }

  void Close() const {
    net::Close(fd_);
  }

 private:
  int fd_;
};

inline Socket NewTcpSocket() {
  return Socket(NewTcpSocketFd());
}

inline Socket NewNonBlockTcpSocket() {
  return Socket(NewNonBlockTcpSocketFd());
}

} // namespace net

#endif //NET_INCLUDE_NET_SOCKET_HPP_
