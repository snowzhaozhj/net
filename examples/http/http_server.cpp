#include <net/http/http_server.hpp>

void HandleRoot(const net::HttpRequest &request, net::HttpReply &reply) {
  reply.SetContent("Hello, world");
}

void HandleA(const net::HttpRequest &request, net::HttpReply &reply) {
  reply.SetContent("A");
}

void HandleB(const net::HttpRequest &request, net::HttpReply &reply) {
  reply.SetContent("B");
}

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::HttpServer http_server(&reactor, listen_addr);
  http_server.SetThreadNum(8);
  http_server.Handle("/", HandleRoot);
  http_server.Handle("/a", HandleA);
  http_server.Handle("/b", HandleB);
  http_server.Start();
  reactor.Run();
}
