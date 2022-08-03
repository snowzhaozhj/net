#include <net/rpc/rpc_client.hpp>
#include <iostream>

void ConnectionCallback(net::RpcClient &client, const net::TcpConnectionPtr &conn) {
  if (conn->Connected()) {
    client.Call("Add", [](int result) {
      std::cout << "1:" << result << std::endl;
    }, 1, 2);
    client.Call("Add", [](int result) {
      std::cout << "2:" << result << std::endl;
    }, 99, 203);
    client.Call("Add", [](int result) {
      std::cout << "3:" << result << std::endl;
    }, -998, 999);
    client.Call("Add", [](int result) {
      std::cout << "4:" << result << std::endl;
    }, 999999, -998);
  }
}

int main() {
  net::Reactor reactor;
  net::InetAddress server_addr(9987);
  net::RpcClient client(&reactor, server_addr);
  client.SetConnectionCallback(ConnectionCallback);
  client.Connect();
  reactor.Run();
}
