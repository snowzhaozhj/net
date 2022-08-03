#ifndef NET_INCLUDE_NET_UTIL_CONTEXT_HPP_
#define NET_INCLUDE_NET_UTIL_CONTEXT_HPP_

#include <string>
#include <unordered_map>
#include <any>
#include <memory>

namespace net {

class Context {
 public:
  Context() = default;

  template<typename T>
  void Set(const std::string &key, T &&value) {
    data_[key] = std::forward<T>(value);
  }

  template<typename T>
  T *Get(const std::string &key) {
    auto it = data_.find(key);
    if (it == data_.end()) {
      return nullptr;
    }
    auto val = std::any_cast<T>(&it->second);
    return val;
  }

  void Remove(const std::string &key) {
    data_.erase(key);
  }

  bool Contains(const std::string &key) {
    auto it = data_.find(key);
    return it != data_.end();
  }

  void Clear() {
    data_.clear();
  }

 private:
  std::unordered_map<std::string, std::any> data_;
};

using ContextPtr = std::shared_ptr<Context>;

}

#endif // NET_INCLUDE_NET_UTIL_CONTEXT_HPP_
