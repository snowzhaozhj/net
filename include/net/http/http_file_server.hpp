#ifndef NET_INCLUDE_NET_HTTP_HTTP_FILE_SERVER_HPP_
#define NET_INCLUDE_NET_HTTP_HTTP_FILE_SERVER_HPP_

#include "net/log.hpp"
#include "net/http/http_request.hpp"
#include "net/http/http_reply.hpp"
#include "net/http/mime_types.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

// TODO: 支持Content-Range字段

namespace net {

namespace fs = std::filesystem;

namespace detail {

bool RemovePrefix(std::string &result, const std::string &path, std::string_view prefix) {
  if (prefix.size() > path.size()) return false;
  auto has_prefix = std::equal(prefix.begin(), prefix.end(),
                               path.begin());
  result = path.substr(prefix.size());
  return true;
}

};  // namespace net::detail

class HttpFileServer {
 public:
  explicit HttpFileServer(fs::path base_dir, std::string url_prefix = "/")
      : base_dir_(std::move(base_dir)),
        url_prefix_(std::move(url_prefix)) {
    NET_ASSERT(fs::is_directory(base_dir_));
    if (url_prefix_.empty() || url_prefix_.back() != '/') {
      url_prefix_.push_back('/');
    }
  }

  void operator()(const HttpRequest &request, HttpReply &reply) {
    std::string url = request.GetRouteUrl();
    std::string rel_path;
    if (!detail::RemovePrefix(rel_path, url, url_prefix_)) {
      if (!detail::RemovePrefix(rel_path, url + '/', url_prefix_)) {
        reply.SetStatusCode(HttpStatusCode::k404NotFound);
        return;
      }
    }
    auto file_path = base_dir_ / rel_path;  // 拼接出在文件系统中的路径
    if (!fs::exists(file_path)) {
      reply.SetStatusCode(HttpStatusCode::k404NotFound);
      return;
    }
    if (fs::is_directory(file_path)) {
      // 如果目录路径不以'/'结束的话，需要重定向到包含'/'的页面
      if (url.empty() || url.back() != '/') {
        LocalRedirect(url + '/', request.GetRawParams(), reply);
        return;
      }
      // 对于目录，首先查找目录下是否有index.html文件
      auto index_file = file_path / kIndexFile;
      if (fs::exists(index_file)) {
        ServeFile(index_file, reply);
      } else {
        // 不存在index.html的话，则发送一个展示目录下文件的html页面
        ServeDir(file_path, reply);
      }
    } else {
      ServeFile(file_path, reply);
    }
  }
 private:
  void ServeFile(const fs::path &file, HttpReply &reply) {
    std::string_view content_type = ToMimeType(file.extension().string());
    std::ifstream ifs(file);
    std::stringstream ss;
    ss << ifs.rdbuf();
    std::string content(ss.str());
    reply.SetContentType(content_type);
    reply.SetContent(content);
  }

  void ServeDir(const fs::path &dir, HttpReply &reply) {
    std::string content;
    content.append("<pre>");
    fs::directory_iterator it(dir);
    for (auto &f: it) {
      // TODO: 对特殊字符转义
      std::string f_url = f.path().filename().string();
      if (f.is_directory()) {
        f_url.append("/");
      }
      std::string f_name = f_url;
      content.append(fmt::format("<a href=\"{}\">{}</a>\n", f_url, f_name));
    }
    content.append("</pre>");
    reply.SetContentType("text/html; charset=utf-8");
    reply.SetContent(content);
  }

  void LocalRedirect(std::string_view new_path, std::string_view params, HttpReply &reply) {
    std::string p(new_path);
    if (!params.empty()) {
      p.push_back('?');
      p += params;
    }
    reply.SetHeader(kLocationField, p);
    reply.SetStatusCode(HttpStatusCode::k301MovedPermanently);
  }

  fs::path base_dir_;
  std::string url_prefix_;
};

} // namespace net

#endif //NET_INCLUDE_NET_HTTP_HTTP_FILE_SERVER_HPP_
