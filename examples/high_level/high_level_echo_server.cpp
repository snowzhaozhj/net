#include <net/tcp/tcp_server.hpp>

void MessageCallback(const net::TcpConnectionPtr &conn, const net::BufferPtr &buffer) {
  conn->Send(buffer->RetriveAll());
}

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::TcpServer server(&reactor, listen_addr);
  server.SetThreadNum(1);
  server.SetMessageCallback(MessageCallback);
  server.Start();
  reactor.Run();
}
