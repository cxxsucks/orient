#include <orient/fs/dumper.hpp>
#include <orient/util/fifo_thpool.hpp>
#include <orient/util/charconv_t.hpp>

#include <algorithm>

namespace orie {
namespace dmp {

char_t dir_dumper::__handle_unknown_dtype(const char_t* path) noexcept {
    orie::stat_t stbuf;
    if (::stat(path, &stbuf) != 0)
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

void dir_dumper::from_fs_impl(str_t& path_slash) noexcept {
    orie::stat_t st;
    orie::dirent_t* ent;
    orie::dir_t* pd;
    size_t len_orig = path_slash.size();

    if (orie::stat(path_slash.c_str(), &st) != 0)
        return clear();
    if (is_ignored)
        return;
    if (up_to_date(st.st_mtime))
        goto updated;

    last_write = st.st_mtime;
    clear(1); // Clear all donon-dir files
    if (!(pd = orie::opendir(path_slash.c_str())))
        return;
    while ((ent = orie::readdir(pd))) {
        if (orie::strcmp(NATIVE_PATH("."), ent->d_name) == 0 ||
                orie::strcmp(NATIVE_PATH(".."), ent->d_name) == 0)
            continue;
        if (ent->d_type == DT_UNKNOWN) {
            path_slash += ent->d_name;
            ent->d_type = __handle_unknown_dtype(path_slash.c_str());
            path_slash.erase(len_orig);
        }

        if (ent->d_type != DT_DIR) 
            add_file(ent->d_name, ent->d_type);
        else
            visit_child_dir(ent->d_name)->valid = true;
    }
    orie::closedir(pd);
    clear(8);

updated:
    std::for_each(my_dirs.begin(), my_dirs.end(),
        [&path_slash, len_orig] (dir_dumper* b) {
            (path_slash += seperator) += b->filename;
            b->from_fs_impl(path_slash);
            path_slash.erase(len_orig);
        });
}

void dir_dumper::from_fs() {
    // value_type* b = new value_type[32768];
    string_type fullp = path(~unsigned());
    if (fullp.empty() || fullp.back() != orie::seperator)
        fullp.push_back(orie::seperator);
    from_fs_impl(fullp);
    // orie::strncpy(b, fullp.c_str(), 16384);
    // from_fs_impl(b, b + 16384);
    // delete []b;
}

dir_dumper *dir_dumper::parent(unsigned depth) const noexcept {
    dir_dumper* res = parent_dir;

    while (--depth && res->parent_dir)
        res = res->parent_dir;
    return res;
}

unsigned int dir_dumper::depth(dir_dumper const *relative_to) const noexcept {
    unsigned res = 0;
    const dir_dumper* cur = this;
    while (cur != relative_to && cur->parent_dir)
        res++, cur = cur->parent_dir;
    if (cur != relative_to && relative_to)
        return ~unsigned();
    return res;
}

file_dumper::string_type dir_dumper::path(unsigned depth) const {
    if (!depth || !parent_dir)
        return filename;
    return parent_dir->path(--depth) + seperator + filename;
}

dir_dumper *dir_dumper::visit_child_dir(const string_type &file_name) {
    if (file_name.empty() || file_name == NATIVE_PATH("."))
        return this;
    else if (file_name == NATIVE_PATH(".."))
        return parent_dir;
    auto diter = std::find_if(my_dirs.cbegin(), my_dirs.cend(),
        [&file_name](const dir_dumper* p) {return p && p->filename == file_name; });
    if (diter != my_dirs.cend())
        return *diter;
    // auto fiter = std::find_if(my_files.cbegin(), my_files.cend(),
    //     [&file_name](const file_dumper* p) {return p && p->filename == file_name; });
    // if (fiter != my_files.cend())
    //     return *fiter;

    return new dir_dumper(file_name, time_t(), this);
    // return my_dirs.back();
}

dir_dumper *dir_dumper::visit_relative_dir(const string_type &rela_path) {
    constexpr static value_type separr[2] = {orie::seperator};
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

// TODO: check repeat
file_dumper* dir_dumper::add_file(const strview_type& name, char_t dtype) {
    my_files.push_back(new file_dumper(string_type(name), dtype));
    return my_files.back();
}

bool dir_dumper::up_to_date(time_t t) const noexcept {
    if (is_ignored)
        return true;
    else return t == last_write;
}

}
}
