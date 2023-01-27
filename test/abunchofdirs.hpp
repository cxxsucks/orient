#pragma once
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>

#include <orient/fs/dumper.hpp>
#include <orient/fs/data_iter.hpp>
#include <orient/util/fifo_thpool.hpp>

using namespace std::filesystem;
using namespace orie::dmp;
using orie::fs_data_iter;

#ifdef _WIN32
#define sleep(sec) Sleep(sec * 1000)
#endif

inline orie::fifo_thpool __dummy_pool(0);
struct ABunchOfDirs {
    path tmpPath, dbPath;
    std::unique_ptr<dumper> dmp = nullptr;
    time_t since;

    ABunchOfDirs(size_t depth = 5) {
        tmpPath = temp_directory_path() 
            / ("ABunchOfDirs_" + std::to_string(std::random_device()()));
        dbPath = temp_directory_path() 
            / ("ABunchOfDirDb" + std::to_string(std::random_device()()));
        if (!directory_entry(tmpPath).exists()) {
            create_directories(tmpPath);
            _prep_dir(depth, tmpPath);
        }
        tmpPath = std::filesystem::canonical(tmpPath);
        refreshDat();
    }

    void writeSomeContent() {
        std::ofstream a(tmpPath / "dir0" / "txtStrstr");
        std::ofstream b(tmpPath / "dir1" / "zzzzz");
        std::ofstream c(tmpPath / "dir0" / "binStrstr");

        for (size_t i = 0; i < 10; ++i) {
            a << "Hello\nWorld!\n";
            b << "Hello\nWorld!\n";
            (c << "Hello\nWorld!\n").put('\0');
        }
        refreshDat(); 
    }

    void refreshDat() {
        dmp.reset(new dumper(dbPath.c_str(), __dummy_pool));
        dmp->_root_path = tmpPath.native();
        dmp->_noconcur_paths.push_back(tmpPath.native());
        dmp->rebuild_database();
    }

    ~ABunchOfDirs() { 
        if (!tmpPath.empty())
            remove_all(tmpPath);
    }

    ABunchOfDirs(ABunchOfDirs&& rhs) = delete;
        // : tmpPath(rhs.tmpPath), dmp(std::move(rhs.dmp))
        // , dat(std::move(rhs.dat)) { rhs.tmpPath.clear(); }
    
private:
    static void _prep_dir(size_t subs, const path& curTmp) {
        for (size_t i = 0; i < subs; ++i) {
            std::ofstream(curTmp / ("file" + std::to_string(i)));
            create_directory(curTmp / ("dir" + std::to_string(i)));
            _prep_dir(i, curTmp / ("dir" + std::to_string(i)));
        }
        if (subs == 3)
            create_symlink("./file1", curTmp / "randomSymlink");
    }
};
