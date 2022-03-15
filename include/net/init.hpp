#ifndef NET_INCLUDE_NET_INIT_HPP_
#define NET_INCLUDE_NET_INIT_HPP_

#include "net/concat.hpp"

#define NET_INIT_FUNCNAME(name) NET_CONCAT(NET_INIT_, name)

#define NET_INIT(name) inline void __attribute__((constructor)) NET_INIT_FUNCNAME(name)()

#endif //NET_INCLUDE_NET_INIT_HPP_
