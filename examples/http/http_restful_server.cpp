#include <net/http/http_server.hpp>
#include <net/http/restful_handler.hpp>

class MyHandler : public net::RestfulHandler {
 public:
  MyHandler() = default;

  void Get(const net::HttpRequest &request, net::HttpReply &reply) override {
    reply.SetContent("Called Get Method");
  }
};

int main() {
  net::Reactor reactor;
  net::InetAddress listen_addr(9987);
  net::HttpServer http_server(&reactor, listen_addr);
  http_server.SetThreadNum(8);
  http_server.Handle("/get", MyHandler{});
  http_server.Start();
  reactor.Run();
}
