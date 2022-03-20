#include <net/tcp/tcp_client.hpp>
#include <iostream>

void ConnectionCallback(const net::TcpConnectionPtr &conn) {
  if (conn->Connected()) {
    std::string line;
    while (std::getline(std::cin, line)) {
      conn->Send(line);
    }
  }
}

int main() {
  net::Reactor reactor;
  net::InetAddress server_addr(9987);
  net::TcpClient client(&reactor, server_addr);
  client.SetConnectionCallback(ConnectionCallback);
  client.Connect();
  reactor.Run();
}
