#include <gtest/gtest.h>
#include <orient/app.hpp>
#include <orient/fs_pred_tree/fs_expr_builder.hpp>
#include "afixedbunchofdirs.hpp"

static AFixedBunchOfDirs __fstore_dirs_global;

struct orieApp : public testing::Test {
    orie::fifo_thpool _pool;
    orie::app _app;

    ABunchOfDirs& info() noexcept { return *__fstore_dirs_global.dirs; }
    orieApp() : _pool(8), _app(_pool) {
        __fstore_dirs_global.create(); 
    }

    size_t _do_tests(orie::sv_t expr_sv) {
        orie::pred_tree::fs_expr_builder builder;
        builder.build(expr_sv);
        std::atomic<size_t> res = 0;
        auto f = [&res] (fs_data_iter&) { ++res; };
        if (builder.has_async())
            _app.run_pooled(*builder.get(), f, std::chrono::hours(1));
        else _app.run(*builder.get(), f);
        return res;
    }

protected:
    void SetUp() override {
        _app.add_root_path(info().tmpPath.native())
            .add_root_path((info().tmpPath / "dir11").native())
            .add_start_path(NATIVE_PATH("/"));
    }
    void TearDown() override {
        std::filesystem::remove(temp_directory_path() / "testData.db");
    }
};

TEST_F(orieApp, replicateRoot) {
    _app.add_root_path((info().tmpPath).native())
        .add_root_path((info().tmpPath).native())
        .update_db();
    ASSERT_FALSE(_app); // No write to database file
    EXPECT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, rootPruned) {
    _app.add_ignored_path((info().tmpPath).native())
        .update_db();
    ASSERT_FALSE(_app); // No write to database file
    // Pruned paths override root paths
    // Only the 2 "dir9" inside "dir11" would match
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, prunedPath) {
    _app.add_ignored_path((info().tmpPath / "dir10").native())
        .add_ignored_path((info().tmpPath / "dir11" / "dir10").native())
        .update_db();
    ASSERT_FALSE(_app); // No write to database file
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, overlapRoot) {
    _app.add_root_path((info().tmpPath / "dir10").native())
        .add_root_path((info().tmpPath / "dir11" / "dir10").native())
        .update_db();
    ASSERT_FALSE(_app); // No write to database file
    EXPECT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, multithreaded) {
    std::ofstream a(info().tmpPath / "dir1" / "file0");
    std::ofstream b(info().tmpPath / "dir2" / "file0");
    std::ofstream c(info().tmpPath / "dir3" / "file0");
    for (size_t i = 0; i < 10; ++i) {
        a << std::string(500, 'z'); //~5Kib
        b << std::string(1000, 'z'); //~10Kib
        (c << "Hello\nWorld!\n").put('\0');
    }
    a << "\nHello\nWorld\n" << std::flush;
    b << "\nHello\nWorld\n" << std::flush; 
    c.flush();

    _app.update_db();
    ASSERT_FALSE(_app); // No write to database file
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-content-strstr 'Hello\nWorld'")));
    EXPECT_EQ(1, _do_tests(NATIVE_SV("-content-regex z{5001}")));
}

TEST_F(orieApp, readDb) {
    auto start_time = std::chrono::system_clock::now();
    _app.read_db((temp_directory_path() / "testData.db").native())
        .update_db();
    ASSERT_TRUE(_app) << "Write to database failed.";
    time_t nodb_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now() - start_time
    ).count();
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));

    _app = orie::app(_pool); // Reset
    SetUp();
    start_time = std::chrono::system_clock::now();
    _app.read_db((temp_directory_path() / "testData.db").native())
        .update_db();
    ASSERT_TRUE(_app) << "Update database failed.";
    time_t hasdb_cost = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now() - start_time
    ).count();
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
    EXPECT_GT(nodb_cost, hasdb_cost * 2) << "Database is not fast enough";
}
