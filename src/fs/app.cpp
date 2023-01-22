#include <orient/app.hpp>
#include <orient/fs/dumper.hpp>
#include <fstream>
#include <algorithm>
#include <cassert>

namespace orie {

app& app::update_db() {
    if (_dumper == nullptr)
        return *this;
    // Having multiple dumpers running gets no performance gain
    static std::mutex global_dump_lock;
    std::lock_guard __lck(global_dump_lock);

    // Move current dumper to a temporary location in case it is being used by
    // jobs. Old database is also scheduled to be deleted on job finish.
    str_t db_path = _dumper->_data_dumped.saving_path();
#ifdef _WIN32
    _dumper->_data_dumped.move_file((db_path + std::to_wstring(::rand())).c_str());
#else
    _dumper->_data_dumped.move_file((db_path + std::to_string(::rand())).c_str());
#endif
    _dumper->_data_dumped._rmfile_on_dtor = true;

    // Setup the new dumper
    std::shared_ptr<dmp::dumper> dumper_new(new dmp::dumper(db_path, _pool));
    dumper_new->_noconcur_paths = _dumper->_noconcur_paths;
    dumper_new->_root_path = _dumper->_root_path;
    dumper_new->_pruned_paths = _dumper->_pruned_paths;

    // Dump with the new dumper. While dumping, old dumper remain functional
    dumper_new->rebuild_database();
    // No lock; shared_ptr is internally thread safe
    dumper_new.swap(_dumper);
#ifndef _WIN32
    chmod(_dumper->_data_dumped.saving_path().c_str(), 0600);
#endif
    return *this;
}

app& app::stop_auto_update() {
    if (_auto_update_thread != nullptr) {
    {
        std::unique_lock __lck(_paths_mut);
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

app& app::set_root_path(str_t path) {
    std::unique_lock __lck(_paths_mut);
    _dumper->_root_path = std::move(path);
    return *this;
}

app& app::add_ignored_path(str_t path) {
    std::lock_guard __lck(_paths_mut);
    _dumper->_pruned_paths.push_back(std::move(path));
    return *this;
}
app& app::add_slow_path(str_t path) {
    std::lock_guard __lck(_paths_mut);
    _dumper->_noconcur_paths.push_back(std::move(path));
    return *this;
}
app& app::add_start_path(str_t path) {
    // Start paths are managed by orie::app and must be rectified
    if (path.empty())
        path = separator;
    else if (path.front() != separator)
        path = separator + path;
    std::lock_guard __lck(_paths_mut);
    _start_paths.push_back(std::move(path));
    return *this;
}

app& app::erase_ignored_path(const str_t& path) {
    std::lock_guard __lck(_paths_mut);
    auto& d = _dumper->_pruned_paths;
    d.erase(std::remove_if(d.begin(), d.end(), 
            [&path] (const str_t& p) {return p == path || p.c_str() + 1 == path;}),
            d.end());
    return *this;
}
app& app::erase_slow_path(const str_t& path) {
    std::lock_guard __lck(_paths_mut);
    auto& d = _dumper->_noconcur_paths;
    d.erase(std::remove_if(d.begin(), d.end(), 
            [&path] (const str_t& p) {return p == path || p.c_str() + 1 == path;}),
            d.end());
    return *this;
}
app& app::erase_start_path(const str_t& path) {
    std::lock_guard __lck(_paths_mut);
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
    std::lock_guard __lck(_paths_mut);
    _dumper.reset();
    if (!path.empty())
        _conf_path = std::move(path);
#if !defined(_WIN32) || defined(_MSC_VER)
    std::basic_ifstream<char_t> ifs(_conf_path);
#else
    std::basic_ifstream<char_t> ifs(orie::xxstrcpy(sv_t(_conf_path)));
#endif
    if (!ifs.is_open())
        return *this;

#ifdef _MSC_VER
    ifs.imbue(std::locale("en_US.UTF-8"));
#endif
    str_t conf_cont;
    // Conf file are small enough to be loaded to memory entirely
    std::getline(ifs, conf_cont, NATIVE_PATH('\0'));
    sv_t conf_sv(conf_cont);

    int last_at = -1;
    while (!conf_sv.empty()) {
        auto [cur_sz, cur_tok] =
            orie::next_token(conf_sv.data(), conf_sv.size());
        conf_sv.remove_prefix(cur_sz);

        switch (last_at) {
        case 0: // DB_PATH
            _dumper.reset(new dmp::dumper(cur_tok, _pool)); 
            last_at = -1; break;
        case 1: // ROOT_PATH
            _dumper->_root_path = cur_tok;
            last_at = -1; break;
        case 2: // IGNORED_PATH
            _dumper->_pruned_paths.push_back(std::move(cur_tok));
            last_at = -1; break;
        case 3: // SLOW_PATH
            _dumper->_noconcur_paths.push_back(std::move(cur_tok)); 
            last_at = -1; break;

        default:
            if (cur_tok == NATIVE_PATH("DB_PATH")) {
                last_at = 0;
            } else if (cur_tok == NATIVE_PATH("ROOT_PATH")) {
                if (_dumper == nullptr) goto warning;
                last_at = 1;
            } else if (cur_tok == NATIVE_PATH("IGNORED_PATH")) {
                if (_dumper == nullptr) goto warning;
                last_at = 2;
            } else if (cur_tok == NATIVE_PATH("SLOW_PATH")) {
                if (_dumper == nullptr) goto warning;
                last_at = 3;
            }  // Ignore all others
            break;
    warning:
            NATIVE_STDERR << NATIVE_PATH("DB_PATH must precede all paths. Found")
                          << cur_tok << NATIVE_PATH('\n');
        }
    }
    return *this;
}

app& app::write_conf(str_t path) {
    if (!path.empty())
        _conf_path = std::move(path);
#if !defined(_WIN32) || defined(_MSC_VER)
    std::basic_ofstream<char_t> ofs(_conf_path);
#else
    std::basic_ofstream<char_t> ofs(orie::xxstrcpy(sv_t(_conf_path)));
#endif
    if (!ofs.is_open())
        return *this;

#ifdef _MSC_VER
    ofs.imbue(std::locale("en_US.UTF-8"));
#endif
    ofs << NATIVE_PATH("DB_PATH `") << db_path() << char_t('`');
    for (const auto& p : _dumper->_root_path) 
        ofs << NATIVE_PATH("\nROOT_PATH `") << p << char_t('`');
    for (const auto& p : _dumper->_noconcur_paths)
        ofs << NATIVE_PATH("\nSLOW_PATH `") << p << char_t('`');
    for (const auto& p : _dumper->_pruned_paths)
        ofs << NATIVE_PATH("\nIGNORED_PATH `") << p << char_t('`');
    ofs.put(char_t('\n'));
    return *this;
}

app::job_list app::get_jobs(fsearch_expr& expr) {
    job_list jobs;
    jobs.reserve(_start_paths.size());
    // A lock must be introduced or an updatedb may alter data_dumped between
    // construct dataiter(+5 lines) and copy data_dumped to job list(+11 lines)
    std::lock_guard __lck(_paths_mut);

    // Construct jobs
    for (sv_t p : _start_paths) {
        fs_data_iter it(&_dumper->_data_dumped, p);
        if (it == it.end()) // Invalid starting path
            continue;

        jobs.emplace_back(
            _dumper, // std::shared_ptr is internally thread safe
            std::make_unique<
                pred_tree::async_job<fs_data_iter, sv_t>
            >(it, it.end(), expr)
        );
    }
    return jobs;
}

app::app(fifo_thpool& p) : _pool(p) {}
app::app(app&& rhs) noexcept
    : _conf_path(std::move(rhs._conf_path))
    , _start_paths(std::move(rhs._start_paths))
    // COPY rhs's dumper ptr since rhs's jobs are not finished
    , _dumper(rhs._dumper), _pool(rhs._pool)
{
    rhs.stop_auto_update();
    // Wait all rhs's jobs to finish
    std::unique_lock __lck(rhs._paths_mut);
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
    std::unique_lock __lck(_paths_mut);
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

    res.set_db_path((conf_dir + "/default.db").c_str())
       .set_root_path("/")
#ifdef MAC_OS_X_VERSION_10_0
       .add_ignored_path("/System/Library")
       .add_ignored_path("/Volumes")
       .add_ignored_path("/System/Volumes/Data")
       .add_ignored_path("/private/var/folders")
       .add_ignored_path("/private/var/run")
       .add_ignored_path("/private/var/tmp")
       .add_ignored_path("/private/tmp")
       .write_conf();

#else // GNU/Linux
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
    res.set_db_path((confDir + L"\\default.db").c_str());

    pathLen = ::GetEnvironmentVariableW(L"TEMP", pathBuf, 255);
    if (pathLen != 0 && pathLen < 255)
        res.add_ignored_path(pathBuf);
    res.add_ignored_path(L"C:\\Windows")
       .add_ignored_path(L"C:\\Windows.old");
       .write_conf();
    return res;
}
#endif

}