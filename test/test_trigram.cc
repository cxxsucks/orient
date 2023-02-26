#include <orient/fs/trigram.hpp>
#include <gtest/gtest.h>
#include <filesystem>
#include <random>
using namespace orie::dmp;

TEST(trigramParse, baseStrstr) {
    ASSERT_EQ(char_to_trigram('a', 'b', 'c'),
              char_to_trigram('A', 'b', 'c'));
    ASSERT_EQ(char_to_trigram('b', 'c', 'a'),
              char_to_trigram('b', 'C', 'a'));
    ASSERT_EQ(char_to_trigram('c', 'a', 'b'),
              char_to_trigram('c', 'a', 'B'));
    ASSERT_EQ(char_to_trigram('a', 'b', 'c'),
              char_to_trigram('A', 'B', 'C'));

    uint32_t buf[32] = {};
    ASSERT_EQ(5, strstr_trigram_ext(NATIVE_SV("0aaabcd"), buf, 32));
    // EXPECT_EQ(2068, buf[0]);
    // EXPECT_EQ(2073, buf[1]);

    EXPECT_EQ(0, strstr_trigram_ext(orie::sv_t(nullptr, 0), buf, 32));
    EXPECT_EQ(0, strstr_trigram_ext(NATIVE_SV(""), buf, 32));
    EXPECT_EQ(0, strstr_trigram_ext(NATIVE_SV("ab"), buf, 32));

    for (size_t i = 0; i <= 15; i++) 
        EXPECT_EQ(i, strstr_trigram_ext(NATIVE_SV("/*\\?[]56789012345"), buf, i));
    EXPECT_EQ(0, buf[15]);
    EXPECT_EQ(15, strstr_trigram_ext(NATIVE_SV("/*\\?[]56789012345"), buf, 32));
}

TEST(trigramParse, baseGlob) {
    uint32_t buf[32] = {};
    // This is basename and '/' has no special meaning
    ASSERT_EQ(glob_trigram_ext(NATIVE_SV("//////"), buf, 32, false),
              std::make_pair(size_t(4), true));
    EXPECT_EQ(buf[0], buf[1]);
    ASSERT_EQ(glob_trigram_ext(NATIVE_SV("\\\\\\\\\\\\"), buf, 32, false),
              std::make_pair(size_t(1), true));

    // [] pauses "streak" but do not reset
    ASSERT_EQ(glob_trigram_ext(NATIVE_SV("abc[def]ghi"), buf, 32, false),
              std::make_pair(size_t(2), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("]]][[*\\]]ghi"), buf, 32, false),
              std::make_pair(size_t(2), true));

    // Lone '[' and ']' are normal characters in basename glob patterns
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012[345"), buf, 32, false),
              std::make_pair(size_t(5), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012[34\\]567"), buf, 32, false),
              std::make_pair(size_t(8), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012]345"), buf, 32, false),
              std::make_pair(size_t(5), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012\\[34]567"), buf, 32, false),
              std::make_pair(size_t(8), true));

    // * and ? pauses "streak" but do not reset on basename
    ASSERT_EQ(glob_trigram_ext(NATIVE_SV("abc?def*ghi*"), buf, 32, false),
              std::make_pair(size_t(3), true));

    // No results
    EXPECT_EQ(glob_trigram_ext(orie::sv_t(nullptr, 0), buf, 32, false).first, 0);
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV(""), buf, 32, false).first, 0);
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("01"), buf, 32, false).first, 0);
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("01*01[23]?01?"), buf, 32, false).first, 0);
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("*?*?*[*?]?*?"), buf, 32, false).first, 0);
}

TEST(trigramParse, fullGlobNoSep) {
    uint32_t buf[32] = {};
    // `glob_trigram_ext` also works as long as '/' is not in pattern
    // [] pauses "streak" but do not reset
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("abcde"), buf, 32, true),
              std::make_pair(size_t(3), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("abc[def]ghi"), buf, 32, true),
              std::make_pair(size_t(2), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("abc[*\\]]ghi"), buf, 32, true),
              std::make_pair(size_t(2), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("]]][[*\\]]ghi"), buf, 32, true),
              std::make_pair(size_t(1), true));

    // * and ? reset all results
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("01234?0123*000*"), buf, 32, true),
              std::make_pair(size_t(1), false));
    EXPECT_EQ(buf[0], char_to_trigram('0', '0', '0') & 4095);
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("01234?0123*000"), buf, 32, true),
              std::make_pair(size_t(1), true));
    EXPECT_EQ(buf[0], char_to_trigram('0', '0', '0') & 4095);
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("01234?0123[*?]000"), buf, 32, true),
              std::make_pair(size_t(3), true));
    EXPECT_EQ(buf[2], char_to_trigram('0', '0', '0') & 4095);

    // Lone '[' and ']' reset results in fullpath glob
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012[345*?"), buf, 32, true),
              std::make_pair(size_t(1), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012[34\\]567"), buf, 32, true),
              std::make_pair(size_t(1), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("0123]456"), buf, 32, true),
              std::make_pair(size_t(1), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("012\\[34]5678"), buf, 32, true),
              std::make_pair(size_t(2), true));
    EXPECT_EQ(glob_trigram_ext(NATIVE_SV("0123]45"), buf, 32, true).first, 0);
}

TEST(trigramParse, fullStrstr) {
    uint32_t buf[32] = {};
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/01234"), false, buf, 32),
              std::make_pair(size_t(3), false)); // true means is basename
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/01234/"), false, buf, 32),
              std::make_pair(size_t(3), false));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/01234/01"), false, buf, 32),
              std::make_pair(size_t(3), false));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/01234/012"), false, buf, 32),
              std::make_pair(size_t(1), false));

    // Corner Cases
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV(""), false, buf, 32).first, 0);
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("0/0/0/"), false, buf, 32).first, 0);
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("////"), false, buf, 32).first, 0);
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012////"), false, buf, 32),
              std::make_pair(size_t(1), false));
}

TEST(trigramParse, fullGlob) {
    uint32_t buf[32] = {};
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/*01234"), true, buf, 32),
              std::make_pair(size_t(3), true));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/*01234?"), true, buf, 32),
              std::make_pair(size_t(3), false));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/*01234/"), true, buf, 32),
              std::make_pair(size_t(3), false));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/*01234/01"), true, buf, 32),
              std::make_pair(size_t(3), false));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("012345/*01234/*012"), true, buf, 32),
              std::make_pair(size_t(1), true));

    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("*012[345]678"), true, buf, 32),
              std::make_pair(size_t(2), true));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("*012[345]67"), true, buf, 32),
              std::make_pair(size_t(1), true));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("*012[345678"), true, buf, 32),
              std::make_pair(size_t(1), true));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("*012]345678"), true, buf, 32),
              std::make_pair(size_t(4), true));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("*012[34/5]6789"), true, buf, 32),
              std::make_pair(size_t(2), true));
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("*012[34/5]67"), true, buf, 32),
              std::make_pair(size_t(1), false));

    // Corner Cases
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV(""), true, buf, 32).first, 0);
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("?/*/[]/"), true, buf, 32).first, 0);
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("////"), true, buf, 32).first, 0);
    EXPECT_EQ(fullpath_trigram_ext(NATIVE_SV("?012[34]////"), true, buf, 32),
              std::make_pair(size_t(1), false));
}

// Simulate a search from 100K "batchs" spanning 4 "pages" with each batch
// containing ~400 trigrams each (400 characters)
// The target "Hello World" is in batch 90K, at the beginning of last page.
const std::filesystem::path tmpPath(std::filesystem::temp_directory_path() /
                                    "trigramTest");
static void trigramTestFilePrep() {
    static std::mutex testFileMut;
    std::lock_guard __lck(testFileMut);
    if (std::filesystem::exists(tmpPath) &&
        std::filesystem::file_size(tmpPath) >= 1000000)
        return;

    orie::char_t buf[401] = {};
    std::mt19937 rd(123456789); // For reproduceability
    std::uniform_int_distribution<orie::char_t> dist(0x21, 0x7e);
    arr2d_writer writer(tmpPath.native());

    for (size_t i = 0; i < 90000; i++) {
        for (size_t j = 0; j < 400; j++)
            buf[j] = dist(rd);
        place_trigram(orie::sv_t(buf, 400), i, writer);
        if (i % 30000 == 29999)
            writer.append_pending_to_file();
    }

    place_trigram(NATIVE_SV("Hello World!"), 90000, writer);
    for (size_t i = 90001; i < 100000; i++) {
        for (size_t j = 0; j < 400; j++)
            buf[j] = dist(rd);
        place_trigram(orie::sv_t(buf, 400), i, writer);
    }
    writer.append_pending_to_file();
}

static std::pair<bool, bool> // Has result and is fullpath
_do_tests(orie::sv_t pat, bool glob, bool full) {
    trigramTestFilePrep();
    arr2d_reader reader(tmpPath.c_str());
    trigram_query query(&reader);

    glob ? query.reset_glob_needle(pat, full)
         : query.reset_strstr_needle(pat, full);
    uint32_t res;
    while ((res = query.next_batch_possible()) < 90000)
        ; // Skip first 3 pages
    return std::make_pair(res == 90000, query.is_fullpath());
}

TEST(trigramSearch, strstr) {
    EXPECT_EQ(_do_tests(NATIVE_SV("hello"), false, false),
              std::make_pair(true, false));
    EXPECT_EQ(_do_tests(NATIVE_SV("he1lo"), false, false),
              std::make_pair(false, false));
}

TEST(trigramSearch, glob) {
    EXPECT_EQ(_do_tests(NATIVE_SV("hel[12]rld"), true, false),
              std::make_pair(true, false));
    EXPECT_EQ(_do_tests(NATIVE_SV("hel*12rld"), true, false),
              std::make_pair(false, false));
}

TEST(trigramSearch, fullStrstr) {
    EXPECT_EQ(_do_tests(NATIVE_SV("abcde/world"), false, true),
              std::make_pair(true, true));
    EXPECT_EQ(_do_tests(NATIVE_SV("world/ab"), false, true),
              std::make_pair(true, true));
    EXPECT_EQ(_do_tests(NATIVE_SV("wor1d/ab"), false, true),
              std::make_pair(false, true));
}

TEST(trigramSearch, fullGlob) {
    EXPECT_EQ(_do_tests(NATIVE_SV("hallo/World"), true, true),
              std::make_pair(true, false));
    EXPECT_EQ(_do_tests(NATIVE_SV("hallo/Wo*ld"), true, true),
              std::make_pair(false, true));
    EXPECT_EQ(_do_tests(NATIVE_SV("Hallo?World"), true, true),
              std::make_pair(true, false));
    EXPECT_EQ(_do_tests(NATIVE_SV("Hallo[a ]World"), true, true),
              std::make_pair(false, false));
    EXPECT_EQ(_do_tests(NATIVE_SV("Hello*ab"), true, true),
              std::make_pair(true, true));
}

TEST(trigramSearch, corner) {
    trigramTestFilePrep();
    trigram_query query; // Null reader AND no pattern
    EXPECT_EQ(query.next_batch_possible(), ~uint32_t());

    // Null reader, has pattern
    query.reset_strstr_needle(NATIVE_SV("Hello"), false);
    EXPECT_TRUE(query.trigram_avaliable());
    EXPECT_EQ(query.next_batch_possible(), ~uint32_t());

    // Has reader, has pattern (normal)
    arr2d_reader reader(tmpPath.c_str());
    query.reset_reader(&reader); 
    EXPECT_TRUE(query.trigram_avaliable());
    EXPECT_NE(query.next_batch_possible(), ~uint32_t());

    // Has reader, no pattern
    query.reset_strstr_needle(NATIVE_SV(""), false);
    EXPECT_FALSE(query.trigram_avaliable());
    EXPECT_EQ(query.next_batch_possible(), ~uint32_t());

    // Very short pattern
    query.reset_strstr_needle(NATIVE_SV("dd"), false);
    EXPECT_FALSE(query.trigram_avaliable());
    EXPECT_EQ(query.next_batch_possible(), ~uint32_t());

    // Very long pattern
    query.reset_glob_needle(NATIVE_SV("abcdefg*hijklmn?[opq].rst"), false);
    EXPECT_TRUE(query.trigram_avaliable());
    // NOTE: intersection stops prematurely when there are at most 4
    // results in a group since iterating 4 groups are faster than
    // intersection query. This especially works for long patterns.
    for (size_t i = 0; i < 11; ++i)
        query.next_batch_possible();
    EXPECT_EQ(query.next_batch_possible(), ~uint32_t());
}
