#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

class Worker {
private:
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_{false};

public:
    void start() {
        running_ = true;
        thread_ = std::thread(&Worker::run, this);
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }

        cv_.notify_one();

        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void run() {
        std::unique_lock<std::mutex> lock(mutex_);

        while (running_) {
            cv_.wait(lock);

            if (running_) {
                std::cout << "Worker processing events...\n";
            }
        }
    }

    ~Worker() {
        stop();
    }
};

int process_event() {
    throw std::runtime_error("Event processing failed");
}

int main() {
    Worker worker;

    try {
        worker.start();

        auto future = std::async(
            std::launch::async,
            process_event
        );

        try {
            future.get();
        }
        catch (const std::exception& error) {
            std::cerr << "Event error: "
                      << error.what() << '\n';
        }

        worker.stop();
    }
    catch (const std::exception& error) {
        std::cerr << "Worker error: "
                  << error.what() << '\n';
    }

    return 0;
}