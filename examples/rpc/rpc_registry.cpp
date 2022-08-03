#include <net/rpc/rpc_registry.hpp>

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9988);
  net::RpcRegistry registry(&reactor, listen_addr);
  registry.Start();
  reactor.Run();
}
