#include "http.h"

#include <boost/asio/connect.hpp>
#include <boost/asio/detail/regex_fwd.hpp>
#include <boost/asio/impl/read_until.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind/bind.hpp>
#include <cinttypes>
#include <iostream>
#include <istream>
#include <string>

Response http::Get(std::string host, std::string query,
                   boost::asio::io_context &io, tcp::socket &s) {
    tcp::resolver resolver(io);
    tcp::resolver::results_type endpoints = resolver.resolve(host, "http");
    boost::asio::connect(s, endpoints);
    boost::asio::streambuf request;
    Response out;

    std::ostream request_stream(&request);

    request_stream << "GET " << query << " HTTP/1.0\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(s, request);

    boost::asio::streambuf response;
    boost::asio::read_until(s, response,
                            "\r\n");  // this reads the  status line

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    // std::cout << "HTTP: " << http_version << "\n";

    // this might be unsafe idk
    out.http_version = std::stod(split(http_version, "/").at(1));

    unsigned int status_code;
    response_stream >> status_code;

    // this might be unsafe idk
    out.status = (StatusCode)status_code;

    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
    }
    if (status_code != 200) {
        std::cout << "Response returned with status code " << status_code
                  << "\n";
    }

    boost::asio::read_until(s, response,
                            "\r\n\r\n");  // read  all  of the headers
    std::string header;
    while (std::getline(response_stream, header) && header != "\r") {
        size_t colon_pos = header.find(":");
        if (colon_pos == std::string::npos) {
            std::cout << "invalid  colon pos\n";
            break;
        }
        std::string field_name = header.substr(0, colon_pos);
        std::string field_value =
            header.substr(colon_pos + 1, std::string::npos);
        out.headers.add(field_name, field_value);
        // std::cout << header << "\n";
    }
    // std::cout << "\n";

    std::string body_buf, body;
    // Write whatever content we already have to output.
    if (response.size() > 0) {
        std::istream(&response) >> body_buf;
        body += body_buf;
        std::cout << body_buf;
    }

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(s, response, boost::asio::transfer_at_least(1),
                             error)) {
        // std::cout << &response;
        std::istream(&response) >> body_buf;
        body += body_buf;
        std::cout << body_buf;
    }
    while(response.size() > 0){
        std::istream(&response) >> body_buf;
        body += body_buf;
    }
    out.body = body;
    return out;
}

std::string http::Get_string(std::string host, std::string query,
                             boost::asio::io_context &io, tcp::socket &s) {
    tcp::resolver resolver(io);
    tcp::resolver::results_type endpoints = resolver.resolve(host, "http");
    boost::asio::connect(s, endpoints);
    std::string out;
    boost::asio::streambuf request;

    std::ostream request_stream(&request);

    request_stream << "GET " << query << " HTTP/1.0\r\n";
    request_stream << "Host: " << host << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    boost::asio::write(s, request);

    boost::asio::streambuf response;
    boost::asio::read_until(s, response,
                            "\r\n");  // this reads the  status line

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    // std::cout << "HTTP: " << http_version << "\n";
    unsigned int status_code;
    response_stream >> status_code;
    // std::cout << "Status: " << status_code << "\n";
    std::string status_message;
    // std::cout << "Status: " << status_message << "\n";
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
    }
    if (status_code != 200) {
        std::cout << "Response returned with status code " << status_code
                  << "\n";
    }

    boost::asio::read_until(s, response,
                            "\r\n\r\n");  // read  all  of the headers
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
        std::cout << header << "\n";
    std::cout << "\n";

    // Write whatever content we already have to output.
    if (response.size() > 0) std::cout << &response;

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    std::string body_buf;
    while (boost::asio::read(s, response, boost::asio::transfer_at_least(1),
                             error)) {
        std::istream(&response) >> body_buf;
        out += body_buf;
        std::cout << body_buf;
    }
    return out;
}
