#include "logging/logger.h"
#include <memory>

void start() {
    auto logger = std::make_unique<logging::Logger>();
    logger->info("API started");
}

int main() {
    start();
    return 0;
}