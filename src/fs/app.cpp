#include <orient/app.hpp>
#include <orient/fs/dumper.hpp>
#include <fstream>
#include <algorithm>
#include <cassert>

namespace orie {

app& app::read_db(str_t path) {
    std::unique_lock __lck(_data_dumped_mut);
    _db_path = std::move(path);
    std::fstream ifs(_db_path, std::ios_base::binary);

    // Check whether the file is a legit database for orient
    uint64_t magic_read = 0;
    ifs.read(reinterpret_cast<char*>(&magic_read), sizeof(uint64_t));
    if (magic_read != magic_num)
        return *this; // Stop reading if not

    assert(ifs.tellg() == sizeof(uint64_t));
    size_t db_sz = ifs.seekg(0, std::ios_base::seekdir::end).tellg();
    db_sz -= sizeof(uint64_t);
    _data_dumped.reset(new std::byte[db_sz + 1]);
    ifs.seekg(sizeof(uint64_t));
    ifs.read(reinterpret_cast<char*>(_data_dumped.get()), db_sz);
    _data_dumped[db_sz] = std::byte(0);
    return *this;
}

app& app::update_db() {
    dumper dump_worker;
{   // Read and reuse the old dumped data
    std::shared_lock __lck(_data_dumped_mut);
    dump_worker.from_raw(_data_dumped.get());
}

    for (const str_t& p : _ignored_paths)
        dump_worker.visit_dir(p).set_ignored(true);
    // All root paths must be pruned along with ignored ones,
    // then sorted from the deepest to the shallowest, to prevent
    // rescanning if there are overlapping root paths.
    std::sort(_root_paths.begin(), _root_paths.end(),
              [] (const str_t& a, const str_t& b) {return a.size() > b.size();});
    for (const str_t& p : _root_paths) {
        auto& to_dump = dump_worker.visit_dir(p);
        to_dump.from_fs();
        to_dump.set_ignored(true);
    }

    // Write updated data back to `data_dumped`
    std::unique_lock __lck(_data_dumped_mut);
    size_t sz = dump_worker.n_bytes();
    _data_dumped.reset(new std::byte[sz + 1]);
    dump_worker.to_raw(_data_dumped.get());
    _data_dumped[sz] = std::byte(0);

    // And then into database file
    std::ofstream ofs(_db_path, std::ios_base::binary);
    if (!ofs.is_open()) 
        return *this;
    ofs.write(reinterpret_cast<const char*>(&magic_num), sizeof(uint64_t));
    ofs.write(reinterpret_cast<const char*>(_data_dumped.get()), sz);
    return *this;
}

app& app::stop_auto_update() {
    if (_auto_update_thread != nullptr) {
    // So that this func do not race with _auto_update_stopped 9 lines above
        std::shared_lock __lck(_data_dumped_mut);
        _auto_update_stopped = true;
        _auto_update_cv.notify_all();
        _auto_update_thread->join();
        _auto_update_thread.reset();
    }
    return *this;
}

app& app::add_ignored_path(str_t path) {
    _ignored_paths.push_back(std::move(path));
    return *this;
}
app& app::add_root_path(str_t path) {
    _root_paths.push_back(std::move(path));
    return *this;
}
app& app::add_start_path(str_t path) {
    _start_paths.push_back(std::move(path));
    return *this;
}

app& app::erase_ignored_path(const str_t& path) {
    _ignored_paths.erase(
        std::remove(_ignored_paths.begin(), _ignored_paths.end(), path),
        _ignored_paths.end()
    );
    return *this;
}
app& app::erase_root_path(const str_t& path) {
    _root_paths.erase(
        std::remove(_root_paths.begin(), _root_paths.end(), path),
        _root_paths.end()
    );
    return *this;
}
app& app::erase_start_path(const str_t& path) {
    _start_paths.erase(
        std::remove(_start_paths.begin(), _start_paths.end(), path),
        _start_paths.end()
    );
    return *this;
}

app& app::read_conf(str_t path) {
    if (!path.empty())
        _conf_path = std::move(path);
    std::basic_ifstream<char_t> ifs(_conf_path);
    if (!ifs.is_open()) {
        _conf_path_valid = false;
        return *this;
    }

#ifdef _MSC_VER
    ifs.imbue(std::locale("en_US.UTF-8"));
#endif
    str_t conf_cont;
    std::getline(ifs, conf_cont, '\0'); // TODO: Paths with '\0'?
    sv_t conf_sv(conf_cont);

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
            _root_paths.emplace_back(std::move(cur_tok));
        } // Ignore all others
    }

    // Provide a default database name if there isn't one in conf file
    if (_db_path.empty())
        _db_path = _conf_path + ".db";
    _conf_path_valid = true;
    return *this;
}

app& app::write_conf(str_t path) {
    if (!path.empty())
        _conf_path = std::move(path);
    std::basic_ofstream<char_t> ofs(_conf_path);
    if (!ofs.is_open()) {
        _conf_path_valid = false;
        return *this;
    }
    // Provide a default database name if not set, 
    // before printing anything to conf file.
    if (_db_path.empty())
        _db_path = _conf_path + ".db";

#ifdef _MSC_VER
    ofs.imbue(std::locale("en_US.UTF-8"));
#endif
    ofs << NATIVE_PATH("DB_PATH `") << _db_path << char_t('`');
    for (const auto& p : _root_paths)
        ofs << NATIVE_PATH("\nROOT `") << p << char_t('`');
    for (const auto& p : _ignored_paths)
        ofs << NATIVE_PATH("\nIGNORED `") << p << char_t('`');
    ofs.put(char_t('\n'));
    _conf_path_valid = true;
    return *this;
}

app::app(fifo_thpool& p) : _pool(p) {}
app::app(app&& rhs) 
    : _conf_path_valid(rhs._conf_path_valid), _pool(rhs._pool)
{
    rhs.stop_auto_update();
    // Wait all rhs's jobs to finish
    std::unique_lock __lck(rhs._data_dumped_mut);
    
    // Move as usual
    _conf_path = std::move(rhs._conf_path);
    _db_path = std::move(rhs._db_path);
    _ignored_paths = std::move(rhs._ignored_paths);
    _root_paths = std::move(rhs._root_paths);
    _start_paths = std::move(rhs._start_paths);
    _data_dumped = std::move(rhs._data_dumped);
}

app app::os_default(fifo_thpool& pool) {
    app res(pool);
    std::string _db_path = ::getenv("HOME");
    ::mkdir((_db_path += "/.config").c_str(), 0755);
    ::mkdir((_db_path += "/orie").c_str(), 0644);
    _db_path += "/default.db";

    return res;
}

#ifdef MAC_OS_X_VERSION_10_0

#elif _WIN32

#else

#endif

}