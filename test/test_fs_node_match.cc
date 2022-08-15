#include "abunchofdirs.hpp"
#include <gtest/gtest.h>
#include <orient/fs_pred_tree/fs_nodes.hpp>

using namespace orie::pred_tree;
static fs_data_iter __global_dummy_iter;

struct fsNodeMatch : public ::testing::Test {
    ABunchOfDirs info;

    size_t _do_tests(orie::pred_tree::fs_node& matcher) {
        fs_data_iter iter(info.dat.get());
        matcher.update_cost();
        return std::count_if(iter, iter.end(), [&matcher] (auto& dat_it)
                             { return matcher.apply_blocked(dat_it); });
    }
};

TEST_F(fsNodeMatch, glob) {
    glob_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("file?")));
    EXPECT_EQ(31, _do_tests(matcher));

    matcher = glob_node();
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("*ymli*")));
    EXPECT_EQ(2, _do_tests(matcher));
}

TEST_F(fsNodeMatch, strstr) {
    strstr_node matcher;
    // Parsing Errors
    ASSERT_THROW(matcher.next_param(NATIVE_PATH_SV("--foo")),
                 invalid_param_name);
    ASSERT_THROW(matcher.apply_blocked(__global_dummy_iter),
                 uninitialized_node);

    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("file")));
    EXPECT_FALSE(matcher.next_param(NATIVE_PATH_SV("file")));
    EXPECT_EQ(31, _do_tests(matcher));

    matcher = strstr_node();
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("ymli")));
    EXPECT_EQ(2, _do_tests(matcher));
}

TEST_F(fsNodeMatch, regex) {
    regex_node matcher;
    // Parsing Errors
    ASSERT_THROW(matcher.next_param(NATIVE_PATH_SV("--foo")),
                 invalid_param_name);
    ASSERT_THROW(matcher.next_param(NATIVE_PATH_SV("[")),
                 std::runtime_error);
    ASSERT_THROW(matcher.apply_blocked(__global_dummy_iter),
                 uninitialized_node);

    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("file\\d")));
    EXPECT_FALSE(matcher.next_param(NATIVE_PATH_SV("file\\d")));
    EXPECT_EQ(31, _do_tests(matcher));

    matcher = regex_node();
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV(".*ymli.*")));
    EXPECT_EQ(2, _do_tests(matcher));
}

TEST_F(fsNodeMatch, icase) {
    glob_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("--ignore-case")));
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("--full")));
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("*DiR4*")));
    EXPECT_EQ(32, _do_tests(matcher));
    matcher = glob_node(false, false, true);
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("fIlE?")));
    EXPECT_EQ(31, _do_tests(matcher));

    regex_node matcher2(false, false, false, true);
    ASSERT_TRUE(matcher2.next_param(NATIVE_PATH_SV("DiR\\d")));
    EXPECT_EQ(31, _do_tests(matcher2));
    matcher2 = regex_node(true, false, false, true);
    ASSERT_TRUE(matcher2.next_param(NATIVE_PATH_SV(".*DIr[0-4].*")));
    EXPECT_EQ(59, _do_tests(matcher2));
}

TEST_F(fsNodeMatch, fullpath) {
    strstr_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("--full")));
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("dir4")));
    EXPECT_EQ(32, _do_tests(matcher));

    regex_node matcher2;
    ASSERT_TRUE(matcher2.next_param(NATIVE_PATH_SV("--full")));
    ASSERT_TRUE(matcher2.next_param(NATIVE_PATH_SV(".*dir[0-4].*")));
    EXPECT_EQ(59, _do_tests(matcher2));
}

TEST_F(fsNodeMatch, lname) {
    strstr_node str_lname(false, true, false);
    glob_node glob_lname(false, true, false);
    // Also tests regex_node's exact matching (2nd param)
    regex_node regex_lname(false, true, true, false);

    ASSERT_TRUE(str_lname.next_param(NATIVE_PATH_SV("file1")));
    ASSERT_TRUE(glob_lname.next_param(NATIVE_PATH_SV("*file1")));
    ASSERT_TRUE(regex_lname.next_param(NATIVE_PATH_SV(".*file1")));
    // 2 symlinks
    EXPECT_EQ(2, _do_tests(str_lname));
    EXPECT_EQ(2, _do_tests(glob_lname));
    EXPECT_EQ(2, _do_tests(regex_lname));
}

TEST_F(fsNodeMatch, type) {
    type_node matcher;
    ASSERT_TRUE(matcher.next_param(NATIVE_PATH_SV("f")));
    EXPECT_EQ(31, _do_tests(matcher));
    ASSERT_TRUE((matcher = type_node()).next_param(NATIVE_PATH_SV("l")));
    EXPECT_EQ(2, _do_tests(matcher));
    ASSERT_TRUE((matcher = type_node()).next_param(NATIVE_PATH_SV("d")));
    EXPECT_EQ(31, _do_tests(matcher));
    ASSERT_TRUE((matcher = type_node()).next_param(NATIVE_PATH_SV("l,f")));
    EXPECT_EQ(33, _do_tests(matcher));
    ASSERT_TRUE((matcher = type_node()).next_param(NATIVE_PATH_SV("l,d")));
    EXPECT_EQ(33, _do_tests(matcher));
    ASSERT_TRUE((matcher = type_node()).next_param(NATIVE_PATH_SV("f,d")));
    EXPECT_EQ(62, _do_tests(matcher));
    ASSERT_TRUE((matcher = type_node()).next_param(NATIVE_PATH_SV("f,l,d")));
    EXPECT_EQ(64, _do_tests(matcher));
}

TEST_F(fsNodeMatch, time) {
    // All files to be matched are created within 1 minute
    num_node matcher(num_node::stamp::AMIN);
    // Parse Errors
    ASSERT_THROW(matcher.next_param("1a"), not_a_number);
    ASSERT_THROW(matcher.next_param("+"), invalid_param_name);
    ASSERT_TRUE(matcher.next_param("-1"));
    EXPECT_EQ(64, _do_tests(matcher));

    matcher = num_node(num_node::stamp::MMIN);
    ASSERT_TRUE(matcher.next_param("+0"));
    EXPECT_EQ(0, _do_tests(matcher));

    matcher = num_node(num_node::stamp::CMIN);
    ASSERT_TRUE(matcher.next_param("0"));
    EXPECT_EQ(64, _do_tests(matcher));
}

TEST_F(fsNodeMatch, size) {
    std::ofstream(info.tmpPath / "dir0" / "largeSize")
        << std::string(69420, ' '); // 67.8KiB
    info.refreshDat();

    num_node matcher(num_node::stamp::SIZE);
    ASSERT_TRUE(matcher.next_param("68k"));
    EXPECT_EQ(1, _do_tests(matcher));

    matcher = num_node(num_node::stamp::SIZE);
    ASSERT_TRUE(matcher.next_param("+60k"));
    EXPECT_EQ(1, _do_tests(matcher));

    matcher = num_node(num_node::stamp::SIZE);
    ASSERT_TRUE(matcher.next_param("-68k"));
    EXPECT_EQ(64, _do_tests(matcher));
}

#ifdef __unix
TEST_F(fsNodeMatch, ugid) {
    char uid_cstr[10] = "", gid_cstr[10] = "";
    orie::to_char_t(uid_cstr, ::getuid());
    orie::to_char_t(gid_cstr, ::getgid());
    num_node matcher(num_node::stamp::UID);
    matcher.next_param(uid_cstr);
    EXPECT_EQ(64, _do_tests(matcher));
    matcher = num_node(num_node::stamp::GID);
    matcher.next_param(gid_cstr);
    EXPECT_EQ(64, _do_tests(matcher));

    // -username also permits uid. Test that here.
    username_node matcher2(false);
    matcher2.next_param(uid_cstr);
    EXPECT_EQ(64, _do_tests(matcher2));
    matcher2 = username_node(true);
    matcher2.next_param(gid_cstr);
    EXPECT_EQ(64, _do_tests(matcher2));

    orie::to_char_t(uid_cstr + 1, ::getuid()); uid_cstr[0] = '-';
    orie::to_char_t(gid_cstr + 1, ::getgid()); gid_cstr[0] = '-';
    matcher = num_node(num_node::stamp::UID);
    matcher.next_param(uid_cstr);
    EXPECT_EQ(0, _do_tests(matcher));
    matcher = num_node(num_node::stamp::GID);
    matcher.next_param(gid_cstr);
    EXPECT_EQ(0, _do_tests(matcher));
}

extern "C" {
#include <pwd.h>
#include <grp.h>
}

TEST_F(fsNodeMatch, username) {
    username_node matcher(false);
    matcher.next_param(::getpwuid(::getuid())->pw_name);
    EXPECT_EQ(64, _do_tests(matcher));
    matcher = username_node(true);
    matcher.next_param(::getgrgid(::getgid())->gr_name);
    EXPECT_EQ(64, _do_tests(matcher));
}

TEST_F(fsNodeMatch, perm) {
    if (::umask(022) != 022) {
        info.~ABunchOfDirs();
        new (&info) ABunchOfDirs;
    }

    access_node rw(R_OK | W_OK), x(X_OK);
    EXPECT_EQ(64, _do_tests(rw));
    EXPECT_EQ(31, _do_tests(x));

    perm_node exact, any, all;
    ASSERT_TRUE(exact.next_param("u+rw,g+r,o+r"));
    ASSERT_TRUE(any.next_param("/go+w,")); 
    ASSERT_TRUE(all.next_param("-0555"));
    EXPECT_EQ(33, _do_tests(exact)); // All files and links to file
    EXPECT_EQ(0, _do_tests(any));  // No writable files
    EXPECT_EQ(31, _do_tests(all)); // Everyone has rx, match dirs

    ::chmod((info.tmpPath / "dir4" / "file2").c_str(), 0666);
    EXPECT_EQ(32, _do_tests(exact)); 
    EXPECT_EQ(1, _do_tests(any));  
}
#endif
