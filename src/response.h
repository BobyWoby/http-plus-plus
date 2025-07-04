#pragma once
#include <boost/asio.hpp>
#include <memory>

#include "common.h"
#include "headers.h"

using boost::asio::ip::tcp;

enum class StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
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

class ResponseWriter : public std::enable_shared_from_this<ResponseWriter>{
   private:
    ResponseWriterStatus _status;

    void handle_write(boost::system::error_code ec, size_t length);

   public:
    tcp::socket _socket;
    Response res;

    ResponseWriter(tcp::socket &socket);
    void write_status_lines();
    void write_headers();
    void write_body();
};
