#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <random>
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
    size_t ranNum = std::random_device()();

    dataIter() {
        tmpPath = std::filesystem::temp_directory_path() 
            / ("dataIter_tst_tmpdir"  + std::to_string(ranNum));
        dbPath = temp_directory_path() 
            / ("dataIter_tst_tmpdir_db" + std::to_string(ranNum));
        create_directories(tmpPath / "dirA");
        create_directories(tmpPath / "dirB/dirBA");
        create_directories(tmpPath / "dirEmpty");
        create_symlink("..", tmpPath / "dirB" / "dirBA" / "linkBAB");

        std::ofstream(tmpPath / "fileA");
        std::ofstream(tmpPath / "fileB");
        std::ofstream(tmpPath / "dirA" / "fileAA");
        std::ofstream(tmpPath / "dirA" / "fileAB");
        std::ofstream(tmpPath / "dirB" / "fileBA");
        std::ofstream(tmpPath / "dirB" / "dirBA" / "fileBAA");

        dmp = new dumper(dbPath.c_str(), __dummy_pool);
        dmp->_root_path = tmpPath.native();
        dmp->_noconcur_paths.push_back(tmpPath.native());
        dmp->rebuild_database();
    }

    ~dataIter() {
        delete dmp;
        remove_all(tmpPath);
        remove_all(dbPath);
        remove_all(dbPath += "_inv");
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
    fs_data_iter it(dmp);
    EXPECT_EQ(_count_dataIt(it), 11);
    EXPECT_THROW(++it, std::out_of_range);
    EXPECT_THROW(it++, std::out_of_range);
}

TEST_F(dataIter, updir) {
    for (fs_data_iter it(dmp); it != it.end(); ++it) {
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
        }        
    }
}

TEST_F(dataIter, downdir) {
    fs_data_iter it(dmp), tmp;
    while (it != it.end() && it.record().file_name_view() != NATIVE_PATH("dirB"))
        ++it;
    tmp = it.current_dir_iter();
    EXPECT_EQ(_count_dataIt(tmp), 2);
}

TEST_F(dataIter, stat) {
    fs_data_iter it(dmp);
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
    fs_data_iter it(dmp);
    it.change_root((tmpPath / "dirB").c_str());
    EXPECT_EQ(_count_dataIt(it), 4);
    it = fs_data_iter(dmp, (tmpPath / "dirB").c_str());
    EXPECT_EQ(_count_dataIt(it), 4);
    it = fs_data_iter(dmp, (tmpPath / "nonExistent").c_str());
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(dmp, (tmpPath / "fileA").c_str());
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(nullptr);
    EXPECT_EQ(it, it.end());
    it = fs_data_iter(nullptr, (tmpPath / "dirB").c_str());
    EXPECT_EQ(it, it.end());
}
