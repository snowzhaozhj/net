add_executable(low_level_echo_server low_level/low_level_echo_server.cpp)
target_link_libraries(low_level_echo_server net)
add_executable(low_level_echo_client low_level/low_level_echo_client.cpp)
target_link_libraries(low_level_echo_client net)

add_executable(middle_level_echo_server middle_level/middle_level_echo_server.cpp)
target_link_libraries(middle_level_echo_server net)
add_executable(middle_level_echo_client middle_level/middle_level_echo_client.cpp)
target_link_libraries(middle_level_echo_client net)

add_executable(high_level_echo_server high_level/high_level_echo_server.cpp)
target_link_libraries(high_level_echo_server net)
add_executable(high_level_echo_client high_level/high_level_echo_client.cpp)
target_link_libraries(high_level_echo_client net)

add_executable(http_server http/http_server.cpp)
target_link_libraries(http_server net)
add_executable(http_static_server http/http_static_server.cpp)
target_link_libraries(http_static_server net)
add_executable(http_restful_server http/http_restful_server.cpp)
target_link_libraries(http_restful_server net)
