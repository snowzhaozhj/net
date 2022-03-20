#ifndef NET_INCLUDE_NET_TCP_TCP_SERVER_HPP_
#define NET_INCLUDE_NET_TCP_TCP_SERVER_HPP_

#include "net/reactor/acceptor.hpp"
#include "net/reactor/reactor_pool.hpp"
#include "net/tcp/tcp_connection.hpp"
#include "net/util/object_pool.hpp"

namespace net {

class TcpServer : noncopyable {
 public:
  using ConnectionCallback = TcpConnection::ConnectionCallback;
  using MessageCallback = TcpConnection::MessageCallback;
  using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;

  TcpServer(Reactor *reactor, const InetAddress &listen_addr)
      : main_reactor_(reactor),
        acceptor_(reactor, listen_addr) {
    acceptor_.SetNewConnectionCallback([this](int conn_fd, const InetAddress &peer_addr) {
      NewConnectionCallback(conn_fd, peer_addr);
    });
  }

  /// @note 请确保线程数大于0
  void SetThreadNum(int thread_num) {
    sub_reactor_pool_.SetThreadNum(thread_num);
  }

  void SetConnectionCallback(const ConnectionCallback &cb) {
    connection_callback_ = cb;
  }
  void SetMessageCallback(const MessageCallback &cb) {
    message_callback_ = cb;
  }
  void SetWriteCompleteCallback(const WriteCompleteCallback &cb) {
    write_complete_callback_ = cb;
  }

  void Start() {
    acceptor_.Listen();
    sub_reactor_pool_.Start();
  }

 private:
  void NewConnectionCallback(int conn_fd, const InetAddress &peer_addr) {
    InetAddress local_addr(GetLocalAddr(conn_fd));
    TcpConnectionPtr connection = connection_pool_.GetShared();
    active_connection_set_.insert(connection);
    Reactor *sub_reactor = sub_reactor_pool_.GetNextReactor();
    connection->Init(sub_reactor, conn_fd, local_addr, peer_addr);
    connection->SetConnectionCallback(connection_callback_);
    connection->SetMessageCallback(message_callback_);
    connection->SetWriteCompleteCallback(write_complete_callback_);
    connection->SetCloseCallback([this](const TcpConnectionPtr &conn) {
      main_reactor_->SubmitTask([this, conn] {
        active_connection_set_.erase(conn); // 删除之后，TcpConnection将会自动回到对象池中
        conn->Destroy();
      });
    });
    sub_reactor->SubmitTask([connection] { connection->Establish(); });
  }

  Reactor *main_reactor_;
  Acceptor acceptor_;
  ReactorPool sub_reactor_pool_;
  ObjectPool<TcpConnection> connection_pool_;
  std::set<TcpConnectionPtr> active_connection_set_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
};

} // namespace net

#endif //NET_INCLUDE_NET_TCP_TCP_SERVER_HPP_
