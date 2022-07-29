#pragma once
#include "predef.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace orie {

/*! @class fs_data_record
 * @brief A non-owning, immutable reference to a file entry
 * in the data obtained from @c file_dumper 
 * Requires no heap allocation, but has less utility compared to
 * @c fs_data_iter */
class fs_data_record {
public:
    using char_type = orie::value_type;
    using category_tag = orie::_category_tag;
    using strview_type = std::basic_string_view<char_type>;
private:
    const void* viewing;
    size_t cur_pos;

public:
    //! @brief Move to the next entry 
    //! @warning Undefined if @a file_type returns unknown_tag (end reached)
    //! @return Change of current depth inside the filesystem.
    ptrdiff_t increment() noexcept;
    //! @brief Get the file name of current entry
    //! @warning Returned string view is @b not null-terminated (end reached)
    //! @warning Undefined if @a file_type returns unknown_tag
    strview_type file_name_view() const noexcept;
    //! @brief Get the type of the file.
    //! @retval unknown_tag End of data reached; no other functions shall be called.
    //! @retval dir_tag Directory @retval link_tag Symbolic Link 
    //! @retval file_tag Any other file
    category_tag file_type() const noexcept;
    //! @brief If the current entry is a directory, return its modified time.
    //! Returns -1 otherwise.
    //! @note This strange behavior exists because only mtime of dirs is saved
    //! in database. Saving mtime of dirs made it possible to skip re-scanning dirs that
    //! remain unchanged since last scan. Saving mtime of files is meaningless.
    time_t dir_mtime() const noexcept;
    //! @brief Check whether the record has just dived into a directory.
    //! @warning Undefined if @a file_type returns unknown_tag (end reached)
    bool start_of_dir() const noexcept;

    // Get internal position.
    size_t pos() const noexcept {return cur_pos;}
    // Compare equality of two records.
    bool operator==(const fs_data_record& rhs) const noexcept {
        return rhs.viewing == viewing && rhs.cur_pos == cur_pos;
    }
    // Compare inequality of two records.
    bool operator!=(const fs_data_record& rhs) const noexcept {
        return rhs.viewing != viewing || rhs.cur_pos != cur_pos;
    }

    // Construct a record. This simply assigns the two parameters
    fs_data_record(const void* view = nullptr, size_t start_at = 0) noexcept
        : viewing(view), cur_pos(start_at) {}
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
    using char_type = orie::value_type;
    using category_tag = orie::_category_tag;
    using string_type = std::basic_string<char_type>;
    using strview_type = std::basic_string_view<char_type>;

    using iterator_category = std::input_iterator_tag;
    using iterator_concept = std::input_iterator_tag;
    using value_type = fs_data_iter;
    using difference_type = std::ptrdiff_t;
    using reference = fs_data_iter&;
    using pointer = fs_data_iter*;

    using gid_t = decltype(orie::stat_t::st_gid);
    using uid_t = decltype(orie::stat_t::st_uid);
    using ino_t = decltype(orie::stat_t::st_ino);

private:
    enum class has_recur_ : uint8_t {
        enable, all_disable, temp_disable 
    };

    fs_data_record cur_rec;
    std::vector<fs_data_record> sub_recs;
    ptrdiff_t push_count;
    string_type prefix;
    has_recur_ recur;

    mutable std::optional<string_type> opt_fullpath;
    mutable std::optional<string_type> opt_filename;
    mutable std::optional<orie::stat_t> opt_stat;

    int _fetch_stat() const noexcept;
public:
    // Whether or not to return full path when calling path()
    bool full_path_default = false;
    // Move *this one level up in filesystem hierchary. Return *this.
    // Invalidates *this (== end) if already at top level.
    fs_data_iter& updir();
    //! @brief Move the iterator to the given path.
    //! @param start_path The path to move to. Must be an unvisited
    //! entry of the data being iterated.
    fs_data_iter& visit(strview_type start_path);
    // Get a non-recursive iterator over the parent directory of current entry.
    fs_data_iter current_dir_iter() const;
    //! @brief Get the internal data record of the current entry or its parents.
    //! @param sub Get the record of the sub-th parent. 0 returns current.
    //! @retval end() On @p sub is greater than current depth.
    const fs_data_record& record(size_t sub = 0) const noexcept;

    // Set whether the iterator recursively delves into child directories. 
    void set_recursive(bool enable) noexcept {
        recur = enable ? has_recur_::enable : has_recur_::all_disable;
    }
    // Skip traversal into current directory. Has no effect if current entry is
    // not a directory.
    void disable_pending_recursion(bool enable = true) noexcept {
        if (recur != has_recur_::all_disable && file_type() == orie::dir_tag)
            recur = enable ? has_recur_::temp_disable : has_recur_::enable;
    }

    //! @brief Get a reference to current path string.
    //! @warning The reference is valid until next @a operator++ call.
    const string_type& path() const { return path(full_path_default); }
    //! @brief Get a reference to current path string.
    //! @param full Whether to return full path or just base name.
    //! @warning The reference is valid until next @a operator++ call.
    //! @note @code record().file_name_view() @endcode returns view to base name
    //! without heap allocation and is valid regardless of the iterator state.
    const string_type& path(bool full) const;
    // Get a reference to parent path string.
    // The reference is valid as long as the iterator is valid
    // and ALWAYS refers to parent path of the iterator at that time.
    const string_type& parent_path() const noexcept {return prefix;}

    // Numeric group id. Always 0 on Windows
    gid_t gid() const noexcept;
    // Numeric user id. Always 0 on Windows
    uid_t uid() const noexcept;
    // Last modified time. Returns cached value for directories
    // unless precise stat is acquired, e.g., calling gid() atime() prior.
    ::time_t mtime() const noexcept;
    // Last accessed time of current entry.
    ::time_t atime() const noexcept;
    // Last changed time of current entry. Creation time instead on Windows.
    ::time_t ctime() const noexcept;
    // Amount of disk space in bytes of current entry.
    ::off_t file_size() const noexcept;
    ino_t inode() const noexcept;
    size_t depth() const noexcept;
    //! @brief File type of current entry.
    //! @see fs_data_record::file_type()
    category_tag file_type() const noexcept { return cur_rec.file_type(); }

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
    fs_data_iter(const void* fsdb = nullptr, size_t start = 0);
    // Construct using pointer to stored filesystem data and starting path.
    // An end iterator will be constructed if the path is not part of the data
    // or is not a directory.
    fs_data_iter(const void* fsdb, strview_type start_path);
    // Copy the content of rhs. Cached stat will be copied, but path strings will not.
    fs_data_iter(const fs_data_iter& rhs);
    // Copy the content of rhs. Cached stat will be copied, but path strings will not.
    fs_data_iter& operator=(const fs_data_iter& rhs);
    fs_data_iter(fs_data_iter&& rhs) = default;
    fs_data_iter& operator=(fs_data_iter&& rhs) = default;
    ~fs_data_iter() noexcept = default;
};

}
