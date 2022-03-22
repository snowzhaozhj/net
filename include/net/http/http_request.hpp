#ifndef NET_INCLUDE_NET_HTTP_HTTP_REQUEST_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_REQUEST_HPP_

#include "net/buffer.hpp"
#include "net/http/common.hpp"

#include <map>
#include <string>
#include <sstream>

namespace net {

class HttpRequest {
 public:
  HttpRequest() : method_(HttpMethod::Invalid), version_(HttpVersion::Invalid) {}

  [[nodiscard]] HttpMethod GetMethod() const { return method_; }
  void SetMethod(HttpMethod method) { method_ = method; }

  [[nodiscard]] const std::string &GetUrl() const { return url_; }
  void SetUrl(const std::string &url) { url_ = url; }

  [[nodiscard]] std::string GetRouteUrl() const {
    auto pos = url_.find('?');
    return (pos == std::string::npos) ? url_ : url_.substr(0, pos);
  }

  [[nodiscard]] HttpVersion GetVersion() const { return version_; }
  void SetVersion(HttpVersion version) { version_ = version; }

  [[nodiscard]] const std::string &GetHeader(const std::string &key,
                                             const std::string &default_value = "") const {
    auto pos = headers_.find(key);
    if (pos != headers_.end()) {
      return pos->second;
    } else {
      return default_value;
    }
  }
  void AddHeader(const std::string &key, const std::string &value) {
    headers_[key] = value;
  }

  [[nodiscard]] const std::string &GetContent() const { return content_; }
  void SetContent(const std::string &content) { content_ = content; }

  std::string SerializedToString() {
    std::string str;
    str.append(net::ToString(method_));
    str.append(" ");
    str.append(url_);
    str.append(" ");
    str.append(net::ToString(version_));
    str.append(kCRLF);
    for (auto &[key, value]: headers_) {
      str.append(key);
      str.append(": ");
      str.append(value);
      str.append(kCRLF);
    }
    str.append(kCRLF);
    str.append(content_);
    return str;
  }

  void SerializedToString(const BufferPtr &buffer) {
    buffer->Append(net::ToString(method_));
    buffer->Append(" ");
    buffer->Append(url_);
    buffer->Append(" ");
    buffer->Append(net::ToString(version_));
    buffer->Append(kCRLF);
    for (auto &[key, value]: headers_) {
      buffer->Append(key);
      buffer->Append(": ");
      buffer->Append(value);
      buffer->Append(kCRLF);
    }
    buffer->Append(kCRLF);
    buffer->Append(content_);
  }

 private:
  HttpMethod method_;
  std::string url_;
  HttpVersion version_;
  std::map<std::string, std::string> headers_;
  std::string content_;
};

} // namespace net

#endif //NET_INCLUDE_NET_HTTP_HTTP_REQUEST_HPP_
