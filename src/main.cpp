#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/system/detail/error_code.hpp>
#include <exception>
#include <iostream>

#include "../include/html.h"
#include "../include/http.h"
#include "../include/request.h"
#include "../include/response.h"
#include "../include/server.h"

boost::asio::io_context io;
boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

std::string APCA_API_BASE_URL = "https://paper-api.alpaca.markets";
void handler(ResponseWriter &writer, Request req) {
    writer.res.status = StatusCode::OK;
    writer.res.body = "Hello World!\n";
    std::string target = req.request_line.target;
    writer.res.headers = Headers::default_headers(writer.res.body.length());
    if (target == "/httpbin/stream/100") {
        // std::cout << res.body << "\n";
    } else if (target == "/") {
        writer.res.body = html::from_file("./assets/index.html");
        // std::cout << writer.res.body;
        writer.res.headers = Headers::default_headers(writer.res.body.length());
        writer.res.headers.headers["Content-Type"] = "text/html";
    }
    writer.write_status_lines();
    writer.write_headers();
    writer.write_body();
}

int main() {
    int port = 42069;
    try {
        ctx.load_verify_file("/etc/ssl/cert.pem");
        // ctx.load_verify_file("./certs/cert.pem");
        ctx.set_default_verify_paths();
        Client client(io, ctx);
        client.fetch("http://httpbin.org/stream/100", "GET",  Headers());
        client.fetch("https://paper-api.alpaca.markets/v2/account", "GET", Headers());
        // client.fetch("https://paper-api.alpaca.markets/v2/assets", "GET");
        // client.fetch("https://google.com", "GET");

        Server s = Server(io, port, &handler);
        io.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    }
}
