#ifndef NET_INCLUDE_NET_RPC_RPC_SERVICE_MANAGER_HPP_
#define NET_INCLUDE_NET_RPC_RPC_SERVICE_MANAGER_HPP_

#include "net/rpc/rpc_protocol.hpp"
#include "net/rpc/rpc_message_builder.hpp"
#include "net/tcp/tcp_connection.hpp"

#include <unordered_map>
#include <unordered_set>
#include <string>

namespace net {

class RpcServiceManager {
 public:
  RpcServiceManager() = default;

  void Register(const TcpConnectionPtr &conn, const BufferPtr &buffer) {
    Serializer serializer(buffer);
    auto request = Get<RpcRegisterRequestBody>(serializer);
    {
      std::lock_guard<std::mutex> lg(mutex_);
      for (const auto &method : request.methods) {
        method_to_ip_port_set_[method].emplace(request.ip_port);
      }
    }
    auto reply = RpcMessageBuilder::RegisterReply(0);
    conn->Send(reply);
  }

  void Discover(const TcpConnectionPtr &conn, const BufferPtr &buffer) {
    Serializer in(buffer);
    auto request = Get<RpcDiscoverRequestBody>(in);
    Serializer out;
    {
      std::lock_guard<std::mutex> lg(mutex_);
      if (method_to_ip_port_set_.count(request.method)) {
        out << method_to_ip_port_set_[request.method];
      } else {
        out << std::unordered_set<std::string>{};
      }
    }
    conn->Send(out.GetBuffer());
  }

  void RemoveConn(const TcpConnectionPtr &conn) {
    std::string ip_port = conn->GetPeerAddr().ToString();
    std::lock_guard<std::mutex> lg(mutex_);
    for (auto &[method, ip_port_set] : method_to_ip_port_set_) {
      ip_port_set.erase(ip_port);
    }
  }

 private:
  std::unordered_map<std::string, std::unordered_set<std::string>> method_to_ip_port_set_;
  std::mutex mutex_;
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_SERVICE_MANAGER_HPP_
