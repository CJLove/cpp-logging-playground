cmake_minimum_required(VERSION 3.1)

project(spdlog_console)

include_directories(/usr/local/include)

set (spdlog_console_SRC
    console.cpp
)

add_executable( spdlog_console ${spdlog_console_SRC})

target_link_libraries( spdlog_console spdlog::spdlog pthread )

