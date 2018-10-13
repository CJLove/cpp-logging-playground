cmake_minimum_required(VERSION 3.1)

project(spdlog_file)

include_directories(/usr/local/include)

set (spdlog_file_SRC
    file.cpp
)

add_executable( spdlog_file ${spdlog_file_SRC})

target_link_libraries( spdlog_file spdlog::spdlog pthread )

