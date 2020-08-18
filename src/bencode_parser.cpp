#include "bencode_parser.h"

#include <iostream>
#include <map>
#include <variant>
#include <optional>
#include <vector>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <string>
#include <deque>

#include <ctype.h>


// Takes a bencoded string out of the stream.
std::string take_bencode_string(std::deque<char> in) {
    std::string len_as_str {""};
    // Collect numbers until we find the first non-decimal char
    while (true) {
        char next_char = in.front();
        if (isdigit(next_char)) {
            len_as_str.push_back(next_char);
            in.pop_front();
        } else {
            // Must be the first char of the string itself
            break;
        }
    }
    const std::int64_t len {std::stoll(len_as_str)};

    // ...and read the actual string
    
    // TODO: The spec states that the number is the "number of characters", but what does that mean for UTF-8 (grapheme clusters/codepoints/...?)
    // Therefore, assume it means the number of bytes (ASCII chars) for now.
    
    // For whatever reason (can't decipher the cryptic template error), copy_n
    // doesn't work directly on the deque into the string
    std::vector<char> data {};
    std::copy_n(in.begin(), len, std::back_inserter(data));
    std::string str(data.begin(), data.end());
    for (uint64_t i = 0; i <= str.length(); i++) {
        in.pop_front();
    }
    return str;
}

// Takes a bencoded integer out of the stream.
std::int64_t take_bencode_integer(std::deque<char> in) {
    std::string str_num {""};
    // Toss the leading i
    in.pop_front();

    // Keep reading until we encounter an 'e'
    while (true) {
        char next_char = in.front();
        in.pop_front();
        if (next_char == 'e') {
            break;
        } else if (isdigit(next_char)) {
            str_num.push_back(next_char);
        } else {
            throw std::runtime_error("Encountered unexpected character while trying to parse BEncoded integer");
        }
    }
    return std::stoll(str_num);
}

std::map<std::string, BEncodeObject> take_bencode_dict(std::deque<char> in) {
    // TODO: Implement
    auto _ = in;
    return {};
}

std::vector<BEncodeObject> take_bencode_list(std::deque<char> in) {
    // TODO: Implement
    auto _ = in;
    return {};
}


BEncodeObject::BEncodeObject(std::deque<char> in) {
    // The type of a BEncode object depends on the first character.
    char first_char = in.front();

    // When we start off, all fields are none
    str = {};
    integer = {};
    dict = {};
    list = {};

    // The first character is a digit -> it's a length-prefixed string
    if (isdigit(first_char)) {
        type = {BEncodeObjectType::String};
        str = {std::optional<std::string>{take_bencode_string(in)}};
    }
    // The first character is i -> it's an integer
    else if (first_char == 'i') {
        type = {BEncodeObjectType::Integer};
        integer = {take_bencode_integer(in)};
    }
    // The first character is d -> it's a dict
    else if (first_char == 'd') {
        type = {BEncodeObjectType::Dict};
        dict = {take_bencode_dict(in)};
    }
    // The first character is l -> it's a list
    else if (first_char == 'l') {
        type = {BEncodeObjectType::List};
        list = {take_bencode_list(in)};
    } else {
        auto err = std::string("Unexpected BEncoded object type starting with character ");
        err.push_back(first_char);
        throw std::runtime_error(err);
    }
}

BEncodeParser::BEncodeParser(const std::deque<char> data){
    m_data = data;
}

std::optional<BEncodeObject> BEncodeParser::next() {
    if (m_data.empty()) {
        return {};
    } else {
        return {BEncodeObject(m_data)};
    };
}