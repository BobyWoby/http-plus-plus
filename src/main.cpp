#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <iostream>

#include "request.h"
#include "server.h"
#include "response.h"

void handler(ResponseWriter &writer, Request req){
    writer.res.status =  StatusCode::OK;
    writer.res.body = "Hello World!\n";
    writer.res.headers =  Headers::default_headers(writer.res.body.length());

    writer.write_status_lines();
    writer.write_headers();
    writer.write_body();
}

int main() {
    int port = 42069;
    try {
        boost::asio::io_context io;
        if(&(handler) == nullptr){
            std::cout << "bro wat\n" ; 
        }
        else{
            std::cout << "ok im confuzzled\n" ; 
        }
        Server s = Server(io, port, &handler);
        io.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    }
}
