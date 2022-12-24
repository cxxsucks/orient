#pragma once
#include "predef.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <atomic>

namespace orie {
class fifo_thpool;

namespace dmp {
class dir_dumper;

class dir_dumper {
public:
    using value_type = orie::char_t;
    using string_type = std::basic_string<value_type>;
    using strview_type = std::basic_string_view<value_type>;

private:
    bool _valid = false;
    bool _is_ignored = false;
    time_t _last_write;
    std::vector<std::byte> _sub_data;
    std::vector<dir_dumper*> _sub_dirs;
    size_t _old_size = 0;
    string_type _filename;
    dir_dumper* _parent_dir;

    void from_fs_impl(str_t& up_path, std::atomic<ptrdiff_t>& idle,
                      fifo_thpool& pool) noexcept;
    // readdir(3) suggests that all apps shall handle DT_UNKNOWN when iterating dirs
    // Handling DT_UNKNOWN is NOT TESTED! Won't be called on most fs.
    static char_t __handle_unknown_dtype(const char_t* fullpath) noexcept;

public:
    void from_fs(fifo_thpool& pool, bool multithreaded = false);
    void set_ignored(bool enabled = true) noexcept  {_is_ignored = enabled;}
    bool ignored() const noexcept {return _is_ignored;}
    // 1 files 2 directories 3 all
    void clear(int type = 3) noexcept ;

    bool up_to_date(time_t t) const noexcept;
    time_t last_write_time() const noexcept {return _last_write;}

    dir_dumper* visit_relative_dir(const string_type& rela_path);
    dir_dumper* visit_child_dir(const string_type& file_name);
    dir_dumper* visit_dir(const string_type& file_name);

    size_t n_bytes() const noexcept ;
    const void* from_raw(const void* raw_src) noexcept ;
/** @brief Writes n_bytes() of raw bytes to dst.
 * @return the one-past-end pointer of written data.
 * @retval Not null only if dst is not null.
 * @warning Make sure the destination is sufficiently pre-allocated.
 * as @code dir_node::n_bytes() @endcode may return a huge value.
 * @note Written data can be read with from_raw(void*) */
    void* to_raw(void* raw_dst) const noexcept ;

    const str_t& filename() const noexcept { return _filename; }
    dir_dumper* parent(unsigned depth = 1) const noexcept;
    unsigned depth(const dir_dumper* relative_to = nullptr) const noexcept;
    string_type path(unsigned depth) const;

    dir_dumper(const string_type& fname, time_t write, dir_dumper* parent);
    dir_dumper() : dir_dumper(str_t(), 0, nullptr) {}
    dir_dumper(const dir_dumper& rhs) = delete;
    dir_dumper(dir_dumper&& rhs) noexcept;
    dir_dumper& operator=(const dir_dumper& rhs) = delete;
    dir_dumper& operator=(dir_dumper&& rhs) noexcept;
    ~dir_dumper() noexcept;
};

#ifdef _WIN32
/** @class fs_dumper
 * @brief Special handler of Windows drive letters by creating a "fake"
 * root directory whose children are drives. 
 * @note Because of this, all Windows paths are prepended with an extra '\\'. */
class fs_dumper {
    std::vector<dir_dumper> _drives;

public:
    size_t n_bytes() const noexcept;
    const void* from_raw(const void* raw_src) noexcept;
    void* to_raw(void* raw_dst) const noexcept;
    dir_dumper* visit_dir(const str_t& file_name);
};

#else // Not Windows. Filesystem root is a normal directory.
typedef dir_dumper fs_dumper;
#endif

} // namespace dmp
typedef dmp::fs_dumper dumper;
} // namespace orie
