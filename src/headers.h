#pragma once
#include <map>

#include "common.h"

	// reg := regexp.MustCompile("[a-zA-Z0-9!#$%&'*+-.^_`|~]+")
inline constexpr  std::array<char, 16> valid_symbols = {
    '!',
    '#',
    '$',
    '%',
    '&',
    '\'',
    '*',
    '+',
    '-',
    '.',
    '^',
    '_',
    '`',
    '|',
    '~',
    ']',
};
class Headers {
   private:
    bool legal_token(std::string str);

   public:
    std::map<std::string, std::string> headers;
    Headers();
    void set(std::string key, std::string val);
    void add(std::string key, std::string val);
    std::string get(std::string key);
    static Headers default_headers(size_t content_length);

    ReturnError<std::pair<bool, int>> parse(std::array<char, MAX_LENGTH> bytes,
                                            int bytes_length);
};
