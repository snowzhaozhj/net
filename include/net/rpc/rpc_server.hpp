#ifndef NET_INCLUDE_NET_RPC_RPC_SERVER_HPP_
#define NET_INCLUDE_NET_RPC_RPC_SERVER_HPP_

#include "net/tcp/tcp_server.hpp"
#include "net/tcp/tcp_client.hpp"
#include "net/rpc/rpc_connection.hpp"
#include "net/rpc/rpc_publisher.hpp"
#include "net/rpc/rpc_router.hpp"

namespace net {

class RpcServer {
 public:
  RpcServer(Reactor *reactor,
            const InetAddress &listen_addr,
            const InetAddress &registry_addr)
      : tcp_server_(reactor, listen_addr),
        registry_client_(reactor, registry_addr) {
    tcp_server_.SetConnectionCallback([this](const TcpConnectionPtr &conn) {
      HandleServerConnection(conn);
    });
    tcp_server_.SetMessageCallback([this](const TcpConnectionPtr &conn, const BufferPtr &msg) {
      HandleServerMessage(conn, msg);
    });
    registry_client_.SetConnectionCallback([this](const TcpConnectionPtr &conn) {
      HandleClientConnection(conn);
    });
    registry_client_.SetMessageCallback([this](const TcpConnectionPtr &conn, const BufferPtr &msg) {
      HandleClientMessage(conn, msg);
    });
  }

  /// @note 请确保线程数大于0
  void SetThreadNum(int thread_num) {
    tcp_server_.SetThreadNum(thread_num);
  }

  template<typename F>
  void Register(std::string name, F func) {
    rpc_router_.Register(std::move(name), func);
  }

  void Start() {
    registry_client_.Connect();
    tcp_server_.Start();
  }

  template<typename T>
  void Publish(const std::string &topic, const T &data) {
    rpc_publisher_.Publish(topic, data);
  }

 private:
  void HandleServerConnection(const TcpConnectionPtr &conn) {
    if (conn->Connected()) {
      LOG_INFO("Connect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseHeader);
      RpcConnection::StartHeartBeat(conn);
    } else {
      LOG_INFO("Disconnect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::StopHeartBeat(conn);
      rpc_publisher_.DeleteSubscriber(conn);
    }
  }
  void HandleServerMessage(const TcpConnectionPtr &conn, const BufferPtr &buffer) {
    LOG_INFO("Received: {}", std::string_view(buffer->GetReadPtr(), buffer->ReadableBytes()));
    while (true) {
      RpcParseStatus parse_status = RpcConnection::GetRpcParseStatus(conn);
      switch (parse_status) {
        case RpcParseStatus::kParseHeader: {
          if (buffer->ReadableBytes() < kRpcHeaderSize) return;
          Serializer serializer(buffer);
          auto rpc_header = Get<RpcHeader>(serializer);
          if (rpc_header.version != kRpcVersion) {
            return;
          }
          switch (rpc_header.message_type) {
            case RpcMessageType::kHeartBeatRequest:
              RpcConnection::HandleHeartBeatRequest(conn);
              break;
            case RpcMessageType::kHeartBeatReply:
              RpcConnection::HandleHeartBeatReply(conn);
              break;
            case RpcMessageType::kRpcRequest:
              RpcConnection::SetRpcHeaderId(conn, rpc_header.id);
            case RpcMessageType::kSubscribeRequest:
            case RpcMessageType::kUnSubscribeRequest:
              RpcConnection::SetContentLength(conn, rpc_header.content_length);
              RpcConnection::SetRpcMessageType(conn, rpc_header.message_type);
              RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseBody);
              break;
              // content为空的且无需处理的报文报文类型
            case RpcMessageType::kPublishReply:
              break;
              // 不合法的报文类型
            case RpcMessageType::kRpcReply:
            case RpcMessageType::kRegisterReply:
            case RpcMessageType::kRegisterRequest:
            case RpcMessageType::kDiscoverRequest:
            case RpcMessageType::kDiscoverReply:
            case RpcMessageType::kPublishRequest:
            case RpcMessageType::kSubscribeReply:
            case RpcMessageType::kUnSubscribeReply:
              return;
          } // switch rpc_header.msg_type
          break;
        } // kParseHeader
        case RpcParseStatus::kParseBody: {
          uint32_t len = RpcConnection::GetContentLength(conn);
          if (buffer->ReadableBytes() < len) return;  // 还没读取完当前报文的所有数据
          RpcMessageType msg_type = RpcConnection::GetRpcMessageType(conn);
          switch (msg_type) {
            case RpcMessageType::kRpcRequest: {
              Serializer serializer(buffer);
              auto rpc_request = Get<RpcRequestBody>(serializer);
              auto rpc_reply = rpc_router_.Call(rpc_request);
              RpcConnection::SendRpcReply(conn, rpc_reply);
              break;
            }
            case RpcMessageType::kSubscribeRequest: {
              std::string topic = buffer->Retrive(len);
              rpc_publisher_.Subscribe(conn, topic);
              break;
            }
            case RpcMessageType::kUnSubscribeRequest: {
              std::string topic = buffer->Retrive(len);
              rpc_publisher_.UnSubscribe(conn, topic);
              break;
            }
            default:
              // 代码逻辑出错了
              LOG_FATAL("Invalid RpcMessageType");
          }
          RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseHeader);
          break;
        } // kParseBody
      } // switch parse_status
      if (buffer->ReadableBytes() == 0) return;
    } // while(true)
  }

  void HandleClientConnection(const TcpConnectionPtr &conn) {
    if (conn->Connected()) {
      LOG_INFO("Connect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseHeader);
      RpcConnection::StartHeartBeat(conn);
      RpcConnection::SendRegisterRequest(conn, rpc_router_.GetAllMethods());
    } else {
      LOG_INFO("Disconnect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::StopHeartBeat(conn);
    }
  }
  void HandleClientMessage(const TcpConnectionPtr &conn, const BufferPtr &buffer) {
    LOG_INFO("Received: {}", std::string_view(buffer->GetReadPtr(), buffer->ReadableBytes()));
    while (true) {
      RpcParseStatus parse_status = RpcConnection::GetRpcParseStatus(conn);
      switch (parse_status) {
        case RpcParseStatus::kParseHeader: {
          if (buffer->ReadableBytes() < kRpcHeaderSize) return;
          Serializer serializer(buffer);
          auto rpc_header = Get<RpcHeader>(serializer);
          if (rpc_header.version != kRpcVersion) {
            return;
          }
          switch (rpc_header.message_type) {
            case RpcMessageType::kHeartBeatRequest:
              RpcConnection::HandleHeartBeatRequest(conn);
              break;
            case RpcMessageType::kHeartBeatReply:
              RpcConnection::HandleHeartBeatReply(conn);
              break;
            case RpcMessageType::kRegisterReply:
              break;
            default:
              return;
          }
          break;
        }
        default:
          return;
      }
      if (buffer->ReadableBytes() == 0) return;
    }
  }

  TcpServer tcp_server_;
  TcpClient registry_client_;
  RpcRouter rpc_router_;
  RpcPublisher rpc_publisher_;
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_SERVER_HPP_
