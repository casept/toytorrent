#ifndef TOYTORRENT_BENCODING_H_
#define TOYTORRENT_BENCODING_H_
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <memory>
#include <deque>
#include <string>

// The various kinds of BEncode-representable objects
enum class BEncodeObjectType { 
    String,
    Integer,
    Dict,
    List
};

// This class describes a BEncoded object.
class BEncodeObject {
    private:
    // This is used so we can get the raw contents for infohash computation.
    std::vector<char> m_raw_bytes;

    public:
    // This object shouldn't be constructed by hand
    BEncodeObject() = delete;
    // Extract the next possible object from a bencoded stream. 
    explicit BEncodeObject(std::deque<char> &in);
    // Get the object in it's BEncoded form
    std::vector<char> as_raw_bytes();
    // What kind of BEncode-representable object this is.
    BEncodeObjectType type;
    // Will be some if the object is a string
    std::optional<std::string> str;
    // Will be some if the object is an integer
    std::optional<std::int64_t> integer;
    // Will be some if the object is a dictionary
    std::optional<std::map<std::string, BEncodeObject>> dict;
    // Will be some if the object is a list
    std::optional<std::vector<BEncodeObject>> list;
};

// Used in tests
bool operator==(const BEncodeObject& lhs, const BEncodeObject& rhs);


// This class provides a bencode parser which can
// be used as an iterator to extract one decoded element at a time.
class BEncodeParser {
  private:
   std::deque<char> m_data;
  public:
    // Constructing one without a stream to read from is nonsense
    BEncodeParser() = delete;
    // Create a parser for the given BEncoded data
    explicit BEncodeParser(std::deque<char> data);
    // TODO: Implement a standard iterator
    // Returns the next member, if available, none otherwise
    std::optional<BEncodeObject> next();
};
#endif