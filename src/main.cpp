#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <iostream>

#include "../include/http.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/server.h"
#include "../include/html.h"

boost::asio::io_context io;

void handler(ResponseWriter &writer, Request req) {
    writer.res.status = StatusCode::OK;
    writer.res.body = "Hello World!\n";
    std::string target = req.request_line.target;
    writer.res.headers = Headers::default_headers(writer.res.body.length());
    if(target == "/httpbin/stream/100"){
        Response res = http::Get("httpbin.org", "/stream/100", io);
        writer.res.headers  =  res.headers;
        writer.res.body = res.body;
        // std::cout << res.body << "\n";
    }else if(target == "/"){
        writer.res.body = html::from_file("./assets/index.html");
        // std::cout << writer.res.body;
        writer.res.headers = Headers::default_headers(writer.res.body.length());
        writer.res.headers.headers["Content-Type"] = "text/html";
    }

    writer.write_status_lines();
    writer.write_headers();
    writer.write_body();

    // http::Get_string("httpbin.org", "/ip", io, writer._socket);
    
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
