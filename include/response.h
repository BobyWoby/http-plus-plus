#pragma once
#include <boost/asio.hpp>
#include <boost/asio/streambuf.hpp>
#include <memory>

#include "common.h"
#include "headers.h"

using boost::asio::ip::tcp;

enum class StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    INTERNAL_ERROR = 500,
};

enum class ResponseWriterStatus {
    initialized,  // write the status lines
    write_headers,
    write_body,
    finished,
};

struct Response {
    StatusCode status;
    double http_version;

    Headers headers;
    std::string body;
};

struct ResponseReturn {
    size_t bytes_read;
    std::string str;
    std::optional<std::string> error;
};

class ResponseStream {
   public:
    StatusCode status;
    double http_version;
    Headers headers;
    boost::asio::streambuf *body_stream;
    tcp::socket read_socket;

    inline ResponseStream(tcp::socket &sock) : read_socket(std::move(sock)) {}

    ResponseReturn Read();
};

template <typename sock>
class ResponseWriter
    : public std::enable_shared_from_this<ResponseWriter<sock>> {
   private:
    ResponseWriterStatus _status;

    void handle_write(boost::system::error_code ec, size_t length);

   public:
    sock _socket;
    Response res;

    ResponseWriter(sock &socket);
    void write_status_lines();
    void write_headers();
    void write_body();
};

template<typename sock>
ResponseWriter<sock>::ResponseWriter(sock& socket)
    : _socket(std::move(socket)), _status(ResponseWriterStatus::initialized) {
    res = {.status = StatusCode::OK,
           .http_version = 1.1,
           .headers = {},
           .body = ""};
}


template<typename sock>
void ResponseWriter<sock>::write_status_lines() {
    std::string msg{};
    std::string error_msg = "OK";
    switch (res.status) {
        case StatusCode::OK:
            error_msg = "OK";
            break;
        case StatusCode::BAD_REQUEST:
            error_msg = "Bad Request";
            break;
        case StatusCode::FORBIDDEN:
            error_msg = "Forbidden";
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

    // std::cout << msg << "\n";

    size_t amt_written =
        boost::asio::write(_socket, boost::asio::buffer(msg, msg.length()));
    if (amt_written > 0) {
        _status = ResponseWriterStatus::write_headers;
    }
}

template<typename sock>
void ResponseWriter<sock>::write_headers() {
    std::string msg{};

    if (res.headers.headers.size() > 0) {
        for (const auto& [key, value] : res.headers.headers) {
            msg += key + ": " + value + "\r\n";
        }
    }
    msg += "\r\n";
    size_t amt_written =
        boost::asio::write(_socket, boost::asio::buffer(msg, msg.length()));
    if (amt_written > 0) {
        _status = ResponseWriterStatus::write_body;
    }
}

template<typename sock>
void ResponseWriter<sock>::write_body() {
    // std::cout << res.body <<"\n";
    // std::cout << "length: "<< res.body.length() << "\n";
    size_t amt_written = boost::asio::write(
        _socket, boost::asio::buffer(res.body, res.body.length()));
    // std::cout << "amt_written:" << amt_written;
    // if (amt_written > 0) {
    //     _status = ResponseWriterStatus::finished;
    // }
}
