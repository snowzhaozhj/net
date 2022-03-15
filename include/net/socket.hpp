#ifndef NET_INCLUDE_NET_SOCKET_HPP_
#define NET_INCLUDE_NET_SOCKET_HPP_

#include "net/buffer.hpp"
#include "net/inet_address.hpp"

#include <unistd.h>
#include <sys/uio.h>

namespace net {

inline constexpr int kMaxSegmentSize = 65536;

/// IPv4 socket
class Socket {
 public:
  explicit Socket(int fd) : fd_(fd) {}

  [[nodiscard]] int GetFd() const { return fd_; }

  void SetReuseAddr(bool on) const {
    int optval = on ? 1 : 0;
    if (::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
      LOG_ERROR("setsocketopt() failed");
    }
  }

  void Bind(const InetAddress &addr) const {
    if (::bind(fd_, SACast(&addr.GetAddr()), sizeof(struct sockaddr_in)) == -1) {
      LOG_ERROR("bind() failed");
    }
  }

  void Listen() const {
    if (::listen(fd_, SOMAXCONN) == -1) {
      LOG_ERROR("listen() failed");
    }
  }

  [[nodiscard]] std::pair<Socket, InetAddress> Accept() const {
    struct sockaddr_in peer_addr{};
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int fd = ::accept4(fd_, SACast(&peer_addr),
                       &addr_len, SOCK_CLOEXEC);
    if (fd == -1) {
      LOG_ERROR("accept4() failed");
    }
    return {Socket(fd), InetAddress(peer_addr)};
  }

  [[nodiscard]] std::pair<Socket, InetAddress> NonBlockAccept() const {
    struct sockaddr_in peer_addr{};
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int fd = ::accept4(fd_, SACast(&peer_addr),
                       &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd == -1) {
      LOG_ERROR("accept4() failed");
    }
    return {Socket(fd), InetAddress(peer_addr)};
  }

  int Connect(const InetAddress &addr) const {
    int ret = connect(fd_, SACast(&addr.GetAddr()), sizeof(struct sockaddr_in));
    if (ret == -1) {
      LOG_ERROR("connect() failed");
    }
    return ret;
  }

  ssize_t Read(const BufferPtr &buffer) const {
    char extrabuf[kMaxSegmentSize];
    struct iovec vec[2];
    auto writable_bytes = buffer->WritableBytes();
    vec[0].iov_base = buffer->GetWritePtr();
    vec[0].iov_len = writable_bytes;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    ssize_t n = ::readv(fd_, vec, 2);
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

  ssize_t Write(const BufferPtr &buffer) const {
    ssize_t n = ::write(fd_, buffer->GetReadPtr(), buffer->ReadableBytes());
    if (n < 0) {
      LOG_ERROR("write() failed");
    }
    return n;
  }

  void ShutDown(int how) const {
    if (::shutdown(fd_, how) < 0) {
      LOG_ERROR("shutdown() failed");
    }
  }

  void Close() const {
    if (::close(fd_) < 0) {
      LOG_ERROR("close() failed");
    }
  }

 private:
  int fd_;
};

Socket NewTcpSocket() {
  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  if (fd == -1) {
    LOG_FATAL("socket() failed");
  }
  return Socket(fd);
}

Socket NewNonBlockTcpSocket() {
  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (fd == -1) {
    LOG_FATAL("socket() failed");
  }
  return Socket(fd);
}

} // namespace net

#endif //NET_INCLUDE_NET_SOCKET_HPP_
