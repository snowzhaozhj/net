#ifndef NET_INCLUDE_NET_DEFER_HPP_
#define NET_INCLUDE_NET_DEFER_HPP_

#include "net/concat.hpp"
#include "net/noncopyable.hpp"

#include <functional>

namespace net {

class Finally : public noncopyable {
 public:
  explicit Finally(std::function<void()> func) : func_(std::move(func)) {}
  ~Finally() {
    if (func_) {
      func_();
    }
  }
 private:
  std::function<void()> func_;
};

} // namespace net

#define defer(x) \
  auto NET_CONCAT(__defer_, __LINE__) = net::Finally([&] { x; });

#endif //NET_INCLUDE_NET_DEFER_HPP_
