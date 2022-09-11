#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#define private public
#include <orient/fs/dumper.hpp>
#undef private

#include "afixedbunchofdirs.hpp"

static AFixedBunchOfDirs __dumper_dirs_global;

struct dumperFiles : public ::testing::Test {
    path tmpPath, dir_path,
        link_path, file_path;
    std::unique_ptr<dir_dumper> dmp;
    bool ok = true;

    dumperFiles() {
        tmpPath = current_path() / 
            ("dumper_tst_tmpdir_" + std::to_string(::time(nullptr)));
        //tmpPath = "./dumper_tst_tmpdir_" + std::to_string(::time(nullptr));
        ok &= create_directories(dir_path = (tmpPath / "dirPath"));
        create_symlink("/tmp", link_path = tmpPath / "linkPath");

        ok &= std::ofstream(file_path = tmpPath / "filePath").is_open();
        ok &= std::ofstream(dir_path / "a").is_open();
        ok &= std::ofstream(dir_path / "b").is_open();

        if (ok) {
            dmp = std::make_unique<dir_dumper>(tmpPath.native(), 0, nullptr);
            dmp->from_fs();
        }
    }

    orie::category_tag fileCateg(const orie::str_t& file_name) {
        auto diter = std::find_if(dmp->my_dirs.cbegin(), dmp->my_dirs.cend(),
            [&file_name](const dir_dumper* p) {return p && p->filename == file_name; });
        if (diter != dmp->my_dirs.cend())
            return orie::dir_tag;
        auto fiter = std::find_if(dmp->my_files.cbegin(), dmp->my_files.cend(),
            [&file_name](const file_dumper& p) {return p.filename == file_name; });
        if (fiter != dmp->my_files.cend())
            return fiter->category;
        return orie::unknown_tag;
    }

    ~dumperFiles() {
        remove_all(tmpPath);
    }
};

TEST_F(dumperFiles, linkBecomeDir) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    ::sleep(1);
    remove_all(dir_path);
    create_symlink("..", dir_path);
    remove(link_path);
    create_directory(link_path);
    dmp->from_fs();

    EXPECT_EQ(orie::link_tag, fileCateg(dir_path.filename().native()));
    EXPECT_EQ(orie::dir_tag, fileCateg(link_path.filename().native()));
}

TEST_F(dumperFiles, linkBecomeFile) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    ::sleep(1);
    remove(file_path);
    create_symlink("..", file_path);
    remove(link_path);
    std::ofstream().open(link_path);
    dmp->from_fs();
    EXPECT_EQ(orie::link_tag, fileCateg(file_path.filename().native()));
    EXPECT_EQ(orie::file_tag, fileCateg(link_path.filename().native()));
}

TEST_F(dumperFiles, dirBecomeFile) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    ::sleep(1);
    remove(file_path);
    create_directory(file_path);
    remove_all(dir_path);
    std::ofstream().open(dir_path);
    dmp->from_fs();
    EXPECT_EQ(orie::dir_tag, fileCateg(file_path.filename().native()));
    EXPECT_EQ(orie::file_tag, fileCateg(dir_path.filename().native()));
}

TEST_F(dumperFiles, fileDelete) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    EXPECT_EQ(orie::file_tag, fileCateg(file_path.filename().native()));
    ::sleep(1);
    remove(file_path);
    dmp->from_fs();
    EXPECT_EQ(orie::unknown_tag, fileCateg(file_path.filename().native()));
    ::sleep(1);
    std::ofstream().open(file_path);
    dmp->from_fs();
    EXPECT_EQ(orie::file_tag, fileCateg(file_path.filename().native()));
}

TEST_F(dumperFiles, dirDelete) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    EXPECT_EQ(orie::dir_tag, fileCateg(dir_path.filename().native()));
    ::sleep(1);
    remove_all(dir_path);
    dmp->from_fs();
    EXPECT_EQ(orie::unknown_tag, fileCateg(dir_path.filename().native()));
    ::sleep(1);
    create_directories(dir_path);
    dmp->from_fs();
    EXPECT_EQ(orie::dir_tag, fileCateg(dir_path.filename().native()));
}

TEST_F(dumperFiles, linkDelete) {
    if (!ok)
        FAIL() << "Unable to set temp directory";

    EXPECT_EQ(orie::link_tag, fileCateg(link_path.filename().native()));
    ::sleep(1);
    remove(link_path);
    dmp->from_fs();
    EXPECT_EQ(orie::unknown_tag, fileCateg(link_path.filename().native()));
    ::sleep(1);
    create_symlink("..", link_path);
    dmp->from_fs();
    EXPECT_EQ(orie::link_tag, fileCateg(link_path.filename().native()));
}

struct dumperSpeed : public ::testing::Test {
    dumperSpeed() {
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

TEST_F(dumperSpeed, readFs) {
    EXPECT_LE(speed_fromFs(), 2000) 
        << "Reading Filesystem Time Limit Exceeded";
    EXPECT_GE(dmp->n_bytes(), 100000)
        << "Too Little is Read";
    std::cerr << dmp->n_bytes() << '\n';
}

TEST_F(dumperSpeed, updateFs) {
    int64_t noDat = speed_fromFs();
    int64_t hasDat = speed_fromFs();
    EXPECT_GE(noDat, hasDat) 
        << "Fail to Utilize Existing Data";
}

TEST_F(dumperSpeed, database) {
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

TEST_F(dumperSpeed, prune) {
    dir_dumper* dir11 = dmp->visit_dir((info().tmpPath / "dir11").native());
    dir_dumper* dir10 = dmp->visit_dir((info().tmpPath / "dir10").native());
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
