#include "../include/https_server.h"

#include <boost/asio/placeholders.hpp>
#include <boost/asio/ssl/context.hpp>
#include <memory>

void SSL_Session::handshake() {
    auto self(shared_from_this());
    _socket.async_handshake(
        boost::asio::ssl::stream_base::server,
        [this, self](const boost::system::error_code &error) {
            if (!error) {
                do_read();
            }
        });
}

void SSL_Session::do_read() {
    auto self(shared_from_this());
    _socket.async_read_some(
        boost::asio::buffer(_data, MAX_LENGTH),
        boost::bind(&SSL_Session::handle_read, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void SSL_Session::handle_read(const boost::system::error_code &ec,
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
                    // this  will run
                    received_request();
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

void SSL_Session::do_write(size_t length) {
    auto self(shared_from_this());
    boost::asio::async_write(
        _socket, boost::asio::buffer(msg.data(), length),
        boost::bind(&SSL_Session::handle_write, shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
}

void SSL_Session::handle_write(boost::system::error_code ec,
        std::size_t length) {
    if (!ec) {
    }
}

// This should be one port 443
HTTPS_Server::HTTPS_Server(boost::asio::io_context &io, int port,
                           std::string cert_chain_filepath,
                           std::string dh_filepath, const Handler &handler)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port)),
      handler_(handler),
      ctx_(boost::asio::ssl::context::sslv23) {
    ctx_.set_options(boost::asio::ssl::context::default_workarounds |
                     boost::asio::ssl::context::no_sslv2 |
                     boost::asio::ssl::context::single_dh_use);
    ctx_.use_certificate_chain_file(cert_chain_filepath);
    ctx_.use_private_key_file(cert_chain_filepath,
                              boost::asio::ssl::context::pem);
    ctx_.use_tmp_dh_file(dh_filepath);

    do_accept();
}

std::string HTTPS_Server::get_password() { return "super_secret_password"; }

void HTTPS_Server::do_accept() {
    acceptor_.async_accept([this](const boost::system::error_code &ec,
                                  tcp::socket socket) {
        if (!ec) {
            std::make_shared<SSL_Session>(std::move(socket), ctx_, handler_);
        } else {
            std::cout << "Error accepting connection: " << ec.what() << "\n";
        }
        do_accept();
    });
}
