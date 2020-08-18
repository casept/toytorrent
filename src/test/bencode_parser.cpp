#include "../bencode_parser.h"
#include <gtest/gtest.h>

#include <deque>
#include <optional>
#include <cstdint>
#include <map>

TEST(BEncode, parse_string) {
    const auto test_string = std::string{"3:foogarbage"};
    const auto expected = std::string{"foo"};

    std::deque<char> data {test_string.begin(), test_string.end()};
    auto parser = BEncodeParser(data);
    auto got = parser.next();
    
    ASSERT_TRUE(got.has_value());
    ASSERT_TRUE(got.value().str.has_value());
    ASSERT_EQ(got.value().type, BEncodeObjectType::String);
    ASSERT_EQ(expected, got.value().str.value());
}

TEST(BEncode, parse_integer) {
    const auto test_int = std::string{"i1337e3:foo"};
    const std::int64_t expected = 1337;

    std::deque<char> data {test_int.begin(), test_int.end()};
    auto parser = BEncodeParser(data);
    auto got = parser.next();
    
    ASSERT_TRUE(got.has_value());
    ASSERT_TRUE(got.value().integer.has_value());
    ASSERT_EQ(got.value().type, BEncodeObjectType::Integer);
    ASSERT_EQ(expected, got.value().integer.value());
}

TEST(BEncode, parse_list) {
    const auto test_list = std::string{"l11:list-item-111:list-item-2e"};

    std::deque<char> data {test_list.begin(), test_list.end()};
    auto parser = BEncodeParser(data);
    auto got = parser.next();
    
    ASSERT_TRUE(got.has_value());
    ASSERT_TRUE(got.value().list.has_value());
    ASSERT_EQ(got.value().type, BEncodeObjectType::List);
    auto got_list = got.value().list.value();
    ASSERT_EQ(got_list.size(), 2);
    // Elements 1 and 2 should be the expected strings
    ASSERT_EQ(got_list.at(0).type, BEncodeObjectType::String);
    ASSERT_EQ(got_list.at(1).type, BEncodeObjectType::String);
    ASSERT_EQ(got_list.at(0).str.value(), "list-item-1");
    ASSERT_EQ(got_list.at(1).str.value(), "list-item-2");

}

TEST(BEncode, parse_dict) {
    const auto test_dict = std::string{"d4:dictd3:1234:test3:4565:thinge4:listl11:list-item-111:list-item-2e6:numberi123456e6:string5:valuee"};

    std::deque<char> data {test_dict.begin(), test_dict.end()};
    auto parser = BEncodeParser(data);
    auto got = parser.next();
    
    ASSERT_TRUE(got.has_value());
    ASSERT_TRUE(got.value().dict.has_value());
    ASSERT_EQ(got.value().type, BEncodeObjectType::Dict);
    // TODO: Recursively check whether the values match, too tired right now
}