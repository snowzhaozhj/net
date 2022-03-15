cmake_minimum_required(VERSION 3.16)
project(net
        VERSION 0.0.1
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/googletest)

set(NET_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(NET_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(NET_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
set(NET_EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)

add_library(net)
target_sources(net
        PUBLIC
            "${NET_INC_DIR}/net/concat.hpp"
            "${NET_INC_DIR}/net/init.hpp"
            "${NET_INC_DIR}/net/defer.hpp"
            "${NET_INC_DIR}/net/log.hpp"
            "${NET_INC_DIR}/net/noncopyable.hpp"
            "${NET_INC_DIR}/net/buffer.hpp"
            "${NET_INC_DIR}/net/inet_address.hpp"
            "${NET_INC_DIR}/net/socket.hpp"
        PRIVATE
            "${NET_SRC_DIR}/net.cpp"
        )
target_include_directories(net
        PUBLIC
            "${NET_INC_DIR}"
        )
target_link_libraries(net spdlog::spdlog)

add_executable(net_test
        "${NET_TEST_DIR}/log_test.cpp"
        "${NET_TEST_DIR}/buffer_test.cpp"
        "${NET_TEST_DIR}/net_test.cpp"
        )
target_link_libraries(net_test net gtest)

add_subdirectory(${NET_EXAMPLE_DIR})