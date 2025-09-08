#pragma once

#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <memory>
#include <string>

#include "../include/response.h"

const std::string http_prefix("http://");
const std::string https_prefix("https://");

using boost::asio::ip::tcp;
class Client {
   private:
    tcp::resolver resolver_;
    std::unique_ptr<boost::asio::ssl::stream<tcp::socket>>
        ssl_socket;       // socket for https requests
    tcp::socket socket_;  // socket for http requests
    boost::asio::streambuf response;
    boost::asio::io_context &io_;
    boost::asio::ssl::context &ctx_;
    Response tmp_res;  // this is because I'm too lazy to return them

    // HTTP Request stuff
    std::string target_ = "/", method_, host_;
    std::string port_ = "80";  // 443 for https, 80 for http
    std::string res;

    bool verify_certificate(bool preverified,
                            boost::asio::ssl::verify_context &ctx);

    Response fetch_ssl(tcp::resolver::results_type endpoints, Headers headers,
                       std::string data);
    Response fetch_http(tcp::resolver::results_type endpoints, Headers headers,
                        std::string data);

    void connect(const tcp::resolver::results_type &endpoints, Headers headers,
                 std::string data);
    void handshake(Headers headers, std::string data);
    void send_request(Headers headers, std::string data);
    void receive_response(size_t length);

    void ssl_receive_standard();  // non-chunked encoding
    void ssl_receive_chunked();

   public:
    bool debug_mode = false;
    // Response fetch(std::string url, std::string method);

    /**
     * @brief sends a http/https request using the provided method and headers.
     *
     * @param url The full web url of  the endpoints (ex.
     * https://myapi.com/resources/x)
     * @param method The HTTP method that will be used (eg. GET, POST, PUT,
     * etc.)
     * @param headers Any addtional header information to be passed in. Defaults
     * to empty;
     * @param data Any data to be  sent to  the endpoint, mostly  for POST
     * requests
     * @return the response from the fetch request in a Response Object
     */
    Response fetch(std::string url, std::string method,
                   Headers headers = Headers(), std::string data = "");
    Client(boost::asio::io_context &io, boost::asio::ssl::context &ctx);
};
