#include <gtest/gtest.h>
#include <orient/fs_pred_tree/fs_expr_builder.hpp>
#include <orient/fs_pred_tree/fs_nodes.hpp>
#include "abunchofdirs.hpp"

struct fsExprBuilder : public testing::Test {
    ABunchOfDirs info;

    size_t _do_tests(orie::pred_tree::fs_node& matcher) {
        fs_data_iter iter(info.dat.get());
        matcher.update_cost();
        size_t cnt = 0;
        try {
            while (iter != iter.end())  {
                if (matcher.apply_blocked(iter))
                    ++cnt;
                ++iter;
            }
        } catch(const orie::pred_tree::quitted&) { }
        
        return cnt;
        // libstdc++ *copies* the iterator in count_if, which invalidates `prune`
        // return std::count_if(iter, iter.end(), [&matcher] (auto& dat_it)
        //                      { return matcher.apply_blocked(dat_it); });
    }
};

TEST_F(fsExprBuilder, single) {
    orie::pred_tree::fs_expr_builder builder;
    builder.build(NATIVE_SV("-name --ignore-case --full *DiR4*"));
    EXPECT_EQ(32, _do_tests(*builder.get()));
    // The regex is illegal
    EXPECT_THROW(
        builder.build(NATIVE_SV("-regex --ignore-case --full *DiR4*")),
        std::runtime_error);
    builder.build(NATIVE_SV("-iregex .*DiR[0-4].*"));
    EXPECT_EQ(59, _do_tests(*builder.get()));
    EXPECT_FALSE(builder.has_action());
}

TEST_F(fsExprBuilder, cond) {
    orie::pred_tree::fs_expr_builder builder;
    builder.build(NATIVE_SV("-type f -o -type l"));
    EXPECT_EQ(33, _do_tests(*builder.get()));

    std::ofstream(info.tmpPath / "dir1" / "file0")
        << std::string(69420, ' '); // 67.8KiB
    builder.build(NATIVE_SV("-size -70k -size +60k "));
    EXPECT_EQ(1, _do_tests(*builder.get()));
    EXPECT_FALSE(builder.has_action());
}

// TODO: Tests for modifiers without builder

TEST_F(fsExprBuilder, updir) {
    orie::pred_tree::fs_expr_builder builder;
    builder.build(NATIVE_SV("-updir -strstr dir2"));
    EXPECT_EQ(16, _do_tests(*builder.get()));
    builder.build(NATIVE_SV("-updir ( -strstr dir2 ) -a -type f"));
    EXPECT_EQ(8, _do_tests(*builder.get()));
    builder.build(NATIVE_SV("-updir ( -updir ( -strstr dir2 ) )"));
    EXPECT_EQ(8, _do_tests(*builder.get()));
    EXPECT_FALSE(builder.has_action());
}

TEST_F(fsExprBuilder, downdir) {
    orie::pred_tree::fs_expr_builder builder;
    builder.build(NATIVE_SV("-downdir  -type l "));
    EXPECT_EQ(2, _do_tests(*builder.get()));
    builder.build(NATIVE_SV("-downdir ( -downdir ( -type -l ) )"));
    EXPECT_EQ(1, _do_tests(*builder.get()));
    builder.build(NATIVE_SV("-downdir -true"));
    EXPECT_EQ(15, _do_tests(*builder.get()));
    EXPECT_FALSE(builder.has_action());
}

TEST_F(fsExprBuilder, prune) {
    orie::pred_tree::fs_expr_builder builder;
    builder.build(NATIVE_SV(
        " ( -name dir4 -a -prune -a -false ) -o -name file0"
    )); // 8 file0 and NO dir4
    EXPECT_EQ(8, _do_tests(*builder.get()));
    EXPECT_FALSE(builder.has_action());

    builder.build(NATIVE_SV(
        " -prunemod -name dir4 -a -name file0"
    )); // 8 file0 and NO dir4
    EXPECT_EQ(8, _do_tests(*builder.get()));
}

TEST_F(fsExprBuilder, quit) {
    orie::pred_tree::fs_expr_builder builder;
    builder.build(NATIVE_SV(" -name file0 -a -quit 6")); 
    // Throws quitted exception when the 6th file0 is found
    EXPECT_EQ(5, _do_tests(*builder.get()));
    EXPECT_FALSE(builder.has_action());

    // Modifier form
    builder.build(NATIVE_SV("-quitmod 4 -name file0")); 
    EXPECT_EQ(3, _do_tests(*builder.get()));

    // Quit immediately when child returned true
    builder.build(NATIVE_SV("-quitmod -name file0")); 
    EXPECT_EQ(0, _do_tests(*builder.get()));
    builder.build(NATIVE_SV("-name file0 -quit")); 
    EXPECT_EQ(0, _do_tests(*builder.get()));

    // There are only 2 "dir3"
    builder.build(NATIVE_SV("-quitmod 6 -name dir3")); 
    EXPECT_EQ(2, _do_tests(*builder.get()));
}
