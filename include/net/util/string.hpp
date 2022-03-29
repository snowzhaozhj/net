#ifndef NET_INCLUDE_NET_UTIL_STRING_HPP_
#define NET_INCLUDE_NET_UTIL_STRING_HPP_

#include <string_view>

namespace net {

bool HasPrefix(std::string_view s, std::string_view prefix) {
  if (prefix.size() > s.size()) return false;
  return std::equal(prefix.begin(), prefix.end(),
                    s.begin());
}

bool RemovePrefix(std::string &result, const std::string &s, std::string_view prefix) {
  if (prefix.size() > s.size()) return false;
  auto has_prefix = std::equal(prefix.begin(), prefix.end(),
                               s.begin());
  result = s.substr(prefix.size());
  return true;
}

}

#endif //NET_INCLUDE_NET_UTIL_STRING_HPP_