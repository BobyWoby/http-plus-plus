#pragma once
// #define MAX_LENGTH 1024

#include <array>
#include <optional>
#include <string>

#include "common.h"
#include "headers.h"
enum class RequestState {
    initialized,
    parsing_headers,
    parsing_body,
    finished,

};
struct RequestLine {
    double http_version;
    std::string method;
    std::string target;
};

inline std::string valid_methods[] = {"GET",    "POST",    "HEAD",    "PUT",
                                      "DELETE", "CONNECT", "OPTIONS", "TRACE"};

class Request {
   public:
    RequestState state = RequestState::initialized;
    RequestLine request_line;
    Headers headers;
    std::string body;

    static Request requestFromRead(std::array<char, MAX_LENGTH> bytes, size_t bytes_read);
    ReturnError<int> parseRequestLine(std::array<char, MAX_LENGTH> bytes,
                                      size_t bytes_length);
    ReturnError<int> parse(std::array<char, MAX_LENGTH> data,
                           size_t bytes_length);

   private:
};

// @code void handler( const boost::system::error_code& error, // Result of
// operation. std::size_t bytes_transferred // Number of bytes read.
