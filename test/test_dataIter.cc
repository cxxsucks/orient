#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <orient/fs/dumper.hpp>
#include <orient/fs/data_iter.hpp>

using namespace std::filesystem;
using orie::dmp::dir_dumper;
using orie::fs_data_iter;

#ifdef _WIN32
#define sleep(sec) Sleep(sec * 1000)
#endif

struct dataIter : public ::testing::Test {
    path tmpPath;
    dir_dumper* dmp = nullptr;
    int8_t* dat = nullptr;

    bool ok = true;

    dataIter() {
        tmpPath = std::filesystem::temp_directory_path() 
            / ("dataIter_tst_tmpdir"  + std::to_string(::time(nullptr)));
        ok &= create_directories(tmpPath / "dirA");
        ok &= create_directories(tmpPath / "dirB/dirBA");
        ok &= create_directories(tmpPath / "dirEmpty");
        create_symlink("..", tmpPath / "dirB" / "dirBA" / "linkBAB");

        ok &= std::ofstream(tmpPath / "fileA").is_open();
        ok &= std::ofstream(tmpPath / "fileB").is_open();
        ok &= std::ofstream(tmpPath / "dirA" / "fileAA").is_open();
        ok &= std::ofstream(tmpPath / "dirA" / "fileAB").is_open();
        ok &= std::ofstream(tmpPath / "dirB" / "fileBA").is_open();
        ok &= std::ofstream(tmpPath / "dirB" / "dirBA" / "fileBAA").is_open();

        if (ok) {
            dmp = new dir_dumper(tmpPath.native(), 0, nullptr);
            dmp->from_fs();
            if (dmp->n_bytes() > 20) {
                dat = new int8_t[dmp->n_bytes() + 1];
                dat[dmp->n_bytes()] = 0;
                dmp->to_raw(dat);
            }
            else ok = false;
        }
    }

    ~dataIter() {
        remove_all(tmpPath);
        delete dmp;
        delete []dat;
    }
};

static size_t _count_dataIt(fs_data_iter& it) {
    size_t elemCount = 0;
    while (it != fs_data_iter::end()) {
        ++it;
        ++elemCount;
    }
    return elemCount;
}

TEST_F(dataIter, increment) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    fs_data_iter it(dat);
    EXPECT_EQ(_count_dataIt(it), 11);
    EXPECT_THROW(it++, std::out_of_range);
    EXPECT_THROW(++it, std::out_of_range);
}

TEST_F(dataIter, updir) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    for (fs_data_iter it(dat); it != it.end(); ++it) {
        if (it.record().file_name_view() == NATIVE_PATH("fileAA")) {
            EXPECT_EQ(it.record(1).file_name_view(), NATIVE_PATH("dirA"));

            auto tmp = it; tmp.updir();
            EXPECT_EQ(tmp.record().file_name_view(), NATIVE_PATH("dirA"));
            EXPECT_EQ(tmp.parent_path(), tmpPath.native() + orie::separator);
        }

        if (it.record().file_name_view() == NATIVE_PATH("fileBAA")) {
            EXPECT_EQ(it.record(1).file_name_view(), NATIVE_PATH("dirBA"));
            EXPECT_EQ(it.record(2).file_name_view(), NATIVE_PATH("dirB"));
            EXPECT_EQ(it.record(3).file_name_view(), tmpPath.native());
            EXPECT_EQ(it.record(4).file_type(), orie::unknown_tag);
        }        
    }
}

TEST_F(dataIter, downdir) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    fs_data_iter it(dat), tmp;
    while (it != it.end() && it.record().file_name_view() != NATIVE_PATH("dirB"))
        ++it;
    tmp = it.current_dir_iter();
    EXPECT_EQ(_count_dataIt(tmp), 2);
}

TEST_F(dataIter, stat) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    fs_data_iter it(dat);
    while (it != it.end() && it.record().file_name_view() != NATIVE_PATH("dirB"))
        ++it;
    time_t orig = it->mtime();
    ::sleep(1);
    std::ofstream(tmpPath / "dirB" / "fileBC") << "RandomString";
    EXPECT_EQ(it->mtime(), orig);
#ifndef _WIN32
    EXPECT_EQ(it->uid(), ::getuid());
#else
    EXPECT_GT(it->atime(), orig);
#endif
    EXPECT_GT(it->mtime(), orig);
}

TEST_F(dataIter, changeRoot) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    fs_data_iter it(dat);
    it.change_root((tmpPath / "dirB").c_str());
    EXPECT_EQ(_count_dataIt(it), 4);
    it = fs_data_iter(dat, (tmpPath / "dirB").c_str());
    EXPECT_EQ(_count_dataIt(it), 4);
    it = fs_data_iter(dat, (tmpPath / "nonExistent").c_str());
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(dat, (tmpPath / "fileA").c_str());
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(nullptr, 0);
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(nullptr, (tmpPath / "dirB").c_str());
    EXPECT_EQ(it, it.end());
}
