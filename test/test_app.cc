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
            _app.run_pooled(*builder.get(), f);
        else _app.run(*builder.get(), f);
        return res;
    }

protected:
    void SetUp() override {
        _app.add_root_path(info().tmpPath.native())
            .add_root_path((info().tmpPath / "dir11").native())
            // Empty path is root path
            .add_start_path(orie::str_t());
    }
    void TearDown() override {
        std::filesystem::remove(temp_directory_path() / "testData.db");
        std::filesystem::remove(temp_directory_path() / "testConf.txt");
        std::filesystem::remove(temp_directory_path() / "testConf.db");
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
    _app.read_db((temp_directory_path() / "testData.db").native())
        .update_db();
    ASSERT_TRUE(_app) << "Write to database failed.";
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));

    _app = orie::app(_pool); // Reset
    SetUp();
    _app.read_db((temp_directory_path() / "testData.db").native())
        .update_db();
    ASSERT_TRUE(_app) << "Update database failed.";
    ASSERT_EQ(4, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, confFile) {
    // Generate configuration
    _app.add_ignored_path((info().tmpPath / "dir10").native())
        .add_ignored_path((info().tmpPath / "dir11" / "dir10").native())
        .update_db((info().tmpPath / "testConf.db").native())
        .write_conf((info().tmpPath / "testConf.txt").native());
    ASSERT_TRUE(_app) << "Write Configuration Failed.";
    ASSERT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));

    // Consume generated configuration & database
    _app = orie::app(_pool);
    ASSERT_TRUE(_app.read_conf((info().tmpPath / "testConf.txt").native()))
        << "Read Configuration Failed.";
    ASSERT_TRUE(_app.read_db()) << "Read database failed";
    _app.add_start_path(orie::str_t());
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
    ASSERT_TRUE(_app.update_db());
    EXPECT_EQ(2, _do_tests(NATIVE_SV("-name dir9")));
}

TEST_F(orieApp, autoUpdate) {
    _app.start_auto_update(std::chrono::milliseconds(80));
    std::ofstream(info().tmpPath / "testConf.txt") << "aaa";
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    ASSERT_EQ(1, _do_tests(NATIVE_SV("-name testConf.txt")));

    // Corner case: stop immediately after start
    // No deadlock or segfault shall happen
    _app.stop_auto_update()
        .start_auto_update(std::chrono::hours(9999))
        .stop_auto_update();
}

TEST_F(orieApp, osDefault) {
    // Erase existing config
    std::cout << "Regenerating filesystem database.\n May take longer than "
              << "usual if a HDD is mounted, but not under /mnt or /run" << std::endl;
#ifdef _WIN32
    std::filesystem::path conf_dir(::getenv("USERPROFILE"));
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
    // Create config
    _app = orie::app::os_default(_pool);
    _app.update_db().add_start_path(orie::str_t());
    // Config file exists, but db file does not
    EXPECT_LE(1, _do_tests(db_exist_teststr));

    // Reset, then read the config (and database)
    _app = orie::app::os_default(_pool);
    _app.update_db().add_start_path(orie::str_t());
    // database file and config file
    EXPECT_LE(2, _do_tests(db_exist_teststr));
}

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
    EXPECT_NE(0, orie::app::main(3, args));
    // -conf file must be valid. 
    // Before app::write_conf, the file does not exist
    EXPECT_NE(0, orie::app::main(4, args));
    _app.write_conf(conf_path.native());
    // Database not initialized
    EXPECT_NE(0, orie::app::main(4, args));
    // Ok; just updatedb. No result
    EXPECT_EQ(0, orie::app::main(5, args));
    // Ok; updatedb and locate. Also no result because by default
    // start from working dir, which is not /tmp
    EXPECT_EQ(0, orie::app::main(6, args + 1));
    // Ok; start at dir10. Print 1 result
    EXPECT_EQ(0, orie::app::main(7, args));
}
