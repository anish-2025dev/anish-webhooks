#include <iostream>
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "config/config.h"
#include "storage/persistent_log.hpp"
#include "types/event.h"

using boost::asio::ip::tcp;


class Session : public std::enable_shared_from_this<Session> {
private:
    tcp::socket socket_;
    boost::asio::streambuf buffer_;
    std::shared_ptr<PersistentLog> persistent_log_;

public:
    Session(
        boost::asio::io_context& io_context,
        std::shared_ptr<PersistentLog> persistent_log
    )
        : socket_(io_context),
          persistent_log_(std::move(persistent_log)) {
    }

    tcp::socket& socket() {
        return socket_;
    }

    void start() {
        std::cout << "Client connected from "
                  << socket_.remote_endpoint() << '\n';

        read_message();
    }

private:
    void read_message() {
        auto self = shared_from_this();

        boost::asio::async_read_until(
            socket_,
            buffer_,
            '\n',
            [self](
                const boost::system::error_code& error,
                std::size_t /*bytes_transferred*/
            ) {
                if (error) {
                    if (error == boost::asio::error::eof) {
                        std::cout << "Client disconnected\n";
                    }
                    else if (
                        error != boost::asio::error::operation_aborted
                    ) {
                        std::cerr << "Read error: "
                                  << error.message() << '\n';
                    }

                    return;
                }

                std::istream input(&self->buffer_);

                std::string message;
                std::getline(input, message);

                std::cout << "Received message: "
                          << message << '\n';

                try {
                    self->persistent_log_->append(message);

                    std::cout << "Event persisted successfully\n";

                    self->send_response("OK\n");
                }
                catch (const std::exception& error) {
                    std::cerr << "Persistence error: "
                              << error.what() << '\n';

                    self->send_response("ERROR\n");
                }
            }
        );
    }

    void send_response(const std::string& response) {
        auto self = shared_from_this();

        auto response_buffer =
            std::make_shared<std::string>(response);

        boost::asio::async_write(
            socket_,
            boost::asio::buffer(*response_buffer),
            [self, response_buffer](
                const boost::system::error_code& error,
                std::size_t /*bytes_transferred*/
            ) {
                if (error) {
                    if (
                        error != boost::asio::error::operation_aborted
                    ) {
                        std::cerr << "Write error: "
                                  << error.message() << '\n';
                    }

                    return;
                }

                self->read_message();
            }
        );
    }
};


class Broker {
private:
    boost::asio::io_context io_context_;

    tcp::acceptor acceptor_;

    boost::asio::signal_set signals_;

    std::shared_ptr<PersistentLog> persistent_log_;

public:
    Broker(
        const std::string& host,
        unsigned short port,
        std::shared_ptr<PersistentLog> persistent_log
    )
        : acceptor_(
              io_context_,
              tcp::endpoint(
                  boost::asio::ip::make_address(host),
                  port
              )
          ),
          signals_(io_context_, SIGINT, SIGTERM),
          persistent_log_(std::move(persistent_log)) {

        acceptor_.set_option(
            tcp::acceptor::reuse_address(true)
        );

        start_accept();

        signals_.async_wait(
            [this](
                const boost::system::error_code& error,
                int signal_number
            ) {
                if (!error) {
                    std::cout
                        << "\nShutdown signal received: "
                        << signal_number
                        << '\n';

                    acceptor_.close();
                    io_context_.stop();
                }
            }
        );
    }

    void run() {
        std::cout << "Broker listening on "
                  << acceptor_.local_endpoint()
                  << '\n';

        io_context_.run();

        std::cout << "Broker stopped\n";
    }

private:
    void start_accept() {
        auto session = std::make_shared<Session>(
            io_context_,
            persistent_log_
        );

        acceptor_.async_accept(
            session->socket(),
            [this, session](
                const boost::system::error_code& error
            ) {
                if (!error) {
                    session->start();
                }
                else if (
                    error != boost::asio::error::operation_aborted
                ) {
                    std::cerr << "Accept error: "
                              << error.message()
                              << '\n';
                }

                if (acceptor_.is_open()) {
                    start_accept();
                }
            }
        );
    }
};


int main() {
    try {
        config::Config config;

        auto persistent_log =
            std::make_shared<PersistentLog>("broker.log");

        Broker broker(
            config.host,
            config.port,
            persistent_log
        );

        types::Event event{
            "event-1",
            "orders",
            R"({"order_id":123})"
        };

        std::cout << "Configured endpoint: "
                  << config.host
                  << ":"
                  << config.port
                  << '\n';

        std::cout << "Persistent log: broker.log\n";

        std::cout << "Sample event: "
                  << event.id
                  << " | "
                  << event.topic
                  << '\n';

        broker.run();
    }
    catch (const std::exception& error) {
        std::cerr << "Fatal error: "
                  << error.what()
                  << '\n';

        return 1;
    }

    return 0;
}