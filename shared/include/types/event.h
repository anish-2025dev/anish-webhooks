#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace types {

struct Event {
    std::string id;
    std::string topic;
    std::string payload;

    Event(std::string id, std::string topic, std::string payload)
        : id(std::move(id)),
          topic(std::move(topic)),
          payload(std::move(payload)) {

        if (this->id.empty()) {
            throw std::invalid_argument("Event id cannot be empty");
        }

        if (this->topic.empty()) {
            throw std::invalid_argument("Event topic cannot be empty");
        }
    }
};

}