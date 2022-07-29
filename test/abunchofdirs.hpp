#pragma once
#include <filesystem>
#include <fstream>
#include <random>
#include <iostream>

#include <orient/fs/dumper.hpp>
#include <orient/fs/data_iter.hpp>

using namespace std::filesystem;
using namespace orie::dmp;
using orie::fs_data_iter;

#ifdef _WIN32
#define sleep(sec) Sleep(sec * 1000)
#endif

struct ABunchOfDirs {
    path tmpPath;
    std::unique_ptr<dir_dumper> dmp = nullptr;
    std::unique_ptr<int8_t[]> dat = nullptr;
    time_t since;

    ABunchOfDirs(size_t depth = 5) {
        tmpPath = temp_directory_path() 
            / ("ABunchOfDirs_" + std::to_string(std::random_device()()));
        if (!directory_entry(tmpPath).exists()) {
            create_directories(tmpPath);
            _prep_dir(depth, tmpPath);
        }
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
        dat.reset();
        dmp.reset(new dir_dumper(tmpPath.native(), 0, nullptr));
        dmp->from_fs();
        if (dmp->n_bytes() > 20) {
            dat.reset(new int8_t[dmp->n_bytes() + 1]);
            dat[dmp->n_bytes()] = 0;
            dmp->to_raw(dat.get());
        }
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
