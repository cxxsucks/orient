#include <orient/fs/dumper.hpp>
#include <orient/fs/trigram.hpp> // for `place_trigram`
#include <orient/util/charconv_t.hpp>
#include <algorithm>

static void __place_a_name(orie::sv_t str, std::vector<std::byte>& d)
{
    uint16_t len16 = static_cast<uint16_t>(str.size());
    const std::byte* ptr = reinterpret_cast<const std::byte*>(&len16);
    d.insert(d.cend(), ptr, ptr + sizeof(uint16_t));
    ptr = reinterpret_cast<const std::byte*>(str.data());
    d.insert(d.cend(), ptr, ptr + sizeof(orie::char_t) * str.size());
}

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
        res.second.emplace_back(1, orie::char_t(tag)) += ent->d_name;
    }

    orie::closedir(dirp);
    return res;
}

size_t dumper::dump_one(const str_t& fullpath, size_t basename_len, arr2d_writer& w,
                        const dir_info_t& info, size_t nth_file)
{
    auto& d = _index._unplaced_dat; // aliase
    sv_t basename_view(fullpath.c_str() + fullpath.size() - basename_len,
                       basename_len);

    // Dumps full path len and full path if group counter reaches 24
    if (nth_file % nfile_in_batch == 0) {
        _pos_of_batches.push_back(d.size());
        w.add_int(0, d.size());
        _chunk_of_batches.push_back(_index.chunk_count());
        w.add_int(1, _index.chunk_count());
        sv_t parent_view(fullpath.c_str(), fullpath.size() == basename_len ?
                         0 : fullpath.size() - basename_len - 1);
        d.push_back(std::byte(orie::next_group_tag));
        __place_a_name(parent_view, d);
    }

    // Dump dir tag, basename len and basename first
    d.push_back(std::byte(orie::dir_tag));
    __place_a_name(basename_view, d);
    place_trigram(basename_view, nth_file / nfile_in_batch, w);
    ++nth_file;

    // For each sub file, dump its file type, name length and name and
    // dump parent path (which is `fullpath` here) if group counter reaches 24
    for (const str_t& subfile_basename : info.second) {
        if (nth_file % nfile_in_batch == 0) {
            _pos_of_batches.push_back(d.size());
            w.add_int(0, d.size());
            _chunk_of_batches.push_back(_index.chunk_count());
            w.add_int(1, _index.chunk_count());
            d.push_back(std::byte(orie::next_group_tag));
            __place_a_name(sv_t(fullpath), d);
        }

        d.push_back(std::byte(subfile_basename.front()));
        basename_view = sv_t(subfile_basename.c_str() + 1,
                             subfile_basename.size() - 1);
        __place_a_name(basename_view, d);
        place_trigram(basename_view, nth_file / nfile_in_batch, w);
        ++nth_file;
    }
    return nth_file;
}

size_t dumper::dump_noconcur(str_t& fullpath, size_t basename_len, arr2d_writer& w,
                             const dir_info_t& info, size_t nth_file)
{
    nth_file = dump_one(fullpath, basename_len, w, info, nth_file);
    // Dump sub directories
    fullpath.push_back(separator);
    size_t subname_since = fullpath.size();
    for (const str_t& subdir_basename : info.first) {
        fullpath += subdir_basename;
        if (!is_pruned(fullpath))
            nth_file = dump_noconcur(fullpath, subdir_basename.size(), w,
                                     fetch_dir_info(fullpath), nth_file);
        fullpath.erase(subname_since);
    }

    // Dump dir pop tag
    fullpath.pop_back(); // Pop separator
    auto& d = _index._unplaced_dat; // aliase
    d.push_back(std::byte(dir_pop_tag));
    if (d.size() >= chunk_size_hint) {
        d.push_back(std::byte(next_chunk_tag));
        _index.add_last_chunk();
        if (!(_index.chunk_count() & 15))
            w.append_pending_to_file();
    }
    return nth_file;
}

size_t dumper::dump_concur(str_t& fullpath, size_t basename_len, arr2d_writer& w,
                           const dir_info_t& info, size_t nth_file)
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
    nth_file = dump_one(fullpath, basename_len, w, info, nth_file);
    // Dump sub directories
    fullpath.push_back(separator);
    size_t subname_since = fullpath.size();
    for (ptrdiff_t i = sub_dir_infos.size() - 1; i >= 0; i--) {
        const str_t& subdir_basename = info.first.at(i);
        fullpath += subdir_basename;
        dir_info_t subdir_info = sub_dir_infos[i].get();
        if (is_noconcur(fullpath))
            nth_file = dump_noconcur(fullpath, subdir_basename.size(), w,
                                     subdir_info, nth_file);
        else
            nth_file = dump_concur(fullpath, subdir_basename.size(), w,
                                   subdir_info, nth_file);
        fullpath.erase(subname_since);
    }

    // Dump dir pop tag
    auto& d = _index._unplaced_dat; // aliase
    fullpath.pop_back(); // Pop separator
    d.push_back(std::byte(dir_pop_tag));
    if (d.size() >= chunk_size_hint) {
        d.push_back(std::byte(next_chunk_tag));
        _index.add_last_chunk();
        if (!(_index.chunk_count() & 15))
            w.append_pending_to_file();
    }
    return nth_file;
}

void dumper::rebuild_database() {
    _index.clear();
    _invidx.clear();
    _pos_of_batches.clear();
    _chunk_of_batches.clear();
    arr2d_writer w(_invidx.saving_path());
    size_t n_file;

    if (_root_path.size() == 1) {
#ifdef _WIN32
        // On Windows, if root path is '\', dump all drives;
        // Dump fake root dir's metadata first
        _index._unplaced_dat.assign({
            std::byte(), std::byte(),
            std::byte(dir_tag), std::byte(), std::byte(),
        }); // Parent path length (0), tag and name length (0)

        // All drives are its sub dir
        for (str_t dri = L"\\A:\\"; dri[1] <= L'Z'; ++dri[1]) {
            if (::GetDriveTypeW(dri.c_str() + 1) != DRIVE_FIXED)
                continue;
            // For Windows drives only, '\' must be present
            // for its `stat` to return without error
            dir_info_t dri_info = fetch_dir_info(dri);
            dri.pop_back(); // dump_*concur must NOT have ending '\\'
            if (is_noconcur(dri) || is_noconcur(_root_path))
                n_file = dump_noconcur(dri, 2, w, dri_info, 1);
            else n_file = dump_concur(dri, 2, w, dri_info, 1);
            dri.push_back(separator);
        }

        _index._unplaced_dat.push_back(std::byte(dir_pop_tag));
#else
        dir_info_t root_info = fetch_dir_info(_root_path);
        str_t dummy;
        if (is_noconcur(_root_path))
            n_file = dump_noconcur(dummy, 0, w, root_info, 0);
        else n_file = dump_concur(dummy, 0, w, root_info, 0);
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
            n_file = dump_noconcur(_root_path, _root_path.size(), w, root_info, 0);
        else n_file = dump_concur(_root_path, _root_path.size(), w, root_info, 0);
    }

    if (n_file % nfile_in_batch == 0) {
        _index._unplaced_dat.push_back(std::byte(next_group_tag));
        _index._unplaced_dat.push_back(std::byte());
        _index._unplaced_dat.push_back(std::byte());
    }
    _index._unplaced_dat.push_back(std::byte(data_end_tag));
    _index.add_last_chunk();
    w.append_pending_to_file();
    _invidx.refresh();
    assert(_pos_of_batches.size() == _chunk_of_batches.size());
}

dumper::dumper(sv_t database_path, fifo_thpool& pool)
    : _root_path({ separator }), _pool(pool)
    , _index(database_path, cached_chunk_cnt, false, false)
    , _invidx(str_t(database_path) + NATIVE_PATH("_inv"))
    , _pos_of_batches(arr2d_intersect::decompress_entire_line(0, &_invidx))
    , _chunk_of_batches(arr2d_intersect::decompress_entire_line(1, &_invidx)) {}

void dumper::move_file(str_t path) {
    _index.move_file(path.c_str());
    _invidx.move_file(std::move(path += NATIVE_PATH("_inv")));
}

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
