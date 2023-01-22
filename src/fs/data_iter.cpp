#include <orient/fs/data_iter.hpp>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <algorithm>

#define _category *reinterpret_cast<const category_tag*>(_viewing)
#define _name_len *reinterpret_cast<const uint16_t*>(          \
        _viewing + sizeof(category_tag))
#define _name_begin reinterpret_cast<const char_t*>(        \
        _viewing + sizeof(category_tag) + sizeof(uint16_t))

namespace orie {

fs_data_record::fs_data_record(dmp::file_mem_chunk* fsdb) noexcept
    : _dataref(fsdb), _cur_pos(0), _viewing(nullptr)
    , _cur_chunk(0), _is_viewing(false) { }

fs_data_record::fs_data_record(const fs_data_record& rhs) noexcept
    // Copied record does NOT acquire a filesystem database view!
    : _dataref(rhs._dataref), _cur_pos(rhs._cur_pos), _viewing(nullptr)
    , _cur_chunk(rhs._cur_chunk), _is_viewing(false) { }

fs_data_record::~fs_data_record() noexcept {
    if (_is_viewing)
        finish_visit();
}

void fs_data_record::start_visit() {
    if (_is_viewing)
        return;
    _viewing = _dataref->start_visit(_cur_chunk, _cur_pos);
    _is_viewing = true;
}
void fs_data_record::finish_visit() noexcept {
    if (!_is_viewing)
        return;
    _dataref->finish_visit();
    _is_viewing = false;
}

ptrdiff_t fs_data_record::increment() {
    assert(_is_viewing);
    ptrdiff_t push_count = _category == dir_tag ? 1 : 0;
    size_t to_inc =  sizeof(category_tag) + _name_len * sizeof(char_t) +
                     sizeof(uint16_t); // Tag, name and name length
    _cur_pos += to_inc;
    _viewing += to_inc;

    // Use while loop in case of multiple dir exit
pop_dirs:
    while (_category == dir_pop_tag) {
        _cur_pos += sizeof(category_tag);
        _viewing += sizeof(category_tag);
        --push_count;
    }
    // A chunk will only end after dir_pop_tag
    if (_category == next_chunk_tag) {
        _dataref->finish_visit();
        ++_cur_chunk;
        _cur_pos = 0;
        _viewing = _dataref->start_visit(_cur_chunk, 0);
        goto pop_dirs;
    }

    assert(push_count < 2);
    return push_count;
}

sv_t fs_data_record::file_name_view() const noexcept {
    assert(_is_viewing);
    return sv_t(_name_begin, _name_len);
}

size_t fs_data_record::file_name_len() const noexcept {
    assert(_is_viewing);
    return size_t(_name_len);
}

category_tag fs_data_record::file_type() const noexcept {
    assert(_is_viewing);
    return _category;
}

#undef _name_begin
#undef _name_len
#undef _category

fs_data_iter::fs_data_iter(dmp::file_mem_chunk* fsdb)
    : _cur_record(fsdb), _push_count(1)
    , _recur(has_recur_::enable), _tag(dir_tag)
{
    if (!fsdb) { _push_count = 0; return; }
    ++*this;
    _push_count = 1;
}

const str_t& fs_data_iter::path() const {
    if (_opt_fullpath.has_value())
        return _opt_fullpath.value();
    assert(_cur_record.is_visiting());
    return _opt_fullpath.emplace(_prefix + str_t(_cur_record.file_name_view()));
}

fs_data_iter& fs_data_iter::operator++() {
    if (!_cur_record.is_visiting())
        _cur_record.start_visit();
    _opt_fullpath.reset(); _opt_stat.reset();
    if (_push_count == 0)
        throw std::out_of_range("Incrementing end fs_data_iter");
        
    // Alternative: tmp.start_visit() at +14 line
    fs_data_record tmp(_cur_record);
    ptrdiff_t pushed = _cur_record.increment();
    while (_recur != has_recur_::enable && pushed > 0)
        pushed += _cur_record.increment();
    _tag = _cur_record.file_type();

    if (pushed + static_cast<ptrdiff_t>(_push_count) < 0) {
        _push_count = 0;
        close_fsdb_view();
        return *this;
    }
    else _push_count += pushed;

    if (pushed == 1) {
        _sub_recs.push_back(tmp);   
        tmp.start_visit();
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
    static const fs_data_record dummy(nullptr);
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
    _opt_fullpath.emplace(_prefix.substr(0, _prefix.size() - 1));
    _opt_stat.reset();
    _cur_record = _sub_recs.back();
    _sub_recs.pop_back();
    _prefix.erase(
        _prefix.find_last_of(orie::separator, _prefix.size() - 2) + 1
    );
    return *this;
}

fs_data_iter &fs_data_iter::change_root(sv_t start_path) {
    // Make sure neither has extra slash
    while (!start_path.empty() && start_path.back() == orie::separator)
        start_path.remove_suffix(1);
    sv_t pref_view(_prefix);
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
        pref_view = sv_t(_prefix);
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
    fs_data_record rec = _cur_record;
    rec.start_visit();
    if (rec.file_type() != category_tag::dir_tag)
        return true;
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
    _fetch_stat();
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
    return _push_count;
    // return std::count(_prefix.begin(), _prefix.end(), orie::separator);
}

// Copy Move Ctor Assignment
fs_data_iter::fs_data_iter(const fs_data_iter& rhs)
    : _cur_record(rhs._cur_record), _sub_recs(rhs._sub_recs)
    , _push_count(rhs._push_count), _prefix(rhs._prefix)
    , _recur(rhs._recur), _tag(rhs._tag)
    , _opt_fullpath(rhs._opt_fullpath), _opt_stat(rhs._opt_stat)
{
    if (_push_count != 0 && !_opt_fullpath.has_value())
        _opt_fullpath.emplace(_prefix + str_t(rhs._cur_record.file_name_view()));
}

fs_data_iter::fs_data_iter(fs_data_iter&& rhs) noexcept
    : _cur_record(rhs._cur_record), _sub_recs(std::move(rhs._sub_recs))
    , _push_count(std::move(rhs._push_count)), _prefix(std::move(rhs._prefix))
    , _recur(std::move(rhs._recur)), _tag(std::move(rhs._tag))
    , _opt_fullpath(std::move(rhs._opt_fullpath))
    , _opt_stat(std::move(rhs._opt_stat)) 
{ 
    if (_push_count != 0 && !_opt_fullpath.has_value())
        _opt_fullpath.emplace(_prefix + str_t(rhs._cur_record.file_name_view()));
}

fs_data_iter& fs_data_iter::operator=(const fs_data_iter& rhs) {
    if (&rhs != this) {
        this->~fs_data_iter();
        new (this) fs_data_iter(rhs);
    }
    return *this;
}

fs_data_iter& fs_data_iter::operator=(fs_data_iter&& rhs) noexcept {
    if (&rhs != this) {
        this->~fs_data_iter();
        new (this) fs_data_iter(std::move(rhs));
    }
    return *this;
}

fs_data_record& fs_data_record::operator=(const fs_data_record& rhs) noexcept {
    if (&rhs != this) {
        this->~fs_data_record();
        new (this) fs_data_record(rhs);
    }
    return *this;
}

}
