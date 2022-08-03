#ifndef NET_INCLUDE_NET_RPC_RPC_CLIENT_HPP_
#define NET_INCLUDE_NET_RPC_RPC_CLIENT_HPP_

#include "net/tcp/tcp_client.hpp"
#include "net/rpc/rpc_connection.hpp"
#include "net/util/traits.hpp"

namespace net {

class RpcClient {
  using RpcCallback = std::function<void(const BufferPtr &)>;
 public:
  using ConnectionCallback = std::function<void(RpcClient &, const TcpConnectionPtr &)>;

  RpcClient(Reactor *reactor, const InetAddress &server_addr)
      : tcp_client_(reactor, server_addr),
        id_(0) {
    tcp_client_.SetConnectionCallback([this](const TcpConnectionPtr &conn) {
      HandleConnection(conn);
    });
    tcp_client_.SetMessageCallback([this](const TcpConnectionPtr &conn, const BufferPtr &msg) {
      HandleMessage(conn, msg);
    });
  }

  void SetConnectionCallback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
  }

  void Connect() {
    tcp_client_.Connect();
  }

  template<typename F, typename ...Args>
  void Call(const std::string &method, F callback, Args &&...args) {
    RpcRequestBody request;
    request.method = method;
    request.args = SerializeArgsToBuffer(std::forward<Args>(args)...);
    RpcConnection::SendRpcRequest(tcp_client_.GetConnection(), ++id_, request);
    id_to_rpc_callback_[id_] = [callback](const BufferPtr &buffer) {
      using TupleArgType = typename FunctionTraits<F>::TupleArgType;
      auto ret = GetReturnValueFromBuffer<TupleArgType>(buffer);
      std::apply(callback, ret);
    };
  }

 private:
  void HandleConnection(const TcpConnectionPtr &conn) {
    if (conn->Connected()) {
      LOG_INFO("Connect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseHeader);
      RpcConnection::StartHeartBeat(conn);
      if (connection_callback_) {
        connection_callback_(*this, conn);
      }
    } else {
      LOG_INFO("Disconnect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::StopHeartBeat(conn);
      if (connection_callback_) {
        connection_callback_(*this, conn);
      }
    }
  }
  void HandleMessage(const TcpConnectionPtr &conn, const BufferPtr &buffer) {
    LOG_INFO("Received: {}", std::string_view(buffer->GetReadPtr(), buffer->ReadableBytes()));
    while (true) {
      RpcParseStatus parse_status = RpcConnection::GetRpcParseStatus(conn);
      switch (parse_status) {
        case RpcParseStatus::kParseHeader: {
          if (buffer->ReadableBytes() < kRpcHeaderSize) return;
          Serializer serializer(buffer);
          auto rpc_header = Get<RpcHeader>(serializer);
          if (rpc_header.version != kRpcVersion) {
            conn->Shutdown();
            return;
          }
          switch (rpc_header.message_type) {
            case RpcMessageType::kHeartBeatRequest:
              RpcConnection::HandleHeartBeatRequest(conn);
              break;
            case RpcMessageType::kHeartBeatReply:
              RpcConnection::HandleHeartBeatReply(conn);
              break;
            case RpcMessageType::kRpcReply:
              RpcConnection::SetContentLength(conn, rpc_header.content_length);
              RpcConnection::SetRpcMessageType(conn, rpc_header.message_type);
              RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseBody);
              RpcConnection::SetRpcHeaderId(conn, rpc_header.id);
              break;
            default:
              conn->Shutdown();
              return;
          }
          break;
        }
        case RpcParseStatus::kParseBody: {
          uint32_t len = RpcConnection::GetContentLength(conn);
          if (buffer->ReadableBytes() < len) return;  // 还没读取完当前报文的所有数据
          RpcMessageType msg_type = RpcConnection::GetRpcMessageType(conn);
          switch (msg_type) {
            case RpcMessageType::kRpcReply: {
              uint32_t id = RpcConnection::GetRpcHeaderId(conn);
              NET_ASSERT(id_to_rpc_callback_.count(id));
              Serializer s(buffer);
              auto reply = Get<RpcReplyBody>(s);
              if (reply.error_code != RpcErrorCode::kSuccess) {
                LOG_ERROR("Rpc Failed");
                break;
              }
              id_to_rpc_callback_[id](reply.ret);
              break;
            }
            default:
              // 代码逻辑出错了
              LOG_FATAL("Invalid RpcMessageType");
          }
          RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseHeader);
          break;
        }
      } // switch(parse_status)
      if (buffer->ReadableBytes() == 0) return;
    } // while(true)
  }

  TcpClient tcp_client_;
  ConnectionCallback connection_callback_;
  std::unordered_map<uint32_t, RpcCallback> id_to_rpc_callback_;
  uint32_t id_;
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_CLIENT_HPP_
