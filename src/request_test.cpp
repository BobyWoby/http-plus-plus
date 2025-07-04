#include "request.h"

#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <string>

#include "common.h"
#include "headers.h"

TEST_CASE("Parsing Request Line", "[request_line]") {
    Request req;
    // char bytes[MAX_LENGTH] =  "";
    std::array<char, 1024> bytes{
        "GET / HTTP/1.1\r\nHost: localhost:42069\r\nUser-Agent: "
        "curl/7.81.0\r\nAccept: */*\r\n\r\n"};
    std::array<char, 1024> request_line{"GET / HTTP/1.1\r\n"};
    // GET / HTTP/1.1rnHost: localhost:42069rn

    std::array<char, 1024> bad_request_line{
        "GET / HTTP/1.1\r\nHost: localhost:42069\r\n"};

    // Test: Invalid number of args
    req = Request();
    bad_request_line = {"GET HTTP/1.1\r\n"};
    REQUIRE(req.parseRequestLine(bad_request_line, 17).error.value() ==
            "Incorrect number of arguments in request line!");

    // Test: Invalid method
    req = Request();
    bad_request_line = {"get / HTTP/1.1\r\n"};
    REQUIRE(req.parseRequestLine(bad_request_line, 16).error.value() ==
            "Invalid method found!");

    // Test: Invalid http version
    req = Request();
    bad_request_line = {"GET / HTTP/2.1\r\n"};
    REQUIRE(req.parseRequestLine(bad_request_line, 16).error.value() ==
            "Only HTTP/1.1 is supported!");

    // Test: Good Request Line
    req = Request();
    auto ret = req.parseRequestLine(bytes, 16);
    REQUIRE(!ret.error.has_value());
    REQUIRE(ret.value == 16);
    REQUIRE(req.request_line.http_version == 1.1);
    REQUIRE(req.request_line.method == "GET");
    REQUIRE(req.request_line.target == "/");
    REQUIRE(req.request_line.target == "/");
}

TEST_CASE("Parsing Request Headers", "[headers]") {
    Headers headers;

    // Test: valid single header
    std::array<char, MAX_LENGTH> bytes = {"Host: localhost:42069\r\n\r\n"};
    ReturnError<std::pair<bool, int>> ret;
    ret = headers.parse(bytes, 26);
    REQUIRE(!ret.error.has_value());
    REQUIRE(headers.headers.at("host") == "localhost:42069");
    REQUIRE(ret.value.second == 23);
    REQUIRE(!ret.value.first);

    // Test: valid single header
    headers = Headers{};
    bytes = {"Accept: */*\r\n\r\n"};
    ret = ReturnError<std::pair<bool, int>>{};
    ret = headers.parse(bytes, 16);
    bytes = slice(bytes, ret.value.second);
    ret = headers.parse(bytes, 16 - ret.value.second);

    REQUIRE(!ret.error.has_value());
    REQUIRE(headers.headers["accept"] == "*/*");
    REQUIRE(ret.value.second == 2);
    REQUIRE(ret.value.first);

    // Test: Valid 2 headers
    headers = Headers{};
    bytes = {"Host: localhost:42069\r\nAccept: */*\r\n\r\n"};
    ret = ReturnError<std::pair<bool, int>>{};
    ret = headers.parse(bytes, 39);
    REQUIRE(!ret.error.has_value());
    REQUIRE(headers.headers["host"] == "localhost:42069");
    REQUIRE(ret.value.second == 23);
    REQUIRE(!ret.value.first);
    bytes = slice(bytes, ret.value.second);
    ret = headers.parse(bytes, 39 - ret.value.second);
    REQUIRE(!ret.error.has_value());
    REQUIRE(headers.headers["accept"] == "*/*");
    REQUIRE(ret.value.second == 13);
    REQUIRE(!ret.value.first);
    // Test: repeating field names
    bytes = {"Host: bananahost:6969\r\n\r\n"};  //  26 chars
    ret = headers.parse(bytes, 26);
    REQUIRE(!ret.error.has_value());
    REQUIRE(headers.headers["host"] == "localhost:42069, bananahost:6969");
    REQUIRE(ret.value.second == 23);
    REQUIRE(!ret.value.first);

    // Test: Invalid spacing header
    headers = Headers();
    bytes = {"       Host : localhost:42069       \r\n\r\n"};  // 41 bytes
    ret = headers.parse(bytes, 41);
    REQUIRE(ret.error.has_value());
    REQUIRE(ret.value.second == 0);
    REQUIRE(!ret.value.first);

    // Test: Invalid character header
    headers = Headers();
    bytes = {"H©st: localhost:42069\r\n\r\n"};  // 27 bytes
    ret = headers.parse(bytes, 27);
    REQUIRE(ret.error.has_value());
    REQUIRE(ret.value.second == 0);
    REQUIRE(!ret.value.first);
}

TEST_CASE("Full Request", "[request]") {
    std::array<char, MAX_LENGTH> bytes = {
        "GET / HTTP/1.1\r\n"
        "Host: localhost:42069\r\n"
        "User-Agent: curl/8.14.1\r\n"
        "Accept: */*\r\n\r\n"};  // 80 bytes
    Request req;
    ReturnError<int> err;
    int bytes_read = 80;
    RequestState state;
    while (true) {
        REQUIRE(bytes_read >= 0);
        err = req.parse(bytes, bytes_read);
        // CAPTURE(req.state);
        INFO("state:" << (int)req.state);
        INFO("bytes:" << bytes.data());
        if(err.error.has_value()){
            FAIL("error:" << err.error.value());
        }
        REQUIRE(!err.error.has_value());

        REQUIRE(err.value <= 80);

        bytes = slice(bytes, err.value);
        bytes_read -= err.value;
        if (req.state == RequestState::finished) break;
    }
    REQUIRE(req.headers.headers.size() == 3);
    REQUIRE(req.headers.headers["user-agent"] == "curl/8.14.1");
    REQUIRE(req.headers.headers["host"] == "localhost:42069");
    REQUIRE(req.headers.headers["accept"] == "*/*");
}
