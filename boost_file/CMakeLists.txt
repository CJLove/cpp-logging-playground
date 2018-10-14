cmake_minimum_required(VERSION 3.1)

project(boost_file)

add_definitions( -DBOOST_LOG_DYN_LINK )

include_directories(${Boost_INCLUDE_DIRS})

set (boost_file_SRC
    file.cpp
)

add_executable( boost_file ${boost_file_SRC})

link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries( boost_file boost_log boost_log_setup boost_thread pthread )

