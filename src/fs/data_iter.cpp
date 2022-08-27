#include <orient/fs/data_iter.hpp>
#include <stdexcept>
#include <cstring>
#include <algorithm>

#define _category *reinterpret_cast<const category_tag*>(      \
        static_cast<const uint8_t*>(viewing) + cur_pos)
#define _name_len *reinterpret_cast<const uint16_t*>(          \
        static_cast<const uint8_t*>(viewing)                   \
            + cur_pos + sizeof(char_type))
#define _name_begin reinterpret_cast<const char_type*>(        \
        static_cast<const uint8_t*>(viewing)                   \
            + cur_pos + sizeof(char_type) + sizeof(uint16_t))
#define _dir_mtime *reinterpret_cast<const time_t*>(           \
        _name_begin + _name_len)

namespace orie {

fs_data_record::strview_type fs_data_record::file_name_view() const noexcept {
    return strview_type(_name_begin, _name_len);
}

ptrdiff_t fs_data_record::increment() noexcept {
    category_tag prev_cate = _category;
    cur_pos += (_name_len + 1) * sizeof(char_type) + sizeof(uint16_t);
    ptrdiff_t push_count = 0;

    if (prev_cate == category_tag::dir_tag) {
        cur_pos += sizeof(time_t);
        ++push_count;
    }
    
    if (prev_cate == category_tag::link_tag) {
        auto linkto_len = *reinterpret_cast<const uint16_t*>(
            static_cast<const uint8_t*>(viewing) + cur_pos);
        cur_pos += (linkto_len * sizeof(char_type) + sizeof(uint16_t));
        // cur_pos += sizeof(uint16_t) + *reinterpret_cast<const uint16_t*>(
            // static_cast<const uint8_t*>(viewing) + cur_pos);
    }

    while (_category == category_tag::dir_pop_tag) {
        cur_pos += sizeof(char_type);
        --push_count;
    }
    if (push_count >= 2)
        std::terminate();
    return push_count;
}

fs_data_record::category_tag fs_data_record::file_type() const noexcept {
    if (viewing == nullptr)
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
    : cur_rec(dat, 0), push_count(1), recur(has_recur_::enable) {
    if (!dat) {push_count = 0; return;}

    while (cur_rec.pos() < start)
        ++*this;
    if (cur_rec.pos() != start || cur_rec.file_type() != category_tag::dir_tag) {
        push_count = 0;
        return;
    }
    // prefix = file_name_view();
    ++*this;
    push_count = 1;
}

fs_data_iter::fs_data_iter(const void* dat, strview_type st)
    : fs_data_iter(dat, 0) { change_root(st); }

fs_data_iter::fs_data_iter(const fs_data_iter& rhs) 
    : cur_rec(rhs.cur_rec), sub_recs(rhs.sub_recs)
    , push_count(rhs.push_count), prefix(rhs.prefix)
    , recur(rhs.recur), opt_stat(rhs.opt_stat) {}

fs_data_iter& fs_data_iter::operator=(const fs_data_iter& rhs) {
    if (&rhs != this) {
        this->~fs_data_iter();
        new (this) fs_data_iter(rhs);
    }
    return *this;
}

const fs_data_iter::string_type &
fs_data_iter::path() const {
    if (opt_fullpath.has_value())
        return opt_fullpath.value();
    return opt_fullpath.emplace(prefix + string_type(cur_rec.file_name_view()));
}

fs_data_iter& fs_data_iter::operator++() {
    opt_fullpath.reset(); opt_stat.reset();
    if (push_count == 0)
        throw std::out_of_range("Incrementing End fsData Iterator");
        
    fs_data_record tmp = cur_rec;
    ptrdiff_t pushed = cur_rec.increment();
    while (recur != has_recur_::enable && pushed > 0)
        pushed += cur_rec.increment();

    if (pushed + static_cast<ptrdiff_t>(push_count) < 0) {
        push_count = 0;
        return *this;
    }
    else push_count += pushed;

    if (pushed == 1) {
        sub_recs.push_back(tmp);   
        (prefix += tmp.file_name_view()) += orie::seperator;
    }
    while (pushed < 0) {
        prefix.erase(
            prefix.find_last_of(
                orie::seperator, prefix.size() - 2
            ) + 1
        );
        ++pushed;
        sub_recs.pop_back();
    }

    if (recur == has_recur_::temp_disable)
        recur = has_recur_::enable;
    return *this;
}

const fs_data_record &fs_data_iter::record(size_t sub) const noexcept {
    static const fs_data_record dummy(nullptr, 0);
    if (sub == 0)
        return cur_rec;
    if (sub > sub_recs.size())
        return dummy;
    return sub_recs.at(sub_recs.size() - sub);
}

fs_data_iter& fs_data_iter::updir() {
    if (sub_recs.empty())
        return (*this = end());
    if (push_count > 1)
        --push_count;
    opt_fullpath.reset(); opt_stat.reset();
    cur_rec = sub_recs.back();
    sub_recs.pop_back();
    prefix.erase(
        prefix.find_last_of(
            orie::seperator, prefix.size() - 2
        ) + 1
    );
    return *this;
}

fs_data_iter &fs_data_iter::change_root(strview_type start_path) {
    // Make sure neither has extra slash
    while (!start_path.empty() && start_path.back() == orie::seperator)
        start_path.remove_suffix(1);
    strview_type pref_view = strview_type(prefix);
    pref_view.remove_suffix(1);

    if (pref_view == start_path)
        return *this; // Current directory. Do not change.
    if (start_path.find(pref_view) != 0)
        return (*this = end());
    bool recur_old = (recur == has_recur_::enable);
    set_recursive(true);

    do {
        ++*this;
        pref_view = strview_type(prefix);
        if (!pref_view.empty()) // Extra Slash
            pref_view.remove_suffix(1);
    } while (start_path != pref_view && push_count != 0);
    
    if (push_count != 0)
        push_count = 1;
    
    set_recursive(recur_old);
   return *this;
}

bool fs_data_iter::empty_dir() const noexcept {
    if (cur_rec.file_type() != category_tag::dir_tag)
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
    res.push_count = 1;

    return res;
}

bool fs_data_iter::operator==(const fs_data_iter& rhs) const noexcept {
    if (push_count == 0 && rhs.push_count == 0)
        return true;
    return push_count == rhs.push_count && cur_rec == rhs.cur_rec;
}

int fs_data_iter::_fetch_stat() const noexcept {
    if (opt_stat.has_value())
        return opt_stat.value().st_size == ~::off_t() ? -10 : 0;
    opt_stat.emplace();
    int ret_stat;
    /* if (_category == category_tag::link_tag && read_link) {
        const uint16_t* plink_len = reinterpret_cast<const uint16_t*>(
            _name_begin + _name_len);
        const char_type* plink_name_begin =
            reinterpret_cast<const char_type*>(plink_len + 1);
        char_type *plink_name_end = 
            const_cast<char_type*>(plink_name_begin + *plink_len);
        
        char_type tmp_end = *plink_name_end;
        *plink_name_end = 0;
        ret_stat = ::stat(plink_name_begin, &opt_stat.value());
        *plink_name_end = tmp_end;
    } else */ 
    ret_stat = orie::stat(path().c_str(), &opt_stat.value());

    if (ret_stat != 0)
        ::memset(&opt_stat.value(), 0xff, sizeof(struct stat));
    return ret_stat;
}

gid_t fs_data_iter::gid() const noexcept{
    _fetch_stat();
    return opt_stat.value().st_gid;
}

uid_t fs_data_iter::uid() const noexcept{
    _fetch_stat();
    return opt_stat.value().st_uid;
}

time_t fs_data_iter::mtime() const noexcept{
    if (!opt_stat.has_value()) {
        time_t _ = cur_rec.dir_mtime();
        if (_ != ~time_t())
            return _;
        _fetch_stat();
    }
    return opt_stat.value().st_mtime;
}

time_t fs_data_iter::atime() const noexcept{
    _fetch_stat();
    return opt_stat.value().st_atime;
}

time_t fs_data_iter:: ctime() const noexcept{
    _fetch_stat();
    return opt_stat.value().st_ctime;
}

off_t fs_data_iter::file_size() const noexcept{
    _fetch_stat();
    return opt_stat.value().st_size;
}

ino_t fs_data_iter::inode() const noexcept {
    _fetch_stat();
    return opt_stat.value().st_ino;
}

mode_t fs_data_iter::mode() const noexcept {
    _fetch_stat();
    return opt_stat.value().st_mode;
}

blksize_t fs_data_iter::io_block_size() const noexcept {
    _fetch_stat();
    return opt_stat.value().st_blksize;
}

size_t fs_data_iter::depth() const noexcept {
    // size_t cnt = 0;
    // for (const char_type c : prefix)
    //     if (c == orie::seperator)
    //         ++cnt;
    // return cnt;
    return std::count(prefix.begin(), prefix.end(), orie::seperator);
}

}
#undef _name_begin
#undef _name_len
#undef _category
