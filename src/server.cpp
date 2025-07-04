#include "server.h"
void Session::handle_read(const boost::system::error_code &ec,
                          std::size_t bytes_read) {
    if (!ec) {
        if (_request.state == RequestState::finished) {
            received_request();
        } else {
            // std::cout << _data.data() << "\n";
            // std::cout << (int)_request.state << "\n";
            ReturnError<int> err;
            while (true) {
                err = _request.parse(_data, bytes_read);
                if (err.error.has_value()) {
                    msg = "Error: ";
                    msg += err.error.value();
                    // std::cout << msg << "\n";
                    // std::cout << err.value << "\n";
                    do_write(msg.length());
                    break;
                }
                _data = slice(_data, err.value);
                if (_request.state == RequestState::finished) {
                    received_request();
                    // this  will run
                    break;
                }
                if ((int)bytes_read - err.value < 0) {
                    do_read();
                }
                bytes_read -= err.value;
            }
            if (_request.state != RequestState::finished) {
                received_request();
            }
        }
    } else {
        std::cout << "error\n";
        std::cout << _data.data() << "\n";
    }
}

void Session::do_read() {
    // std::cout << "reading\n";
    auto self(shared_from_this());
    _socket.async_read_some(
        boost::asio::buffer(_data, MAX_LENGTH),
        boost::bind(&Session::handle_read, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Session::handle_write(boost::system::error_code ec, std::size_t length) {
    if (!ec) {
    }
}

void Session::do_write(size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(
        _socket, boost::asio::buffer(msg.data(), length),
        boost::bind(&Session::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void Server::do_accept() {
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<Session>(std::move(socket), _handler)->start();
            }
            do_accept();
        });
}

Server::Server(boost::asio::io_context &io, int port, const Handler &handler)
    : _acceptor(io, tcp::endpoint(tcp::v4(), port)) {
        if(handler == nullptr){
            std::cout << "handler is null!\n";
        }else{
            _handler = handler;
        }
        std::cout <<  "Server created on  port " << port << "\n";
    do_accept();
}
