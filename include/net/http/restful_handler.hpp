#ifndef NET_INCLUDE_NET_HTTP_RESTFUL_HANDLER_HPP_
#define NET_INCLUDE_NET_HTTP_RESTFUL_HANDLER_HPP_

#include "net/http/http_request.hpp"
#include "net/http/http_reply.hpp"

namespace net {

class RestfulHandler {
 public:
  RestfulHandler() = default;
  virtual ~RestfulHandler() = default;

  void operator()(const HttpRequest &request, HttpReply &reply) {
    switch (request.GetMethod()) {
      case HttpMethod::Get:
        return Get(request, reply);
      case HttpMethod::Head:
        return Head(request, reply);
      case HttpMethod::Post:
        return Post(request, reply);
      case HttpMethod::Put:
        return Put(request, reply);
      case HttpMethod::Delete:
        return Delete(request, reply);
      case HttpMethod::Connect:
        return Connect(request, reply);
      case HttpMethod::Options:
        return Options(request, reply);
      case HttpMethod::Trace:
        return Trace(request, reply);
      case HttpMethod::Patch:
        return Patch(request, reply);
      default:
        break;
    }
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }

  virtual void Get(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Head(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Post(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Put(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Delete(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Connect(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Options(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Trace(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
  virtual void Patch(const HttpRequest &request, HttpReply &reply) {
    reply.SetStatusCode(HttpStatusCode::k405MethodNotAllowed);
  }
};

}

#endif //NET_INCLUDE_NET_HTTP_RESTFUL_HANDLER_HPP_
