#include <gtest/gtest.h>
#include <filesystem>
#include <random>
#include <orient/util/arr2d.hpp>

struct arr2d : public testing::Test {
    std::filesystem::path tmpPath;
    arr2d_writer writer;

    arr2d() 
        : tmpPath(std::filesystem::temp_directory_path() /
                  ("arr2dTest" + std::to_string(std::random_device()())))
        , writer(tmpPath.native()) { }

    void SetUp() override {
        writer._data_pending.resize(15);
        for (size_t i = 0; i < 15; ++i)
            for (size_t j = 0; j < i * 100 + 100; ++j)
                writer._data_pending[i].push_back(j * 7);
        writer.append_pending_to_file();

        writer._data_pending.resize(4);
        for (size_t i = 0; i < 1000; ++i) {
            writer._data_pending[0].push_back(100000 + 13 * i);
            writer._data_pending[1].push_back(100000 + 17 * i);
            // Empty 2nd line
            writer._data_pending[3].push_back(100000 + 19 * i);
        }
        writer.append_pending_to_file();
    }

    ~arr2d() { std::filesystem::remove(tmpPath); }
};

// Simple writer test without using reader
TEST_F(arr2d, writeSimple) {
    FILE* st = fopen(tmpPath.c_str(), "rb");
    ASSERT_NE(st, nullptr);

    std::unique_ptr<uint32_t[]> dat_buf(new uint32_t[1000000]);
    memset(dat_buf.get(), 0xef, 1000000 * sizeof(uint32_t));
    ASSERT_LT(0, fread(dat_buf.get(), sizeof(uint32_t), 1000000, st));
    ASSERT_EQ(15, dat_buf[0]) << "Bad Page 0 Row Count";
    ASSERT_EQ(17, dat_buf[1]) << "Bad Page 0 Row 0 Offset";
    for (size_t i = 0; i < 15; ++i)
        ASSERT_EQ(i * 100 + 100, dat_buf[dat_buf[i + 1]])
            << "Bad Page 0 Row " << i << " Array Size";

    uint32_t pg1_off = dat_buf[16];
    ASSERT_EQ(4, dat_buf[pg1_off]) << "Bad Page 1 Row Count";
    ASSERT_EQ(1000, dat_buf[pg1_off + dat_buf[pg1_off + 1]]);
    ASSERT_EQ(1000, dat_buf[pg1_off + dat_buf[pg1_off + 2]]);
    ASSERT_EQ(0, dat_buf[pg1_off + dat_buf[pg1_off + 3]]);
    ASSERT_EQ(1000, dat_buf[pg1_off + dat_buf[pg1_off + 4]]);
    ASSERT_EQ(0xefefefef, dat_buf[pg1_off + dat_buf[pg1_off + 5]])
        << "Start of Page 2 must be end of data";
    ASSERT_NE(0xefefefef, dat_buf[pg1_off + dat_buf[pg1_off + 5] - 1])
        << "1 Position Before Start of Page 2 must NOT be end of data";
}

TEST_F(arr2d, appendSimple) {
    writer.add_int(4095, 1234);
    writer.add_int(0, 1234);
    EXPECT_EQ(writer._data_pending.size(), 4096);
    EXPECT_EQ(writer._data_pending[0], std::vector<uint32_t>{1234});
    EXPECT_EQ(writer._data_pending[4095], std::vector<uint32_t>{1234});
    for (size_t i = 1; i < 4095; ++i)
        EXPECT_TRUE(writer._data_pending[i].empty());
    EXPECT_ANY_THROW(writer._data_pending.at(4096));
}

TEST_F(arr2d, reader) {
    arr2d_reader reader(tmpPath.c_str());
    for (size_t i = 0; i < 15; ++i)
        ASSERT_EQ(reader.uncmprs_size(i, 0), i * 100 + 100);
    ASSERT_EQ(reader.uncmprs_size(0, 1), 1000);
    ASSERT_EQ(reader.uncmprs_size(1, 1), 1000);
    ASSERT_EQ(reader.uncmprs_size(2, 1), 0);
    ASSERT_EQ(reader.uncmprs_size(3, 1), 1000);

    ASSERT_EQ(nullptr, reader.line_data(15, 0).first);
    ASSERT_EQ(nullptr, reader.line_data(4, 1).first);
    ASSERT_EQ(nullptr, reader.line_data(0, 2).first);
    ASSERT_EQ(~uint32_t(), reader.uncmprs_size(15, 0));
    ASSERT_EQ(~uint32_t(), reader.uncmprs_size(4, 1));
    ASSERT_EQ(~uint32_t(), reader.uncmprs_size(0, 2));
}

TEST_F(arr2d, moveFile) {
    arr2d_reader reader(tmpPath.c_str());
    EXPECT_THROW(reader.move_file(NATIVE_PATH("/foobar/har")), std::system_error);
    for (size_t i = 0; i < 15; ++i)
        ASSERT_EQ(reader.uncmprs_size(i, 0), i * 100 + 100);
    reader.move_file(tmpPath.parent_path() / "arr2dMovedPath");
    for (size_t i = 0; i < 15; ++i)
        ASSERT_EQ(reader.uncmprs_size(i, 0), i * 100 + 100);
    reader.move_file(tmpPath);
    for (size_t i = 0; i < 15; ++i)
        ASSERT_EQ(reader.uncmprs_size(i, 0), i * 100 + 100);
}

TEST_F(arr2d, uncmprsOneLine) {
    EXPECT_TRUE(arr2d_intersect::decompress_entire_line(0, nullptr).empty());
    arr2d_reader reader(tmpPath.c_str());

    auto res = arr2d_intersect::decompress_entire_line(0, &reader);
    EXPECT_EQ(res[0], 0); EXPECT_EQ(res[1], 7);
    EXPECT_EQ(res.back(), 100000 + 13 * 999);
    EXPECT_EQ(res.size(), 1100);

    res = arr2d_intersect::decompress_entire_line(2, &reader);
    EXPECT_EQ(res.back(), 7 * 299);
    EXPECT_EQ(res.size(), 300);
}

TEST_F(arr2d, int) {
    arr2d_reader reader(tmpPath.c_str());
    arr2d_intersect query(&reader);
    query._lines_to_query.assign({0, 5, 10});

    size_t nint = 0;
    EXPECT_EQ(0, query.next_intersect());
    while (~uint32_t() != query.next_intersect())
        ++nint;
    EXPECT_EQ(nint, 99);

    query._lines_to_query.assign({3, 1, 0});
    query.rewind();
    while (query.next_intersect() < 100000)
        ; // Skip page 1
    EXPECT_EQ(query.next_intersect(), 100000 + 13 * 17 * 19);
    EXPECT_EQ(query.next_intersect(), 100000 + 13 * 17 * 19 * 2);
}

TEST_F(arr2d, intCorner) {
    arr2d_reader reader(tmpPath.c_str());
    arr2d_intersect query;

    // Intersect of 0 lines AND no reader
    EXPECT_EQ(~uint32_t(), query.next_intersect());
    // No reader
    query.rewind();
    query._lines_to_query.assign({0, 5, 10});
    EXPECT_EQ(~uint32_t(), query.next_intersect());
    // Intersection of 0 lines
    query._lines_to_query.clear();
    query.set_reader(&reader);
    EXPECT_EQ(~uint32_t(), query.next_intersect());

    // No line 15
    query._lines_to_query.assign({0, 5, 10, 15});
    query.rewind();
    EXPECT_EQ(~uint32_t(), query.next_intersect());

    // Many line 0s
    query._lines_to_query.assign({0, 0, 10, 0});
    query.rewind();
    EXPECT_EQ(0, query.next_intersect());

    // Line 2 page 2 is empty
    query._lines_to_query.assign({0, 2});
    query.rewind();
    while (query.next_intersect() < 100000)
        ; // Skip page 1
    EXPECT_EQ(~uint32_t(), query.next_intersect());
}
