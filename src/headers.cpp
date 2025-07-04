#include "headers.h"

#include <cctype>
#include <cstdio>
#include <format>
#include <iostream>
#include <string>

#include "common.h"

Headers::Headers() { headers = {}; }

void Headers::set(std::string key, std::string val) { headers[key] = val; }

Headers Headers::default_headers(size_t content_length){
    Headers out;
    out.set("Content-Type", "text/plain");
    out.set("Content-Length", std::to_string(content_length));
    out.set("Connection", "close");
    return out;
}


ReturnError<std::pair<bool, int>> Headers::parse(
    std::array<char, MAX_LENGTH> bytes, int bytes_length) {
    std::string data;
    for (int i = 0; i < bytes_length; i++) {
        data += bytes[i];
    }
    if (data.find("\r\n") == std::string::npos) {
        // we haven't found a \r\n (CRLF) yet, and need to read more data
        return {.value = {false, 0}};
    }
    // we only want the first line:
    std::string line = split(data, "\r\n").at(0);
    // if the first line read is  empty,  we've finished reading headers
    if (line.length() == 0) {
        return {.value = {true, 2}};
    }
    if (line.find(":") == std::string::npos) {
        std::string msg = std::format("Missing \":\" in header: {}", line);
        return {.value =  {false, 0}, .error = msg};
    }
    std::string field_name = line.substr(0, line.find(":"));
    std::string field_value =
        line.substr(line.find(":") + 1, std::string::npos);

    if (rtrim(field_name) != field_name) {
        std::string msg = std::format("Invalid whitespace format: {}", line);
        return {.error = msg};
    }
    field_name = to_lower(trim(field_name));
    field_value = to_lower(trim(field_value));

    // check for  illegal chars
    if (!legal_token(field_name)) {
        std::string msg = std::format("Invalid characters in header: {}", line);
        return {.error = msg};
    }
    if (headers.find(field_name) == headers.end()) {
        // insert new header
        headers[field_name] = field_value;
    } else {
        headers[field_name] = headers[field_name] + ", " + field_value;
    }
    int bytes_consumed = line.length() + 2;

    return {.value = {false, bytes_consumed}};
}

bool Headers::legal_token(std::string str) {
    for (auto c : str) {
        bool valid = false;
        if (std::isalnum(c)) {
            continue;
        }
        for (auto symbol : valid_symbols) {
            if (c == symbol) {
                valid = true;
                break;
            }
        }
        if (!valid) return false;
    }
    return true;
}
