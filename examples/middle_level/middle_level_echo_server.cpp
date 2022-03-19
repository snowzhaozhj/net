#include <net/server/acceptor.hpp>

void NewConnectionCallback(int conn_fd, const net::InetAddress &peer_addr) {
  LOG_INFO("new connection from {}", peer_addr.ToString());
  auto buffer = std::make_shared<net::Buffer>();
  while (net::Read(conn_fd, buffer) > 0) {
    net::Write(conn_fd, buffer);
    buffer->Reset();
  }
}

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::Acceptor acceptor(&reactor, listen_addr);
  acceptor.SetNewConnectionCallback(NewConnectionCallback);
  acceptor.Listen();
  reactor.Run();
}
