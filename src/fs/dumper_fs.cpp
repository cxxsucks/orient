#include <orient/fs/dumper.hpp>
#include <orient/util/fifo_thpool.hpp>
#include <orient/util/charconv_t.hpp>
#include <orient/util/fifo_thpool.hpp>

#include <algorithm>

namespace orie {
namespace dmp {

char_t dir_dumper::__handle_unknown_dtype(const char_t* path) noexcept {
    orie::stat_t stbuf;
    if (orie::stat(path, &stbuf) != 0)
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
        return DT_SOCK;
    if (S_ISBLK(stbuf.st_mode))
        return DT_BLK;
    // Should not reach here, but don't crash so easily.
    NATIVE_STDERR << NATIVE_PATH("Unknown file type: ") << path << '\n';
    return DT_REG;
}

void dir_dumper::from_fs_impl(str_t& path_slash,
    std::atomic<ptrdiff_t>& idle, fifo_thpool& pool) noexcept 
{
    orie::dirent_t* ent;
    orie::dir_t* pd;
    size_t len_orig = path_slash.size();

    if (_is_ignored)
        return;
    clear(1); // Clear all non-dir files

    if (!(pd = orie::opendir(path_slash.c_str())))
        return clear();
    while ((ent = orie::readdir(pd))) {
        if (orie::strcmp(NATIVE_PATH("."), ent->d_name) == 0 ||
                orie::strcmp(NATIVE_PATH(".."), ent->d_name) == 0)
            continue;
        if (ent->d_type == DT_UNKNOWN) {
            path_slash += ent->d_name;
            ent->d_type = __handle_unknown_dtype(path_slash.c_str());
            path_slash.erase(len_orig);
        }

        if (ent->d_type != DT_DIR) {
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
            const std::byte* ptr = reinterpret_cast<const std::byte*>(&tag);
            _sub_data.insert(_sub_data.cend(), ptr, ptr + sizeof(category_tag));
            ptr = reinterpret_cast<const std::byte*>(&name_len);
            _sub_data.insert(_sub_data.cend(), ptr, ptr + sizeof(uint16_t));
            ptr = reinterpret_cast<const std::byte*>(&ent->d_name);
            _sub_data.insert(_sub_data.cend(), ptr, ptr + sizeof(char_t) * name_len);
        } 
        else 
            visit_child_dir(ent->d_name)->_valid = true;
    }
    orie::closedir(pd);
    clear(8);

    // this dir updated; update sub dirs
    for (dir_dumper* b : _sub_dirs) {
        (path_slash += separator) += b->_filename;
        if (idle > 0) {
            --idle;
            pool.enqueue([b, path_slash, &idle, &pool] () {
                str_t path_cpy(path_slash);
                b->from_fs_impl(path_cpy, idle, pool);
                ++idle;
            });
        } else {
            b->from_fs_impl(path_slash, idle, pool);
        }
        path_slash.erase(len_orig);
    }
}

void dir_dumper::from_fs(fifo_thpool& pool, bool multithreaded) {
    string_type fullp = path(~unsigned());
    if (fullp.empty() || fullp.back() != orie::separator)
        fullp.push_back(orie::separator);
    std::atomic<ptrdiff_t> idle = multithreaded ?  pool.n_workers() : 0;
    ptrdiff_t thread_cnt = idle;
    from_fs_impl(fullp, idle, pool);
    // Since the time dumping filesystem is measured in seconds,
    // actively polling once per 10ms is acceptable
    while (idle != thread_cnt)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

dir_dumper *dir_dumper::parent(unsigned depth) const noexcept {
    dir_dumper* res = _parent_dir;

    while (--depth && res->_parent_dir)
        res = res->_parent_dir;
    return res;
}

unsigned int dir_dumper::depth(dir_dumper const *relative_to) const noexcept {
    unsigned res = 0;
    const dir_dumper* cur = this;
    while (cur != relative_to && cur->_parent_dir)
        res++, cur = cur->_parent_dir;
    if (cur != relative_to && relative_to)
        return ~unsigned();
    return res;
}

str_t dir_dumper::path(unsigned depth) const {
    if (!depth || !_parent_dir)
        return _filename;
    return _parent_dir->path(--depth) + separator + _filename;
}

dir_dumper *dir_dumper::visit_child_dir(const string_type &file_name) {
    if (file_name.empty() || file_name == NATIVE_PATH("."))
        return this;
    else if (file_name == NATIVE_PATH(".."))
        return _parent_dir;
    auto diter = std::find_if(_sub_dirs.cbegin(), _sub_dirs.cend(),
        [&file_name](const dir_dumper* p) {return p && p->_filename == file_name; });
    if (diter != _sub_dirs.cend())
        return *diter;

    return new dir_dumper(file_name, time_t(), this);
}

dir_dumper *dir_dumper::visit_relative_dir(const string_type &rela_path) {
    constexpr static value_type separr[2] = {orie::separator};
    auto cont = orie::str_split<std::vector<string_type>, value_type>(rela_path, separr);
    dir_dumper* visiting = this;

    for (const string_type& name : cont)
        if (!(visiting = visiting->visit_child_dir(name)))
            return nullptr;
    return visiting;
}

dir_dumper *dir_dumper::visit_dir(const string_type& full_path) {
    string_type my_path = path(~unsigned());
    if (full_path.find(my_path) != 0)
        return nullptr;
    return visit_relative_dir(full_path.substr(my_path.size()));
}

#ifdef _WIN32
size_t fs_dumper::n_bytes() const noexcept {
    // Like dir_dumper::n_bytes, but filename is always empty.
    size_t res = sizeof(uint16_t) + sizeof(category_tag)
                //  + sizeof(char_t) * _filename.size()
                 + sizeof(char_t);
    for (const dir_dumper& d : _drives)
        res += d.n_bytes();
    return res;
}

void* fs_dumper::to_raw(void* dst) const noexcept {
    if (nullptr == dst)
        return nullptr;
    // Like dir_dumper::to_raw, but with name length
    *reinterpret_cast<char_t*>(dst) = category_tag::dir_tag;
    dst = reinterpret_cast<char_t*>(dst) + 1;
    *reinterpret_cast<uint16_t*>(dst) = 0;
    dst = reinterpret_cast<uint16_t*>(dst) + 1;

    for (const dir_dumper& d : _drives)
        dst = d.to_raw(dst);
    *reinterpret_cast<category_tag*>(dst) = orie::dir_pop_tag;
    return reinterpret_cast<category_tag*>(dst) + 1;
}

dir_dumper* fs_dumper::visit_dir(const str_t& file_name) {
    if (file_name.size() < 2)
        return nullptr;
    wchar_t drive_letter = file_name[0];
    str_t left = file_name.substr(3);
    // Handle possible redundant starting '\\'
    if (drive_letter == separator) {
        drive_letter = file_name[1];
        if (!left.empty())
			left = left.substr(1);
    }
    drive_letter = ::towupper(drive_letter);

    auto diter = std::find_if(_drives.begin(), _drives.end(),
        [drive_letter](const dir_dumper& p) {
            return p.filename().front() == drive_letter; 
        });
    if (diter != _drives.cend())
        return diter->visit_relative_dir(left);
    return _drives.emplace_back(str_t(1, drive_letter) + L':', time_t(), nullptr)
                  .visit_relative_dir(left);
}
#endif // WIN32

}
}
