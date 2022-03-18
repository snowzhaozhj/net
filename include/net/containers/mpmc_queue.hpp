#ifndef NET_INCLUDE_NET_CONTAINERS_MPMC_QUEUE_HPP_
#define NET_INCLUDE_NET_CONTAINERS_MPMC_QUEUE_HPP_

#include <concurrentqueue.h>

namespace net::containers {

template<typename T>
using MPMCQueue = moodycamel::ConcurrentQueue<T>;

} // namespace net::containers

#endif //NET_INCLUDE_NET_CONTAINERS_MPMC_QUEUE_HPP_
