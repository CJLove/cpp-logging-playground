cmake_minimum_required(VERSION 3.1)

project(spdlog_udp)

include_directories(/usr/local/include)

set (spdlog_udp_SRC
    udp.cpp
)

add_executable( spdlog_udp ${spdlog_udp_SRC})

target_link_libraries( spdlog_udp spdlog::spdlog pthread )

