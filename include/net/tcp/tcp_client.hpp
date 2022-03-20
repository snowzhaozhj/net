#ifndef NET_INCLUDE_NET_TCP_TCP_CLIENT_HPP_
#define NET_INCLUDE_NET_TCP_TCP_CLIENT_HPP_

#include "net/reactor/connector.hpp"
#include "net/tcp/tcp_connection.hpp"

namespace net {

class TcpClient {
 public:
  using ConnectionCallback = TcpConnection::ConnectionCallback;
  using MessageCallback = TcpConnection::MessageCallback;
  using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;

  TcpClient(Reactor *reactor, const InetAddress &server_addr)
      : reactor_(reactor),
        connector_(reactor, server_addr),
        connection_(nullptr),
        retry_(false),
        stopped_(false) {
    connector_.SetNewConnectionCallback([this](int conn_fd) {
      NewConnectionCallback(conn_fd);
    });
  }

  void SetRetry(bool retry) { retry_ = retry; }

  void SetConnectionCallback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
  }
  void SetMessageCallback(const MessageCallback &cb) {
    message_callback_ = cb;
  }
  void SetWriteCompleteCallback(const WriteCompleteCallback &cb) {
    write_complete_callback_ = cb;
  }

  void Connect() {
    stopped_ = false;
    connector_.Start();
  }
  /// 主动断开连接
  /// @note 非线程安全
  void Disconnect() {
    stopped_ = true;
    if (connection_) {
      connection_->Shutdown();
    }
  }
  /// 停止进行连接
  /// @note 非线程安全
  void Stop() {
    stopped_ = true;
    connector_.Stop();
  }
 private:
  void NewConnectionCallback(int conn_fd) {
    connection_ = std::make_shared<TcpConnection>();
    connection_->Init(reactor_, conn_fd);
    connection_->SetConnectionCallback(connection_callback_);
    connection_->SetMessageCallback(message_callback_);
    connection_->SetWriteCompleteCallback(write_complete_callback_);
    connection_->SetCloseCallback([this](const TcpConnectionPtr &conn) {
      conn->Destroy();
      connection_.reset();
      if (retry_ && !stopped_) {
        connector_.Restart();
      }
    });
    connection_->Establish();
  }
  Reactor *reactor_;
  Connector connector_;
  TcpConnectionPtr connection_;
  bool retry_;
  bool stopped_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
};

} // namespace net

#endif //NET_INCLUDE_NET_TCP_TCP_CLIENT_HPP_
