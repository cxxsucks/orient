#include <gtest/gtest.h>
#include <orient/pred_tree/builder.hpp>
#include <chrono>

using test_builder_type = orie::pred_tree::builder<const int, std::string_view>;

static void __do_test(int argc0, const char* const* argv0, bool expect)
{
    std::string expr_str;
    for (int i = 1; i < argc0; ++i)
        (expr_str += argv0[i]) += ' ';

    test_builder_type builder;
    // All pred trees here only consist of `true false not` whose result is 
    // constant and has nothing to with the value (0 here)
    EXPECT_EQ(expect, builder.build(argc0, argv0)->apply_blocked(0))
        << "Failed (command as argc and argv):" << expr_str;
    EXPECT_EQ(expect, builder.build(expr_str)->apply_blocked(0))
        << "Failed (command as a string):" << expr_str;
}

TEST(builder, pred) {
    const char* cmd1[2] = {"./a.out", "-true"},
        *cmd2[] = {"", "(", "-true", ")"},
        *cmd3[] = {"", "-true", "-or", "-false"},
        // Test 4 is the first 7 args of Test 5
        *cmd45[] = {"", "-true", "-a", "(", "-false", "-o", "-true", ")", "-and", "-false"},
        *cmd6[] = {"", "-true", "-a", "(", "-true", ")"};

    __do_test(2, cmd1, true);
    __do_test(4, cmd2, true);
    __do_test(4, cmd3, true);
    __do_test(8, cmd45, true);
    __do_test(10, cmd45, false);
    __do_test(6, cmd6, true);
}

TEST(builder, modifier) {
    const char* cmd1[] = {"./a.out", "!", "-true"},
        *cmd2[] = {"", "!", "(", "-true", ")"},
        *cmd3[] = {"", "!", "(", "-true", "-o", "-false", ")"},
        *cmd4[] = {"", "-true", "-a", "!", "(", "-false", "-o", "-true", ")" },
        *cmd5[] = {"", "-not", "-true", "-o", "!", "(", "-false", "-o", "-true", ")"};

    __do_test(3, cmd1, false);
    __do_test(5, cmd2, false);
    __do_test(7, cmd3, false);
    __do_test(9, cmd4, false);
    __do_test(10, cmd5, false);
}

TEST(builder, nextParam) {
    const char* cmd1[] = {"./a.out", "-testuse", "aaa"},
        *cmd2[] = {"", "(", "-testuse", "aaa", ")"},
        *cmd3[] = {"", "-testuse", "aaa", "-or", "!", "-testuse", "bbb"},
        *cmd4[] = {"", "!", "-testuse", "bbb", "-o", 
                   "!", "(", "-false", "-o", "-testuse", "aaa", ")"};

    __do_test(3, cmd1, true);
    __do_test(5, cmd2, true);
    __do_test(7, cmd3, true);
    __do_test(12, cmd4, false);
}

TEST(builder, fallback) {
    const char* cmd1[] = {"./a.out", "-true", "-true"},
        *cmd2[] = {"", "(", "-true", ")", "-true"},
        *cmd3[] = {"", "(", "-true", "-true", ")", "-true"},
        // Test 4 is the first 7 args of Test 5
        *cmd45[] = {"", "-true", "(", "-false", "-o", "-true", ")", "-false"};

    __do_test(3, cmd1, true);
    __do_test(5, cmd2, true);
    __do_test(6, cmd3, true);
    __do_test(7, cmd45, true);
    __do_test(8, cmd45, false);
}

TEST(builder, fail) {
    test_builder_type tst;
    EXPECT_THROW(tst.build("-ture"), orie::pred_tree::unknown_node_name);
    EXPECT_THROW(tst.build("-testuse a error"), orie::pred_tree::unknown_node_name);

    EXPECT_THROW(tst.build("( -true"), orie::pred_tree::parentheses_mismatch);
    EXPECT_THROW(tst.build("-true )"), orie::pred_tree::parentheses_mismatch);
    EXPECT_THROW(tst.build("( ( -true )"), orie::pred_tree::parentheses_mismatch);
    EXPECT_THROW(tst.build("( -true ) )"), orie::pred_tree::parentheses_mismatch);
    EXPECT_THROW(tst.build("-true ( )"), orie::pred_tree::empty_parentheses);

    // Missing predicates at end of expression
    EXPECT_THROW(tst.build(""), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-true -a"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-true ( -true -a ) -true"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-true ( -true -a ! ) -true"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-true ( -true ! ) -true"), orie::pred_tree::missing_predicate);

    // Missing predicates in the middle of expression
    EXPECT_THROW(tst.build("-a -false"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-a ! -false"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("! -a false"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-true -a -o -false"), orie::pred_tree::missing_predicate);
    EXPECT_THROW(tst.build("-true -a ! -o -false"), orie::pred_tree::missing_predicate);

    tst.set_bridge_fallback("");
    EXPECT_THROW(tst.build("-true -false"), orie::pred_tree::missing_bridge);
    EXPECT_THROW(tst.build("-true ( -false -o -true )"), orie::pred_tree::missing_bridge);
}

TEST(builder, updateCost) {
    std::string bm = "( -false -o -true )";
    for (size_t i = 0; i < 15; ++i)
        bm = "( " + bm + " -a " + bm + " )";

    test_builder_type slow, fast;
    slow.build(bm);
    bm += " -a -false";
    fast.build(bm);
    fast.get()->update_cost();

    auto start_time = std::chrono::system_clock::now();
    ASSERT_TRUE(slow.get()->apply_blocked(0));
    time_t slow_cost = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now() - start_time
    ).count();

    start_time = std::chrono::system_clock::now();
    ASSERT_FALSE(fast.get()->apply_blocked(0));
    time_t fast_cost = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now() - start_time
    ).count();
    EXPECT_LT(fast_cost, slow_cost);
}
