#include "abunchofdirs.hpp"
#include <gtest/gtest.h>
#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <orient/pred_tree/async_job.hpp>

using namespace orie::pred_tree;

struct actionNode : public ::testing::Test {
    ABunchOfDirs info;

    size_t _do_tests(orie::pred_tree::fs_node& matcher) {
        fs_data_iter iter(info.dat.get());
        matcher.update_cost();
        return std::count_if(iter, iter.end(), [&matcher] (auto& dat_it)
                             { return matcher.apply_blocked(dat_it); });
    }

    actionNode() : info(4) {
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

TEST_F(actionNode, execGrep) {
    type_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_SV("f")));
    exec_node exec(false, false);
    EXPECT_TRUE(exec.next_param(NATIVE_SV("grep")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("-lIi")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("hello")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("{}")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV(";")));
    EXPECT_FALSE(exec.next_param(NATIVE_SV("fooBar")));
    auto cond = matcher & exec;
    EXPECT_EQ(2, _do_tests(cond));
}

TEST_F(actionNode, execMultiBracket) {
    strstr_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_SV("file0")));
    exec_node exec(false, false);
    EXPECT_TRUE(exec.next_param(NATIVE_SV("cp")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("{}")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("{}.new")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV(";")));
    EXPECT_FALSE(exec.next_param(NATIVE_SV("fooBar")));
    // All "file0"s are copied
    auto cond = matcher & exec;
    size_t orig = _do_tests(cond);

    info.refreshDat();
    EXPECT_EQ(orig * 2, _do_tests(matcher));
}

TEST_F(actionNode, execPlus) {
    // TODO: reliant on reading stdout
    type_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_SV("f")));
    exec_node exec(false, false);
    EXPECT_TRUE(exec.next_param(NATIVE_SV("grep")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("-lIi")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("hello")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("{}")));
    EXPECT_TRUE(exec.next_param(NATIVE_SV("+")));
    EXPECT_FALSE(exec.next_param(NATIVE_SV("fooBar")));
    // Output 2 lines
    auto cond = matcher & exec;
    _do_tests(cond);
}

TEST_F(actionNode, execSubdir) {
    // TODO: reliant on reading stdout
    exec_node matcher(false, true);
    EXPECT_TRUE(matcher.next_param(NATIVE_SV("sh")));
    EXPECT_TRUE(matcher.next_param(NATIVE_SV("-c")));
    // .../dir3/dir2/dir1 and a newline
#ifdef _WIN32
    auto test_str = std::to_wstring(info.tmpPath.native().size() + 16);
#else
    auto test_str = std::to_string(info.tmpPath.native().size() + 16);
#endif
    test_str = NATIVE_PATH("test $(pwd | wc -c) -eq ") + test_str;
    EXPECT_TRUE(matcher.next_param(test_str));
    EXPECT_TRUE(matcher.next_param(NATIVE_SV(";")));
    EXPECT_FALSE(matcher.next_param(NATIVE_SV("fooBar")));
    // Output 2 lines
    EXPECT_EQ(2, _do_tests(matcher));
}

TEST_F(actionNode, delNode) {
    regex_node matcher;
    del_node del;
    ASSERT_TRUE(matcher.next_param(NATIVE_SV(".*[01]")));
    EXPECT_FALSE(del.next_param(NATIVE_SV("fooBar")));
    auto cond = matcher & del;
    ASSERT_EQ(24, _do_tests(cond));

    info.refreshDat();
    EXPECT_EQ(0, _do_tests(matcher));
    auto no = ~matcher;
    EXPECT_EQ(7, _do_tests(no));
}

TEST_F(actionNode, fprint0) {
    print_node print(false, '\0'); // -fprint0
    orie::str_t out_path = (info.tmpPath / "a.out").native();
    ASSERT_TRUE(print.next_param(out_path));
    EXPECT_FALSE(print.next_param(NATIVE_SV("fooBar")));
    EXPECT_EQ(31, _do_tests(print));

    // Close the printing stream by resetting the node
    print = print_node(false, '\n');  
    // Throws if unable to open output file
#ifndef _WIN32
    // TODO: no chmod on Windows
    ::chmod(out_path.c_str(), 0444); // -r--r--r--
    ASSERT_THROW(print.next_param(out_path), std::runtime_error);
    ASSERT_THROW(_do_tests(print), uninitialized_node); // No file opened
#endif // !_WIN32

    // There should be 31 null separators in output file
    orie::str_t out_content;
    std::getline(std::basic_ifstream<orie::char_t>(
        out_path, std::ios_base::binary
    ), out_content);
    EXPECT_EQ(31, std::count(out_content.begin(), out_content.end(), '\0'));

    // FIXME: no write error when write to file fails
    // ASSERT_TRUE(print.next_param("/dev/full"));
    // EXPECT_EQ(0, _do_tests(print));
}
