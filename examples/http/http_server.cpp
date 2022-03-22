#include <net/http/http_server.hpp>

void Handle(const net::HttpRequest &request, net::HttpReply &reply) {
  reply.SetContent("Hello, world");
}

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::HttpServer http_server(&reactor, listen_addr);
  http_server.SetThreadNum(8);
  http_server.SetHandleFunction(Handle);
  http_server.Start();
  reactor.Run();
}
