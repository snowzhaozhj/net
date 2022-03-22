#ifndef NET_INCLUDE_NET_HTTP_HTTP_REPLY_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_REPLY_HPP_

#include "net/buffer.hpp"
#include "net/http/common.hpp"

#include <map>

namespace net {

class HttpReply {
 public:
  HttpReply() : version_(HttpVersion::Invalid), status_code_(HttpStatusCode::kUnknown) {}

  [[nodiscard]] HttpVersion GetVersion() const { return version_; }
  void SetVersion(HttpVersion version) { version_ = version; }

  [[nodiscard]] HttpStatusCode GetStatusCode() const { return status_code_; }
  void SetStatusCode(HttpStatusCode status_code) { status_code_ = status_code; }

  [[nodiscard]] std::string GetStatusMessage() const { return net::ToString(status_code_); }

  [[nodiscard]] const std::string &GetHeader(const std::string &key,
                                             const std::string &default_value = "") const {
    auto pos = headers_.find(key);
    if (pos != headers_.end()) {
      return pos->second;
    } else {
      return default_value;
    }
  }
  void SetHeader(const std::string &key, const std::string &value) {
    headers_[key] = value;
  }

  [[nodiscard]] const std::string &GetContent() const { return content_; }
  void SetContent(const std::string &content) { content_ = content; }

  std::string SerializedToString() {
    AddContentLengthField();
    std::string str;
    str.append(net::ToString(version_));
    str.append(" ");
    str.append(std::to_string(static_cast<int>(status_code_)));
    str.append(" ");
    str.append(net::ToString(status_code_));
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
    AddContentLengthField();
    buffer->Append(net::ToString(version_));
    buffer->Append(" ");
    buffer->Append(std::to_string(static_cast<int>(status_code_)));
    buffer->Append(" ");
    buffer->Append(net::ToString(status_code_));
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
  void AddContentLengthField() {
    headers_["Content-Length"] = fmt::format("{}", content_.size());
  }

  HttpVersion version_;
  HttpStatusCode status_code_;
  std::map<std::string, std::string> headers_;
  std::string content_;
};

}

#endif //NET_INCLUDE_NET_HTTP_HTTP_REPLY_HPP_
