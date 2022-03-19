#include <net/client/connector.hpp>
#include <iostream>

void NewConnectionCallback(int fd) {
  auto buffer = std::make_shared<net::Buffer>();
  std::string line;
  while (std::getline(std::cin, line)) {
    buffer->Append(line);
    net::Write(fd, buffer);
    buffer->Reset();
  }
}

int main() {
  net::Reactor reactor;
  net::InetAddress server_addr(9987);
  net::Connector connector(&reactor, server_addr);
  connector.SetNewConnectionCallback(NewConnectionCallback);
  connector.Start();
  reactor.Run();
}
