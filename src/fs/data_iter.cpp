#include <orient/fs/data_iter.hpp>
#include <stdexcept>
#include <cstring>
#include <algorithm>

#define _category *reinterpret_cast<const category_tag*>(      \
        static_cast<const uint8_t*>(_viewing) + _cur_pos)
#define _name_len *reinterpret_cast<const uint16_t*>(          \
        static_cast<const uint8_t*>(_viewing)                   \
            + _cur_pos + sizeof(char_type))
#define _name_begin reinterpret_cast<const char_type*>(        \
        static_cast<const uint8_t*>(_viewing)                   \
            + _cur_pos + sizeof(char_type) + sizeof(uint16_t))
#define _dir_mtime *reinterpret_cast<const time_t*>(           \
        _name_begin + _name_len)

namespace orie {

sv_t fs_data_record::file_name_view() const noexcept {
    return sv_t(_name_begin, _name_len);
}

size_t fs_data_record::file_name_len() const noexcept {
    return static_cast<size_t>(_name_len);
}

ptrdiff_t fs_data_record::increment() noexcept {
    category_tag prev_cate = _category;
    _cur_pos += sizeof(char_t) + _name_len * sizeof(char_t) +
                sizeof(uint16_t); // Tag, name and name length
    ptrdiff_t push_count = 0;

    if (prev_cate == category_tag::dir_tag) {
        _cur_pos += sizeof(time_t);
        ++push_count;
    }

    // Use while loop in case of multiple dir exit
    // One possible situation is multiple layers of empty dirs.
    while (_category == category_tag::dir_pop_tag) {
        _cur_pos += sizeof(char_type);
        --push_count;
    }
    if (push_count >= 2)
        std::terminate();
    return push_count;
}

fs_data_record::category_tag fs_data_record::file_type() const noexcept {
    if (_viewing == nullptr)
        return category_tag::unknown_tag;
    switch (_category) {
    case category_tag::dir_tag:
    case category_tag::file_tag:
    case category_tag::link_tag:
    case category_tag::dir_pop_tag:
        return _category;
    default:
        return category_tag::unknown_tag;
    }
}

time_t fs_data_record::dir_mtime() const noexcept{
    if (_category == category_tag::dir_tag)
        return *reinterpret_cast<const time_t*>(_name_begin + _name_len);
    return ~time_t();
}

fs_data_iter::fs_data_iter(const void* dat, size_t start) 
    : _cur_record(dat, 0), _push_count(1), _recur(has_recur_::enable) {
    if (!dat) {_push_count = 0; return;}

    while (_cur_record.pos() < start)
        ++*this;
    if (_cur_record.pos() != start || _cur_record.file_type() != category_tag::dir_tag) {
        _push_count = 0;
        return;
    }
    // prefix = file_name_view();
    ++*this;
    _push_count = 1;
}

fs_data_iter::fs_data_iter(const void* dat, strview_type st)
    : fs_data_iter(dat, 0) { change_root(st); }

fs_data_iter::fs_data_iter(const fs_data_iter& rhs) 
    : _cur_record(rhs._cur_record), _sub_recs(rhs._sub_recs)
    , _push_count(rhs._push_count), _prefix(rhs._prefix)
    , _recur(rhs._recur), _opt_stat(rhs._opt_stat) {}

fs_data_iter& fs_data_iter::operator=(const fs_data_iter& rhs) {
    if (&rhs != this) {
        this->~fs_data_iter();
        new (this) fs_data_iter(rhs);
    }
    return *this;
}

const fs_data_iter::string_type &
fs_data_iter::path() const {
    if (_opt_fullpath.has_value())
        return _opt_fullpath.value();
    return _opt_fullpath.emplace(_prefix + string_type(_cur_record.file_name_view()));
}

fs_data_iter& fs_data_iter::operator++() {
    _opt_fullpath.reset(); _opt_stat.reset();
    if (_push_count == 0)
        // No throwing or errors to make allowance for `-quit`
        return *this; 
        
    fs_data_record tmp = _cur_record;
    ptrdiff_t pushed = _cur_record.increment();
    while (_recur != has_recur_::enable && pushed > 0)
        pushed += _cur_record.increment();

    if (pushed + static_cast<ptrdiff_t>(_push_count) < 0) {
        _push_count = 0;
        return *this;
    }
    else _push_count += pushed;

    if (pushed == 1) {
        _sub_recs.push_back(tmp);   
        (_prefix += tmp.file_name_view()) += orie::separator;
    }
    while (pushed < 0) {
        _prefix.erase(
            _prefix.find_last_of(
                orie::separator, _prefix.size() - 2
            ) + 1
        );
        ++pushed;
        _sub_recs.pop_back();
    }

    if (_recur == has_recur_::temp_disable)
        _recur = has_recur_::enable;
    return *this;
}

const fs_data_record &fs_data_iter::record(size_t sub) const noexcept {
    static const fs_data_record dummy(nullptr, 0);
    if (sub == 0)
        return _cur_record;
    if (sub > _sub_recs.size())
        return dummy;
    return _sub_recs.at(_sub_recs.size() - sub);
}

fs_data_iter& fs_data_iter::updir() {
    if (_sub_recs.empty())
        return (*this = end());
    if (_push_count > 1)
        --_push_count;
    _opt_fullpath.reset(); _opt_stat.reset();
    _cur_record = _sub_recs.back();
    _sub_recs.pop_back();
    _prefix.erase(
        _prefix.find_last_of(
            orie::separator, _prefix.size() - 2
        ) + 1
    );
    return *this;
}

fs_data_iter &fs_data_iter::change_root(strview_type start_path) {
    // Make sure neither has extra slash
    while (!start_path.empty() && start_path.back() == orie::separator)
        start_path.remove_suffix(1);
    strview_type pref_view = strview_type(_prefix);
    if (!pref_view.empty()) // pref_view always has slash
		pref_view.remove_suffix(1);

    // Current directory or changing root of end iterator
    if (pref_view == start_path || _push_count == 0)
        return *this; // Do not change.
    if (start_path.find(pref_view) != 0)
        return (*this = end());
    bool recur_old = (_recur == has_recur_::enable);
    set_recursive(true);

    do {
        ++*this;
        pref_view = strview_type(_prefix);
		// Remove Extra Slash. 
        if (!pref_view.empty()) 
            pref_view.remove_suffix(1);
    } while (start_path != pref_view && _push_count != 0);
    
    if (_push_count != 0)
        _push_count = 1;
    
    set_recursive(recur_old);
   return *this;
}

bool fs_data_iter::empty_dir() const noexcept {
    if (_cur_record.file_type() != category_tag::dir_tag)
        return true;
    fs_data_record rec = record();
    // If a dir is not empty, it would descend into itself 
    // after incrementing, resulting a positive return.
    return rec.increment() <= 0;
}

fs_data_iter fs_data_iter::current_dir_iter() const {
    if (empty_dir())
        return end();
    fs_data_iter res(*this);
    res.set_recursive(true);
    ++res;
    res.set_recursive(false);
    res._push_count = 1;

    return res;
}

bool fs_data_iter::operator==(const fs_data_iter& rhs) const noexcept {
    if (_push_count == 0 && rhs._push_count == 0)
        return true;
    return _push_count == rhs._push_count && _cur_record == rhs._cur_record;
}

int fs_data_iter::_fetch_stat() const noexcept {
    if (_opt_stat.has_value())
        return _opt_stat.value().st_size == ~::off_t() ? -10 : 0;
    _opt_stat.emplace();
    int ret_stat = orie::stat(path().c_str(), &_opt_stat.value());

    if (ret_stat != 0)
        ::memset(&_opt_stat.value(), 0xff, sizeof(struct stat));
    return ret_stat;
}

gid_t fs_data_iter::gid() const noexcept{
    _fetch_stat();
    return _opt_stat.value().st_gid;
}

uid_t fs_data_iter::uid() const noexcept{
    _fetch_stat();
    return _opt_stat.value().st_uid;
}

time_t fs_data_iter::mtime() const noexcept{
    if (!_opt_stat.has_value()) {
        time_t _ = _cur_record.dir_mtime();
        if (_ != ~time_t())
            return _;
        _fetch_stat();
    }
    return _opt_stat.value().st_mtime;
}

time_t fs_data_iter::atime() const noexcept{
    _fetch_stat();
    return _opt_stat.value().st_atime;
}

time_t fs_data_iter:: ctime() const noexcept{
    _fetch_stat();
    return _opt_stat.value().st_ctime;
}

off_t fs_data_iter::file_size() const noexcept{
    _fetch_stat();
    return _opt_stat.value().st_size;
}

ino_t fs_data_iter::inode() const noexcept {
    _fetch_stat();
    return _opt_stat.value().st_ino;
}

mode_t fs_data_iter::mode() const noexcept {
    _fetch_stat();
    return _opt_stat.value().st_mode;
}

#ifndef _WIN32
blksize_t fs_data_iter::io_block_size() const noexcept {
    _fetch_stat();
    return _opt_stat.value().st_blksize;
}
#endif // !_WIN32

size_t fs_data_iter::depth() const noexcept {
    // size_t cnt = 0;
    // for (const char_type c : prefix)
    //     if (c == orie::separator)
    //         ++cnt;
    // return cnt;
    return std::count(_prefix.begin(), _prefix.end(), orie::separator);
}

}
#undef _name_begin
#undef _name_len
#undef _category
