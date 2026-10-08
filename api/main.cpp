#include <iostream>

#include "config/config.h"
#include "types/event.h"

int main() {
    config::Config config;

    types::Event event{
        "event-1",
        "orders",
        R"({"order_id":123})"
    };

    std::cout << "API listening on "
              << config.host << ":" << config.port << '\n';

    std::cout << "Event: "
              << event.id << " | "
              << event.topic << '\n';

    return 0;
}