#include <orient/fs/dumper.hpp>

#include <cstring>
#include <algorithm>

namespace orie {
namespace dmp {

// CONSTRUCTORS
file_dumper::file_dumper(const string_type& fname, char_t dtype) : filename(fname) {
    switch (dtype) {
    case DT_LNK:
        category = category_tag::link_tag; return;
    case DT_BLK:
        category = category_tag::blk_tag; return;
    case DT_FIFO:
        category = category_tag::fifo_tag; return;
    case DT_SOCK:
        category = category_tag::sock_tag; return;
    case DT_CHR:
        category = category_tag::char_tag; return;
    case DT_REG:
        category = category_tag::file_tag; return;
    }
    // Should not reach, but don't crash because of this
    // std::terminate()
    category = category_tag::file_tag;
}

dir_dumper::dir_dumper(const string_type& fname, time_t write, dir_dumper* parent)
    : last_write(write), filename(fname), parent_dir(parent)
{
    if (parent_dir)
        parent->my_dirs.push_back(this);
}

dir_dumper::dir_dumper(dir_dumper&& rhs) noexcept 
    : dir_dumper(std::move(rhs.filename), rhs.last_write, rhs.parent_dir) 
{
    my_files = std::move(rhs.my_files);
    my_dirs = std::move(rhs.my_dirs);
    old_size = rhs.old_size;
    rhs.old_size = 0;
    for (dir_dumper* pdir : my_dirs)
        pdir->parent_dir = this;
}

dir_dumper::~dir_dumper() noexcept {
    std::for_each(my_dirs.begin(), my_dirs.end(), [](dir_dumper* sub) {
        sub->parent_dir = nullptr;
        delete sub;
    });
    if (parent_dir) {
        std::vector<dir_dumper*> &a = parent_dir->my_dirs;
        a.erase(std::remove(a.begin(), a.end(), this), a.end());
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

const void* file_dumper::from_raw(const void* src) noexcept {
    if (!src) return nullptr;
    category_tag tag = *reinterpret_cast<const category_tag*>(src);
    switch (tag) {
    case category_tag::blk_tag:
    case category_tag::fifo_tag:
    case category_tag::file_tag:
    case category_tag::link_tag:
    case category_tag::sock_tag:
        category = tag;
        break;
    default:
        return nullptr; // Invalid Tag
    }

    src = reinterpret_cast<const category_tag*>(src) + 1;
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
    category_tag tag = *reinterpret_cast<const category_tag*>(src);
    if (tag != orie::dir_tag)
        return nullptr;

    src = reinterpret_cast<const category_tag*>(src) + 1;
    uint16_t name_len = *reinterpret_cast<const uint16_t*>(src);
    src = reinterpret_cast<const uint16_t*>(src) + 1;
    filename = string_type(reinterpret_cast<const value_type*>(src), name_len);
    src = reinterpret_cast<const value_type*>(src) + name_len;
    last_write = *reinterpret_cast<const time_t*>(src);
    src = reinterpret_cast<const time_t*>(src) + 1;
    
    tag = *reinterpret_cast<const category_tag*>(src);
    while (src && tag != orie::dir_pop_tag) {
        switch (tag) {
        default: return nullptr;
        case category_tag::file_tag:  
        case category_tag::blk_tag:
        case category_tag::fifo_tag:
        case category_tag::link_tag:
        case category_tag::sock_tag:
            src = my_files.emplace_back().from_raw(src);
            break;
        case category_tag::dir_tag:
            src = (new dir_dumper(string_type(), time_t(), this))->from_raw(src); 
            break;
        }
        tag = *reinterpret_cast<const category_tag*>(src);
    }
    return src ? reinterpret_cast<const category_tag*>(src) + 1 : nullptr;
}

// Writes n_bytes() of raw bytes to dst. Returns the one-past-end pointer of written data.
// If dst is not null, the return is not null. Make sure the destination is pre-allocated.
// Written data can be read with from_raw(void*)
void* file_dumper::to_raw(void* dst) const noexcept {
    if (!dst) return nullptr;
    *reinterpret_cast<value_type*>(dst) = category;
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
    *reinterpret_cast<value_type*>(dst) = category_tag::dir_tag;
    dst = reinterpret_cast<value_type*>(dst) + 1;
    uint16_t name_len = static_cast<uint16_t>(filename.size());
    *reinterpret_cast<uint16_t*>(dst) = name_len;
    dst = reinterpret_cast<uint16_t*>(dst) + 1;
    ::memcpy(dst, filename.c_str(), name_len * sizeof(value_type));
    dst = reinterpret_cast<value_type*>(dst) + name_len;
    *reinterpret_cast<time_t*>(dst) = last_write;
    dst = reinterpret_cast<time_t*>(dst) + 1;

    for (const dir_dumper* pdir : my_dirs)
        dst = pdir->to_raw(dst);
    for (const file_dumper& file : my_files)
        dst = file.to_raw(dst);
    *reinterpret_cast<value_type*>(dst) = orie::dir_pop_tag;
    return reinterpret_cast<value_type*>(dst) + 1;
}

size_t file_dumper::n_bytes() const noexcept {
    return sizeof(uint16_t) + sizeof(category_tag)
           + sizeof(value_type) * filename.size();
}

size_t dir_dumper::n_bytes() const noexcept {
    size_t res = sizeof(uint16_t) + sizeof(category_tag)
                 + sizeof(value_type) * filename.size()
                 + sizeof(time_t) + sizeof(value_type);
    for (const dir_dumper* pdir : my_dirs)
        res += pdir->n_bytes();
    for (const file_dumper& file : my_files)
        res += file.n_bytes();
    return res;
}

// TODO: The parameter is SO DUMB!
void dir_dumper::clear(int all) noexcept {
    if (all & 1) 
        my_files.clear();
    
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

}
}
