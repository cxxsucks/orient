#pragma once
#include "abunchofdirs.hpp"
#include <mutex>

struct AFixedBunchOfDirs {
    std::unique_ptr<ABunchOfDirs> dirs = nullptr;
    std::mutex mut;

    void create(size_t bunchSize = 12) {
        std::lock_guard<std::mutex> __g(mut);
        if (dirs == nullptr)
            dirs.reset(new ABunchOfDirs(bunchSize));
    }
};