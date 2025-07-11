#include "../include/http.h"

#include <sys/socket.h>

#include <boost/asio/completion_condition.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/detail/regex_fwd.hpp>
#include <boost/asio/impl/read.hpp>
#include <boost/asio/impl/read_until.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/host_name_verification.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/verify_mode.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <iostream>
#include <istream>
#include <string>

Client::Client(boost::asio::io_context& io, boost::asio::ssl::context& ctx)
    : ssl_socket(io, ctx), socket_(io), resolver_(io), io_(io), ctx_(ctx) {}

bool Client::verify_certificate(bool preverified,
                                boost::asio::ssl::verify_context& ctx) {
    // You can also verify the certificate, but we don't do that here
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    // std::cout << "Verifying " << subject_name << "\n";

    return preverified;
}

void Client::connect(const tcp::resolver::results_type& endpoints,
                     Headers headers, std::string data) {
    boost::system::error_code ec;
    boost::asio::connect(ssl_socket.lowest_layer(), endpoints);
    handshake(headers, data);
    // boost::asio::async_connect(
    //     ssl_socket.lowest_layer(), endpoints,
    //     [this](const boost::system::error_code ec, const tcp::endpoint& /**/)
    //     {
    //         if (ec) {
    //             std::cout << "Connection failed: " << ec.message() << "\n";
    //         } else {
    //             std::cout << "Connection successful, trying handshake...\n";
    //             handshake();
    //         }
    //     });
}

void Client::handshake(Headers headers, std::string data) {
    ssl_socket.handshake(boost::asio::ssl::stream_base::client);
    send_request(headers, data);
    // ssl_socket.async_handshake(
    //     boost::asio::ssl::stream_base::client,
    //     [this](const boost::system::error_code& ec) {
    //         if (ec) {
    //             std::cout << "Handshake failed: " << ec.message() << "\n";
    //         } else {
    //             std::cout << "Handshake success! Trying Request...\n";
    //             send_request();
    //         }
    //     });
}
void Client::send_request(Headers headers, std::string data) {
    std::string get_request = method_ + " " + target_ +
                              " HTTP/1.1\r\n"
                              "Host: " +
                              host_ + "\r\n";
    for (auto [key, val] : headers.headers) {
        get_request += key + ": " + val + "\r\n";
    }
    get_request += "Connection: close\r\n\r\n";
    if (data != "") {
        get_request += data;
    }
    if (debug_mode) {
        std::cout << get_request << "\n";
    }

    // std::cout << "\n" << get_request << "\n";
    size_t length = boost::asio::write(
        ssl_socket, boost::asio::buffer(get_request, get_request.length()));
    receive_response(length);

    // boost::asio::async_write(
    //     ssl_socket, boost::asio::buffer(get_request, get_request.length()),
    //     [this](const boost::system::error_code& ec, size_t length) {
    //         if (!ec) {
    //             std::cout << "Reqeust success! Receiving response...\n";
    //             receive_response(length);
    //         } else {
    //             std::cout << "Write Failed: " << ec.message() << "\n";
    //         }
    //     });
}

/**
 * @brief sends a http/https request using the provided method and headers.
 *
 * @param url The full web url of  the endpoints (ex.
 * https://myapi.com/resources/x)
 * @param method The HTTP method that will be used (eg. GET, POST, PUT, etc.)
 * @param headers Any addtional header information to be passed in. Defaults to
 * empty;
 * @param data Any data to be  sent to  the endpoint, mostly  for POST requests
 * @return the response from the fetch request in a Response Object
 */
Response Client::fetch(std::string url, std::string method, Headers headers,
                       std::string data) {
    Response out;
    method_ = method;
    target_ = "/";
    auto res1 =
        std::mismatch(http_prefix.begin(), http_prefix.end(), url.begin());
    auto res2 =
        std::mismatch(https_prefix.begin(), https_prefix.end(), url.begin());

    if (res1.first == http_prefix.end()) {
        // it's an http request
        host_ = url.substr(http_prefix.length(), std::string::npos);
        // std::cout << host_ << "\n";
        port_ = "80";
    } else if (res2.first == https_prefix.end()) {
        // it's an https request
        // std::cout << "https found\n";
        host_ = url.substr(https_prefix.length(), std::string::npos);
        // std::cout << host_ << "\n";
        port_ = "443";
    }
    size_t slash_pos = host_.find("/");
    if (slash_pos != std::string::npos) {
        target_ = host_.substr(slash_pos, std::string::npos);
        host_ = host_.substr(0, slash_pos);
    }

    auto endpoints = resolver_.resolve(host_, port_);
    if (port_ == "443") {
        try {
            out = fetch_ssl(endpoints, headers, data);
        } catch (std::exception& e) {
            std::cout << e.what() << "\n";
        }
    } else {
        try {
            out = fetch_http(endpoints, headers, data);
        } catch (std::exception& e) {
            std::cout << e.what() << "\n";
        }
    }
    return out;
}

Response Client::fetch_http(tcp::resolver::results_type endpoints,
                            Headers headers, std::string data) {
    Response out;
    boost::asio::connect(socket_, endpoints);
    std::string request = method_ + " " + target_ +
                          " HTTP/1.1\r\n"
                          "Connection: close\r\n"
                          "Host: " +
                          host_ + "\r\n";
    for (auto [key, val] : headers.headers) {
        request += key + ": " + val + "\r\n";
    }
    request += "\r\n";
    if (data != "") {
        request += data;
    }

    // std::cout << "\n" << request << "\n";
    boost::asio::write(socket_, boost::asio::buffer(request, request.length()));

    // std::cout << "RESPONSE-------\n\n";
    boost::asio::read_until(socket_, response,
                            "\r\n");  // this reads the  status line

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    // std::cout << "VERSION: " << http_version << "\n";
    if (http_version.find("/") == std::string::npos) {
        std::cout << "invalid version!\n";
    }
    std::string version_str =
        http_version.substr(http_version.find("/") + 1, std::string::npos);
    // std::cout << "version_str: " << version_str << "\n";
    double version = std::stod(version_str);
    out.http_version = version;

    unsigned int status_code;
    response_stream >> status_code;
    // std::cout << "STATUS: " << status_code << "\n";
    if (status_code) out.status = StatusCode(status_code);

    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
    }
    if (status_code != 200) {
        std::cout << "Response returned with status code " << status_code
                  << "\n";
    }

    boost::asio::read_until(socket_, response,
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
    }

    std::string body_buf, body;
    // Write whatever content we already have to output.
    if (response.size() > 0) {
        std::istream(&response) >> body_buf;
        body += body_buf;
        // std::cout << body_buf;
    }

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket_, response,
                             boost::asio::transfer_at_least(1), error)) {
        std::istream(&response) >> body_buf;
        body += body_buf;
        // std::cout << body_buf;
    }
    while (response.size() > 0) {
        std::istream(&response) >> body_buf;
        body += body_buf;
    }
    out.body = body;
    // std::cout << body << "\n";
    socket_ = tcp::socket(io_);
    return out;
}

Response Client::fetch_ssl(tcp::resolver::results_type endpoints,
                           Headers headers, std::string data) {
    SSL_set_tlsext_host_name(ssl_socket.native_handle(), host_.c_str());
    ssl_socket.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_socket.set_verify_callback(std::bind(&Client::verify_certificate, this,
                                             std::placeholders::_1,
                                             std::placeholders::_2));
    connect(endpoints, headers, data);
    return tmp_res;
}

void Client::receive_response(size_t length) {
    tmp_res = Response();
    // std::cout << "RESPONSE-------\n\n";
    boost::asio::read_until(ssl_socket, response,
                            "\r\n");  // this reads the  status line

    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    // std::cout << "VERSION: " << http_version << "\n";
    double version = std::stod(
        http_version.substr(http_version.find("/") + 1, std::string::npos));
    tmp_res.http_version = version;

    unsigned int status_code;
    response_stream >> status_code;
    // std::cout << "STATUS: " << status_code << "\n";
    if (status_code) tmp_res.status = StatusCode(status_code);

    std::string status_message;
    std::getline(response_stream, status_message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
    }
    if (status_code != 200) {
        std::cout << "Response returned with status code " << status_code
                  << "\n";
    }

    boost::asio::read_until(ssl_socket, response,
                            "\r\n\r\n");  // read  all  of the headers
    std::string header;
    while (std::getline(response_stream, header) && header != "\r") {
        size_t colon_pos = header.find(":");
        if (colon_pos == std::string::npos) {
            std::cout << "invalid  colon pos\n";
            break;
        }
        std::string field_name = trim(header.substr(0, colon_pos));
        std::string field_value =
            trim(header.substr(colon_pos + 1, std::string::npos));

        tmp_res.headers.add(field_name, field_value);
        if (debug_mode) {
            std::cout << field_name << ": "
                      << tmp_res.headers.headers[field_name] << "\n";
        }
    }
    bool chunked = false;
    if (tmp_res.headers.headers.find("Transfer-Encoding") !=
        tmp_res.headers.headers.end()) {
        if (tmp_res.headers.headers["Transfer-Encoding"] == "chunked") {
            chunked = true;
        }
    }
    if (!chunked) {
        if (debug_mode) {
            std::cout << "standard\n";
        }
        ssl_receive_standard();
    } else {
        if (debug_mode) {
            std::cout << "chunked\n";
        }
        ssl_receive_chunked();
    }
}

void Client::ssl_receive_chunked() {
    std::string body_buf{""}, body{""}, tmp_buf{""};
    int bytes_in_message = -1;
    boost::system::error_code error;
    // check if theres  anything  still in response?
    if (response.size() > 0) {
        std::istream(&response) >> body_buf;
        // if we  can't find the \r\n then we need more data
        if (body_buf.find("\r\n") != std::string::npos) {
            // if we can find it then  its the bytes in the message and we need
            // to read it

            std::cout << "first loop: "
                      << body_buf.substr(0, body_buf.find("\r\n")) << "\n";

            bytes_in_message = std::stoi(
                body_buf.substr(0, body_buf.find("\r\n")), nullptr, 16);

            std::cout << bytes_in_message << "\n";
            body_buf.erase(0, body_buf.find("\r\n") + 2);
            body += body_buf;
            bytes_in_message -= body.length();
            std::cout << "body1"<< body  << "\n";
        } else {
            // read until the  first \r\n, the response might have more  data
            // but bytes_read will contain until the end of the \r\n
            size_t bytes_read =
                boost::asio::read_until(ssl_socket, response, "\r\n");
            std::istream(&response) >> tmp_buf;
            body_buf += tmp_buf.substr(0, bytes_read - 2);
            tmp_buf.erase(0, bytes_read - 2);

            std::cout << "second loop: " << body_buf << "\n";
            bytes_in_message = std::stoi(body_buf, nullptr, 16);
            std::cout << bytes_in_message << "\n";
            body += tmp_buf;
            std::cout << "body2: " << body  << "\n";
            bytes_in_message -= body.length();

            body_buf = "";
            tmp_buf = "";
        }
    }

    while (bytes_in_message > 0) {
        // read until  we get the 0\r\n\r\n line
        // size_t bytes_read =
        //     boost::asio::read(ssl_socket, response,
        //                       boost::asio::transfer_exactly(bytes_in_message+2));
        size_t bytes_read = boost::asio::read_until(ssl_socket, response, "\r\n");
        std::cout << "\n\nbytes_read: " << bytes_read << "\n";
        std::cout << "response size: " << response.size() << "\n";

        std::ostringstream ss = {};
        ss << std::istream(&response).rdbuf();
        tmp_buf = ss.str();
        std::cout << "tmp_buf: " <<escape_string(tmp_buf)  << "\n";
        if(tmp_buf == "0\r\n\r\n"){
            break;
        }

        body_buf += tmp_buf.substr(0, bytes_read - 2);
        tmp_buf.erase(0, bytes_read);


        body += body_buf;
        // std::cout << "body_buf: " <<escape_string(body_buf)  << "\n";
        body_buf = tmp_buf;
        tmp_buf = "";
        response.consume(response.size());

        // read the next byte_length
        bytes_read = boost::asio::read_until(ssl_socket, response, "\r\n");
        std::cout << "--------next slice --------\n";
        std::cout << "bytes read: " << bytes_read << "\n";
        std::cout << "response size: " << response.size() << "\n";

        ss = {};
        ss << std::istream(&response).rdbuf();
        tmp_buf = ss.str();
        // std::cout << "tmp_buf: " << (tmp_buf.length()) << "\n";

        body_buf += tmp_buf.substr(0, bytes_read - 2);
        tmp_buf.erase(0, bytes_read); 
        std::cout << "body_buf (number): " << escape_string(body_buf) << "\n";
        std::cout << "byte to read (hex)" << body_buf << "\n";
        std::cout  << "tmp_buf: " << escape_string(tmp_buf) << "\n";
        bytes_in_message = std::stoi(body_buf, nullptr, 16);
        std::cout << "bytes to  read: " << bytes_in_message << "\n";
        body += tmp_buf;

        body_buf = "";
        tmp_buf = "";
    }
    std::cout << "\n\n" <<  body <<  "\n\n";
    tmp_res.body = body;
    // reset the socket
    ssl_socket = boost::asio::ssl::stream<tcp::socket>(io_, ctx_);
}

void Client::ssl_receive_standard() {
    std::string body_buf{""}, body{""};
    // Write whatever content we already have to output.
    if (response.size() > 0) {
        std::istream(&response) >> body_buf;
        body += body_buf;
        body_buf = "";
        // std::cout << body_buf;
    }

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(ssl_socket, response,
                             boost::asio::transfer_at_least(1), error)) {
        // std::cout << &response;
        std::istream(&response) >> body_buf;
        body += body_buf;
        body_buf = "";
        // std::cout << body_buf;
    }
    while (response.size() > 0) {
        std::istream(&response) >> body_buf;
        body += body_buf;
    }
    // std::cout << body << "\n";
    tmp_res.body = body;
    // reset the socket
    ssl_socket = boost::asio::ssl::stream<tcp::socket>(io_, ctx_);
}
