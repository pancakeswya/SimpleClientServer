#include <boost/asio.hpp>
#include <iostream>
#include <string>

namespace tcp {

class Client {
 public:
  Client(std::string_view ip, std::string_view port) : socket_(io_context_) {
    boost::asio::ip::tcp::resolver resolver(io_context_);
    boost::asio::connect(socket_, resolver.resolve(ip.data(), port.data()));
  }

  void Read() {
    boost::asio::streambuf sbuf;
    boost::asio::read_until(socket_, sbuf, "\0");
    std::cout << std::istream(&sbuf).rdbuf() << std::endl;
  }

  void Write() {
    std::string message;
    std::cin >> message;
    auto result = boost::asio::write(socket_, boost::asio::buffer(message));
    std::cout << "data sent: " << message.length() << '/' << result << std::endl;
  }

  ~Client() {
    boost::system::error_code code;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, code);
    if (code) {
      std::cout << "Client error on socket shutdown: " << code.message() << std::endl;
    }
  }

 private:
  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket socket_;
};

} // namespace tcp::client

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "usage ./client <ip> <port>" << std::endl;
    return 1;
  }
  tcp::Client(argv[1], argv[2]).Read();
  return 0;
}