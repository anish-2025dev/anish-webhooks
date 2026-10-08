#pragma once

#include <cstdlib>
#include <string>

namespace config {

struct Config {
    std::string host = "0.0.0.0";
    int port = [] {
        const char* value = std::getenv("API_PORT");
        return value ? std::stoi(value) : 8080;
    }();
};

}