cmake_minimum_required(VERSION 3.1)

project(boost_console)

add_definitions( -DBOOST_LOG_DYN_LINK )

include_directories(${Boost_INCLUDE_DIRS})

set (boost_console_SRC
    console.cpp
)

add_executable( boost_console ${boost_console_SRC})

target_link_libraries( boost_console boost_log boost_log_setup boost_thread pthread )

