cmake_minimum_required(VERSION 3.1)

project(spdlog_tcp)

include_directories(/usr/local/include /usr/local/include/spdlog )

set (spdlog_tcp_SRC
    tcp.cpp
)

add_executable( spdlog_tcp ${spdlog_tcp_SRC})

target_link_libraries( spdlog_tcp spdlog::spdlog Threads::Threads )

