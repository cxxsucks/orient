#include <orient/fs/dumper.hpp>
#include <orient/fs/data_iter.hpp> // fs_data_record

#include <cstring>
#include <cassert>
#include <algorithm>

namespace orie {
namespace dmp {

// CONSTRUCTORS
dir_dumper::dir_dumper(const string_type& fname, time_t write, dir_dumper* parent)
    : _last_write(write), _filename(fname), _parent_dir(parent)
{
    if (_parent_dir)
        parent->_sub_dirs.push_back(this);
}

dir_dumper::dir_dumper(dir_dumper&& rhs) noexcept 
    : dir_dumper(std::move(rhs._filename), rhs._last_write, rhs._parent_dir) 
{
    _sub_data = std::move(rhs._sub_data);
    _sub_dirs = std::move(rhs._sub_dirs);
    _old_size = rhs._old_size;
    rhs._old_size = 0;
    for (dir_dumper* pdir : _sub_dirs)
        pdir->_parent_dir = this;
}

dir_dumper::~dir_dumper() noexcept {
    std::for_each(_sub_dirs.begin(), _sub_dirs.end(), [](dir_dumper* sub) {
        sub->_parent_dir = nullptr;
        delete sub;
    });
    if (_parent_dir) {
        std::vector<dir_dumper*> &a = _parent_dir->_sub_dirs;
        a.erase(std::remove(a.begin(), a.end(), this), a.end());
        _parent_dir = nullptr;
    }
}

dir_dumper& dir_dumper::operator=(dir_dumper&& rhs) noexcept {
    if (this != &rhs) {
        this->~dir_dumper();
        new (this) dir_dumper(std::move(rhs));
    }
    return *this;
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
    _filename = string_type(reinterpret_cast<const value_type*>(src), name_len);
    src = reinterpret_cast<const value_type*>(src) + name_len;
    _last_write = *reinterpret_cast<const time_t*>(src);
    src = reinterpret_cast<const time_t*>(src) + 1;
    
    // dir_pop_tag_right after dir_tag, implying an empty dir
    if (dir_pop_tag == *reinterpret_cast<const category_tag*>(src))
        return reinterpret_cast<const category_tag*>(src) + 1; 

    // read indexed files with fs_data_record
    fs_data_record rec(src, 0); 
    ptrdiff_t popped = 0;
    while (popped >= 0 && rec.file_type() != dir_tag) {
        popped = rec.increment();
    }
    _sub_data.assign(reinterpret_cast<const std::byte*>(src),
                     reinterpret_cast<const std::byte*>(src) + rec.pos() + popped);
    assert(popped <= 0); // Cannot enter a directory
    if (popped < 0)
        // +1 for one-past-end of dir_pop_tag
        return reinterpret_cast<const std::byte*>(src) + rec.pos() + popped + 1;
    else
        src = reinterpret_cast<const std::byte*>(src) + rec.pos();

    // If popped==0, then the record is still inside the directory
    // therefore current position must be a directory
    assert(*reinterpret_cast<const category_tag*>(src) == dir_tag);
    while (src != nullptr &&
           (tag = *reinterpret_cast<const category_tag*>(src)) != dir_pop_tag)
    {
        assert(tag == orie::dir_tag);
        src = (new dir_dumper(string_type(), time_t(), this))->from_raw(src); 
    }
    return src ? reinterpret_cast<const category_tag*>(src) + 1 : nullptr;
}

// Writes n_bytes() of raw bytes to dst. Returns the one-past-end pointer of written data.
// If dst is not null, the return is not null.
// Make sure the destination is sufficiently pre-allocated.
// as dir_dumper::n_bytes() may return a huge value.
// Written data can be read with from_raw(void*)
void* dir_dumper::to_raw(void* dst) const noexcept {
    if (!dst) return nullptr;
    // dir_tag, name length, basename, mtime
    *reinterpret_cast<value_type*>(dst) = category_tag::dir_tag;
    dst = reinterpret_cast<value_type*>(dst) + 1;
    uint16_t name_len = static_cast<uint16_t>(_filename.size());
    *reinterpret_cast<uint16_t*>(dst) = name_len;
    dst = reinterpret_cast<uint16_t*>(dst) + 1;
    ::memcpy(dst, _filename.c_str(), name_len * sizeof(value_type));
    dst = reinterpret_cast<value_type*>(dst) + name_len;
    *reinterpret_cast<time_t*>(dst) = _last_write;
    dst = reinterpret_cast<time_t*>(dst) + 1;

    // Sub files, sub dirs, and dir_pop_tag
    dst = std::copy(_sub_data.cbegin(), _sub_data.cend(), 
                    reinterpret_cast<std::byte*>(dst));
    for (const dir_dumper* pdir : _sub_dirs)
        dst = pdir->to_raw(dst);
    *reinterpret_cast<value_type*>(dst) = orie::dir_pop_tag;
    return reinterpret_cast<value_type*>(dst) + 1;
}

size_t dir_dumper::n_bytes() const noexcept {
    size_t res = sizeof(uint16_t) + sizeof(category_tag)
                 + sizeof(value_type) * _filename.size()
                 + sizeof(time_t) + sizeof(value_type);
    for (const dir_dumper* pdir : _sub_dirs)
        res += pdir->n_bytes();
    res += _sub_data.size();
    return res;
}

// TODO: The parameter is SO DUMB!
void dir_dumper::clear(int all) noexcept {
    if (all & 1) 
        _sub_data.clear();
    
    if (all & 2) {
        std::for_each(_sub_dirs.begin(), _sub_dirs.end(), [](dir_dumper* sub) {
            sub->_parent_dir = nullptr;
            delete sub;
        });
        _sub_dirs.clear(), _old_size = 0;
        return;
    }

    if ((all & 8)) {
        if (_old_size) {
            auto old_end = _sub_dirs.begin() + _old_size;
            std::for_each(_sub_dirs.begin(), old_end, [](dir_dumper*& b) {
                if (b->_valid == false) 
                    b->_parent_dir = nullptr, delete b, b = nullptr;
            });
            _sub_dirs.erase(std::remove(_sub_dirs.begin(), old_end, nullptr), old_end);
        }
        std::for_each(_sub_dirs.begin(), _sub_dirs.end(), [](dir_dumper* b) {
            b->_valid = false;
        });
    }
    _old_size = _sub_dirs.size();
}

}
}
