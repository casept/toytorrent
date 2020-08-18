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
#include <utility>

#include <ctype.h>


// Takes a bencoded string out of the stream.
std::string take_bencode_string(std::deque<char> &in) {
    std::string len_as_str {""};
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
            throw std::runtime_error{"Non-numeric characters not allowed in length specifier of BEncode string"};
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
    for (std::size_t i = 0; i < str.length(); i++) {
        in.pop_front();
    }
    return str;
}

// Takes a bencoded integer out of the stream.
std::int64_t take_bencode_integer(std::deque<char> &in) {
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

std::map<std::string, BEncodeObject> take_bencode_dict(std::deque<char> &in) {
    std::map<std::string, BEncodeObject> out {};
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
            auto key = BEncodeObject(in);
            if (key.type != BEncodeObjectType::String) {
                throw std::runtime_error{"BEncode dictionary keys must be strings"};
            }
            // The key's value
            auto value = BEncodeObject(in);
            out.emplace(key.str.value(), value);
        }
    }
    return out;
}

std::vector<BEncodeObject> take_bencode_list(std::deque<char> &in) {
    std::vector<BEncodeObject> out {};
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
            out.push_back(BEncodeObject(in));
        }
    }
    return out;
}

bool operator==(const BEncodeObject& lhs, const BEncodeObject& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    switch (lhs.type) {
        case BEncodeObjectType::Integer:
            return lhs.integer == rhs.integer;
        case BEncodeObjectType::String:
            return lhs.str == rhs.str;
        case BEncodeObjectType::Dict:
            return lhs.dict == rhs.dict;
        case BEncodeObjectType::List:
            return lhs.list == rhs.list;
    }
    // Should be unreachable
    return false;
}


BEncodeObject::BEncodeObject(std::deque<char> &in) {
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
        return;
    }
    // The first character is i -> it's an integer
    switch (first_char) {
    case 'i': {
        type = {BEncodeObjectType::Integer};
        integer = {take_bencode_integer(in)};
        break;
    }
    // The first character is d -> it's a dict
    case 'd': {
        type = {BEncodeObjectType::Dict};
        dict = {take_bencode_dict(in)};
        break;
    }
    // The first character is l -> it's a list
    case 'l': {
        type = {BEncodeObjectType::List};
        list = {take_bencode_list(in)};
        break;
    }
    default: {
        auto err = std::string("Unexpected BEncoded object type starting with character ");
        err.push_back(first_char);
        throw std::runtime_error(err);
    }
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