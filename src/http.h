#pragma once

#include <boost/asio/io_context.hpp>
#include <string>

#include "response.h"

namespace http {
void connect_handler(const boost::system::error_code& error, std::string msg,
                     tcp::socket s);
Response Get(std::string host, std::string query, boost::asio::io_context &io,
                       tcp::socket &s);
std::string Get_string(std::string host, std::string query, boost::asio::io_context &io,
                       tcp::socket &s);
}  // namespace http
