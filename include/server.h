#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <format>
#include <iostream>
#include <memory>
#include <functional>

#include "common.h"
#include "request.h"
#include "response.h"

using boost::asio::ip::tcp;

// probably  wanna pass in a response writer of some kind
typedef std::function<void(ResponseWriter<tcp::socket>&, Request req)> Handler;

class Session : public std::enable_shared_from_this<Session> {
   private:
    std::array<char, MAX_LENGTH> _data;
    std::array<char, MAX_LENGTH> buffer;
    std::string msg;
    Request _request;
    Handler _handler;
    tcp::socket _socket;

    inline void received_request() {
        ResponseWriter<tcp::socket> writer{_socket};
        _handler(writer, _request);
    }

    void handle_read(const boost::system::error_code &ec,
                     std::size_t bytes_read);
    void do_read();
    void handle_write(boost::system::error_code ec, std::size_t length);
    void do_write(size_t length);

   public:
    inline Session(tcp::socket socket, const Handler &handler)
        : _socket(std::move(socket)), _handler(handler){
            if(handler == nullptr){
                std::cout << "nullptr passed in!\n";
            }
        }
    inline void start() { do_read(); }
};

class Server {
   private:
    tcp::acceptor _acceptor;
    Handler _handler;
    void do_accept();

   public:
    Server(boost::asio::io_context &io, int port, const Handler &handler);
};

