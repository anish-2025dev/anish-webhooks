#include "storage/persistent_log.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <stdexcept>
#include <string>

PersistentLog::PersistentLog(const std::string& file_path)
    : file_path_(file_path) {
    recover();
}

void PersistentLog::recover() {
    std::ifstream log_file(file_path_, std::ios::in | std::ios::binary);

    // The log does not exist yet.
    if (!log_file.is_open()) {
        next_offset_ = 0;
        return;
    }

    std::string line;
    std::uint64_t recovered_offset = 0;

    while (std::getline(log_file, line)) {
        if (line.empty()) {
            continue;
        }

        const std::size_t first_separator = line.find('|');

        if (first_separator == std::string::npos) {
            throw std::runtime_error(
                "Invalid record in persistent log"
            );
        }

        const std::size_t second_separator =
            line.find('|', first_separator + 1);

        if (second_separator == std::string::npos) {
            throw std::runtime_error(
                "Invalid record in persistent log"
            );
        }

        const std::string offset_text =
            line.substr(0, first_separator);

        const std::string length_text =
            line.substr(
                first_separator + 1,
                second_separator - first_separator - 1
            );

        const std::string payload =
            line.substr(second_separator + 1);

        try {
            const std::uint64_t offset =
                std::stoull(offset_text);

            const std::uint64_t payload_length =
                std::stoull(length_text);

            if (payload.size() != payload_length) {
                throw std::runtime_error(
                    "Persistent log record length mismatch"
                );
            }

            if (offset != recovered_offset) {
                throw std::runtime_error(
                    "Persistent log offset sequence is invalid"
                );
            }

            ++recovered_offset;
        }
        catch (const std::invalid_argument&) {
            throw std::runtime_error(
                "Invalid numeric value in persistent log"
            );
        }
        catch (const std::out_of_range&) {
            throw std::runtime_error(
                "Numeric value out of range in persistent log"
            );
        }
    }

    next_offset_ = recovered_offset;
}

std::uint64_t PersistentLog::append(const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex_);

    const std::uint64_t offset = next_offset_;

    const std::string record =
        std::to_string(offset)
        + "|"
        + std::to_string(data.size())
        + "|"
        + data
        + "\n";

    std::ofstream log_file(
        file_path_,
        std::ios::out | std::ios::app | std::ios::binary
    );

    if (!log_file.is_open()) {
        throw std::runtime_error(
            "Failed to open persistent log: " + file_path_
        );
    }

    log_file.write(
        record.data(),
        static_cast<std::streamsize>(record.size())
    );

    if (!log_file.good()) {
        throw std::runtime_error(
            "Failed to write to persistent log: " + file_path_
        );
    }

    log_file.flush();

    if (!log_file.good()) {
        throw std::runtime_error(
            "Failed to flush persistent log: " + file_path_
        );
    }

    const int file_descriptor =
        open(file_path_.c_str(), O_WRONLY);

    if (file_descriptor == -1) {
        throw std::runtime_error(
            "Failed to open persistent log for fsync: "
            + file_path_
        );
    }

    if (fsync(file_descriptor) == -1) {
        close(file_descriptor);

        throw std::runtime_error(
            "Failed to fsync persistent log: "
            + file_path_
        );
    }

    close(file_descriptor);

    ++next_offset_;

    return offset;
}