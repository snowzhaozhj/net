#include <net/socket.hpp>

int main() {
  auto sock = net::NewTcpSocket();
  net::InetAddress listen_addr(9987);
  sock.Bind(listen_addr);
  sock.Listen();
  LOG_INFO("Listening {}", listen_addr.ToString());
  while (true) {
    auto [conn_sock, peer_addr] = sock.Accept();
    LOG_INFO("new connection from {}", peer_addr.ToString());
    auto buffer = std::make_shared<net::Buffer>();
    while (conn_sock.Read(buffer) > 0) {
      conn_sock.Write(buffer);
      buffer->Reset();
    }
  }
  sock.Close();
}

