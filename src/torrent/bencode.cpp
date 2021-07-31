#include "bencode.hpp"

#include <ctype.h>
#include <fmt/core.h>

#include <algorithm>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace tt::bencode {
Exception::Exception(const std::string_view &msg) : m_msg(msg) {}

const char *Exception::what() const noexcept { return this->m_msg.c_str(); }

// Takes a bencoded string out of the stream.
std::string take_string(std::deque<char> &in) {
    std::string len_as_str{""};
    // Collect numbers until we find the first non-decimal char
    while (true) {
        char next_char = in.front();
        in.pop_front();
        if (isdigit(next_char)) {
            len_as_str.push_back(next_char);
        } else if (next_char == ':') {
            break;
        } else {
            // Before the ':', no other chars are allowed
            throw Exception{"Non-numeric characters not allowed in length specifier of BEncode string"};
        }
    }
    const std::int64_t len{std::stoll(len_as_str)};

    // ...and read the actual string

    // TODO: The spec states that the number is the "number of characters", but what does that mean for UTF-8 (grapheme
    // clusters/codepoints/...?) Therefore, assume it means the number of bytes (ASCII chars) for now.

    // For whatever reason (can't decipher the cryptic template error), copy_n
    // doesn't work directly on the deque into the string
    std::vector<char> data{};
    std::copy_n(in.begin(), len, std::back_inserter(data));
    std::string str(data.begin(), data.end());
    for (std::size_t i = 0; i < str.length(); i++) {
        in.pop_front();
    }
    return str;
}

// Takes a bencoded integer out of the stream.
std::int64_t take_integer(std::deque<char> &in) {
    std::string str_num{""};
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
            throw Exception{"Encountered unexpected character while trying to parse BEncoded integer"};
        }
    }
    return std::stoll(str_num);
}

std::map<std::string, Object> take_dict(std::deque<char> &in) {
    std::map<std::string, Object> out{};
    // Don't want the leading 'd'
    in.pop_front();
    while (true) {
        char next_char = in.front();
        // Check whether the dict is finished
        if (next_char == 'e') {
            in.pop_front();
            break;
        } else {
            // The dict key (must be a string)
            auto key = Object(in);
            if (key.type != ObjectType::String) {
                throw Exception{"BEncode dictionary keys must be strings"};
            }
            // The key's value
            auto value = Object(in);
            out.emplace(key.str.value(), value);
        }
    }
    return out;
}

std::vector<Object> take_list(std::deque<char> &in) {
    std::vector<Object> out{};
    // Don't want the leading 'l'
    in.pop_front();
    // Recursively extract list objects. If an object is followed by an 'e',
    // it was the last one in this list.
    while (true) {
        char next_char = in.front();
        auto str = std::string{"Next chara is "};
        str.push_back(next_char);
        if (next_char == 'e') {
            in.pop_front();
            break;
        } else {
            out.push_back(Object(in));
        }
    }
    return out;
}

bool operator==(const Object &lhs, const Object &rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    switch (lhs.type) {
        case ObjectType::Integer:
            return lhs.integer == rhs.integer;
        case ObjectType::String:
            return lhs.str == rhs.str;
        case ObjectType::Dict:
            return lhs.dict == rhs.dict;
        case ObjectType::List:
            return lhs.list == rhs.list;
    }
    // Should be unreachable
    return false;
}

Object::Object(std::deque<char> &in) {
    m_raw_bytes = std::vector<char>(in.begin(), in.end());

    // The type of a BEncode object depends on the first character.
    char first_char = in.front();

    // When we start off, all fields are none
    str = {};
    integer = {};
    dict = {};
    list = {};

    // The first character is a digit -> it's a length-prefixed string
    if (isdigit(first_char)) {
        type = {ObjectType::String};
        str = {std::optional<std::string>{take_string(in)}};
        return;
    }
    // The first character is i -> it's an integer
    switch (first_char) {
        case 'i': {
            type = {ObjectType::Integer};
            integer = {take_integer(in)};
            break;
        }
        // The first character is d -> it's a dict
        case 'd': {
            type = {ObjectType::Dict};
            dict = {take_dict(in)};
            break;
        }
        // The first character is l -> it's a list
        case 'l': {
            type = {ObjectType::List};
            list = {take_list(in)};
            break;
        }
        default: {
            auto char_str = std::string("");
            char_str.push_back(first_char);
            auto raw_str = std::string(m_raw_bytes.begin(), m_raw_bytes.end());
            const auto err = fmt::format("Unexpected BEncoded object type starting with character {} (raw data: {})",
                                         char_str, raw_str);
            throw Exception{err};
        }
    }
}

std::vector<char> Object::as_raw_bytes() { return m_raw_bytes; }

Parser::Parser(const std::deque<char> data) { m_data = data; }

std::optional<Object> Parser::next() {
    if (m_data.empty()) {
        return {};
    } else {
        return {Object(m_data)};
    }
}
}  // namespace tt::bencode