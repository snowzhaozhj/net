#ifndef NET_INCLUDE_NET_HTTP_HTTP_ROUTE_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_ROUTE_HPP_

#include "net/http/http_request.hpp"
#include "net/http/http_reply.hpp"

namespace net {

// 非常简陋的实现，只支持绝对匹配
// TODO: 支持对结尾'/'的处理
// TODO: 采用RadixTree

namespace detail {

bool HasPrefix(std::string_view s, std::string_view prefix) {
  if (prefix.size() > s.size()) return false;
  return std::equal(prefix.begin(), prefix.end(),
                    s.begin());
}

} // namespace detail

class HttpRoute {
 public:
  using HandleFunction = std::function<void(const HttpRequest &, HttpReply &)>;

  HttpRoute() = default;

  void RegisterHandler(const std::string &path, const HandleFunction &handle_function) {
    path_to_handle_function_[path] = handle_function;
  }

  HandleFunction *GetHandler(const std::string &path) {
    auto pos = path_to_handle_function_.find(path);
    if (pos != path_to_handle_function_.end()) {
      return &(pos->second);
    }
    for (auto &[p, h]: path_to_handle_function_) {
      if (detail::HasPrefix(path, p)) {
        return &h;
      }
    }
    return nullptr;
  }

 private:
  std::unordered_map<std::string, HandleFunction> path_to_handle_function_;
};

} // namespace net

#endif //NET_INCLUDE_NET_HTTP_HTTP_ROUTE_HPP_
