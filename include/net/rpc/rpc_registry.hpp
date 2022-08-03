#ifndef NET_INCLUDE_NET_RPC_RPC_REGISTRY_HPP_
#define NET_INCLUDE_NET_RPC_RPC_REGISTRY_HPP_

#include "net/tcp/tcp_server.hpp"
#include "net/rpc/rpc_publisher.hpp"
#include "net/rpc/rpc_service_manager.hpp"
#include "net/rpc/rpc_connection.hpp"

namespace net {

class RpcRegistry {
 public:
  RpcRegistry(Reactor *reactor, const InetAddress &listen_addr)
      : tcp_server_(reactor, listen_addr) {
    tcp_server_.SetConnectionCallback([this](const TcpConnectionPtr &conn) {
      HandleConnection(conn);
    });
    tcp_server_.SetMessageCallback([this](const TcpConnectionPtr &conn, const BufferPtr &buffer) {
      HandleMessage(conn, buffer);
    });
  }

  /// @note 请确保线程数大于0
  void SetThreadNum(int thread_num) {
    tcp_server_.SetThreadNum(thread_num);
  }

  void Start() {
    tcp_server_.Start();
  }

 private:
  void HandleConnection(const TcpConnectionPtr &conn) {
    if (conn->Connected()) {
      LOG_INFO("Connect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::SetRpcParseStatus(conn, RpcParseStatus::kParseHeader);
      RpcConnection::StartHeartBeat(conn);
    } else {
      LOG_INFO("Disconnect: {}", conn->GetPeerAddr().ToString());
      RpcConnection::StopHeartBeat(conn);
      rpc_publisher_.DeleteSubscriber(conn);
      rpc_service_manager_.RemoveConn(conn);
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
            case RpcMessageType::kRegisterRequest:
            case RpcMessageType::kDiscoverRequest:
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
            case RpcMessageType::kRegisterReply:
            case RpcMessageType::kRpcRequest:
            case RpcMessageType::kRpcReply:
            case RpcMessageType::kDiscoverReply:
            case RpcMessageType::kPublishRequest:
            case RpcMessageType::kSubscribeReply:
            case RpcMessageType::kUnSubscribeReply:
              conn->Shutdown();
              return;
          }
          break;
        } // kParseHeader
        case RpcParseStatus::kParseBody: {
          uint32_t len = RpcConnection::GetContentLength(conn);
          if (buffer->ReadableBytes() < len) return;  // 还没读取完当前报文的所有数据
          RpcMessageType msg_type = RpcConnection::GetRpcMessageType(conn);
          switch (msg_type) {
            case RpcMessageType::kRegisterRequest:
              rpc_service_manager_.Register(conn, buffer);
              break;
            case RpcMessageType::kDiscoverRequest:
              rpc_service_manager_.Discover(conn, buffer);
              break;
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
      }
      if (buffer->ReadableBytes() == 0) return;
    } // while (true)
  }

  TcpServer tcp_server_;
  RpcPublisher rpc_publisher_;
  RpcServiceManager rpc_service_manager_;
};

}

#endif // NET_INCLUDE_NET_RPC_RPC_REGISTRY_HPP_
