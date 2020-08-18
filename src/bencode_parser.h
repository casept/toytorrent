#ifndef TOYTORRENT_BENCODING_H_
#define TOYTORRENT_BENCODING_H_
#include <iostream>
#include <optional>
#include <vector>
#include <map>
#include <memory>
#include <deque>

// The various kinds of BEncode-representable objects
enum class BEncodeObjectType { 
    String,
    Integer,
    Dict,
    List
};

// This class describes a BEncoded object.
class BEncodeObject {
    public:
    // Extract the next possible object from a bencoded stream. 
    BEncodeObject(std::deque<char> in);
    
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