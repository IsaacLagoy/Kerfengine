#pragma once

#include <string>


// ANSI escape codes for colored text printing
constexpr std::string ANSI_RESET = "\033[0m";
constexpr std::string ANSI_RED = "\033[31m";
constexpr std::string ANSI_GREEN = "\033[32m";
constexpr std::string ANSI_YELLOW = "\033[33m";

// Math const
constexpr float INFINITY_FLOAT = std::numeric_limits<float>::infinity();