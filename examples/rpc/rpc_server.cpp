#include <net/rpc/rpc_server.hpp>

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::InetAddress registry_addr(9988);
  net::RpcServer server(&reactor, listen_addr, registry_addr);
  server.Register("Add", [](int a, int b) {
    return a + b;
  });
  server.Start();
  reactor.Run();
}
