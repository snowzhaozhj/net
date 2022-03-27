#include <net/http/http_server.hpp>
#include <net/http/http_file_server.hpp>

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::HttpServer http_server(&reactor, listen_addr);
  http_server.SetThreadNum(8);
  http_server.Handle("/static/", net::HttpFileServer(".", "/static/"));
  http_server.Start();
  reactor.Run();
}
