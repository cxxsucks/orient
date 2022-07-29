#include <orient/fs/dumper.hpp>

#include <cstring>
#include <algorithm>

namespace orie {
namespace dmp {

// CONSTRUCTORS
file_dumper::file_dumper(dir_dumper* parent) 
        : parent_dir(parent) {
    if (parent)
        parent->file_data(false).push_back(this);    
}

file_dumper::file_dumper(const string_type& fname, dir_dumper* parent)
        : file_dumper(parent) {
    filename = fname;
}

dir_dumper::dir_dumper(const string_type& fname, time_t write, dir_dumper* parent)
        : file_dumper(fname, nullptr), last_write(write) {
    parent_dir = parent;
    if (parent_dir)
        parent->dir_data(false).push_back(this);
}

link_dumper::link_dumper(const string_type& fname,
    const string_type& link_dir, dir_dumper* parent)
        : file_dumper(fname, parent), link_to_name(link_dir) {}

dir_dumper::dir_dumper(dir_dumper&& rhs) noexcept 
        : dir_dumper(std::move(rhs.filename), rhs.last_write, rhs.parent_dir) {
    my_files = std::move(rhs.my_files);
    my_dirs = std::move(rhs.my_dirs);
    old_size = rhs.old_size;
    rhs.old_size = 0;
    for (dir_dumper* pdir : my_dirs)
        pdir->parent_dir = this;
    for (file_dumper* file : my_files)
        file->parent_dir = this;
}
// DESTRUCTORS
file_dumper::~file_dumper() noexcept {
    if (parent_dir) {
        std::vector<file_dumper*> *a = &parent_dir->file_data(false);
        a->erase(std::remove(a->begin(), a->end(), this), a->end());
    }
}

dir_dumper::~dir_dumper() noexcept {
    std::for_each(my_dirs.begin(), my_dirs.end(), [](dir_dumper* sub) {
        sub->parent_dir = nullptr;
        delete sub;
    });
    std::for_each(my_files.begin(), my_files.end(), [](file_dumper* sub) {
        sub->parent_dir = nullptr;
        delete sub;
    });
    if (parent_dir) {
        std::vector<dir_dumper*> *a = &parent_dir->dir_data(false);
        a->erase(std::remove(a->begin(), a->end(), this), a->end());
        parent_dir = nullptr;
    }
}

dir_dumper& dir_dumper::operator=(dir_dumper&& rhs) noexcept {
    if (this != &rhs) {
        this->~dir_dumper();
        new (this) dir_dumper(std::move(rhs));
    }
    return *this;
}

file_dumper *dir_dumper::update_link_node(dir_dumper *root, bool force) {
    if(!root) 
        root = parent(~unsigned());
    for (dir_dumper* sub : dir_data(false))
        sub->update_link_node(root, force);
    return nullptr;
}

file_dumper* link_dumper::update_link_node(dir_dumper* root, bool force) {
    if (link_to && !force) 
        return link_to;

    if(!root) 
        root = parent(~unsigned());
    file_dumper* found = root->visit_full(link_to_name, false);
    return (link_to = found);
}

const void* file_dumper::from_raw(const void* src) noexcept {
    if (!src) return nullptr;
    src = reinterpret_cast<const value_type*>(src) + 1;
    uint16_t name_len = *reinterpret_cast<const uint16_t*>(src);
    src = reinterpret_cast<const uint16_t*>(src) + 1;
    filename = string_type(reinterpret_cast<const value_type*>(src), name_len);
    return reinterpret_cast<const value_type*>(src) + name_len;
}
// Read n_bytes() of raw bytes from src. Returns the one-past-end pointer of read data.
// If src is not null and the data is valid, the return is not null.
// Bad data results in null return.
const void* dir_dumper::from_raw(const void* src) noexcept {
    if (!src)
        return nullptr;
    value_type tag = *reinterpret_cast<const value_type*>(src);
    if (tag != orie::dir_tag)
        return nullptr;
    src = file_dumper::from_raw(src);
    last_write = *reinterpret_cast<const time_t*>(src);
    src = reinterpret_cast<const time_t*>(src) + 1;
    
    tag = *reinterpret_cast<const value_type*>(src);
    while (src && tag != orie::dir_pop_tag) {
        switch (tag) {
        default: return nullptr;
        case orie::file_tag:
            src = (new file_dumper(this))->from_raw(src); break;
        case orie::dir_tag:
            src = (new dir_dumper(string_type(), time_t(), this))->from_raw(src); break;
        case orie::link_tag:
            src = (new link_dumper(string_type(), string_type(), this))->from_raw(src); break;
        }
        tag = *reinterpret_cast<const value_type*>(src);
    }
    return src ? reinterpret_cast<const value_type*>(src) + 1 : nullptr;
}
// Read n_bytes() of raw bytes from src. Returns the one-past-end pointer of read data.
// If src is not null, the return is not null.
// Bad data results in undefined behavior.
const void *link_dumper::from_raw(const void *src) noexcept {
    if (!(src = file_dumper::from_raw(src)))
        return nullptr;
    uint16_t linkto_len = *reinterpret_cast<const uint16_t*>(src);
    src = reinterpret_cast<const uint16_t*>(src) + 1;
    link_to_name = string_type(reinterpret_cast<const value_type*>(src), linkto_len);
    return reinterpret_cast<const value_type*>(src) + linkto_len;
}
// Writes n_bytes() of raw bytes to dst. Returns the one-past-end pointer of written data.
// If dst is not null, the return is not null. Make sure the destination is pre-allocated.
// Written data can be read with from_raw(void*)
void* file_dumper::to_raw(void* dst) const noexcept {
    if (!dst) return nullptr;
    *reinterpret_cast<value_type*>(dst) = orie::file_tag;
    dst = reinterpret_cast<value_type*>(dst) + 1;
    uint16_t name_len = static_cast<uint16_t>(filename.size());
    *reinterpret_cast<uint16_t*>(dst) = name_len;
    dst = reinterpret_cast<uint16_t*>(dst) + 1;
    ::memcpy(dst, filename.c_str(), name_len * sizeof(value_type));
    return reinterpret_cast<value_type*>(dst) + name_len;
}
// Writes n_bytes() of raw bytes to dst. Returns the one-past-end pointer of written data.
// If dst is not null, the return is not null.
// Make sure the destination is sufficiently pre-allocated.
// as dir_dumper::n_bytes() may return a huge value.
// Written data can be read with from_raw(void*)
void* dir_dumper::to_raw(void* dst) const noexcept {
    if (!dst) return nullptr;
    value_type* begin = reinterpret_cast<value_type*>(dst);
    dst = file_dumper::to_raw(dst);
    // *begin = is_ignored ? dir_ignore_tag : dir_tag;
    *begin = orie::dir_tag;
    *reinterpret_cast<time_t*>(dst) = last_write;
    dst = reinterpret_cast<time_t*>(dst) + 1;

    for (const dir_dumper* pdir : my_dirs)
        dst = pdir->to_raw(dst);
    for (const file_dumper* file : my_files)
        dst = file->to_raw(dst);
    *reinterpret_cast<value_type*>(dst) = orie::dir_pop_tag;
    return reinterpret_cast<value_type*>(dst) + 1;
}

void* link_dumper::to_raw(void* dst) const noexcept {
    if (!dst) 
        return nullptr;
    value_type* begin = reinterpret_cast<value_type*>(dst);
    dst = file_dumper::to_raw(dst);
    *begin = orie::link_tag;
    uint16_t linkto_len = static_cast<uint16_t>(link_to_name.size());
    *reinterpret_cast<uint16_t*>(dst) = linkto_len;
    dst = reinterpret_cast<uint16_t*>(dst) + 1;
    ::memcpy(dst, link_to_name.c_str(), linkto_len * sizeof(value_type));
    return reinterpret_cast<value_type*>(dst) + linkto_len;
}

size_t file_dumper::n_bytes() const noexcept {
    return sizeof(uint16_t) + sizeof(value_type) * (filename.size() + 1);
}

size_t dir_dumper::n_bytes() const noexcept {
    size_t res = file_dumper::n_bytes() + sizeof(time_t) + sizeof(value_type);
    for (const dir_dumper* pdir : my_dirs)
        res += pdir->n_bytes();
    for (const file_dumper* file : my_files)
        res += file->n_bytes();
    return res;
}

size_t link_dumper::n_bytes() const noexcept {
    return file_dumper::n_bytes() + sizeof(uint16_t) + 
        sizeof(value_type) * link_to_name.size();
}

void link_dumper::set_ignored(bool enabled) noexcept {
    if (!link_to)
        update_link_node();
    if (link_to)
        link_to->set_ignored(enabled);
}

void dir_dumper::clear(int all) noexcept {
    if (all & 1) {
        std::for_each(my_files.begin(), my_files.end(), [](file_dumper* sub) {
            sub->parent_dir = nullptr;
            delete sub;
        });
        my_files.clear();
    }
    if (all & 2) {
        std::for_each(my_dirs.begin(), my_dirs.end(), [](dir_dumper* sub) {
            sub->parent_dir = nullptr;
            delete sub;
        });
        my_dirs.clear(), old_size = 0;
        return;
    }
    if ((all & 8)) {
        if (old_size) {
            auto old_end = my_dirs.begin() + old_size;
            std::for_each(my_dirs.begin(), old_end, [](dir_dumper*& b) {
                if (b->valid == false) 
                    b->parent_dir = nullptr, delete b, b = nullptr;
            });
            my_dirs.erase(std::remove(my_dirs.begin(), old_end, nullptr), old_end);
        }
        std::for_each(my_dirs.begin(), my_dirs.end(), [](dir_dumper* b) {
            b->valid = false;
        });
    }
    old_size = my_dirs.size();
}
// 4 resets the connection between *this and linked-to directory
void link_dumper::clear(int all) noexcept {
    if (!link_to) return;
    link_to->clear(all & 11);
    if (all & 4)
        link_to = nullptr;
}

}
}
