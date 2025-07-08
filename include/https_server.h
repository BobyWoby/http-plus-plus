#pragma once

#include <boost/asio/ssl/context.hpp>
#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <format>
#include <functional>
#include <iostream>
#include <memory>

#include "common.h"
#include "request.h"
#include "response.h"

using boost::asio::ip::tcp;

// probably  wanna pass in a response writer of some kind
typedef std::function<void(
    ResponseWriter<boost::asio::ssl::stream<tcp::socket>> &, Request req)>
    Handler;

class SSL_Session : public std::enable_shared_from_this<SSL_Session> {
   private:
    std::array<char, MAX_LENGTH> _data;
    std::array<char, MAX_LENGTH> buffer;
    std::string msg;
    Request _request;
    Handler _handler;
    boost::asio::ssl::stream<tcp::socket> _socket;

    inline void received_request() {
        ResponseWriter<boost::asio::ssl::stream<tcp::socket>> writer{_socket};
        _handler(writer, _request);
    }

    void handle_read(const boost::system::error_code &ec,
                     std::size_t bytes_read);
    void do_read();
    void handle_write(boost::system::error_code ec, std::size_t length);
    void do_write(size_t length);
    void handshake();

   public:
    inline SSL_Session(tcp::socket socket, boost::asio::ssl::context &ctx,
                       const Handler &handler)
        : _socket(std::move(socket), ctx), _handler(handler) {
        if (handler == nullptr) {
            std::cout << "nullptr passed in!\n";
        }
    }
    inline void start() { do_read(); }
};

class HTTPS_Server {
   private:
    tcp::acceptor acceptor_;
    Handler handler_;
    boost::asio::ssl::context ctx_;

    void do_accept();
    std::string get_password();

   public:
    HTTPS_Server(boost::asio::io_context &io, int port,
                 std::string cert_chain_filepath, std::string dh_filepath,
                 const Handler &handler);
};
