#include <orient/fs/dumper.hpp>
#include <orient/util/charconv_t.hpp>
#include <algorithm>

static orie::char_t
__handle_unknown_dtype(const orie::char_t* fullp) noexcept {
    orie::stat_t stbuf;
    if (orie::stat(fullp, &stbuf) != 0)
        // Cannot stat? Dump it as a regular file
        return DT_REG;

    if (S_ISREG(stbuf.st_mode))
        return DT_REG;
    if (S_ISDIR(stbuf.st_mode))
        return DT_DIR;
    if (S_ISLNK(stbuf.st_mode))
        return DT_LNK;
    if (S_ISFIFO(stbuf.st_mode))
        return DT_FIFO;
    if (S_ISCHR(stbuf.st_mode))
        return DT_CHR;
    if (S_ISSOCK(stbuf.st_mode))
        return orie::char_t(DT_SOCK); // On Windows these are negative values
    if (S_ISBLK(stbuf.st_mode))
        return orie::char_t(DT_BLK); 
    // Should not reach here, but don't crash so easily.
    orie::NATIVE_STDERR << NATIVE_PATH("Unknown file type: ") << fullp << '\n';
    return DT_REG;
}

namespace orie {
namespace dmp {

dumper::dir_info_t
dumper::fetch_dir_info(str_t& fullpath) {
    orie::dir_t* dirp = orie::opendir(fullpath.c_str());
    if (dirp == nullptr)
        return dir_info_t();
    orie::dirent_t* ent;
    dir_info_t res;

    while ((ent = orie::readdir(dirp)) != nullptr) {
        if (orie::strcmp(NATIVE_PATH("."), ent->d_name) == 0 ||
                orie::strcmp(NATIVE_PATH(".."), ent->d_name) == 0)
            continue;

        if (ent->d_type == DT_UNKNOWN) {
            size_t len_orig = fullpath.size();
            (fullpath += separator) += ent->d_name;
            ent->d_type = __handle_unknown_dtype(fullpath.c_str());
            fullpath.erase(len_orig);
        }
        if (ent->d_type == DT_DIR) {
            res.first.emplace_back(ent->d_name);
            continue;
        }

        // from d_type to orie::category_type
        category_tag tag;
        switch (ent->d_type) {
        case DT_LNK:  tag = orie::link_tag; break;
        case DT_BLK:  tag = orie::blk_tag; break;
        case DT_FIFO: tag = orie::fifo_tag; break;
        case DT_SOCK: tag = orie::sock_tag; break;
        case DT_CHR:  tag = orie::char_tag; break;
        case DT_REG:  tag = orie::file_tag; break;
        default: 
            // Should not reach, but don't crash because of this
            std::cerr << "Warning: unknown file type " 
                      << static_cast<int>(ent->d_type) << '\n';
            continue; // Do not add it
        }
        // Category, name length and name
        uint16_t name_len = static_cast<uint16_t>(orie::strlen(ent->d_name));
        res.second.push_back(static_cast<std::byte>(tag));
        const std::byte* ptr = reinterpret_cast<const std::byte*>(&name_len);
        res.second.insert(res.second.cend(), ptr, ptr + sizeof(uint16_t));
        ptr = reinterpret_cast<const std::byte*>(&ent->d_name);
        res.second.insert(res.second.cend(), ptr, 
                          ptr + sizeof(char_t) * name_len);
    }

    orie::closedir(dirp);
    return res;
}

void dumper::dump_noconcur(str_t& fullpath, size_t basename_len, 
                           const dir_info_t& info)
{   // Dump dir tag, basename len, basename and sub files first 
    uint16_t len16(basename_len);
    auto& d = _data_dumped._unplaced_dat; // aliase
    d.push_back(std::byte(orie::dir_tag));
    const std::byte* ptr = reinterpret_cast<const std::byte*>(&len16);
    d.insert(d.cend(), ptr, ptr + sizeof(uint16_t));
    ptr = reinterpret_cast<const std::byte*>(fullpath.c_str() + fullpath.size()
                                             - basename_len);
    d.insert(d.cend(), ptr, ptr + sizeof(char_t) * basename_len);
    d.insert(d.cend(), info.second.cbegin(), info.second.cend());

    // Dump sub directories
    fullpath.push_back(separator);
    size_t subname_since = fullpath.size();
    for (const str_t& subdir_basename : info.first) {
        fullpath += subdir_basename;
        if (!is_pruned(fullpath)) 
            dump_noconcur(fullpath, subdir_basename.size(), 
                          fetch_dir_info(fullpath));
        fullpath.erase(subname_since);
    }

    // Dump dir pop tag
    fullpath.pop_back(); // Pop separator
    d.push_back(std::byte(dir_pop_tag));
    if (d.size() >= chunk_size_hint) {
        d.push_back(std::byte(next_chunk_tag));
        _data_dumped.add_last_chunk();
    }
}

void dumper::dump_concur(str_t& fullpath, size_t basename_len, 
                         const dir_info_t& info)
{
    std::vector<std::future<dir_info_t>> sub_dir_infos(info.first.size());
    str_t fullpath_cpy(fullpath + separator);
    std::transform(info.first.begin(), info.first.end(), sub_dir_infos.begin(),
                   [this, &fullpath_cpy] (const str_t& basename) {
                    return _pool.enqueue([this, &fullpath_cpy, &basename] () {
                        str_t p = fullpath_cpy + basename;
                        if (is_pruned(p))
                            return dir_info_t();
                        return fetch_dir_info(p);
                    });
                   });

    // Dump dir tag, basename len, basename and sub files
    // while `fetch_dir_info` are running in background
    uint16_t len16(basename_len);
    auto& d = _data_dumped._unplaced_dat; // aliase
    d.push_back(std::byte(orie::dir_tag));
    const std::byte* ptr = reinterpret_cast<const std::byte*>(&len16);
    d.insert(d.cend(), ptr, ptr + sizeof(uint16_t));
    ptr = reinterpret_cast<const std::byte*>(fullpath.c_str() + fullpath.size()
                                             - basename_len);
    d.insert(d.cend(), ptr, ptr + sizeof(char_t) * basename_len);
    d.insert(d.cend(), info.second.cbegin(), info.second.cend());

    // Dump sub directories
    fullpath.push_back(separator);
    size_t subname_since = fullpath.size();
    for (ptrdiff_t i = sub_dir_infos.size() - 1; i >= 0; i--) {
        const str_t& subdir_basename = info.first.at(i);
        fullpath += subdir_basename;
        dir_info_t subdir_info = sub_dir_infos[i].get();
        if (is_noconcur(fullpath))
            dump_noconcur(fullpath, subdir_basename.size(), subdir_info);
        else
            dump_concur(fullpath, subdir_basename.size(), subdir_info);
        fullpath.erase(subname_since);
    }

    // Dump dir pop tag
    fullpath.pop_back(); // Pop separator
    d.push_back(std::byte(dir_pop_tag));
    if (d.size() >= chunk_size_hint) {
        d.push_back(std::byte(next_chunk_tag));
        _data_dumped.add_last_chunk();
    }
}

void dumper::rebuild_database() {
    _data_dumped.clear();

    if (_root_path.size() == 1) {
#ifdef _WIN32
        // On Windows, if root path is '\', dump all drives;
        // Dump fake root dir's metadata first
        _data_dumped._unplaced_dat.assign({
            std::byte(dir_tag), std::byte(), std::byte()
        }); // Tag and name length (0)

        // All drives are its sub dir
        for (str_t dri = L"\\C:\\"; dri[1] <= L'Z'; ++dri[1]) {
            if (::GetDriveTypeW(dri.c_str() + 1) != DRIVE_FIXED)
                continue;
            // For Windows drives only, '\' must be present to fetch info
            dir_info_t dri_info = fetch_dir_info(dri);
            dri.pop_back(); // dump_*concur must start without '\'
            if (is_noconcur(dri) || is_noconcur(_root_path))
                dump_noconcur(dri, 2, dri_info);
            else dump_concur(dri, 2, dri_info);
            dri.push_back(separator);
        }

        _data_dumped._unplaced_dat.push_back(std::byte(dir_pop_tag));
#else
        dir_info_t root_info = fetch_dir_info(_root_path);
        str_t dummy;
        if (is_noconcur(_root_path))
            dump_noconcur(dummy, 0, root_info);
        else dump_concur(dummy, 0, root_info);
#endif

    } else {
        dir_info_t root_info = fetch_dir_info(_root_path);
#ifdef _WIN32
        // on Windows, if failed, append '\' and try again
        // since for Windows drives only, '\' must be present to fetch info
        if (root_info.first.empty() && root_info.second.empty()) {
            _root_path.push_back(separator);
            root_info = fetch_dir_info(_root_path);
            _root_path.pop_back();
        }
#endif

        if (is_noconcur(_root_path))
            dump_noconcur(_root_path, _root_path.size(), root_info);
        else dump_concur(_root_path, _root_path.size(), root_info);
    }
    _data_dumped._unplaced_dat.push_back(std::byte(data_end_tag));
    _data_dumped.add_last_chunk();
}

dumper::dumper(sv_t database_path, fifo_thpool& pool)
    : _root_path({ separator })
    , _data_dumped(database_path, 4, false, false), _pool(pool) {}

bool dumper::is_pruned(const str_t& fullp) {
    return std::find(_pruned_paths.cbegin(), _pruned_paths.cend(),
                     fullp) != _pruned_paths.cend();
}

bool dumper::is_noconcur(const str_t& fullp) {
    return std::find(_noconcur_paths.cbegin(), _noconcur_paths.cend(),
                     fullp) != _noconcur_paths.cend();
}

}
}