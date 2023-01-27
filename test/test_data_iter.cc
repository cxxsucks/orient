#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <orient/fs/dumper.hpp>
#include <orient/fs/data_iter.hpp>
#include <orient/util/fifo_thpool.hpp>

using namespace std::filesystem;
using orie::dmp::dumper;
using orie::fs_data_iter;

#ifdef _WIN32
#define sleep(sec) Sleep(sec * 1000)
#endif

inline orie::fifo_thpool __dummy_pool(0);
struct dataIter : public ::testing::Test {
    path tmpPath, dbPath;
    dumper* dmp = nullptr;

    dataIter() {
        tmpPath = std::filesystem::temp_directory_path() 
            / ("dataIter_tst_tmpdir"  + std::to_string(::time(nullptr)));
        dbPath = temp_directory_path() 
            / ("dataIter_tst_tmpdir_db" + std::to_string(::time(nullptr)));
        create_directories(tmpPath / "dirA");
        create_directories(tmpPath / "dirB/dirBA");
        create_directories(tmpPath / "dirEmpty");
        create_symlink("..", tmpPath / "dirB" / "dirBA" / "linkBAB");

        std::ofstream(tmpPath / "fileA").is_open();
        std::ofstream(tmpPath / "fileB").is_open();
        std::ofstream(tmpPath / "dirA" / "fileAA").is_open();
        std::ofstream(tmpPath / "dirA" / "fileAB").is_open();
        std::ofstream(tmpPath / "dirB" / "fileBA").is_open();
        std::ofstream(tmpPath / "dirB" / "dirBA" / "fileBAA").is_open();

        dmp = new dumper(dbPath.c_str(), __dummy_pool);
        dmp->_root_path = tmpPath.native();
        dmp->_noconcur_paths.push_back(tmpPath.native());
        dmp->rebuild_database();
    }

    ~dataIter() {
        remove_all(tmpPath);
        delete dmp;
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
    fs_data_iter it(&dmp->_data_dumped);
    EXPECT_EQ(_count_dataIt(it), 11);
    EXPECT_THROW(++it, std::out_of_range);
    EXPECT_THROW(it++, std::out_of_range);
}

TEST_F(dataIter, updir) {
    for (fs_data_iter it(&dmp->_data_dumped); it != it.end(); ++it) {
        if (it.record().file_name_view() == NATIVE_PATH("fileAA")) {
            auto tmp = it; tmp.updir();
            EXPECT_EQ(tmp.basename(), NATIVE_SV("dirA"));
            EXPECT_EQ(tmp.parent_path(), tmpPath.native() + orie::separator);
        }

        if (it.record().file_name_view() == NATIVE_PATH("fileBAA")) {
            auto tmp = it;
            EXPECT_EQ(tmp.updir().basename(), NATIVE_SV("dirBA"));
            EXPECT_EQ(tmp.updir().basename(), NATIVE_SV("dirB"));
            EXPECT_EQ(tmp.updir().basename(), tmpPath.filename().native());
            EXPECT_EQ(tmp.updir(), fs_data_iter::end());
        }        
    }
}

TEST_F(dataIter, downdir) {
    fs_data_iter it(&dmp->_data_dumped), tmp;
    while (it != it.end() && it.record().file_name_view() != NATIVE_PATH("dirB"))
        ++it;
    tmp = it.current_dir_iter();
    EXPECT_EQ(_count_dataIt(tmp), 2);
}

TEST_F(dataIter, stat) {
    fs_data_iter it(&dmp->_data_dumped);
    while (it != it.end() && it.record().file_name_view() != NATIVE_PATH("dirB"))
        ++it;
    time_t orig = it->mtime();
    ::sleep(1);
    std::ofstream(tmpPath / "dirB" / "fileBC") << "RandomString";
    // Even if the dir is modified, the mtime in fs_data_iter
    // would remain unchanged because it is cached to reduce calls to stat
    EXPECT_EQ(it->mtime(), orig);
}

TEST_F(dataIter, changeRoot) {
    fs_data_iter it(&dmp->_data_dumped);
    it.change_root((tmpPath / "dirB").c_str());
    EXPECT_EQ(_count_dataIt(it), 4);
    it = fs_data_iter(&dmp->_data_dumped, (tmpPath / "dirB").c_str());
    EXPECT_EQ(_count_dataIt(it), 4);
    it = fs_data_iter(&dmp->_data_dumped, (tmpPath / "nonExistent").c_str());
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(&dmp->_data_dumped, (tmpPath / "fileA").c_str());
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(nullptr);
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(nullptr, (tmpPath / "dirB").c_str());
    EXPECT_EQ(it, it.end());
}
