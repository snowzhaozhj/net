#ifndef NET_INCLUDE_NET_LOG_HPP_
#define NET_INCLUDE_NET_LOG_HPP_

#include "net/init.hpp"

#include <spdlog/spdlog.h>
#include <cstdlib>
#include <cassert>

#ifndef LOG_ENABLED
#define LOG_ENABLED 1
#endif

#if LOG_ENABLED

#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define LOG_FATAL(...) do { \
  SPDLOG_CRITICAL(__VA_ARGS__); \
  abort();                  \
} while (false)
#define NET_ASSERT(expr) do { \
  if (!(expr))                \
    LOG_FATAL("assert failed"); \
} while (false)

NET_INIT(log) {
  spdlog::set_pattern("[%Y-%m-%d %T.%e] [thread %t] [%l] [%@] [%!] %v");
}

#else

#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_ERROR(...)
#define LOG_FATAL(...) abort()
#defein NET_ASSERT(expr) assert(expr)

#endif

#endif //NET_INCLUDE_NET_LOG_HPP_
