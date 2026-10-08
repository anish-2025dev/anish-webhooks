#pragma once

#include <cstdint>
#include <mutex>
#include <string>

class PersistentLog {
public:
    explicit PersistentLog(const std::string& file_path);

    std::uint64_t append(const std::string& data);

private:
    void recover();

    std::string file_path_;
    std::uint64_t next_offset_{0};

    std::mutex mutex_;
};