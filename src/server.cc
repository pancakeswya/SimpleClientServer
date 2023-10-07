#include <boost/asio.hpp>
#include <iostream>
#include <optional>

#include <ctime>
#include <iomanip>

namespace tcp {

std::string CurrentTime() {
  auto time = std::time(nullptr);
  std::ostringstream oss;
  oss << std::put_time(std::gmtime(&time), "%F %T");
  return oss.str();
}

class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(boost::asio::ip::tcp::socket &&socket) : socket_(std::move(socket)) {}

  void Write() {
    boost::asio::async_write(socket_, boost::asio::buffer(CurrentTime()),
                             [self = shared_from_this()](boost::system::error_code code, size_t bytes) {
      if (code) {
        std::cout << "Server writing error: " << code.message() << std::endl;
        self->socket_.close();
      } else {
        std::cout << "bytes transferred " << bytes << std::endl;
      }
    });
  }

  void Read() {
    boost::asio::async_read_until(socket_, sbuf_, "\0", [self = shared_from_this()](boost::system::error_code code, size_t bytes) {
      if (code) {
        std::cout << "Server reading error: " << code.message() << std::endl;
        self->socket_.close();
      } else {
        std::cout << std::istream(&self->sbuf_).rdbuf() << std::endl;
      }
    });
  }

 private:
  boost::asio::streambuf sbuf_;
  boost::asio::ip::tcp::socket socket_;
};

class Server {
 public:
  explicit Server(std::string_view port) :
      acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), std::stoul(port.data()))) {}

  void Run() {
    Accept();
    io_context_.run();
  }

 protected:
  void Accept() {
    socket_.emplace(io_context_);
    acceptor_.async_accept(*socket_, [&](boost::system::error_code code) {
      if (code) {
        std::cout << "Server accepting error: " << code.message() << std::endl;
        socket_->close();
      } else {
        std::make_shared<Session>(std::move(*socket_))->Write();
      }
      Accept();
    });
  }

  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::optional<boost::asio::ip::tcp::socket> socket_;
};

} // namespace tcp

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage ./server <port>" << std::endl;
  }
  tcp::Server(argv[1]).Run();
  return 0;
}

