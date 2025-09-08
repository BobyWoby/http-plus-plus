# HTTP++
HTTP++ is a HTTP/HTTPS library written using tcp sockets. It is built on top of Boost.asio for the socket side and OpenSSL for TLS Certification.

## Overview
To setup an HTTP server, you must specify a boost::asio::io\_context, a port number, and request handler, which has the function signature `void handler(ResponseWriter<boost::asio::basic_stream_socket<boost::asio::ip::tcp>> &, Request)`

An example:
```C++
boost::asio::io_context io;
boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23);

void handler(ResponseWriter<tcp::socket> &writer, Request req) {
    writer.res.status = StatusCode::OK;
    writer.res.body = "Hello World!\n";
    std::string target = req.request_line.target;
    writer.res.headers = Headers::default_headers(writer.res.body.length());
    if (target == "/httpbin/stream/100") {
        Client client(io, ctx);
        Headers headers;
        Response res = client.fetch("http://httpbin.org/stream/100", "GET",  Headers(), "");
        // Do something with the response from httpbin here
    } else if (target == "/") {
        writer.res.body = html::from_file("./assets/index.html");
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

        Server s = Server(io, port, &handler);
        io.run();
    } catch (std::exception &e) {
        std::cerr << e.what() << "\n";
    }
}
```
This example shows the standard functionality of the library, setting up a handler, and making outgoing http requests using the client class.


## Installation
To install, install the source code into your favorite directory for libraries by running, and cd into the directory:
```bash
git clone https://github.com/BobyWoby/http-plus-plus.git
cd http-plus-plus
```
Once you are in the directory, create a build directory and cd into that:
```bash
mkdir build && cd build
```
Run the build process with cmake:
```bash
cmake ..
cmake build .
```

Now, if you want to install the library directly to your system:
```bash
sudo cmake install .
```

## Linking
In order to link the library with CMake, you must make sure that OpenSSL and Boost is also installed on your system.
Once that is finished, it's really simple to install, as you can just call `find_package` and then link the library
```CMake
# find the package, as well as boost and openssl
find_package(http REQUIRED)
find_package(Boost REQUIRED COMPONENTS thread system asio)
find_package(OpenSSL REQUIRED)


# After the add_executable
target_link_libraries(target-name PRIVATE http::http)
target_link_libraries(target-name PRIVATE Boost::headers Boost::thread
                                              ${OPENSSL_LIBRARIES})
```
