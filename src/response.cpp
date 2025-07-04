#include "response.h"

#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind/bind.hpp>
#include <format>
#include <iostream>
#include <sstream>

ResponseWriter::ResponseWriter(tcp::socket& socket)
    : _socket(std::move(socket)), _status(ResponseWriterStatus::initialized) {
    res = {.status = StatusCode::OK,
           .http_version = 1.1,
           .headers = {},
           .body = ""};
}

void ResponseWriter::write_status_lines() {
    std::string msg{};
    std::string error_msg = "OK";
    switch (res.status) {
        case StatusCode::OK:
            error_msg = "OK";
            break;
        case StatusCode::BAD_REQUEST:
            error_msg = "Bad Request";
            break;
        case StatusCode::INTERNAL_ERROR:
            error_msg = "Internal Server Error";
            break;
        default:
            break;
    }
    std::stringstream sstream;
    sstream.precision(2);
    sstream << "HTTP/" << res.http_version << " "
            << std::to_string(int(res.status)) << " " << error_msg << "\r\n";
    msg = sstream.str();

    std::cout << msg << "\n";

    size_t amt_written =
        boost::asio::write(_socket, boost::asio::buffer(msg, msg.length()));
    if (amt_written > 0) {
        _status = ResponseWriterStatus::write_headers;
    }
}

void ResponseWriter::write_headers() {
    std::string msg{};

    if (res.headers.headers.size() > 0) {
        for (const auto& [key, value] : res.headers.headers) {
            msg += key + ": " + value + "\r\n";
        }
    }
    msg += "\r\n";
    std::cout << msg << "\n";
    size_t amt_written =
        boost::asio::write(_socket, boost::asio::buffer(msg, msg.length()));
    if (amt_written > 0) {
        _status = ResponseWriterStatus::write_body;
    }
}
void ResponseWriter::write_body() {
    size_t amt_written = boost::asio::write(
        _socket, boost::asio::buffer(res.body, res.body.length()));
    if (amt_written > 0) {
        _status = ResponseWriterStatus::finished;
    }
}
