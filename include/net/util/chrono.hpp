#ifndef NET_INCLUDE_NET_UTIL_CHRONO_HPP_
#define NET_INCLUDE_NET_UTIL_CHRONO_HPP_

#include <chrono>

namespace net {

using TimePoint = std::chrono::system_clock::time_point;
using Duration = std::chrono::system_clock::duration;

TimePoint GetNow() {
  return std::chrono::system_clock::now();
}

constexpr timespec ToTimespec(Duration duration) {
  using namespace std::chrono;
  auto secs = duration_cast<seconds>(duration);
  duration -= secs;
  return timespec{secs.count(), duration.count()};
}

constexpr timespec ToTimespec(TimePoint tp) {
  using namespace std::chrono;
  auto secs = time_point_cast<seconds>(tp);
  auto ns = time_point_cast<nanoseconds>(tp) - time_point_cast<nanoseconds>(secs);
  return timespec{secs.time_since_epoch().count(), ns.count()};
}

} // namespace net

#endif //NET_INCLUDE_NET_UTIL_CHRONO_HPP_
