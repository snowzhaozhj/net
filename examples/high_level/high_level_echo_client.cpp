#include <net/tcp/tcp_client.hpp>
#include <iostream>

void ConnectionCallback(const net::TcpConnectionPtr &conn) {
  if (conn->Connected()) {
    LOG_INFO("connected");
    conn->Send("Hello");
    conn->Shutdown();
//    std::string line;
//    while (std::getline(std::cin, line)) {
//      conn->Send(line);
//    }
  } else {
    LOG_INFO("disconnected");
  }
}

void MessageCallback(const net::TcpConnectionPtr &conn, const net::BufferPtr &buf) {
  LOG_INFO("{}", buf->RetriveAll());
}

int main() {
  net::Reactor reactor;
  net::InetAddress server_addr(9987);
  net::TcpClient client(&reactor, server_addr);
  client.SetConnectionCallback(ConnectionCallback);
  client.Connect();
  reactor.Run();
}
