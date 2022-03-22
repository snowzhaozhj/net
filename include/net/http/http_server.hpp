#ifndef NET_INCLUDE_NET_HTTP_HTTP_SERVER_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_SERVER_HPP_

#include "net/tcp/tcp_server.hpp"
#include "net/http/http_parser.hpp"

namespace net {

class HttpServer {
 public:
  using HandleFunction = std::function<void(const HttpRequest &, HttpReply &)>;

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

  void SetHandleFunction(const HandleFunction &handle_function) {
    handle_function_ = handle_function;
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
      if (handle_function_) {
        reply.SetStatusCode(HttpStatusCode::k200Ok);  // 默认返回200 OK
        handle_function_(request, reply);
      } else {
        reply.SetStatusCode(HttpStatusCode::k501NotImplemented);
      }
    } else {
      reply.SetStatusCode(HttpStatusCode::k400BadRequest);
    }
    conn->Send(reply.SerializedToString());
  }

  TcpServer tcp_server_;
  HandleFunction handle_function_;  // TODO: 添加路由
};

} // namespace net

#endif //NET_INCLUDE_NET_HTTP_HTTP_SERVER_HPP_
