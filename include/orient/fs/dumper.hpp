#pragma once
#include "predef.hpp"
#include <vector>
#include <string>
#include <string_view>

namespace orie {
namespace dmp {

class dir_dumper;
class link_dumper;

class file_dumper {
public:
    using value_type = orie::char_t;
    using string_type = std::basic_string<value_type>;
    using strview_type = std::basic_string_view<value_type>;

/** @brief Find the n-th parent of the node, or nullptr if *this has no parent.
 * @note A very large depth will result in stored root being returned.
 * @note Therefore obtain root node with parent(~unsigned()) */
    dir_dumper* parent(unsigned depth = 1) const noexcept;
    unsigned depth(const dir_dumper* relative_to = nullptr) const noexcept;
    string_type path(unsigned depth) const;
    const string_type& file_name() const noexcept { return filename; }
    virtual size_t n_bytes() const noexcept;

    virtual const std::vector<dir_dumper*>& sub_dirs(bool read_link) const noexcept;
    virtual const std::vector<file_dumper*>& sub_files(bool read_link) const noexcept;
    std::vector<dir_dumper*>& dir_data(bool read_link) noexcept {
        return const_cast<std::vector<dir_dumper*>&>(sub_dirs(read_link));
    }
    std::vector<file_dumper*>& file_data(bool read_link) noexcept {
        return const_cast<std::vector<file_dumper*>&>(sub_files(read_link));
    }

    virtual file_dumper* add_file(const strview_type&) {return nullptr;}
    virtual dir_dumper* add_dir(const strview_type&, time_t) {return nullptr;}
    virtual link_dumper* add_link(const strview_type&, const strview_type&) {return nullptr;}

    virtual bool up_to_date(time_t) const noexcept {return true;}
    virtual file_dumper* update_link_node(dir_dumper* = nullptr, bool = false) {return nullptr;}
    virtual void set_ignored(bool) noexcept {}
    // TODO: dot and dotdot
    virtual file_dumper* visit_relative(const string_type& s, bool = true)
        {return s.empty() ? this : nullptr;}
    virtual file_dumper* visit_one(const string_type& s, bool = true)
        {return s.empty() ? this : nullptr;}
    // TODO: parent()
    virtual file_dumper* visit_full(const string_type& s, bool = true)
        {return s.empty() ? this : nullptr;}
    virtual void clear(int = 3) noexcept {}
    virtual ~file_dumper() noexcept;
protected:
    friend class dir_dumper;

    string_type filename;
    dir_dumper* parent_dir;
    virtual const void* from_raw(const void* raw_src) noexcept;
    virtual void* to_raw(void* raw_dst) const noexcept;

    // TODO: Parent pushback
    file_dumper(dir_dumper* parent = nullptr);
    file_dumper(const string_type& fname, dir_dumper* parent);
};

class dir_dumper : public file_dumper {
    bool valid = false;
    bool is_ignored = false;
    time_t last_write;
    std::vector<file_dumper*> my_files;
    std::vector<dir_dumper*> my_dirs;
    size_t old_size = 0;
protected:
    void from_fs_impl(value_type*, value_type*) noexcept;
public:
    void from_fs();
    void set_ignored(bool enabled = true) noexcept override {is_ignored = enabled;}
    bool ignored() const noexcept {return is_ignored;}
    file_dumper* update_link_node(dir_dumper* = nullptr, bool = false) override;
    // 1 files 2 directories 3 all
    void clear(int type = 3) noexcept override;

    const std::vector<dir_dumper*>& sub_dirs(bool) const noexcept override {
        return my_dirs;
    }
    const std::vector<file_dumper*>& sub_files(bool) const noexcept override {
        return my_files;
    }
    bool up_to_date(time_t t) const noexcept override;
    time_t last_write_time() const noexcept {return last_write;}

    file_dumper* add_file(const strview_type& name) override;
    dir_dumper* add_dir(const strview_type& name, time_t t) override;
    link_dumper* add_link(const strview_type& name, const strview_type& link_to) override;

    file_dumper* visit_relative(const string_type& rela_path, bool add_on_fail) override;
    file_dumper* visit_one(const string_type& file_name, bool add_on_fail) override;
    file_dumper *visit_full(const string_type& full_path, bool add_on_fail) override;

    size_t n_bytes() const noexcept override;
    const void* from_raw(const void* raw_src) noexcept override;
/** @brief Writes n_bytes() of raw bytes to dst.
 * @return the one-past-end pointer of written data.
 * @retval Not null only if dst is not null.
 * @warning Make sure the destination is sufficiently pre-allocated.
 * as @code dir_node::n_bytes() @endcode may return a huge value.
 * @note Written data can be read with from_raw(void*) */
    void* to_raw(void* raw_dst) const noexcept override;

    dir_dumper(const string_type& fname, time_t write, dir_dumper* parent);
    dir_dumper(const dir_dumper& rhs) = delete;
    dir_dumper(dir_dumper&& rhs) noexcept;
    dir_dumper& operator=(const dir_dumper& rhs) = delete;
    dir_dumper& operator=(dir_dumper&& rhs) noexcept;
    ~dir_dumper() noexcept;
};

class link_dumper : public file_dumper {
    file_dumper* link_to = nullptr;
    string_type link_to_name;

    const void* from_raw(const void* src) noexcept override;
    void* to_raw(void* dst) const noexcept override;
    link_dumper(const string_type& fname,
        const string_type& link_to, dir_dumper* parent);
    friend class dir_dumper;
public:
    file_dumper* update_link_node(dir_dumper* = nullptr, bool = false) override;
    file_dumper* visit_relative(const string_type& rela_path, bool force = true) override;
    file_dumper* visit_one(const string_type& file_name, bool force = true) override;
    file_dumper* visit_full(const string_type& full_path, bool force = true) override;
    const std::vector<dir_dumper*>& sub_dirs(bool read_link) const noexcept override;
    const std::vector<file_dumper*>& sub_files(bool read_link) const noexcept override;

    void set_ignored(bool enabled = true) noexcept override;
    file_dumper* add_file(const strview_type& name) override;
    dir_dumper* add_dir(const strview_type& name, time_t t) override;
    link_dumper* add_link(const strview_type& name,
        const strview_type& link_to) override;
    bool up_to_date(time_t t) const noexcept override;

    size_t n_bytes() const noexcept override;

    void clear(int type = 3) noexcept override;
    ~link_dumper() noexcept override = default;
};

} // namespace dmp
typedef dmp::dir_dumper dumper;
} // namespace orie
