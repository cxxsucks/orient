#include <orient/fs/data_iter.hpp>
#include <orient/fs/trigram.hpp>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <algorithm>

#define _category *reinterpret_cast<const category_tag*>(_viewing)
#define _name_len *reinterpret_cast<const uint16_t*>(           \
        _viewing + sizeof(category_tag))
#define _name_begin reinterpret_cast<const char_t*>(            \
        _viewing + sizeof(category_tag) + sizeof(uint16_t))

#define _par_len *reinterpret_cast<const uint16_t*>(_viewing)
#define _par_name_begin reinterpret_cast<const char_t*>(        \
        _viewing + sizeof(uint16_t))

namespace orie {

#ifdef __GNUC_STDC_INLINE__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
// Default ctor creates an invalid record and the other fields are unused
fs_data_record::fs_data_record(dmp::dumper* dumper) noexcept
    : _dumper(dumper), _cur_chunk(~uint32_t()), _cur_batch(~uint32_t())
    , _is_viewing(false) { }

fs_data_record::fs_data_record(const fs_data_record& rhs) noexcept
    // Copied record does NOT acquire a filesystem database view!
    : _dumper(rhs._dumper), _cur_chunk(rhs._cur_chunk), _cur_batch(rhs._cur_batch)
    , _viewing(nullptr), _cur_pos(rhs._cur_pos)
    , _in_batch_pos(rhs._in_batch_pos), _is_viewing(false) { }
#ifdef __GNUC_STDC_INLINE__
#pragma GCC diagnostic pop
#endif

fs_data_record::~fs_data_record() noexcept { finish_visit(); }

sv_t fs_data_record::change_batch(size_t batch) noexcept {
    if (__unlikely(_dumper == nullptr))
        return sv_t();
    finish_visit();

    if (batch >= _dumper->batch_count()) {
        _cur_chunk = ~uint32_t();
        return sv_t();
    }
    // TODO: forward index files larger than 32bit limit?
    _cur_pos = _dumper->in_chunk_pos_of_batch(batch);
    _cur_chunk = _dumper->chunk_of_batch(batch);
    start_visit();

    assert(*reinterpret_cast<const category_tag*>(_viewing) == next_group_tag);
    ++_viewing; ++_cur_pos;
    sv_t res(_par_name_begin, _par_len);
    size_t toadd = res.size() * sizeof(char_t) + sizeof(uint16_t);
    _cur_pos += toadd;
    _viewing += toadd;
    _in_batch_pos = 0;
    _cur_batch = batch;
    return res;
}

void fs_data_record::start_visit() {
    if (_is_viewing)
        return;
    _viewing = _dumper->start_visit(_cur_chunk, _cur_pos);
    _is_viewing = true;
}
void fs_data_record::finish_visit() noexcept {
    if (!_is_viewing)
        return;
    _dumper->finish_visit();
    _is_viewing = false;
}

ptrdiff_t fs_data_record::increment() {
    assert(_is_viewing);
    ptrdiff_t push_count = _category == dir_tag ? 1 : 0;
    size_t to_inc = sizeof(category_tag) + _name_len * sizeof(char_t) +
                    sizeof(uint16_t); // Tag, name and name length
    _cur_pos += to_inc;
    _viewing += to_inc;

    // Use while loop in case of multiple dir exit
pop_dirs:
    while (__unlikely(_category == dir_pop_tag)) {
        _cur_pos += sizeof(category_tag);
        _viewing += sizeof(category_tag);
        --push_count;
    }
    // A chunk will only end after dir_pop_tag
    if (__unlikely(_category == next_chunk_tag)) {
        _dumper->finish_visit();
        ++_cur_chunk;
        _cur_pos = 0;
        _viewing = _dumper->start_visit(_cur_chunk, 0);
        goto pop_dirs;
    }

    assert(push_count < 2);
    ++_in_batch_pos;
    if (__unlikely(_in_batch_pos == _dumper->nfile_in_batch)) {
        assert(*reinterpret_cast<const category_tag*>(_viewing) ==
               next_group_tag);
        ++_viewing; ++_cur_pos;
        size_t toadd = size_t(_par_len) * sizeof(char_t) + sizeof(uint16_t);
        _cur_pos += toadd;
        _viewing += toadd;
        _in_batch_pos = 0;
        ++_cur_batch;
    }
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

fs_data_iter::fs_data_iter(dmp::dumper* dumper)
    : _cur_record(dumper), _push_count(1)
    , _recur(iter_mode::enable), _tag(dir_tag)
{
    if (!dumper) { _push_count = 0; return; }
    [[maybe_unused]] sv_t _empty = _cur_record.change_batch(0);
    assert(_empty.empty());
    ++*this;
    // `_prefix` always have an extra slash at its back
    _root_path_len = static_cast<uint32_t>(_prefix.size() - 1);
    _root_depth = std::count(_prefix.cbegin(), _prefix.cend(), separator) - 1;
    _push_count = 1;
}

const str_t& fs_data_iter::path() const {
    if (_opt_fullpath.has_value())
        return _opt_fullpath.value();
    assert(_cur_record.is_visiting());
    return _opt_fullpath.emplace(_prefix + str_t(_cur_record.file_name_view()));
}

fs_data_iter& fs_data_iter::operator++() {
    if (__unlikely(!_cur_record.is_visiting()))
        _cur_record.start_visit();
    _opt_fullpath.reset(); _opt_stat.reset();
    if (_push_count == 0)
        throw std::out_of_range("Incrementing end fs_data_iter");
        
    fs_data_record prev_rec(_cur_record);
    ptrdiff_t pushed = _cur_record.increment();
    while (__unlikely(_recur != iter_mode::enable) && pushed > 0)
        pushed += _cur_record.increment();
    _tag = _cur_record.file_type();

    if (__unlikely(pushed + static_cast<ptrdiff_t>(_push_count) < 0)) {
        _push_count = 0;
        _cur_record.finish_visit();
        return *this;
    }
    else _push_count += pushed;

    if (__unlikely(pushed == 1)) {
        prev_rec.start_visit();
        (_prefix += prev_rec.file_name_view()) += orie::separator;
    }
    while (__unlikely(pushed < 0)) {
        _prefix.erase(
            _prefix.find_last_of(
                orie::separator, _prefix.size() - 2
            ) + 1
        );
        ++pushed;
    }

    if (__unlikely(_recur == iter_mode::temp_disable))
        _recur = iter_mode::enable;
    return *this;
}

fs_data_iter& fs_data_iter::updir() {
    _opt_fullpath.emplace(_prefix.c_str(), _prefix.size() - 1);
    _prefix.erase(
        _prefix.find_last_of(orie::separator, _prefix.size() - 2) + 1
    );
    _cur_record = fs_data_record(_cur_record.dumper());
    _tag = dir_tag;
    _opt_stat.reset();
    return *this;
}

void fs_data_iter::change_batch(size_t batch_at) noexcept {
    // Both views contain no ending separator
    sv_t root_view = sv_t(_prefix).substr(0, _root_path_len);
    sv_t dest = _cur_record.change_batch(batch_at);

    if (batch_at == 0) {
        // Data iters at the beginning of data (0th batch) by definition
        // has a depth of 0 that may be mistakenly treated as end
        // of data who also has a depth of 0. Escape it by an increment.
        _prefix.clear();
        ++*this;
        _push_count = 1;
        return;
    } 

    // if (!dest.starts_with(root_view))
    if (dest.substr(0, root_view.size()) != root_view ||
        batch_at >= _cur_record.dumper()->batch_count())
        _push_count = 0;
    else {
        // It works. Don't touch.
        _push_count = std::count(dest.begin(), dest.end(), separator)
                    - _root_depth + 1;  // Why the +1 is needed?
        (_prefix = dest).push_back(separator);
        _tag = _cur_record.file_type();
    }
}

void fs_data_iter::change_batch(dmp::trigram_query& qry) {
    uint32_t batch_at;
    while ((batch_at = qry.next_batch_possible()) < _cur_record.at_batch())
        ;
    change_batch(batch_at);
}

fs_data_iter &fs_data_iter::change_root(sv_t root_new) {
    // Make sure neither has extra slash
    while (!root_new.empty() && root_new.back() == orie::separator)
        root_new.remove_suffix(1);
    sv_t pref_view(_prefix);
    if (!pref_view.empty()) // pref_view always has slash
		pref_view.remove_suffix(1);

    // Current directory or changing root of end iterator
    if (pref_view == root_new || _push_count == 0)
        return *this; // Do not change.
    if (root_new.substr(0, pref_view.size()) != pref_view)
        return (*this = end());

    bool recur_old = (_recur == iter_mode::enable);
    set_recursive(true);
    // TODO: Handle paths like /abcdef/gh where basename has no trigrams
    sv_t basename = root_new.substr(root_new.find_last_of(separator) + 1);

    static dmp::trigram_query chroot_qry;
    static std::mutex chroot_qry_lck;
    std::unique_lock lck(chroot_qry_lck);
    _cur_record.dumper()->to_query_of_this_index(chroot_qry);
    chroot_qry.reset_strstr_needle(basename, false);

    if (chroot_qry.trigram_size()) {
        while (_push_count != 0) {
            change_batch(chroot_qry);
            for (size_t i = 0; i < _cur_record.dumper()->nfile_in_batch + 2; i++) {
                if (_push_count == 0)
                    return *this;
                ++*this;
                pref_view = sv_t(_prefix);
                // Remove Extra Slash. 
                if (__likely(!pref_view.empty()))
                    pref_view.remove_suffix(1);
                if (root_new == pref_view)
                    goto done;
            }
        }
            
    } else {
        lck.unlock(); // Just iteration
        do {
            ++*this;
            pref_view = sv_t(_prefix);
            // Remove Extra Slash. 
            if (__likely(!pref_view.empty()))
                pref_view.remove_suffix(1);
            if (root_new == pref_view)
                goto done;
        } while (_push_count != 0);
    }
    return *this; // End reached

done:
    _push_count = 1;
    set_recursive(recur_old);
    (_prefix = root_new).push_back(separator);
    _root_path_len = root_new.size();
    _root_depth = std::count(root_new.begin(), root_new.end(), separator);
    return *this;
}

bool fs_data_iter::empty_dir() const noexcept {
    if (!_cur_record.valid())
        return false;
    if (_tag != category_tag::dir_tag)
        return true;
    fs_data_record rec = _cur_record;
    // If a dir is not empty, it would descend into itself 
    // after incrementing, resulting a positive return.
    rec.start_visit();
    return rec.increment() <= 0;
}

fs_data_iter fs_data_iter::current_dir_iter() const {
    if (!_cur_record.valid()) {
        fs_data_iter res(_cur_record.dumper(), path());
        res.set_recursive(false);
        return res;
    }

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

sv_t fs_data_iter::basename() const {
    if (_cur_record.is_visiting())
        return _cur_record.file_name_view();
    sv_t res(_opt_fullpath.value());
    return res.substr(_opt_fullpath.value().find_last_of(separator) + 1);
}

int fs_data_iter::_fetch_stat() const noexcept {
    if (_opt_stat.has_value())
        return _opt_stat.value().st_size == ~::off_t() ? -10 : 0;
    _opt_stat.emplace();
    int ret_stat = orie::stat(path().c_str(), &_opt_stat.value());

    if (__unlikely(ret_stat != 0))
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

// Copy Move Ctor Assignment
fs_data_iter::fs_data_iter(const fs_data_iter& rhs)
    : _cur_record(rhs._cur_record) , _push_count(rhs._push_count)
    , _prefix(rhs._prefix), _root_path_len(rhs._root_path_len)
    , _root_depth(rhs._root_depth), _recur(rhs._recur), _tag(rhs._tag)
    , _opt_fullpath(rhs._opt_fullpath), _opt_stat(rhs._opt_stat)
{
    if (_push_count != 0 && !_opt_fullpath.has_value())
        _opt_fullpath.emplace(_prefix + str_t(rhs.basename()));
}

fs_data_iter::fs_data_iter(fs_data_iter&& rhs) noexcept
    : _cur_record(rhs._cur_record) , _push_count(rhs._push_count)
    , _prefix(std::move(rhs._prefix)), _root_path_len(rhs._root_path_len)
    , _root_depth(rhs._root_depth), _recur(rhs._recur), _tag(rhs._tag)
    , _opt_fullpath(std::move(rhs._opt_fullpath))
    , _opt_stat(std::move(rhs._opt_stat)) 
{ 
    if (_push_count != 0 && !_opt_fullpath.has_value())
        _opt_fullpath.emplace(_prefix + str_t(rhs.basename()));
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
