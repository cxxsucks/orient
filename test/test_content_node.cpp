#include "abunchofdirs.hpp"
#include <gtest/gtest.h>
#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <orient/pred_tree/async_job.hpp>

using namespace orie::pred_tree;

struct contentNode : public ::testing::Test {
    ABunchOfDirs info;

    size_t _do_tests(orie::pred_tree::fs_node& matcher) {
        fs_data_iter iter(info.dat.get());
        matcher.update_cost();
        orie::fifo_thpool pool(8);
        std::atomic<size_t> res = 0;

        async_job<fs_data_iter, orie::sv_t> job(
            iter, iter.end(), matcher, pool,
            [&res] (fs_data_iter&) { ++res; }, false
        );
        job.join();
        return res;
    }

    contentNode() {
        std::ofstream a(info.tmpPath / "dir1" / "file0");
        std::ofstream b(info.tmpPath / "dir2" / "file0");
        std::ofstream c(info.tmpPath / "dir3" / "file0");

        for (size_t i = 0; i < 10; ++i) {
            a << std::string(500, 'z'); //~5Kib
            b << std::string(1000, 'z'); //~10Kib
            (c << "Hello\nWorld!\n").put('\0');
        }
        a << "\nHello\nWorld\n"; 
        b << "\nHello\nWorld\n"; 
    }
};

TEST_F(contentNode, blockedStrstr) {
    // No bin, no icase
    content_strstr_node matcher(true, false, false);
    ASSERT_TRUE(matcher.next_param("Hello\nWorld"));
    EXPECT_EQ(2, _do_tests(matcher));

    // bin, icase
    matcher = content_strstr_node(true, true, true);
    ASSERT_TRUE(matcher.next_param("hElLo\nWoRlD"));
    EXPECT_EQ(3, _do_tests(matcher));
}

TEST_F(contentNode, blockedRegex) {
    // No bin, icase
    content_regex_node matcher(true, false, true);
    ASSERT_TRUE(matcher.next_param("He.lO"));
    EXPECT_EQ(2, _do_tests(matcher));

    // bin, no icase
    matcher = content_regex_node(true, true, true);
    ASSERT_TRUE(matcher.next_param("z{5001}"));
    EXPECT_EQ(1, _do_tests(matcher));
}


TEST_F(contentNode, strstr) {
    // No bin, no icase
    content_strstr_node matcher(false, false, false);
    ASSERT_TRUE(matcher.next_param("Hello\nWorld"));
    EXPECT_EQ(2, _do_tests(matcher));

    // bin, icase
    matcher = content_strstr_node(false, true, true);
    ASSERT_TRUE(matcher.next_param("hElLo\nWoRlD"));
    EXPECT_EQ(3, _do_tests(matcher));
}

TEST_F(contentNode, regex) {
    // No bin, icase
    content_regex_node matcher(false, false, true);
    ASSERT_TRUE(matcher.next_param("He.lO"));
    EXPECT_EQ(2, _do_tests(matcher));

    // bin, no icase
    matcher = content_regex_node(false, true, true);
    ASSERT_TRUE(matcher.next_param("z{5001}"));
    EXPECT_EQ(1, _do_tests(matcher));
}
