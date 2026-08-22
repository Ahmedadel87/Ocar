#pragma once
#include <stdexcept>
#include <string>

inline void panic(const std::string& error_message) {
    throw std::runtime_error(error_message);
}
