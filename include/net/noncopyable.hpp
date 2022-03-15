#ifndef NET_INCLUDE_NET_NONCOPYABLE_HPP_
#define NET_INCLUDE_NET_NONCOPYABLE_HPP_

namespace net {

class noncopyable {
 public:
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;
 public:
  noncopyable() = default;
  ~noncopyable() = default;
};

} // namespace net

#endif //NET_INCLUDE_NET_NONCOPYABLE_HPP_
