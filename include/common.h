#pragma once
#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

#define MAX_LENGTH 1024

template <typename T>
struct ReturnError {
    T value;
    std::optional<std::string> error;
};

inline std::vector<std::string> split(std::string str, std::string delim) {
    std::vector<std::string> output;
    while (str.find(delim) != std::string::npos) {
        output.push_back(str.substr(0, str.find(delim)));
        str.erase(0, str.find(delim) + delim.length());
    }
    output.push_back(str);
    return output;
}

inline std::string to_lower(std::string str) {
    std::string out;
    for (auto c : str) {
        out += std::tolower(c);
    }
    return out;
}

inline std::string rtrim(std::string str) {
    std::string::iterator first_non_whitespace =
        std::find_if(str.rbegin(), str.rend(), [](unsigned char c) {
            return !std::isspace(c);
        }).base();
    str.erase(first_non_whitespace, str.end());
    return str;
}
inline std::string ltrim(std::string str) {
    std::string::iterator first_non_whitespace =
        std::find_if(str.begin(), str.end(),
                     [](unsigned char c) { return !std::isspace(c); });

    str.erase(str.begin(), first_non_whitespace);
    return str;
    ;
}
inline std::string trim(std::string str) {
    str = rtrim(str);
    str = ltrim(str);
    return str;
}

// returns the subarray from begin to the end of bytes
inline std::array<char, MAX_LENGTH> slice(std::array<char, MAX_LENGTH> bytes,
                                          size_t begin) {
    std::array<char, MAX_LENGTH> out;
    std::copy(bytes.begin()+begin, bytes.end(),  out.begin());
    return out;
}
