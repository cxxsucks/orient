#include <orient/fs/dumper.hpp>
#include <orient/fs/data_iter.hpp> // fs_data_record

#include <cstring>
#include <cassert>
#include <algorithm>

namespace orie {
namespace dmp {

// CONSTRUCTORS
dir_dumper::dir_dumper(const string_type& fname, time_t, dir_dumper* parent)
    : _filename(fname), _parent_dir(parent)
{
    if (_parent_dir)
        parent->_sub_dirs.push_back(this);
}

dir_dumper::dir_dumper(dir_dumper&& rhs) noexcept 
    : dir_dumper(std::move(rhs._filename), 0, rhs._parent_dir) 
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

// Writes n_bytes() of raw bytes to dst. Returns the one-past-end pointer of written data.
// If dst is not null, the return is not null.
// Make sure the destination is sufficiently pre-allocated.
// as dir_dumper::n_bytes() may return a huge value.
// Written data can be read with from_raw(void*)
void* dir_dumper::to_raw(void* dst) const noexcept {
    if (!dst) return nullptr;
    // dir_tag, name length, basename
    *reinterpret_cast<value_type*>(dst) = category_tag::dir_tag;
    dst = reinterpret_cast<value_type*>(dst) + 1;
    uint16_t name_len = static_cast<uint16_t>(_filename.size());
    *reinterpret_cast<uint16_t*>(dst) = name_len;
    dst = reinterpret_cast<uint16_t*>(dst) + 1;
    ::memcpy(dst, _filename.c_str(), name_len * sizeof(value_type));
    dst = reinterpret_cast<value_type*>(dst) + name_len;

    // Sub files, sub dirs, and dir_pop_tag
    dst = std::copy(_sub_data.cbegin(), _sub_data.cend(), 
                    reinterpret_cast<std::byte*>(dst));
    for (const dir_dumper* pdir : _sub_dirs)
        dst = pdir->to_raw(dst);
    *reinterpret_cast<value_type*>(dst) = orie::dir_pop_tag;
    return reinterpret_cast<value_type*>(dst) + 1;
}

void dir_dumper::compact() {
    // places raw data of both dirs and files in _sub_data
    size_t to_add = 0;
    for (const dir_dumper* pdir : _sub_dirs)
        to_add += pdir->n_bytes();
    size_t old_sz = _sub_data.size();
    _sub_data.insert(_sub_data.cend(), to_add, std::byte());
    // Old end pointer
    std::byte* dst = _sub_data.data() + old_sz;
    for (const dir_dumper* pdir : _sub_dirs)
        dst = static_cast<std::byte*>(pdir->to_raw(dst));
    assert(dst == _sub_data.size() + _sub_data.data());
    clear(2);
}

size_t dir_dumper::n_bytes() const noexcept {
    size_t res = sizeof(uint16_t) + sizeof(category_tag) +
                 sizeof(value_type) * _filename.size() + sizeof(value_type);
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
