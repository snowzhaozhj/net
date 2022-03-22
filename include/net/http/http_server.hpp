#ifndef NET_INCLUDE_NET_HTTP_HTTP_SERVER_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_SERVER_HPP_

#include "net/tcp/tcp_server.hpp"
#include "net/http/http_parser.hpp"
#include "net/http/http_route.hpp"

namespace net {

class HttpServer {
 public:
  using HandleFunction = HttpRoute::HandleFunction;

  HttpServer(Reactor *reactor, const InetAddress &listen_addr)
      : tcp_server_(reactor, listen_addr) {
    tcp_server_.SetMessageCallback([this](const TcpConnectionPtr &conn, const BufferPtr &buffer) {
      MessageCallback(conn, buffer);
    });
  }

  /// @note 请确保线程数大于0
  void SetThreadNum(int thread_num) {
    tcp_server_.SetThreadNum(thread_num);
  }

  void Handle(const std::string &path, const HandleFunction &handle_function) {
    route_.RegisterHandler(path, handle_function);
  }

  void Start() {
    tcp_server_.Start();
  }

 private:
  void MessageCallback(const TcpConnectionPtr &conn, const BufferPtr &buffer) {
    HttpRequest request;
    HttpReply reply;
    reply.SetVersion(HttpVersion::Http11);
    if (net::Parse(buffer, request)) {
      auto handler = route_.GetHandler(request.GetRouteUrl());
      if (handler) {
        reply.SetStatusCode(HttpStatusCode::k200Ok);  // 默认返回200 OK
        (*handler)(request, reply);
      } else {
        reply.SetStatusCode(HttpStatusCode::k404NotFound);
      }
    } else {
      reply.SetStatusCode(HttpStatusCode::k400BadRequest);
    }
    conn->Send(reply.SerializedToString());
  }

  TcpServer tcp_server_;
  HttpRoute route_;
};

} // namespace net

#endif //NET_INCLUDE_NET_HTTP_HTTP_SERVER_HPP_
