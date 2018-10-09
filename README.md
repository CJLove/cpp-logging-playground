# cpp-logging-playground

This repo includes sample code using various C++ logging frameworks.

Requirements:
- Filtering by various logging levels (e.g. info, warn, err, debug, fatal)
- Support logging to stdout/stderr (for use in containers)
- Support logging to file with rotation support (for use outside of containers)
- Thread-safe
- Type-safe
- Customizable logging formats
- Extensible for user-defined types
- Async support

## spdlog/fmt
Dependencies:
* https://github.com/gabime/spdlog (header-only)
* https://github.com/fmtlib/fmt

## boost::log
Dependencies: boost