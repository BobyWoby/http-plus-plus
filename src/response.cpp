#include "../include/response.h"

#include <boost/asio.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <sstream>

// template<typename sock>
// ResponseWriter<sock>::ResponseWriter(sock& socket)
//     : _socket(std::move(socket)), _status(ResponseWriterStatus::initialized) {
//     res = {.status = StatusCode::OK,
//            .http_version = 1.1,
//            .headers = {},
//            .body = ""};
// }
//
// template<typename sock>
// void ResponseWriter<sock>::write_status_lines() {
//     std::string msg{};
//     std::string error_msg = "OK";
//     switch (res.status) {
//         case StatusCode::OK:
//             error_msg = "OK";
//             break;
//         case StatusCode::BAD_REQUEST:
//             error_msg = "Bad Request";
//             break;
//         case StatusCode::FORBIDDEN:
//             error_msg = "Forbidden";
//             break;
//         case StatusCode::INTERNAL_ERROR:
//             error_msg = "Internal Server Error";
//             break;
//         default:
//             break;
//     }
//     std::stringstream sstream;
//     sstream.precision(2);
//     sstream << "HTTP/" << res.http_version << " "
//             << std::to_string(int(res.status)) << " " << error_msg << "\r\n";
//     msg = sstream.str();
//
//     // std::cout << msg << "\n";
//
//     size_t amt_written =
//         boost::asio::write(_socket, boost::asio::buffer(msg, msg.length()));
//     if (amt_written > 0) {
//         _status = ResponseWriterStatus::write_headers;
//     }
// }

// template<typename sock>
// void ResponseWriter<sock>::write_headers() {
//     std::string msg{};
//
//     if (res.headers.headers.size() > 0) {
//         for (const auto& [key, value] : res.headers.headers) {
//             msg += key + ": " + value + "\r\n";
//         }
//     }
//     msg += "\r\n";
//     size_t amt_written =
//         boost::asio::write(_socket, boost::asio::buffer(msg, msg.length()));
//     if (amt_written > 0) {
//         _status = ResponseWriterStatus::write_body;
//     }
// }
// template<typename sock>
// void ResponseWriter<sock>::write_body() {
//     // std::cout << res.body <<"\n";
//     // std::cout << "length: "<< res.body.length() << "\n";
//     size_t amt_written = boost::asio::write(
//         _socket, boost::asio::buffer(res.body, res.body.length()));
//     // std::cout << "amt_written:" << amt_written;
//     // if (amt_written > 0) {
//     //     _status = ResponseWriterStatus::finished;
//     // }
// }
