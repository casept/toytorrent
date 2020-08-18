#include "../bencode_parser.h"
#include <gtest/gtest.h>

#include <deque>
#include <optional>
#include <cstdint>

TEST(BEncode, parse_string) {
    const auto test_string = std::string{"3foogarbage"};
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
    const auto test_int = std::string{"i1337e3foo"};
    const std::int64_t expected = 1337;

    std::deque<char> data {test_int.begin(), test_int.end()};
    auto parser = BEncodeParser(data);
    auto got = parser.next();
    
    ASSERT_TRUE(got.has_value());
    ASSERT_TRUE(got.value().integer.has_value());
    ASSERT_EQ(got.value().type, BEncodeObjectType::Integer);
    ASSERT_EQ(expected, got.value().integer.value());
}

// TODO
TEST(BEncode, parse_dict) {
    ASSERT_TRUE(false);
}


TEST(BEncode, parse_list) {
    ASSERT_TRUE(true);
}