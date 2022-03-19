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

SA *SACast(struct sockaddr_in *addr) {
  return reinterpret_cast<SA *>(addr);
}

const SA *SACast(const struct sockaddr_in *addr) {
  return reinterpret_cast<const SA *>(addr);
}

} // namespace net

#endif //NET_INCLUDE_NET_INET_ADDRESS_HPP_
