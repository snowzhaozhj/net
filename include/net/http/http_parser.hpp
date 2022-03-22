#ifndef NET_INCLUDE_NET_HTTP_HTTP_PARSER_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_PARSER_HPP_

#include "net/http/http_request.hpp"
#include "net/http/http_reply.hpp"

namespace net {

namespace detail {

inline bool ParseRequestLine(const net::BufferPtr &buffer, HttpRequest &request) {
  auto pos = buffer->Find(' ');
  if (pos == nullptr) return false;
  request.SetMethod(net::ToHttpMethod(buffer->RetriveTo(pos)));
  buffer->HasRead(1);
  pos = buffer->Find(' ');
  if (pos == nullptr) return false;
  request.SetUrl(buffer->RetriveTo(pos));
  buffer->HasRead(1);
  pos = buffer->Search(kCRLF);
  if (pos == nullptr) return false;
  request.SetVersion(net::ToHttpVersion(buffer->RetriveTo(pos)));
  buffer->HasRead(kCRLFSize);
  return true;
}

inline bool ParseHeaders(const net::BufferPtr &buffer, HttpRequest &request) {
  auto end = buffer->Search(kCRLF);
  if (!end) return false;
  do {
    auto pos = buffer->Find(':');
    if (pos == nullptr) break;  // 读到了空行
    std::string key = buffer->RetriveTo(pos);
    buffer->HasRead(1);
    auto first_not_space = buffer->FindIf([](char c) { return !isspace(c); });
    buffer->HasRead(first_not_space - buffer->GetReadPtr());
    std::string value = buffer->RetriveTo(end);
    request.AddHeader(key, value);
    buffer->HasRead(kCRLFSize);
    end = buffer->Search(kCRLF);
    if (end == nullptr) return false;
  } while (true);
  return true;
}

} // namespace net::detail

inline bool Parse(const net::BufferPtr &buffer, HttpRequest &request) {
  if (!detail::ParseRequestLine(buffer, request)) return false;
  if (!detail::ParseHeaders(buffer, request)) return false;
  buffer->HasRead(kCRLFSize); // 读取空行
  request.SetContent(buffer->RetriveAll());
  return true;
}

// TODO: bool Parse(const net::BufferPtr &buffer, HttpReply &reply)

} // namespace net

#endif //NET_INCLUDE_NET_HTTP_HTTP_PARSER_HPP_
