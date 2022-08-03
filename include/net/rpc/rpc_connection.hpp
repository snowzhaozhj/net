#ifndef NET_INCLUDE_NET_RPC_RPC_CONNECTION_HPP_
#define NET_INCLUDE_NET_RPC_RPC_CONNECTION_HPP_

#include "net/tcp/tcp_connection.hpp"
#include "net/rpc/rpc_message_builder.hpp"

namespace net {

class RpcConnection {
  using TimerId = TimerQueue::TimerId;
 public:

#define CONTEXT_GETTER_SETTER(cls, name, key) \
  static void Set##name(const TcpConnectionPtr &conn, cls obj) {  \
    conn->GetContext().Set(key, obj);                                    \
  } \
  static cls Get##name(const TcpConnectionPtr &conn)   {           \
    auto pobj = conn->GetContext().Get<cls>(key);                 \
    NET_ASSERT(pobj != nullptr);              \
    return *pobj;                                            \
  }

  CONTEXT_GETTER_SETTER(RpcParseStatus, RpcParseStatus, kRpcParseStatusKey)
  CONTEXT_GETTER_SETTER(uint32_t, ContentLength, kRpcContentLengthKey)
  CONTEXT_GETTER_SETTER(RpcMessageType, RpcMessageType, kRpcMessageTypeKey)
  CONTEXT_GETTER_SETTER(uint32_t, RpcHeaderId, kRpcHeaderIdKey)

#undef CONTEXT_GETTER_SETTER

  // 确保该函数在conn所在Reactor的线程中调用
  static void StartHeartBeat(const TcpConnectionPtr &conn) {
    Reactor *reactor = conn->GetReactor();
    TimerId hb_id = reactor->AddTimerEvery(kHeartBeatInterval, [reactor, conn]() {
      if (!conn->Connected()) return;
      LOG_INFO("Send HeartBeat");

      // 构造并发送HeartBeatRequest
      auto heart_beat_req = RpcMessageBuilder::HeartBeatRequest(0);
      conn->Send(heart_beat_req);

      // 开启Timeout计时器
      if (!conn->GetContext().Contains(kHeartBeatTimeoutTimerKey)) {
        // 如果之前没有设置过超时器
        auto timeout_id = reactor->AddTimerAfter(kHeartBeatTimeout, [conn]() {
          LOG_INFO("Timeouted");
          if (conn->Connected()) {
            conn->Shutdown();
          }
        });
        LOG_INFO("Start Timeout timer {} on {}", timeout_id, fmt::ptr(reactor));
        conn->GetContext().Set(kHeartBeatTimeoutTimerKey, timeout_id);
      }
    });
    conn->GetContext().Set(kHeartBeatSendTimerKey, hb_id);
  }

  // 确保该函数在conn所在Reactor的线程中调用
  static void StopHeartBeat(const TcpConnectionPtr &conn) {
    Reactor *reactor = conn->GetReactor();
    auto *send_id = conn->GetContext().Get<TimerId>(kHeartBeatSendTimerKey);
    if (send_id) {
      reactor->CancleTimer(*send_id);
    }
    auto *timeout_id = conn->GetContext().Get<TimerId>(kHeartBeatTimeoutTimerKey);
    if (timeout_id) {
      reactor->CancleTimer(*timeout_id);
    }
  }

  static void HandleHeartBeatRequest(const TcpConnectionPtr &conn) {
    auto heart_beat_reply = RpcMessageBuilder::HeartBeatReply(0);
    conn->Send(heart_beat_reply);
  }

  static void HandleHeartBeatReply(const TcpConnectionPtr &conn) {
    auto timeout_id = conn->GetContext().Get<TimerId>(kHeartBeatTimeoutTimerKey);
    NET_ASSERT(timeout_id != nullptr);
    auto reactor = conn->GetReactor();
    reactor->CancleTimer(*timeout_id);
    LOG_INFO("Stop Timeout Timer {} on {}", *timeout_id, fmt::ptr(reactor));
    conn->GetContext().Remove(kHeartBeatTimeoutTimerKey);
  }

  static void SendRpcRequest(const TcpConnectionPtr &conn, uint32_t id, const RpcRequestBody &request) {
    Serializer s;
    s << request;
    auto rpc_content = s.GetBuffer();
    auto rpc_header = RpcMessageBuilder::RpcRequestHeader(id, rpc_content);
    conn->Send(rpc_header);
    conn->Send(rpc_content);
  }

  static void SendRpcReply(const TcpConnectionPtr &conn, const RpcReplyBody &reply) {
    Serializer s;
    s << reply;
    auto rpc_content = s.GetBuffer();
    uint32_t id = RpcConnection::GetRpcHeaderId(conn);
    auto rpc_header = RpcMessageBuilder::RpcReplyHeader(id, rpc_content);
    conn->Send(rpc_header);
    conn->Send(rpc_content);
  }

  static void SendRegisterRequest(const TcpConnectionPtr &conn, const std::vector<std::string> &methods) {
    Serializer s;
    std::string ip_port = conn->GetLocalAddr().ToString();
    s << ip_port << methods;
    auto rpc_content = s.GetBuffer();
    auto rpc_header = RpcMessageBuilder::RegisterRequestHeader(0, rpc_content);
    conn->Send(rpc_header);
    conn->Send(rpc_content);
  }

 private:
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_CONNECTION_HPP_
