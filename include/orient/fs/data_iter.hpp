#pragma once
#include "predef.hpp"
#include "dumper.hpp"
#include <optional>

namespace orie {

/*! @class fs_data_record
 * @brief A non-owning, immutable reference to a file entry
 * in the data obtained from @c file_dumper 
 * Requires no heap allocation, but has less utility compared to
 * @c fs_data_iter */
class fs_data_record {
public:
    using char_type = orie::char_t;
    using category_tag = orie::category_tag;
    using strview_type = std::basic_string_view<char_type>;

private:
    dmp::file_mem_chunk* _dataref;
    size_t _cur_pos;
    uint8_t _cur_chunk;

    // Cached from dumped data
    category_tag _tag;
    str_t _basename;

public:
    //! @brief Move to the next entry 
    //! @warning Undefined if @a file_type returns unknown_tag (end reached)
    //! @return Change of current depth inside the filesystem.
    ptrdiff_t increment() noexcept;

    //! @brief Get the file name of current entry
    //! @warning Undefined if @a file_type returns unknown_tag
    const str_t& basename() const noexcept { return _basename; }
    size_t file_name_len() const noexcept { return _basename.size(); }

    //! @brief Get the type of the file.
    //! @retval unknown_tag End of data reached; no other functions shall be called.
    category_tag file_type() const noexcept { return _tag; }

    // Compare equality of two records.
    bool operator==(const fs_data_record& rhs) const noexcept {
        return rhs._cur_pos == _cur_pos && rhs._cur_chunk == _cur_chunk &&
               rhs._dataref == _dataref;
    }
    // Compare inequality of two records.
    bool operator!=(const fs_data_record& rhs) const noexcept {
        return rhs._cur_pos != _cur_pos || rhs._cur_chunk != _cur_chunk ||
               rhs._dataref != _dataref;
    }

    // Construct a record. This simply assigns the two parameters
    fs_data_record(dmp::file_mem_chunk* fsdb = nullptr) noexcept;
    ~fs_data_record() noexcept = default;
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

private:
    enum class has_recur_ : uint8_t {
        enable, all_disable, temp_disable 
    };

    fs_data_record _cur_record;
    std::vector<fs_data_record> _sub_recs;
    size_t _push_count;
    str_t _prefix;
    has_recur_ _recur;

    mutable std::optional<str_t> _opt_fullpath;
    mutable std::optional<orie::stat_t> _opt_stat;

    int _fetch_stat() const noexcept;
public:
    // Move *this one level up in filesystem hierchary. Return *this.
    // Invalidates *this (== end) if already at top level.
    fs_data_iter& updir();

    //! @brief Move the iterator to the given path.
    //! @param start_path The path to move to. Must be an unvisited
    //! entry of the data being iterated.
    fs_data_iter& change_root(sv_t start_path);

    // Get a non-recursive iterator over the parent directory of current entry.
    fs_data_iter current_dir_iter() const;

    //! @brief Get the internal data record of the current entry or its parents.
    //! @param sub Get the record of the sub-th parent. 0 returns current.
    //! @retval end() On @p sub is greater than current depth.
    const fs_data_record& record(size_t sub = 0) const noexcept;

    // Set whether the iterator recursively delves into child directories. 
    void set_recursive(bool enable) noexcept {
        _recur = enable ? has_recur_::enable : has_recur_::all_disable;
    }
    // Skip traversal into current directory. Has no effect if current entry is
    // not a directory.
    void disable_pending_recursion(bool enable = true) noexcept {
        if (_recur != has_recur_::all_disable && file_type() == orie::dir_tag)
            _recur = enable ? has_recur_::temp_disable : has_recur_::enable;
    }

    //! @brief Get a reference to current full path string.
    //! @warning The reference is valid until next @a operator++ call.
    const str_t& path() const;
    //! @brief Get the base name current path
    //! @note Same as @code record().basename() @endcode
    const str_t& basename() const { return record().basename(); }
    // Get a reference to parent path string.
    // The reference is valid as long as the iterator is valid
    // and ALWAYS refers to parent path of the iterator at that time.
    const str_t& parent_path() const noexcept {return _prefix;}
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

    size_t depth() const noexcept;
    //! @brief File type of current entry.
    //! @see fs_data_record::file_type()
    category_tag file_type() const noexcept { return _cur_record.file_type(); }

    // Simply return a copy of *this.
    fs_data_iter begin() const {return *this;}
    // An invalid fs_data_iter. All end iterators are equal.
    static fs_data_iter end() {return fs_data_iter();}
    // Simply return a reference of *this. May change in the distant future.
    reference operator*() noexcept {return *this;}
    // Simply return a pointer to *this. May change in the distant future.
    pointer operator->() noexcept {return this;}

    // Increment the iterator.
    fs_data_iter& operator++();
    // Compares two iterators. All end fs_data_iters are equal.
    bool operator==(const fs_data_iter& rhs) const noexcept;
    fs_data_iter operator++(int) {
        fs_data_iter tmp(*this); ++*this; return tmp; 
    }
    // Compares inequality of two iterators. All end fs_data_iters are equal.
    bool operator!=(const fs_data_iter& rhs) const noexcept {
        return !(*this == rhs);
    }

    // Construct using pointer to stored filesystem data and internal position
    // An end iterator will be constructed if the record is not a directory.
    fs_data_iter(dmp::file_mem_chunk* fsdb = nullptr);
    // Construct using pointer to stored filesystem data and starting path.
    // An end iterator will be constructed if the path is not part of the data
    // or is not a directory.
    fs_data_iter(dmp::file_mem_chunk* fsdb, sv_t start_path)
        : fs_data_iter(fsdb) { change_root(start_path); }
    // Copy the content of rhs. Cached stat will be copied, but path strings will not.
    fs_data_iter(const fs_data_iter& rhs);
    // Copy the content of rhs. Cached stat will be copied, but path strings will not.
    fs_data_iter& operator=(const fs_data_iter& rhs);
    fs_data_iter(fs_data_iter&& rhs) = default;
    fs_data_iter& operator=(fs_data_iter&& rhs) = default;
    ~fs_data_iter() noexcept = default;
};

}
