#include <net/socket.hpp>

#include <iostream>

int main() {
  auto sock = net::NewTcpSocket();
  net::InetAddress server_addr("127.0.0.1", 9987);
  int ret = sock.Connect(server_addr);
  if (ret == -1) return 0;
  std::string line;
  while (getline(std::cin, line)) {
    auto buffer = std::make_shared<net::Buffer>(line);
    sock.Write(buffer);
  }
  return 0;
}
