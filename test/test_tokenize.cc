#include <gtest/gtest.h>
#include <orient/util/charconv_t.hpp>

static std::vector<std::string>
__do_totok(std::string_view content) {
    std::vector<std::string> ans;
    while (!content.empty()) {
        char buf[256];
        auto [read_sz, tok_sz] = orie::next_token(
            content.data(), content.size(), buf, 256
        );

        content.remove_prefix(read_sz);
        if (tok_sz == 0 && content.empty())
            return ans;
        ans.emplace_back(buf, tok_sz);
    }
    return ans;
}

TEST(tokenize, space) {
    EXPECT_TRUE(__do_totok("      ").empty());
    EXPECT_TRUE(__do_totok("\t\n   ").empty());

    std::vector<std::string> ans = {"aaa", "bbb"};
    EXPECT_EQ(ans, __do_totok("aaa bbb"));
    EXPECT_EQ(ans, __do_totok("   aaa   bbb   "));
    EXPECT_EQ(ans, __do_totok("\taaa\nb'b'b\t"));

    // \0 has no special meaning
    EXPECT_EQ(1, __do_totok(std::string_view("\t\0\n   ", 6)).size());
}

TEST(tokenize, quote) {
    EXPECT_TRUE(__do_totok("''``''").empty());
    EXPECT_TRUE(__do_totok("   ''``''\"\"   ").empty());

    EXPECT_THROW(__do_totok("'"), std::out_of_range);
    EXPECT_THROW(__do_totok("'''''"), std::out_of_range);
    EXPECT_THROW(__do_totok("`'`'"), std::out_of_range);
    EXPECT_NO_THROW(__do_totok("`\\`"));

    std::vector<std::string> ans = {"", "", "'"};
    EXPECT_EQ(ans, __do_totok("'' \"\"`` `'`"));

    ans = {"a'a", "b`b"};
    EXPECT_EQ(ans, __do_totok("a\\'a b\\`b"));
    EXPECT_EQ(ans, __do_totok("a\"'a\" \"b`b\""));
    EXPECT_EQ(ans, __do_totok("a\"'a\" 'b`b'"));
    EXPECT_EQ(ans, __do_totok("a`'a` 'b`b'"));
}

TEST(tokenize, backslash) {
    EXPECT_THROW(__do_totok("aaa\\"), std::out_of_range);
    EXPECT_THROW(__do_totok("\\\\\\"), std::out_of_range);
    EXPECT_NO_THROW(__do_totok("'aaa\\'"));
    EXPECT_THROW(__do_totok("'aaa\\"), std::out_of_range);

    std::vector<std::string> ans = {"a a", "b\\b"};
    EXPECT_EQ(ans, __do_totok("a\\ a b\\\\b"));
    EXPECT_EQ(ans, __do_totok("a\" a\" \"b\\b\""));
}

TEST(tokenize, stdString) {
    std::string_view tst = "1234567890\n"
        "1234567890123456789012345678901\n";
    EXPECT_EQ(std::make_pair(size_t(11), std::string("1234567890")),
              orie::next_token(tst.data(), tst.size()));
    tst.remove_prefix(10);
    EXPECT_EQ(std::string("1234567890123456789012345678901"),
              orie::next_token(tst.data(), tst.size()).second);
}

TEST(tokenize, buffer) {
    std::string_view str8 = "8InTotal '8InTotal' 8In'Total'",
        str9 = "9In\\ Total '9In Total' 9In' Total'";

    while (!str8.empty()) {
        char buf[10];
        ::memset(buf, 0x7f, 10);

        auto [read_sz, tok_sz] = orie::next_token(
            str8.data(), str8.size(), buf, 8
        );
        str8.remove_prefix(read_sz);
        EXPECT_EQ(buf[8], 0x7f); // Must not write out of buffer
        EXPECT_EQ(buf[7], 'l');
        EXPECT_EQ(tok_sz, 8);

        std::tie(read_sz, tok_sz) = 
            orie::next_token(str9.data(), str9.size(), buf, 8);
        EXPECT_EQ(read_sz, ~size_t());
        EXPECT_EQ(tok_sz, ~size_t());
        EXPECT_EQ(buf[8], 0x7f); // Must not write out of buffer

        std::tie(read_sz, tok_sz) =
            orie::next_token( str9.data(), str9.size(), buf, 9);
        EXPECT_EQ(tok_sz, 9);
        str9.remove_prefix(read_sz);
    }
}
