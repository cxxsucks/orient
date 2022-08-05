#include <orient/fs/dumper.hpp>
#include <orient/util/fifo_thpool.hpp>
#include <orient/util/stringconv_t.hpp>

#include <algorithm>

namespace orie {
namespace dmp {

void dir_dumper::from_fs_impl(value_type* upPath, value_type* lnkBuf) noexcept {
    orie::stat_t st;
    orie::dirent_t* ent;
    orie::dir_t* pd;
    size_t lenOrig = orie::strlen(upPath);

    if (orie::stat(upPath, &st) != 0)
        return clear();
    if (is_ignored)
        return;
    if (up_to_date(st.st_mtime))
        goto updated;

    last_write = st.st_mtime;
    clear(1);
    if (!(pd = orie::opendir(upPath)))
        return;
    while ((ent = orie::readdir(pd))) {
        if (orie::strcmp(NATIVE_PATH("."), ent->d_name) == 0 ||
                orie::strcmp(NATIVE_PATH(".."), ent->d_name) == 0)
            continue;
        if (ent->d_type != DT_DIR && ent->d_type != DT_LNK) {
            add_file(ent->d_name);
            continue;
        }

        if (ent->d_type == DT_DIR) {
            add_dir(ent->d_name, 0)->valid = true;
        } else if (ent->d_type == DT_LNK) {
#ifdef __unix
            auto fd = ::open(upPath, O_DIRECTORY | O_RDONLY);
            if (fd < 0) continue;
            ssize_t len = ::readlinkat(fd, ent->d_name, lnkBuf, 16384);
            if (len > 0) {
                lnkBuf[len] = '\0';
                add_link(ent->d_name, lnkBuf);
            }
            upPath[lenOrig] = '\0';
            ::close(fd);
#else
            static_cast<void>(lnkBuf);
            add_link(ent->d_name, strview_type());
#endif // !_WIN32
        }
    }
    orie::closedir(pd);
    clear(8);
updated:
    static constexpr orie::value_type sepArr[2] = {
        orie::seperator, '\0'
    };
    std::for_each(my_dirs.begin(), my_dirs.end(),
        [upPath, lnkBuf, lenOrig] (dir_dumper* b) {
            orie::strncat(upPath, b->filename.c_str(), 16384 - lenOrig);
            orie::strncat(upPath, sepArr, 16384 - lenOrig - b->filename.size());
            b->from_fs_impl(upPath, lnkBuf);
            upPath[lenOrig] = '\0';
        });
}

void dir_dumper::from_fs() {
    value_type* b = new value_type[32768];
    string_type fullp = path(depth());
    if (fullp.empty() || fullp.back() != orie::seperator)
        fullp.push_back(orie::seperator);
    orie::strncpy(b, fullp.c_str(), 16384);
    from_fs_impl(b, b + 16384);
    delete []b;
}

dir_dumper *file_dumper::parent(unsigned depth) const noexcept {
    dir_dumper* res = parent_dir;

    while (--depth && res->parent_dir)
        res = res->parent_dir;
    return res;
}

const std::vector<dir_dumper*> &file_dumper::sub_dirs(bool) const noexcept {
    static std::vector<dir_dumper*> dummy;
    dummy.clear();
    return dummy;
}

const std::vector<file_dumper*> &file_dumper::sub_files(bool) const noexcept {
    static std::vector<file_dumper*> dummy;
    dummy.clear();
    return dummy;
}

const std::vector<dir_dumper*> &link_dumper::sub_dirs(bool read_link) const noexcept {
    if (!read_link || !link_to) {
        return file_dumper::sub_dirs(false);
    }
    return link_to->sub_dirs(false);
}

const std::vector<file_dumper*> &link_dumper::sub_files(bool read_link) const noexcept {
    if (!read_link || !link_to) {
        return file_dumper::sub_files(false);
    }
    return link_to->sub_files(false);
}

unsigned int file_dumper::depth(dir_dumper const *relative_to) const noexcept {
    unsigned res = 0;
    const file_dumper* cur = this;
    while (cur != relative_to && cur->parent_dir)
        res++, cur = cur->parent_dir;
    if (cur != relative_to && relative_to)
        return ~unsigned();
    return res;
}

file_dumper::string_type file_dumper::path(unsigned depth) const {
    if (!depth || !parent_dir)
        return filename;
    return parent_dir->path(--depth) + filename;
}

file_dumper *dir_dumper::visit_one(const string_type &file_name, bool force) {
    if (file_name.empty() || file_name == NATIVE_PATH("."))
        return this;
    else if (file_name == NATIVE_PATH(".."))
        return parent_dir;
    auto diter = std::find_if(my_dirs.cbegin(), my_dirs.cend(),
        [&file_name](const dir_dumper* p) {return p && p->filename == file_name; });
    if (diter != my_dirs.cend())
        return *diter;
    auto fiter = std::find_if(my_files.cbegin(), my_files.cend(),
        [&file_name](const file_dumper* p) {return p && p->filename == file_name; });
    if (fiter != my_files.cend())
        return *fiter;
    if (!force)
        return nullptr;
    return new dir_dumper(file_name, time_t(), this);
    // return my_dirs.back();
}

file_dumper *dir_dumper::visit_relative(const string_type &rela_path, bool force) {
    constexpr static value_type separr[2] = {orie::seperator};
    auto cont = orie::str_split<std::vector<string_type>, value_type>(rela_path, separr);
    file_dumper* visiting = this;

    for (const string_type& name : cont)
        if (!(visiting = visiting->visit_one(name, force)))
            return nullptr;
    return visiting;
}

file_dumper *dir_dumper::visit_full(const string_type& full_path, bool force) {
    string_type my_path = path(~unsigned());
    if (full_path.find(my_path) != 0)
        return nullptr;
    return visit_relative(full_path.substr(my_path.size()), force);
}

file_dumper *link_dumper::visit_one(const string_type &file_name, bool force) {
    if (!link_to)
        update_link_node();
    if (link_to)
        return link_to->visit_one(file_name, force);
    return nullptr;
}

file_dumper *link_dumper::visit_relative(const string_type &rela_path, bool force) {
    if (!link_to)
        update_link_node();
    if (link_to)
        return link_to->visit_relative(rela_path, force);
    return nullptr;
}

file_dumper *link_dumper::visit_full(const file_dumper::string_type &full_path, bool force) {
    string_type my_path = path(~unsigned());
    if (full_path.find(my_path) != 0)
        return nullptr;
    return visit_relative(full_path.substr(my_path.size()), force);
}

// TODO: check repeat
file_dumper* dir_dumper::add_file(const strview_type& name) {
    return new file_dumper(string_type(name), this);
}

dir_dumper *dir_dumper::add_dir(const strview_type &name, time_t t) {
    auto dIter = std::find_if(my_dirs.cbegin(), my_dirs.cend(),
        [&name](const dir_dumper* d) {return d->filename == name;});
    if (dIter == my_dirs.cend())
        return new dir_dumper(string_type(name), t, this);
    return *dIter;
}
// TODO: check repeat
link_dumper *dir_dumper::add_link(const strview_type &name,
    const strview_type &link_to) {
    return new link_dumper(string_type(name), string_type(link_to), this);
}

file_dumper *link_dumper::add_file(const strview_type &name) {
    if (!link_to)
        update_link_node();
    if (link_to)
        return link_to->add_file(name);
    return nullptr;
}

dir_dumper *link_dumper::add_dir(const strview_type &name, time_t t) {
    if (!link_to)
        update_link_node();
    if (link_to)
        return link_to->add_dir(name, t);
    return nullptr;
}

link_dumper *link_dumper::add_link(const strview_type &name,
    const strview_type &link_name) {
    if (!link_to)
        update_link_node();
    if (link_to)
        return link_to->add_link(name, link_name);
    return nullptr;
}

bool dir_dumper::up_to_date(time_t t) const noexcept {
    if (is_ignored)
        return true;
    else return t == last_write;
}

bool link_dumper::up_to_date(time_t t) const noexcept {
    if (!link_to) return false;
    return link_to->up_to_date(t);
}

}
}
