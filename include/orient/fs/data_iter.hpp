#pragma once
#include "predef.hpp"
#include "dumper.hpp"
#include <optional>

namespace orie {
namespace dmp { class trigram_query; }


/*! @class fs_data_record
 * @brief A non-owning, immutable reference to a file entry
 * in the data obtained from @c file_dumper 
 * Requires no heap allocation, but has less utility compared to
 * @c fs_data_iter */
class fs_data_record {
private:
    dmp::dumper* _dumper;

    // Current chunk in Forward Index, 0~4095
    uint32_t _cur_chunk;
    // Current batch in Inverted Index
    uint32_t _cur_batch;

    const std::byte* _viewing;
    // Current position in current chunk of Forward Index
    size_t _cur_pos;

    // Current position in the batch of Inverted Index
    uint8_t _in_batch_pos;
    bool _is_viewing;

public:
    //! @brief Open views to filesystem database 
    //! making all methods return valid data and @c increment work
    //! @note @c finish_visit auto called on dtor after calling it
    void start_visit();
    //! @brief Close views to filesystem database
    //! @warning Invalidates @c increment and @c file_* methods
    //! and existing string view from @c file_name_view
    void finish_visit() noexcept;
    bool is_visiting() const noexcept { return _is_viewing; }

    //! @brief Move to the next entry. @c start_visit must be called beforehand
    //! @warning Undefined if @a file_type returns unknown_tag (end reached)
    //! @return Change of current depth inside the filesystem.
    //! @throw std::out_of_range when encountering incomplete data
    ptrdiff_t increment();

    //! @brief Move to the @p batch th batch of the index dumped by @p dumper
    //! @note Calls @c start_visit inside and enables viewing
    sv_t change_batch(size_t batch) noexcept;

    //! @brief Get the file name of current entry
    //! @c start_visit must be called beforehand 
    //! @warning Returned string view is @b not null-terminated
    //! @warning Undefined if @a file_type returns unknown_tag
    sv_t file_name_view() const noexcept;
    //! @brief Get the file name length of current entry
    //! @c start_visit must be called beforehand 
    size_t file_name_len() const noexcept;

    // Simple getters
    uint8_t in_batch_pos() const noexcept { return _in_batch_pos; };
    uint32_t at_batch() const noexcept { return _cur_batch; };
    dmp::dumper* dumper() const noexcept { return _dumper; };
    bool valid() const noexcept { return _cur_chunk < 4096; }

    //! @brief Get the type of the file.
    //! @retval unknown_tag End of data reached; no other functions shall be called.
    category_tag file_type() const noexcept;

    // Compare equality of two records.
    bool operator==(const fs_data_record& rhs) const noexcept {
        return rhs._cur_pos == _cur_pos && rhs._cur_chunk == _cur_chunk &&
               rhs._dumper == _dumper;
    }
    // Compare inequality of two records.
    bool operator!=(const fs_data_record& rhs) const noexcept {
        return rhs._cur_pos != _cur_pos || rhs._cur_chunk != _cur_chunk ||
               rhs._dumper != _dumper;
    }
    // A record is smaller if the current position is smaller
    bool operator<(const fs_data_record& rhs) const noexcept {
        assert(_dumper == rhs._dumper);
        return _cur_chunk == rhs._cur_chunk ? _cur_pos < rhs._cur_pos
                                            : _cur_chunk < rhs._cur_chunk;
    }
    bool operator>(const fs_data_record& rhs) const noexcept {
        assert(_dumper == rhs._dumper);
        return _cur_chunk == rhs._cur_chunk ? _cur_pos > rhs._cur_pos
                                            : _cur_chunk > rhs._cur_chunk;
    }

    // Must call `change_batch` before calling `increment`
    fs_data_record(dmp::dumper* dumper = nullptr) noexcept;
    // Copied record does NOT obtain a view
    fs_data_record(const fs_data_record& rhs) noexcept;
    fs_data_record& operator=(const fs_data_record& res) noexcept;
    ~fs_data_record() noexcept;
    // Move ctor IS copy ctor
    // fs_data_record(fs_data_record&& rhs) noexcept;
};

/*! @class fs_data_iter
 * @brief A iterator iterating over filesystem data obtained by @c dir_dumper.
 * It is designed for efficiency rather than useability, requiring the least 
 * filesystem visit and heap allocation possible. In exchange, public functions 
 * simply return raw types.
 * @note Still, heap allocation exists. Try pass by reference when possible. 
 * @note Its @a value_type and is itself, which is somewhat absurd. 
 * Will change in a distant release. */
class fs_data_iter {
public:
    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = fs_data_iter;
    using difference_type = std::ptrdiff_t;
    using reference = fs_data_iter&;
    using pointer = fs_data_iter*;

    enum iter_mode : uint8_t {
        enable, all_disable, temp_disable
    };

private:
    fs_data_record _cur_record;
    size_t _push_count;
    str_t _prefix;
    uint32_t _root_path_len, _root_depth;
    iter_mode _recur;

    // Cached file type, done in operator++
    category_tag _tag;
    // Cached fullpath
    mutable std::optional<str_t> _opt_fullpath;
    // Cached stat
    mutable std::optional<orie::stat_t> _opt_stat;
    int _fetch_stat() const noexcept;

public:
    // Disable view to database. Re-established on ++ call.
    void close_index_view() noexcept {
        if (_cur_record.is_visiting()) {
            path(); // Call path() to construct full path string
            _cur_record.finish_visit();
        }
    }

    // Move *this one level up in filesystem hierchary AND drops it
    // in a "special state" where `stat`-related functions are valid,
    // whereas others are not. Returns *this.
    fs_data_iter& updir();

    //! @brief Move the iterator to the given path.
    //! @param start_path The path to move to. Must be an unvisited
    //! entry of the data being iterated.
    fs_data_iter& change_root(sv_t start_path);

    // Get a non-recursive iterator over the parent directory of current entry.
    fs_data_iter current_dir_iter() const;

    //! @brief Move to the @p batch th batch of the index
    //! @note Calls @c start_visit inside and enables viewing
    void change_batch(size_t batch) noexcept;
    //! @brief Move to the @b next batch containing the trigram
    //! @param qry The trigram query already has a pattern set 
    //! @note Becomes @a end if none could be found
    void change_batch(dmp::trigram_query& qry);
    // Set this iterator to "normal" directory iteration mode, if not, then
    // set whether the iterator recursively delves into child directories. 
    void set_recursive(bool enable) noexcept {
        _recur = enable ? iter_mode::enable : iter_mode::all_disable;
    }
    // Set this iterator to "normal" directory iteration mode, if not, then
    // skip traversal into current directory. Has no effect if current entry is
    // not a directory.
    void disable_pending_recursion(bool enable = true) noexcept {
        if (_recur != iter_mode::all_disable && file_type() == orie::dir_tag)
            _recur = enable ? iter_mode::temp_disable : iter_mode::enable;
    }

    //! @brief Get a reference to current full path string.
    //! @warning The reference is valid until next @a operator++ call.
    const str_t& path() const;
    //! @brief Get the base name current path.
    //! @note Same as @code record().basename() @endcode when iter mode on
    sv_t basename() const;

    // Get a reference to parent path string.
    // The reference is valid as long as the iterator is valid
    // and ALWAYS refers to parent path of the iterator at that time.
    const str_t &parent_path() const noexcept { return _prefix; }

    //! @brief Whether the directory is empty. @c true for non-dirs;
    //! @warning Undefined if @a file_type returns unknown_tag (end reached)
    bool empty_dir() const noexcept;

    /* Convenience functions for getting the fields of `stat` struct */
    // Numeric group id. Always 0 on Windows
    gid_t gid() const noexcept;
    // Numeric user id. Always 0 on Windows
    uid_t uid() const noexcept;
    // Inode number of the file
    ino_t inode() const noexcept;
    // (Type and) Permission of the file. 
    // `file_type` is better for getting type.
    mode_t mode() const noexcept;
    // Last modified time. Returns cached value for directories
    // unless precise stat is acquired, e.g., calling gid() atime() prior.
    time_t mtime() const noexcept;
    // Last accessed time of current entry.
    time_t atime() const noexcept;
    // Last changed time of current entry. Creation time instead on Windows.
    time_t ctime() const noexcept;
    // Amount of disk space in bytes of current entry.
    off_t file_size() const noexcept;
#ifndef _WIN32
    // Preferrable IO block size
    blksize_t io_block_size() const noexcept;
#endif // !_WIN32

    // Simple getters
    const fs_data_record& record() const noexcept { return _cur_record; }
    const arr2d_reader& invidx_reader() const noexcept {
        return _cur_record.dumper()->_invidx;
    }
    size_t depth() const noexcept { return _push_count; }
    //! @brief File type of current entry.
    //! @see fs_data_record::file_type()
    category_tag file_type() const noexcept { return _tag; }

    // Simply return a copy of *this.
    fs_data_iter begin() const { return *this; }
    // An invalid fs_data_iter. All end iterators are equal.
    static fs_data_iter end() { return fs_data_iter(); }
    // Simply return a reference of *this. May change in the distant future.
    reference operator*() noexcept { return *this; }
    // Simply return a pointer to *this. May change in the distant future.
    pointer operator->() noexcept { return this; }

    // Increment the iterator.
    fs_data_iter& operator++();
    fs_data_iter operator++(int) {
        fs_data_iter tmp(*this); ++*this; return tmp; 
    }
    // Compares two iterators. All end fs_data_iters are equal.
    bool operator==(const fs_data_iter& rhs) const noexcept;
    // Compares inequality of two iterators. All end fs_data_iters are equal.
    bool operator!=(const fs_data_iter& rhs) const noexcept {
        return !(*this == rhs);
    }
    bool operator<(const fs_data_iter &rhs) const noexcept {
        return _cur_record < rhs._cur_record;
    }
    bool operator>(const fs_data_iter &rhs) const noexcept {
        return _cur_record > rhs._cur_record;
    }

    // Construct using pointer to stored filesystem data and internal position
    // An end iterator will be constructed if the record is not a directory.
    fs_data_iter(dmp::dumper* dumper = nullptr);
    // Construct using pointer to stored filesystem data and starting path.
    // An end iterator will be constructed if the path is not part of the data
    // or is not a directory.
    fs_data_iter(dmp::dumper* dumper, sv_t start_path)
        : fs_data_iter(dumper) { change_root(start_path); }

    // Copy the content of rhs.
    // Copied or moved iterator has iteration mode OFF!!!
    fs_data_iter(const fs_data_iter& rhs);
    // Copy the content of rhs. Cached stat will be copied, but path strings will not.
    // Copied or moved iterator has iteration mode OFF!!!
    fs_data_iter& operator=(const fs_data_iter& rhs);
    // Copied or moved iterator has iteration mode OFF!!!
    fs_data_iter(fs_data_iter&& rhs) noexcept;
    // Copied or moved iterator has iteration mode OFF!!!
    fs_data_iter& operator=(fs_data_iter&& rhs) noexcept;
    ~fs_data_iter() noexcept = default;
};

}
