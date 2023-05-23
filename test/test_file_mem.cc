#include <gtest/gtest.h>
#include <orient/util/file_mem_chunk.hpp>
#include <orient/util/fifo_thpool.hpp>
#include <filesystem>
#include <random>

struct fileMemChunk : public testing::Test {
    std::filesystem::path tmpPath;
    orie::dmp::file_mem_chunk chunk;

    // ~fileMemChunk() noexcept { std::filesystem::remove(tmpPath); }
    fileMemChunk() 
        : tmpPath(std::filesystem::temp_directory_path() /
                  ("fileMemTest" + std::to_string(std::random_device()())))
        , chunk(tmpPath.c_str(), 3, true, true) { }

    void SetUp() override {
        chunk._unplaced_dat.assign(1111111, std::byte('a'));
        chunk._unplaced_dat.push_back(std::byte('z'));
        chunk.add_last_chunk();
        chunk._unplaced_dat.assign(1111112, std::byte('b'));
        chunk._unplaced_dat.push_back(std::byte('z'));
        chunk.add_last_chunk();
        chunk._unplaced_dat.assign(1111113, std::byte('c'));
        chunk._unplaced_dat.push_back(std::byte('z'));
        chunk.add_last_chunk();
        chunk._unplaced_dat.assign(1111114, std::byte('d'));
        chunk._unplaced_dat.push_back(std::byte('z'));
        chunk.add_last_chunk();
    }

    const std::byte* visitA() {
        EXPECT_THROW(chunk.start_visit(0, 1111112), std::out_of_range);
        const std::byte* res = chunk.start_visit(0, 1111110);
        EXPECT_EQ(res[0], std::byte('a'));
        EXPECT_EQ(res[1], std::byte('z'));
        chunk.finish_visit();
        return res;
    }

    const std::byte* visitB() {
        EXPECT_THROW(chunk.start_visit(1, 1111113), std::out_of_range);
        const std::byte* res = chunk.start_visit(1, 1111111);
        EXPECT_EQ(res[0], std::byte('b'));
        EXPECT_EQ(res[1], std::byte('z'));
        chunk.finish_visit();
        return res;
    }

    const std::byte* visitC() {
        EXPECT_THROW(chunk.start_visit(2, 1111114), std::out_of_range);
        const std::byte* res = chunk.start_visit(2, 1111112);
        EXPECT_EQ(res[0], std::byte('c'));
        EXPECT_EQ(res[1], std::byte('z'));
        chunk.finish_visit();
        return res;
    }

    const std::byte* visitD() {
        EXPECT_THROW(chunk.start_visit(3, 1111115), std::out_of_range);
        const std::byte* res = chunk.start_visit(3, 1111113);
        EXPECT_EQ(res[0], std::byte('d'));
        EXPECT_EQ(res[1], std::byte('z'));
        chunk.finish_visit();
        return res;
    }
};

TEST_F(fileMemChunk, visitCache) {
    EXPECT_TRUE(chunk._unplaced_dat.empty());
    // Second visit shall be cached
    EXPECT_EQ(visitA(), visitA());
    EXPECT_EQ(visitB(), visitB());
    EXPECT_EQ(visitC(), visitC());
    EXPECT_EQ(visitD(), visitD());
}

TEST_F(fileMemChunk, pushCache) {
    EXPECT_TRUE(chunk._unplaced_dat.empty());
    // D, B, C are in the 3 caches
    const std::byte* bRes = visitB(),
    *cRes = visitC(), *dRes = visitD();
    EXPECT_EQ(bRes, visitB());
    EXPECT_EQ(cRes, visitC());
    EXPECT_EQ(dRes, visitD());

    // Read file and overwrite cache
    EXPECT_EQ(visitA(), visitA());
    EXPECT_NE(bRes, visitB());
    EXPECT_NE(cRes, visitC());
    EXPECT_NE(dRes, visitD());
}

TEST_F(fileMemChunk, pushEmpty) {
    ASSERT_EQ(4, chunk.chunk_count());
    chunk._unplaced_dat.assign(123456, std::byte('e'));
    chunk.add_last_chunk();
    ASSERT_EQ(5, chunk.chunk_count());
    chunk.add_last_chunk(); // empty
    ASSERT_EQ(5, chunk.chunk_count());
}

TEST_F(fileMemChunk, moveFile) {
    chunk.move_file((tmpPath.native() + NATIVE_PATH(".old")).c_str());

    // D, B, C are in the 3 caches
    const std::byte* bRes = visitB(),
    *cRes = visitC(), *dRes = visitD();
    EXPECT_EQ(bRes, visitB());
    EXPECT_EQ(cRes, visitC());
    EXPECT_EQ(dRes, visitD());

    // Read from new file and rewrite cache
    EXPECT_EQ(visitA(), visitA());
    EXPECT_NE(bRes, visitB());
    EXPECT_NE(cRes, visitC());
    EXPECT_NE(dRes, visitD());
}

TEST_F(fileMemChunk, read) {
    orie::dmp::file_mem_chunk chunk2(tmpPath.c_str(), 3, false, false);

    ASSERT_EQ(4, chunk2.chunk_count());
    EXPECT_EQ(chunk2.chunk_size(0), chunk.chunk_size(0));
    EXPECT_EQ(chunk2.chunk_size(1), chunk.chunk_size(1));
    EXPECT_EQ(chunk2.chunk_size(2), chunk.chunk_size(2));
    EXPECT_EQ(chunk2.chunk_size(3), chunk.chunk_size(3));

    EXPECT_EQ(visitA(), visitA());
    EXPECT_EQ(visitB(), visitB());
    EXPECT_EQ(visitC(), visitC());
    EXPECT_EQ(visitD(), visitD());
}

TEST_F(fileMemChunk, clear) {
    chunk.clear();
    EXPECT_EQ(0, chunk.chunk_count());
    EXPECT_THROW(chunk.chunk_size(0), std::out_of_range);
    EXPECT_THROW(chunk.start_visit(0, 0), std::out_of_range);

    SetUp();
    EXPECT_EQ(4, chunk.chunk_count());
    EXPECT_EQ(visitA(), visitA());
    EXPECT_EQ(visitB(), visitB());
    EXPECT_EQ(visitC(), visitC());
    EXPECT_EQ(visitD(), visitD());
}

// To MSVC Debugger users: Huge amounts of exceptions ahead!
// MSVC Debugger prints a line for each exception, caught or not,
// cluttering the debug console immediately.
TEST_F(fileMemChunk, concurVisit) {
    orie::fifo_thpool largePool(25);
    for (size_t i = 0; i < 150; ++i)
        largePool.enqueue([this] () {
            visitA(); visitB(); visitC(); visitD();
        });
}

TEST_F(fileMemChunk, concurPush) {
    orie::fifo_thpool largePool(25);
    chunk._unplaced_dat.assign(123456, std::byte('e'));
    for (size_t i = 0; i < 200; ++i)
        largePool.enqueue([this] () {
            visitA(); visitB(); visitC(); visitD();
        });
    largePool.enqueue([this] () {chunk.add_last_chunk();});
}

TEST_F(fileMemChunk, concurMoveFile) {
    orie::fifo_thpool largePool(25);
    for (size_t i = 0; i < 200; ++i)
        largePool.enqueue([this] () {
            visitA(); visitB(); visitC(); visitD();
        });
    largePool.enqueue([this] () {
        chunk.move_file((tmpPath.native() + NATIVE_PATH(".old")).c_str());
        chunk.move_file((tmpPath.native() + NATIVE_PATH(".new")).c_str());
        chunk.move_file(tmpPath.native().c_str());
    });
}
