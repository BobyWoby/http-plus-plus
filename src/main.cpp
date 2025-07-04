#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <iostream>

#include "http.h"
#include "request.h"
#include "response.h"
#include "server.h"

boost::asio::io_context io;

void handler(ResponseWriter &writer, Request req) {
    writer.res.status = StatusCode::OK;
    writer.res.body = "Hello World!\n";
    writer.res.headers = Headers::default_headers(writer.res.body.length());

    writer.write_status_lines();
    writer.write_headers();
    writer.write_body();

    Response res = http::Get("httpbin.org", "/", io,  writer._socket);
    // http::Get_string("httpbin.org", "/ip", io, writer._socket);
    std::cout <<res.body <<  "\n";
    
}

int main() {
    int port = 42069;
    try {
        Server s = Server(io, port, &handler);
        io.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    }
}
