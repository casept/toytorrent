#pragma once

#include <deque>
#include <exception>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace tt::bencode {
// Thrown on (de)serialization failure.
class Exception : public std::exception {
   public:
    std::string m_msg{};
    Exception(const std::string_view&);
    const char* what() const noexcept override;
};

// The various kinds of BEncode-representable objects
enum class ObjectType { String, Integer, Dict, List };

// This class describes a BEncoded object.
class Object {
   private:
    // This is used so we can get the raw contents for infohash computation.
    std::vector<char> m_raw_bytes;

   public:
    // This object shouldn't be constructed by hand
    Object() = delete;
    // Extract the next possible object from a bencoded stream.
    explicit Object(std::deque<char>& in);
    // Get the object in it's BEncoded form
    std::vector<char> as_raw_bytes();
    // What kind of BEncode-representable object this is.
    ObjectType type;
    // Will be some if the object is a string
    std::optional<std::string> str;
    // Will be some if the object is an integer
    std::optional<std::int64_t> integer;
    // Will be some if the object is a dictionary
    std::optional<std::map<std::string, Object>> dict;
    // Will be some if the object is a list
    std::optional<std::vector<Object>> list;
};

// Used in tests
bool operator==(const Object& lhs, const Object& rhs);

// This class provides a bencode parser which can
// be used as an iterator to extract one decoded element at a time.
class Parser {
   private:
    std::deque<char> m_data;

   public:
    // Constructing one without a stream to read from is nonsense
    Parser() = delete;
    // Create a parser for the given BEncoded data
    explicit Parser(std::deque<char> data);
    // TODO: Implement a standard iterator
    // Returns the next member, if available, none otherwise
    std::optional<Object> next();
};
}  // namespace tt::bencode