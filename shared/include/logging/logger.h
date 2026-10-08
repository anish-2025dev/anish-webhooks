#pragma once

#include <iostream>
#include <string>

namespace logging {

class Logger {
public:
    void info(const std::string& message) {
        std::cout << "[INFO] " << message << '\n';
    }
};

}