#include <orient/app.hpp>
#include <orient/fs/dumper.hpp>
#include <orient/fs_pred_tree/fs_expr_builder.hpp>
#include <fstream>
#include <algorithm>
#include <cassert>

namespace orie {

app& app::read_db(str_t path) {
    std::unique_lock __lck(_member_mut);
    if (!path.empty())
        _db_path = std::move(path);
// Only MSVC provides std::fstream ctor accepting wchar
#if !defined(_WIN32) || defined(_MSC_VER)
    std::ifstream ifs(_db_path, std::ios_base::binary);
#else
    std::ifstream ifs(orie::xxstrcpy(sv_t(_db_path)), std::ios_base::binary);
#endif // !_WIN32 || _MSC_VER

    // Check whether the file is a legit database for orient
    uint64_t magic_read = 0;
    ifs.read(reinterpret_cast<char*>(&magic_read), sizeof(uint64_t));
    if (magic_read != magic_num) {
        _conf_path_valid = false;
        return *this; // Stop reading if magic number mismatch
    }

    assert(ifs.tellg() == sizeof(uint64_t));
    size_t db_sz = ifs.seekg(0, std::ios_base::end).tellg();
    db_sz -= sizeof(uint64_t);
    _data_dumped.reset(new std::byte[db_sz + 1]);
    ifs.seekg(sizeof(uint64_t));
    ifs.read(reinterpret_cast<char*>(_data_dumped.get()), db_sz);
    _data_dumped[db_sz] = std::byte(0);
    _conf_path_valid = true;
    return *this;
}

app& app::update_db(str_t path) {
    dumper dump_worker;
{   if (!path.empty())
        _db_path = std::move(path);
    // Read and reuse the old dumped data
    std::shared_lock __lck(_member_mut);

    for (const str_t& p : _ignored_paths) {
        auto* to_dump = dump_worker.visit_dir(p);
        if (!to_dump) {
            NATIVE_STDERR << NATIVE_PATH("Invalid pruned path:")
                          << p << NATIVE_PATH('\n');
            continue;
        }
        to_dump->clear();
        to_dump->set_ignored(true);
    }

    // All root paths must be pruned along with ignored ones,
    // Sort root paths from the deepest to the shallowest, preventing
    // rescanning if there are overlapping root paths.
    std::sort(_root_paths.begin(), _root_paths.end(), 
        [] (const auto& a, const auto& b) {
            return a.first.size() > b.first.size();
    });
    for (const auto& p : _root_paths) {
        auto* to_dump = dump_worker.visit_dir(p.first);
        if (!to_dump) {
            NATIVE_STDERR << NATIVE_PATH("Invalid root path:")
                          << p.first << NATIVE_PATH('\n');
            continue;
        }
        to_dump->from_fs(_pool, p.second);
        to_dump->compact();
        to_dump->set_ignored(true);
    }
}

    // Write updated data back to `data_dumped`
    std::unique_lock __lck(_member_mut);
    size_t sz = dump_worker.n_bytes();
    _data_dumped.reset(new std::byte[sz + 1]);
    dump_worker.to_raw(_data_dumped.get());
    _data_dumped[sz] = std::byte(0);

    // And then into database file
#if !defined(_WIN32) || defined(_MSC_VER)
    std::ofstream ofs(_db_path, std::ios_base::binary);
#else
    std::ofstream ofs(orie::xxstrcpy(sv_t(_db_path)), std::ios_base::binary);
#endif
    if (!ofs.is_open()) {
        _conf_path_valid = false;
        return *this;
    }
    ofs.write(reinterpret_cast<const char*>(&magic_num), sizeof(uint64_t));
    ofs.write(reinterpret_cast<const char*>(_data_dumped.get()), sz);
    _conf_path_valid = ofs.good();
#ifndef _WIN32
    ::chmod(_db_path.c_str(), 0600);
#endif
    return *this;
}

app& app::stop_auto_update() {
    if (_auto_update_thread != nullptr) {
    {
        std::unique_lock __lck(_member_mut);
        _auto_update_stopped = true;
    }
        // In the worst case, update starts right before notification
        // Will join after update finishes in this case.
        _auto_update_cv.notify_all();
        _auto_update_thread->join();
        _auto_update_thread.reset();
    }
    return *this;
}

app& app::add_ignored_path(str_t path) {
    if (path.empty())
        path = separator;
    else if (path.front() != separator)
        path = separator + path;
    std::unique_lock __lck(_member_mut);
    _ignored_paths.push_back(std::move(path));
    return *this;
}
app& app::add_root_path(str_t path, bool concur) {
    if (path.empty())
        path = separator;
    else if (path.front() != separator)
        path = separator + path;
    std::unique_lock __lck(_member_mut);
    _root_paths.emplace_back(std::move(path), concur);
    return *this;
}
app& app::add_start_path(str_t path) {
    if (path.empty())
        path = separator;
    else if (path.front() != separator)
        path = separator + path;
    std::unique_lock __lck(_member_mut);
    _start_paths.push_back(std::move(path));
    return *this;
}

app& app::erase_ignored_path(const str_t& path) {
    std::unique_lock __lck(_member_mut);
    _ignored_paths.erase(
        std::remove_if(_ignored_paths.begin(), _ignored_paths.end(), 
            [&path] (const str_t& p) {return p == path || p.c_str() + 1 == path;}),
        _ignored_paths.end()
    );
    return *this;
}
app& app::erase_root_path(const str_t& path) {
    std::unique_lock __lck(_member_mut);
    _root_paths.erase(
        std::remove_if(_root_paths.begin(), _root_paths.end(), 
            [&path] (const auto& p) {
                return p.first == path || p.first.c_str() + 1 == path;
            }),
        _root_paths.end()
    );
    return *this;
}
app& app::erase_start_path(const str_t& path) {
    std::unique_lock __lck(_member_mut);
    _start_paths.erase(
        std::remove_if(_start_paths.begin(), _start_paths.end(), 
            [&path] (const str_t& p) {return p == path || p.c_str() + 1 == path;}),
        _start_paths.end()
    );
    return *this;
}

app& app::read_conf(str_t path) {
    // Members are modified across the function, therefore
    // the lock applies from start to finish
    std::unique_lock __lck(_member_mut);
    if (!path.empty())
        _conf_path = std::move(path);
#if !defined(_WIN32) || defined(_MSC_VER)
    std::basic_ifstream<char_t> ifs(_conf_path);
#else
    std::basic_ifstream<char_t> ifs(orie::xxstrcpy(sv_t(_conf_path)));
#endif
    if (!ifs.is_open()) {
        _conf_path_valid = false;
        return *this;
    }

#ifdef _MSC_VER
    ifs.imbue(std::locale("en_US.UTF-8"));
#endif
    str_t conf_cont;
    std::getline(ifs, conf_cont, NATIVE_PATH('\0')); // TODO: Paths with '\0'?
    sv_t conf_sv(conf_cont);
    _root_paths.clear();
    _ignored_paths.clear();

    while (!conf_sv.empty()) {
        auto [cur_sz, cur_tok] =
            orie::next_token(conf_sv.data(), conf_sv.size());
        conf_sv.remove_prefix(cur_sz);
        if (cur_tok == NATIVE_PATH("DB_PATH")) {
            std::tie(cur_sz, cur_tok) =
                orie::next_token(conf_sv.data(), conf_sv.size());
            conf_sv.remove_prefix(cur_sz);
            _db_path = std::move(cur_tok);
        } else if (cur_tok == NATIVE_PATH("IGNORED")) {
            std::tie(cur_sz, cur_tok) =
                orie::next_token(conf_sv.data(), conf_sv.size());
            conf_sv.remove_prefix(cur_sz);
            _ignored_paths.emplace_back(std::move(cur_tok));
        } else if (cur_tok == NATIVE_PATH("ROOT")) {
            std::tie(cur_sz, cur_tok) =
                orie::next_token(conf_sv.data(), conf_sv.size());
            conf_sv.remove_prefix(cur_sz);
            _root_paths.emplace_back(std::move(cur_tok), false);
            std::tie(cur_sz, cur_tok) =
                orie::next_token(conf_sv.data(), conf_sv.size());
            // SSD field is optional
            if (cur_tok == NATIVE_PATH("SSD")) {
                conf_sv.remove_prefix(cur_sz);
                _root_paths.back().second = true;
            }
        } // Ignore all others
    }

    // Provide a default database name if there isn't one in conf file
    if (_db_path.empty())
        _db_path = _conf_path + NATIVE_PATH(".db");
    _conf_path_valid = true;
    return *this;
}

app& app::write_conf(str_t path) {
    if (!path.empty())
        _conf_path = std::move(path);
    // Provide a default database name if not set, 
    // before printing anything to conf file.
    if (_db_path.empty())
        _db_path = _conf_path + NATIVE_PATH(".db");
#if !defined(_WIN32) || defined(_MSC_VER)
    std::basic_ofstream<char_t> ofs(_conf_path);
#else
    std::basic_ofstream<char_t> ofs(orie::xxstrcpy(sv_t(_conf_path)));
#endif
    if (!ofs.is_open()) {
        _conf_path_valid = false;
        return *this;
    }

#ifdef _MSC_VER
    ofs.imbue(std::locale("en_US.UTF-8"));
#endif
    ofs << NATIVE_PATH("DB_PATH `") << _db_path << char_t('`');
    for (const auto& p : _root_paths) {
        ofs << NATIVE_PATH("\nROOT `") << p.first << char_t('`');
        if (p.second)
            ofs << NATIVE_PATH(" SSD");
    }
    for (const auto& p : _ignored_paths)
        ofs << NATIVE_PATH("\nIGNORED `") << p << char_t('`');
    ofs.put(char_t('\n'));
    _conf_path_valid = true;
    return *this;
}

app::job_list app::get_jobs(fsearch_expr& expr) {
    job_list jobs;
    jobs.reserve(_start_paths.size());
    // A lock must be introduced or an updatedb may alter data_dumped between
    // construct dataiter(+5 lines) and copy data_dumped to job list(+11 lines)
    std::shared_lock __lck(_member_mut);

    // Construct jobs
    for (sv_t p : _start_paths) {
        fs_data_iter it(_data_dumped.get(), p);
        if (it == it.end()) // Invalid starting path
            continue;

        jobs.emplace_back(
            _data_dumped, // std::shared_ptr is internally thread safe
            std::make_unique<
                pred_tree::async_job<fs_data_iter, sv_t>
            >(it, it.end(), expr)
        );
    }
    return jobs;
}

app::app(fifo_thpool& p) : _pool(p) {}
app::app(app&& rhs) noexcept
    : _conf_path_valid(rhs._conf_path_valid), _pool(rhs._pool)
{
    rhs.stop_auto_update();
    // Wait all rhs's jobs to finish
    std::unique_lock __lck(rhs._member_mut);
    
    // Move as usual
    _conf_path = std::move(rhs._conf_path);
    _db_path = std::move(rhs._db_path);
    _ignored_paths = std::move(rhs._ignored_paths);
    _root_paths = std::move(rhs._root_paths);
    _start_paths = std::move(rhs._start_paths);
    _data_dumped = std::move(rhs._data_dumped);
}

app& app::operator=(app&& r) noexcept {
    if (this != &r) {
        this->~app();
        new (this) app(std::move(r));
    }
    return *this;
}

app::~app() { 
    stop_auto_update(); 
    // Wait for other jobs to finish
    std::unique_lock __lck(_member_mut);
}

#ifndef _WIN32
app app::os_default(fifo_thpool& pool) {
    std::string conf_dir = ::getenv("HOME");
    ::mkdir((conf_dir += "/.config").c_str(), 0755);
    ::mkdir((conf_dir += "/orie").c_str(), 0700);
    app res(pool);
    // Use existing conf file when possible
    if (res.read_conf(conf_dir + "/default.txt"))
        return res;

#ifdef MAC_OS_X_VERSION_10_0
    res.read_db(conf_dir + "/default.db")
       .add_root_path("", true)
       .add_root_path("/usr", true)
       .add_root_path("/Library", true)
       .add_root_path("/Applications", true)
       .add_root_path("/Users", true)
       .add_root_path("/private", true)
       .add_ignored_path("/System/Library")
       .add_ignored_path("/Volumes")
       .add_ignored_path("/System/Volumes/Data")
       .add_ignored_path("/private/var/folders")
       .add_ignored_path("/private/var/run")
       .add_ignored_path("/private/var/tmp")
       .add_ignored_path("/private/tmp")
       .write_conf();

#else // GNU/Linux
    res.read_db(conf_dir + "/default.db")
       .add_root_path("/", true)
       .add_root_path("/usr", true)
       .add_root_path("/home", true)
       .add_ignored_path("/proc")
       .add_ignored_path("/run")
       .add_ignored_path("/media")
       .add_ignored_path("/mnt")
       .add_ignored_path("/tmp")
       .add_ignored_path("/var/tmp")
       .write_conf();

#endif
    ::chmod((conf_dir + "/default.txt").c_str(), 0600);
    return res;
}

#else // Windows
app app::os_default(fifo_thpool& pool) {
    wchar_t pathBuf[256];
    auto pathLen = ::GetEnvironmentVariableW(L"AppData", pathBuf, 230);
    if (pathLen == 0 || pathLen >= 230) {
        std::cerr << "Cannot find %AppData%; using D:\\.orie as database directory.";
        ::wcscpy(pathBuf, L"D:");
        pathLen = 2;
    }

    app res(pool);
    std::wstring confDir = pathBuf;
    if (res.read_conf(confDir + L"\\.orie\\default.txt"))
        return res;

    ::CreateDirectoryW((confDir += L"\\.orie").c_str(), nullptr);
    res.read_db(confDir + L"\\default.db");

    pathLen = ::GetEnvironmentVariableW(L"UserProfile", pathBuf, 255);
    if (pathLen != 0 && pathLen < 255)
        res.add_root_path(pathBuf, true);
    pathLen = ::GetEnvironmentVariableW(L"TEMP", pathBuf, 255);
    if (pathLen != 0 && pathLen < 255)
        res.add_ignored_path(pathBuf);
    res.add_ignored_path(L"C:\\Windows")
       .add_ignored_path(L"C:\\Windows.old");

    // Add a root for each drive and its "Program File" directory
    for (wchar_t dri[4] = L"C:\\"; dri[0] <= L'Z'; ++dri[0]) {
        if (::GetDriveTypeW(dri) != DRIVE_FIXED)
            continue;
        dri[2] = L'\0';
        res.add_root_path(dri, true);
        ::wcscpy(pathBuf, dri);
        ::wcscat(pathBuf, L"\\Program Files");
        DWORD ftyp = ::GetFileAttributesW(pathBuf);
        if (ftyp != INVALID_FILE_ATTRIBUTES && ftyp & FILE_ATTRIBUTE_DIRECTORY)
            res.add_root_path(pathBuf, true);

        ::wcscat(pathBuf, L" (x86)");
        ftyp = ::GetFileAttributesW(pathBuf);
        if (ftyp != INVALID_FILE_ATTRIBUTES && ftyp & FILE_ATTRIBUTE_DIRECTORY)
            res.add_root_path(pathBuf, true);
        dri[2] = L'\\';
    }
    res.write_conf();
    return res;
}
#endif

int app::main(int exe_argc, const char_t* const* exe_argv) noexcept {
try {
    int expr_since = 1;
    bool updatedb_flag = false, startpath_flag = false;
    orie::fifo_thpool pool;
    orie::app app(orie::app::os_default(pool));
    while (expr_since < exe_argc) {
        // -conf is the only global option implemented :(
        // More would be added if a non-bloated argparser is found :(
        // (No hashmap, set, string or vector; only string_view, array, ...)
        if (NATIVE_SV("-conf") == exe_argv[expr_since]) {
            if (expr_since + 1 == exe_argc ||
                !app.read_conf(exe_argv[expr_since + 1])) {
                orie::NATIVE_STDOUT << "Unable to read configuration.\n";
                return 3;
            }
            else {
                expr_since += 2;
                continue;
            }
        }
        else if (NATIVE_SV("-updatedb") == exe_argv[expr_since]) {
            updatedb_flag = true;
            ++expr_since;
            continue;
        }

        if (exe_argv[expr_since][0] == char_t('-'))
            break;
        orie::char_t realpath_buf[path_max] = {};
        ssize_t realpath_len =
            orie::realpath(exe_argv[expr_since], realpath_buf, path_max);
        if (realpath_len < 0 || realpath_len >= path_max)
            NATIVE_STDERR << exe_argv[expr_since]
                          << NATIVE_PATH(": No such directory\n");
        app.add_start_path(realpath_buf);
        startpath_flag = true;
        ++expr_since;
    }

    orie::pred_tree::fs_expr_builder builder;
    if (expr_since == exe_argc)
        builder.build(updatedb_flag ? 
            NATIVE_SV("-false") : NATIVE_SV("-true"));
    else
        builder.build(exe_argc - expr_since + 1, exe_argv + expr_since - 1);
    bool has_action = builder.has_action();
    auto callback = [has_action] (orie::fs_data_iter& it) {
        static std::mutex out_mut;
        if (!has_action) {
            std::lock_guard __lck(out_mut);
#ifdef _WIN32
            // Do not print trailing '\\'
            orie::NATIVE_STDOUT << it.path().c_str() + 1 << '\n';
#else
            orie::NATIVE_STDOUT << it.path() << '\n';
#endif
        }
    };

    if (updatedb_flag)
        app.update_db();
    else app.read_db();
    if (app._data_dumped == nullptr) {
        NATIVE_STDERR << "Database not initialized. Please run with "
                         "-updatedb first.\n";
        return 4;
    }

    if (!startpath_flag) {
        char_t cwd_buf[path_max];
#ifdef _WIN32
        // TODO: cwd is longer than path_max
        ::GetCurrentDirectoryW(path_max, cwd_buf);
        app.add_start_path(cwd_buf);
#else
        app.add_start_path(::getcwd(cwd_buf, path_max));
#endif
    }
    if (builder.has_async()) 
        app.run_pooled(*builder.get(), callback);
    else app.run(*builder.get(), callback);

} catch (std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
    return 0;
}

}