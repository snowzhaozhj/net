#include <net/tcp/tcp_server.hpp>

void ConnectionCallback(const net::TcpConnectionPtr &conn) {
  if (conn->Connected()) {
    LOG_INFO("connected");
  } else {
    LOG_INFO("disconnected");
  }
}

void MessageCallback(const net::TcpConnectionPtr &conn, const net::BufferPtr &buffer) {
  auto content = buffer->RetriveAll();
  LOG_INFO("{}", content);
  conn->Send(content);
}

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::TcpServer server(&reactor, listen_addr);
  server.SetThreadNum(1);
  server.SetMessageCallback(MessageCallback);
  server.SetConnectionCallback(ConnectionCallback);
  server.Start();
  reactor.Run();
}
