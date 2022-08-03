#ifndef NET_INCLUDE_NET_INET_ADDRESS_HPP_
#define NET_INCLUDE_NET_INET_ADDRESS_HPP_

#include "net/log.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <string_view>

namespace net {

/// IPv4地址
class InetAddress {
  static std::string ExtractIP(std::string_view ip_port) {
    auto pos = ip_port.find(':');
    return std::string(ip_port.data(), pos);
  }
  static uint16_t ExtractPort(std::string_view ip_port) {
    auto pos = ip_port.find(':');
    return std::atoi(ip_port.data() + pos + 1);
  }
 public:
  explicit InetAddress(uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
  }
  InetAddress(std::string_view ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip.data(), &addr_.sin_addr) != 1) {
      LOG_ERROR("inet_pton() failed");
    }
    addr_.sin_port = htons(port);
  }

  explicit InetAddress(std::string_view ip_port)
      : InetAddress(ExtractIP(ip_port), ExtractPort(ip_port)) {
  }

  explicit InetAddress(const struct sockaddr_in &addr) : addr_(addr) {}

  [[nodiscard]] const sockaddr_in &GetAddr() const { return addr_; }

  [[nodiscard]] std::string ToString() const {
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr_.sin_addr, host, sizeof(host));
    uint16_t port = ntohs(addr_.sin_port);
    return fmt::format("{}:{}", host, port);
  }

 private:
  struct sockaddr_in addr_{};
};

using SA = struct sockaddr;

inline SA *SACast(struct sockaddr_in *addr) {
  return reinterpret_cast<SA *>(addr);
}

inline const SA *SACast(const struct sockaddr_in *addr) {
  return reinterpret_cast<const SA *>(addr);
}

inline struct sockaddr_in GetLocalAddr(int fd) {
  struct sockaddr_in local_addr{};
  bzero(&local_addr, sizeof(local_addr));
  socklen_t addr_len = sizeof(local_addr);
  if (::getsockname(fd, SACast(&local_addr), &addr_len) < 0) {
    LOG_ERROR("getsockname() failed");
  }
  return local_addr;
}

inline struct sockaddr_in GetPeerAddr(int fd) {
  struct sockaddr_in peer_addr{};
  bzero(&peer_addr, sizeof(peer_addr));
  socklen_t addr_len = sizeof(peer_addr);
  if (::getpeername(fd, SACast(&peer_addr), &addr_len) < 0) {
    LOG_ERROR("getpeername() failed");
  }
  return peer_addr;
}

} // namespace net

#endif //NET_INCLUDE_NET_INET_ADDRESS_HPP_
