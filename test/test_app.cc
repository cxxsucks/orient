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
        _app.run(*builder.get(), f);
        return res;
    }

protected:
    void SetUp() override {
        // set_db_path must precedes all getters and setters
        _app.set_db_path(info().dbPath.c_str())
            .set_root_path(info().tmpPath.native())
            // Empty path is root path
            .add_start_path(orie::str_t());
    }
    void TearDown() override {
        std::filesystem::remove(_app.db_path());
        std::filesystem::remove(_app.db_path() + NATIVE_PATH("_inv"));
        std::filesystem::remove(temp_directory_path() / "testConf.txt");
    }
};

TEST_F(orieApp, replicateRoot) {
    _app.set_root_path((info().tmpPath).native())
        .set_root_path((info().tmpPath).native())
        .update_db();
    EXPECT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

/* Since v0.4.0 pruned root path is undefined
TEST_F(orieApp, rootPruned) {
    _app.add_ignored_path((info().tmpPath).native())
        .update_db();
    ASSERT_FALSE(_app); // No write to database file
    // Pruned paths override root paths
    // Only the 2 "dir9" inside "dir11" would match
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
} 
*/

TEST_F(orieApp, prunedPath) {
    _app.add_ignored_path((info().tmpPath / "dir10").native())
        .add_ignored_path((info().tmpPath / "dir11" / "dir10").native())
        .update_db();
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, slowPaths) {
    _app.add_slow_path((info().tmpPath / "dir10").native())
        .add_slow_path((info().tmpPath / "dir11" / "dir10").native())
        .update_db();
    EXPECT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, slowRoot) {
    _app.add_slow_path(info().tmpPath.native())
        .update_db();
    EXPECT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, manyStartPath) {
    _app.add_start_path(info().tmpPath.native())
        .add_start_path(canonical(temp_directory_path()).native())
        .update_db();
    // Root, /tmp and /tmp/ABunchOfDirs, 4 results each
    EXPECT_EQ(12, _do_tests(NATIVE_SV("-name dir9")));
    _app.erase_start_path(orie::str_t())
        .add_start_path(NATIVE_PATH("foobar"))
        .add_start_path(NATIVE_PATH("/foo/bar"));
    // Removed "/" and added 2 nonexistent paths
    EXPECT_EQ(8, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, concurPreds) {
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
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-content-strstr 'Hello\nWorld'")));
    EXPECT_EQ(1, _do_tests(NATIVE_SV("-content-regex z{5001}")));
}

TEST_F(orieApp, readDb) {
    _app.update_db();
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));

    _app = orie::app(_pool); // Reset
    SetUp();
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
    _app.update_db();
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, confFile) {
    // Generate configuration
    _app.add_ignored_path((info().tmpPath / "dir10").native())
        .add_ignored_path((info().tmpPath / "dir11" / "dir10").native())
        .update_db()
        .write_conf((info().tmpPath / "testConf.txt").native());
    ASSERT_TRUE(_app) << "Write Configuration Failed.";
    ASSERT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));

    // Consume generated configuration & database
    _app = orie::app(_pool);
    _app.read_conf((info().tmpPath / "testConf.txt").native())
        .add_start_path(orie::str_t());
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
    _app.update_db();
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, autoUpdate) {
    _app.start_auto_update(std::chrono::milliseconds(80), true);
    std::ofstream(info().tmpPath / "testConf.txt") << "aaa";
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_EQ(1, _do_tests(NATIVE_SV("-name testConf.txt")));

    // Corner case: stop immediately after start
    // No deadlock or segfault shall happen
    _app.stop_auto_update()
        .start_auto_update(std::chrono::hours(9999), true)
        .stop_auto_update();
}

TEST_F(orieApp, osDefault) {
    // Erase existing config
    std::cout << "Regenerating filesystem database.\n May take longer than "
              << "usual if a HDD is mounted, but not under /mnt or /run" << std::endl;
#ifdef _WIN32
    std::filesystem::path conf_dir(::getenv("APPDATA"));
    conf_dir /= ".orie";
    orie::sv_t db_exist_teststr(L"-updir ( -name .orie ) -a -name default.*");
#else
    std::filesystem::path conf_dir(::getenv("HOME"));
    conf_dir = (conf_dir / ".config" / "orie");
    orie::sv_t db_exist_teststr("-updir ( -name orie ) -a -name default.* "
                                "-a -perm 0600");
#endif

    std::filesystem::remove(conf_dir / "default.txt");
    std::filesystem::remove(conf_dir / "default.db");
    std::filesystem::remove(conf_dir / "default.db_inv");
    // Create config
    _app = orie::app::os_default(_pool);
    _app.update_db().add_start_path(orie::str_t());
    // database file and config file
    EXPECT_LE(2, _do_tests(db_exist_teststr));
}

// Cannot test on non-msvc Windows because of inconsistent signiture
#if defined(_MSC_VER) || !defined(_WIN32)

#ifdef _MSC_VER
#define wmain fake_main
#include "../src/main.cc"
#undef wmain
#else
#define main fake_main
#include "../src/main.cc"
#undef main
#endif

TEST_F(orieApp, main) {
    auto conf_path = info().tmpPath / "mainConf.txt",
         start_path = info().tmpPath / "dir10";
    const orie::char_t* args[] = {
        NATIVE_PATH("a.out"), 
        start_path.c_str(),
        NATIVE_PATH("-conf"), conf_path.c_str(),
        NATIVE_PATH("-updatedb"),
        NATIVE_PATH("-name"), NATIVE_PATH("dir9")
    };
    // -conf accepts 1 argument
    EXPECT_NE(0, fake_main(3, args));
    // -conf file must be valid. 
    // Before app::write_conf, the file does not exist
    EXPECT_NE(0, fake_main(4, args));
    _app.write_conf(conf_path.native());
    // Database not initialized
    EXPECT_NE(0, fake_main(4, args));
    // Ok; just updatedb. No result
    EXPECT_EQ(0, fake_main(5, args));
    // Ok; updatedb and locate. Also no result because by default
    // start from working dir, which is not /tmp
    EXPECT_EQ(0, fake_main(6, args + 1));
    // Ok; start at dir10. Print 1 result
    EXPECT_EQ(0, fake_main(7, args));
    std::flush(orie::NATIVE_STDOUT);
}
#endif
