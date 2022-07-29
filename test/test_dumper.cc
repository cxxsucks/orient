#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <orient/fs/dumper.hpp>

#include "afixedbunchofdirs.hpp"
static AFixedBunchOfDirs __dumper_dirs_global;

struct dumper_files : public ::testing::Test {
    path tmpPath, dirPath,
        linkPath, filePath;
    dir_dumper* dmp = nullptr;
    bool ok = true;

    dumper_files() {
        tmpPath = current_path() / 
            ("dumper_tst_tmpdir_" + std::to_string(::time(nullptr)));
        //tmpPath = "./dumper_tst_tmpdir_" + std::to_string(::time(nullptr));
        ok &= create_directories(dirPath = (tmpPath / "dirPath"));
        create_symlink("/tmp", linkPath = tmpPath / "linkPath");

        ok &= std::ofstream(filePath = tmpPath / "filePath").is_open();
        ok &= std::ofstream(dirPath / "a").is_open();
        ok &= std::ofstream(dirPath / "b").is_open();

        if (ok) {
            dmp = new dir_dumper(tmpPath.native(), 0, nullptr);
            dmp->from_fs();
        }
    }

    ~dumper_files() {
        remove_all(tmpPath);
        delete dmp;
    }
};

TEST_F(dumper_files, link_become_dir) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    ::sleep(1);
    remove_all(dirPath);
    create_symlink("..", dirPath);
    remove(linkPath);
    create_directory(linkPath);
    dmp->from_fs();
    EXPECT_NE(nullptr, dynamic_cast<link_dumper*>(dmp->visit_full(dirPath, false)));
    EXPECT_NE(nullptr, dynamic_cast<dir_dumper*>(dmp->visit_full(linkPath, false)));
}

TEST_F(dumper_files, link_become_file) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    ::sleep(1);
    remove(filePath);
    create_symlink("..", filePath);
    remove(linkPath);
    std::ofstream().open(linkPath);
    dmp->from_fs();
    EXPECT_NE(nullptr, dynamic_cast<link_dumper*>(dmp->visit_full(filePath, false)));
    EXPECT_EQ(nullptr, dynamic_cast<link_dumper*>(dmp->visit_full(linkPath, false)));
    EXPECT_NE(nullptr, dmp->visit_full(linkPath, false));
}

TEST_F(dumper_files, dir_become_file) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    ::sleep(1);
    remove(filePath);
    create_directory(filePath);
    remove_all(dirPath);
    std::ofstream().open(dirPath);
    dmp->from_fs();
    EXPECT_NE(nullptr, dynamic_cast<dir_dumper*>(dmp->visit_full(filePath, false)));
    EXPECT_EQ(nullptr, dynamic_cast<dir_dumper*>(dmp->visit_full(dirPath, false)));
    EXPECT_NE(nullptr, dmp->visit_full(dirPath, false));
}

TEST_F(dumper_files, file_delete) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    EXPECT_NE(nullptr, dmp->visit_full(filePath, false));
    ::sleep(1);
    remove(filePath);
    dmp->from_fs();
    EXPECT_EQ(nullptr, dmp->visit_full(filePath, false));
    ::sleep(1);
    std::ofstream().open(filePath);
    dmp->from_fs();
    EXPECT_NE(nullptr, dmp->visit_full(filePath, false));
}

TEST_F(dumper_files, dir_delete) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    EXPECT_NE(nullptr, dmp->visit_full(dirPath / "a", false));
    ::sleep(1);
    remove_all(dirPath);
    dmp->from_fs();
    EXPECT_EQ(nullptr, dmp->visit_full(dirPath / "a", false));
    ::sleep(1);
    create_directories(dirPath);
    dmp->from_fs();
    EXPECT_NE(nullptr, dynamic_cast<dir_dumper*>(dmp->visit_full(dirPath, false)));
}

TEST_F(dumper_files, link_delete) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    EXPECT_NE(nullptr, dynamic_cast<link_dumper*>(dmp->visit_full(linkPath, false)));
    ::sleep(1);
    remove(linkPath);
    dmp->from_fs();
    EXPECT_EQ(nullptr, dmp->visit_full(linkPath, false));
    ::sleep(1);
    create_symlink("..", linkPath);
    dmp->from_fs();
    EXPECT_NE(nullptr, dynamic_cast<link_dumper*>(dmp->visit_full(linkPath, false)));
}

struct dumper_speed : public ::testing::Test {
    dumper_speed() {
        __dumper_dirs_global.create(13);
        dmp.reset(new dir_dumper(info().tmpPath.native(), 0, nullptr));
    }
protected:
    int64_t speed_fromFs() {
        auto startTime = std::chrono::system_clock::now();
        dmp->from_fs();
        execCost = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - startTime
        ).count();
        return execCost;
    }
    ABunchOfDirs& info() noexcept { return *__dumper_dirs_global.dirs; }

    std::unique_ptr<dir_dumper> dmp;
    int64_t execCost = 0;
};

TEST_F(dumper_speed, read_fs) {
    EXPECT_LE(speed_fromFs(), 2000) 
        << "Reading Filesystem Time Limit Exceeded";
    EXPECT_GE(dmp->n_bytes(), 100000)
        << "Too Little is Read";
    std::cerr << dmp->n_bytes() << '\n';
}

TEST_F(dumper_speed, update_fs) {
    int64_t noDat = speed_fromFs();
    int64_t hasDat = speed_fromFs();
    EXPECT_GE(noDat, hasDat) 
        << "Fail to Utilize Existing Data";
}

TEST_F(dumper_speed, database) {
    speed_fromFs();
    size_t fromFsSz = dmp->n_bytes();
    std::unique_ptr<int8_t[]> buf(new int8_t[fromFsSz]);
    ::memset(buf.get(), 0xee, 1);
    dmp->to_raw(buf.get());
    ASSERT_NE(0xee, buf[fromFsSz - 1])
        << "n_bytes() returned smaller than actual size";
    dmp->clear();
    dmp->from_raw(buf.get());
    EXPECT_EQ(dmp->n_bytes(), fromFsSz)
        << "Content Changes After Saving and Loading";
}

TEST_F(dumper_speed, prune) {
    file_dumper* dir11 = dmp->visit_full((info().tmpPath / "dir11").native(), true);
    file_dumper* dir10 = dmp->visit_full((info().tmpPath / "dir10").native(), true);
    dir10->set_ignored(true);
    dmp->from_fs();
    EXPECT_LE(dir10->n_bytes(), 1000);
    EXPECT_GE(dir11->n_bytes(), 1000);

    dir11->set_ignored(true); dir11->clear();
    dir10->set_ignored(false);
    dmp->from_fs();
    EXPECT_LE(dir11->n_bytes(), 1000);
    EXPECT_GE(dir10->n_bytes(), 1000);
}
