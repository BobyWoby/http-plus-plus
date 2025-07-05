#include "../include/request.h"

#include <iostream>
#include <string>

#include "../include/common.h"

ReturnError<int> Request::parseRequestLine(std::array<char, MAX_LENGTH> bytes,
                                           size_t bytes_length) {
    std::string data;
    // it's easier  to  process a string
    for (int i = 0; i < bytes_length; i++) {
        data += bytes[i];
    }
    if (data.find("\r\n") == std::string::npos) {
        return {.value = 0};  // keep reading
    }
    std::string request_line = split(data, "\r\n").at(0);
    std::vector request_parts = split(request_line, " ");
    if (request_parts.size() != 3) {
        return {.value = 0,
                .error = "Incorrect number of arguments in request line!"};
    }
    bool valid_method = false;
    for (auto method : valid_methods) {
        if (method == request_parts.at(0)) {
            valid_method = true;
        }
    }
    if (!valid_method) {
        return {.value = 0, .error = "Invalid method found!"};
    }
    if (request_parts.at(2) != "HTTP/1.1") {
        return {.value = 0, .error = "Only HTTP/1.1 is supported!"};
    }
    int bytes_consumed = request_line.length();
    this->request_line.method = request_parts.at(0);
    this->request_line.target = request_parts.at(1);

    std::string version_str = split(request_parts.at(2), "/").at(1);
    double version = std::stod(version_str);
    this->request_line.http_version = version;

    this->state = RequestState::parsing_headers;
    return {.value = bytes_consumed + 2};
}

ReturnError<int> Request::parse(std::array<char, MAX_LENGTH> data,
                                size_t bytes_length) {
    ReturnError<int> bytes_consumed;
    ReturnError<std::pair<bool, int>> header_ret;
    size_t content_length = 0;
    switch (state) {
        case RequestState::initialized:
            // parse the  request line
            bytes_consumed = this->parseRequestLine(data, bytes_length);
            break;
        case RequestState::parsing_headers:
            header_ret = this->headers.parse(data, bytes_length);
            bytes_consumed.value = header_ret.value.second;
            if (header_ret.error.has_value()) {
                return {.value = 0, .error = header_ret.error.value()};
            }
            if (header_ret.value.first) {
                if (headers.headers.find("content-length") ==
                    headers.headers.end()) {
                    
                    state = RequestState::finished;
                } else if (std::stod(headers.headers["content-length"]) == 0) {
                    state = RequestState::finished;
                } else {
                    state = RequestState::parsing_body;
                }
                return {.value = header_ret.value.second};
            }
            break;
        case RequestState::parsing_body:
            if (headers.headers.find("content-length") ==
                headers.headers.end()) {
                state = RequestState::finished;
                return {.value = 0};
            }
            for (int i = 0; i < bytes_length; i++) {
                body += data[i];
            }
            bytes_consumed.value = bytes_length;
            try {
                content_length = std::stod(headers.headers["content-length"]);
            } catch (std::exception &e) {
                return {.error = std::format("Invalid  Content-Length: {}",
                                             e.what())};
            }
            if (content_length <= 0) {
                return {.value = 0};
            }
            if (body.length() > content_length) {
                return {.error = std::format(
                            "Body greater than content  length({}  > {})",
                            body.length(), content_length)};
            } else if (body.length() == content_length) {
                state = RequestState::finished;
            }
            break;
        case RequestState::finished:
            break;
        default:
            return {};
    }
    return bytes_consumed;
}

Request Request::requestFromRead(std::array<char, MAX_LENGTH> bytes,
                                 size_t bytes_read) {
    Request out;
    while (out.state != RequestState::finished) {
    }
    return out;
}
