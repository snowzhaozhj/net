#ifndef NET_INCLUDE_NET_UTIL_STRING_HPP_
#define NET_INCLUDE_NET_UTIL_STRING_HPP_

#include <string_view>

namespace net {

inline bool HasPrefix(std::string_view s, std::string_view prefix) {
  if (prefix.size() > s.size()) return false;
  return std::equal(prefix.begin(), prefix.end(),
                    s.begin());
}

/// @brief 如果s包含前缀prefix，则将去除前缀后的s写入result\n
/// @brief 如果s不包含前缀prefix，则不对result进行修改
/// @return 返回true表示s包含前缀prefix，返回false表示s不包含前缀prefix
inline bool RemovePrefix(std::string &result, std::string_view s, std::string_view prefix) {
  if (!HasPrefix(s, prefix)) return false;
  result = s.substr(prefix.size());
  return true;
}

}

#endif //NET_INCLUDE_NET_UTIL_STRING_HPP_