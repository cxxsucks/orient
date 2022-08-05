#pragma once
#include <orient/pred_tree/node.hpp>
#include <orient/fs/data_iter.hpp>

#include <fstream>
#include <string>
#include <mutex>

extern "C" {
#include <pcre2.h>
}

namespace orie {
namespace pred_tree {

using fs_node = node<fs_data_iter, sv_t>;
using fs_mod_node = mod_base_node<fs_data_iter, sv_t>;

/* Predicates that require no syscalls; info read directly from fs dump */
// PRED: -name -iname -strstr -istrstr -path -ipath -lname
// ARG: --icase --readlink STR
class strstr_glob_node;
// PRED: -regex -iregex -bregex -ibregex ARG: --icase --full STR
class regex_node;
// PRED: -type
// TODO: Currently only file, directory, link
class type_node;
// PRED: -prune
struct prune_node;

/* Predicates that require only one or two syscalls, 
 * doing no change to actual filesystem, including stat(2), access(2),
 * getpwnam(3), getgrnam(3) (which read /etc/passwd), getfilecon(8) */
// PRED: -size -[amc][(time)(min)] -[ug]id -ino -depth
// ALSO_COMARE_PRED: -[amc]newer -samefile
// ARGS: --margin NUM -- [+-]NUM_OR_COMPARE_FILE[cbkMGT]
class num_node;
// PRED: -empty
struct empty_node;
// PRED: -access -readable -writable -executable
// ARG: --readable --writable --executable
class access_node;
// PRED: -perm
// ARG: /- followed by octal or sympolic permission bit
class perm_node;
// PRED: -user -group -nogroup -nouser
class username_node;

#ifdef ORIE_NEED_SELINUX
extern "C" {
#include <selinux/selinux.h>
}
// PRED: -context
class selcontext_node;
#endif

/* These predicates (actions) May change current filesystem,
 * through outputing, command execution and file deletion. */
// PRED: -print -fprint
// TODO: -printf -fprintf -ls -fls
class print_node;
// TODO: -exec -execdir -ok -okdir
class exec_node;
// TODO: -delete
class del_node;

/* Content Matching Predicates. They will be asynchorously
 * executed because of their "slow" nature, and thus invalidating -prune */
// PRED: -content-strstr ARG: --icase --block --binary STR
class content_strstr_node;
// PRED: -content-regex ARG: --icase --block --binary STR
class content_regex_node;

// MODIF: -downdir
class downdir_node;
// MODIF: -updir
class updir_node;
// MODIF: -pathmod
struct pathmod_node;
// TODO: Action Modifiers
// -prune-if -delete-if -[f]print[f]-if -exec-if

class strstr_glob_node : public fs_node {
    std::array<char_t, 252 / sizeof(char_t)> _pattern;
    bool _is_glob;
    bool _is_full;
    bool _is_lname;
    bool _is_icase;
    // Total 256B

public:
    double success_rate() const noexcept override { return 0.05; }
    double cost() const noexcept override {
        return _is_glob ? 2.5e-7 : 1e-7;
    }
    fs_node* clone() const override {
        return new strstr_glob_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    strstr_glob_node(bool is_glob, bool full = false, 
                     bool lname = false, bool icase = false) 
        : _is_glob(is_glob) , _is_full(full) 
        , _is_lname(lname), _is_icase(icase) { }
};

class regex_node : public fs_node {
    // Need a custom deleter
    std::shared_ptr<pcre2_code> _re;
    std::shared_ptr<pcre2_match_data> _match_dat;
    bool _is_full;
    bool _is_exact;
    bool _is_lname;
    bool _is_icase;

public:
    double success_rate() const noexcept override { return 0.05; }
    double cost() const noexcept override { return 1e-6; }
    fs_node* clone() const override {
        return new regex_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    regex_node(bool full = false, bool exact = false,
               bool lname = false, bool icase = false) 
        : _match_dat(pcre2_match_data_create(1, nullptr), pcre2_match_data_free)
        , _is_full(full) , _is_exact(exact), _is_lname(lname), _is_icase(icase) { }
};

class type_node : public fs_node {
    std::array<orie::_category_tag, 8> _permitted;

public:
    double success_rate() const noexcept override;
    double cost() const noexcept override { return 2e-8; }
    fs_node* clone() const override {
        return new type_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    type_node() { 
        ::memset(_permitted.data(), unknown_tag, sizeof(_permitted));
    };
};

class num_node : public fs_node {
public:
    enum class compar {GREATER, SMALL, EQUAL};
    enum class stamp {
        ATIME, MTIME, CTIME,
        AMIN, MMIN, CMIN,
        UID, GID, 
        SIZE, INODE, DEPTH
    };

private:
    stamp _stm;
    compar _comp;
    double _targ = 0.0;
    double _margin = 0.01;

    double __num_consume(sv_t numstr) const noexcept;
    double __path_to_num(sv_t fullpath) const noexcept;

public:
    double success_rate() const noexcept override { return 0.1; }
    double cost() const noexcept override { 
        return _stm == stamp::DEPTH ? 1e-7 : 1e-5;
    }
    fs_node* clone() const override {
        return new num_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    num_node(stamp stm, compar cmp = compar::EQUAL)
        : _stm(stm), _comp(cmp) {}
};

struct empty_node : public fs_node {
    double success_rate() const noexcept override { return 1.0; }
    double cost() const noexcept override { return 1e-5; }
    fs_node* clone() const override {
        return new empty_node(*this);
    }
    bool communicative() const noexcept override {return false;}

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;
};

struct prune_node : public fs_node {
    double success_rate() const noexcept override { return 1.0; }
    double cost() const noexcept override { return 1e-8; }
    fs_node* clone() const override {
        return new prune_node(*this);
    }
    bool communicative() const noexcept override {return false;}

    bool apply_blocked(fs_data_iter& it) override; 
};

class print_node : public fs_node {
    // Stream for -fprint
    std::shared_ptr<std::basic_ofstream<char_t>> _ofs;
    // Output mutex
    std::mutex _out_mut;
    // Format string
    str_t _format;

public:
    double success_rate() const noexcept override { return 1.0; }
    double cost() const noexcept override { return 1e-4; }
    fs_node* clone() const override {
        return new print_node(*this);
    }
    bool communicative() const noexcept override {return false;}

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    // Ctor used in -print -print0 -fprint -fprint0
    // Set _format to "%p" + split, _ofs to nullptr
    // Dest file is set by next_param.
    print_node(char_t split) : _format(str_t(NATIVE_PATH("%p")) + split) {}
    // Ctor used in -printf -fprintf
    // Dest file AND format are set by next_param.
    print_node() = default;
    print_node(const print_node& r) 
        : _ofs(r._ofs), _format(r._format) {}
    print_node& operator=(const print_node& r) {
        if (this != &r) {
            this->~print_node();
            new (this) print_node(r);
        }
        return *this;
    }
};

class exec_node : public fs_node {
    // The command containing "{}" to execute
    std::vector<str_t> _exec_cmds;
    // Filenames that have not yet get substituted in {}
    std::vector<str_t> _names_to_pass;
    // Minimum length to substitute in each {}
    // Each {} may be replaced by multiple names
    ptrdiff_t _name_min_len = 16384;
    ptrdiff_t _name_len_left = 16384;
    bool _parse_finished = false;
    // -ok variant
    bool _stdin_confirm;
    // -execdir variant
    bool _from_subdir;

public:
    double success_rate() const noexcept override { 
        // -exec COMMAND +[NUM] varient always return true,
        // whose `name_len` must be >0 in order to fit multiple names.
        return _name_min_len <= 0 ? 0.5 : 1.0;
    }
    double cost() const noexcept override { return 2e-4; }
    fs_node* clone() const override {
        return new exec_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    exec_node(bool ok, bool from_subdir) 
        : _stdin_confirm(ok), _from_subdir(from_subdir) {}
};

/**
 * @brief "Best-effort" deletion node. It attempts to delete directories
 * after scanning and possibly deleting all its children, but does not
 * guarantee that in an asynchorous context.
 * @note If all nodes are synchorous, i.e., no @a -content-*, 
 * dirs will be deleted after its children (if it is empty after that).
 * @note Would not delete non-empty directory, like @c rmdir(1)
 * @return Whether the deletion is successful for files
 */
class del_node : public fs_node {
    // Saves the directories "ready" to be deleted
    std::vector<fs_data_iter> _todel_dirs_stack;
    std::mutex _todel_mut;

public:
    double success_rate() const noexcept override { return 0.8; }
    double cost() const noexcept override { return 1e-5; }
    fs_node* clone() const override {
        return new del_node();
    }
    bool communicative() const noexcept override {return false;}

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    del_node() noexcept = default;
    // DO NOT copy the deletion stack!
    del_node(const del_node&) = delete;
    del_node& operator=(const del_node&) = delete;
    // Delete remaining dirs in stack
    ~del_node() noexcept;
};

class access_node : public fs_node {
    int _access_test_mode = 0;

public:
    double success_rate() const noexcept override { 
        double res = 1.0;
        if (_access_test_mode | R_OK) res *= 0.9;
        if (_access_test_mode | W_OK) res *= 0.6;
        if (_access_test_mode | X_OK) res *= 0.3;
        return res;
    }
    double cost() const noexcept override { return 1e-5; }
    fs_node* clone() const override {
        return new access_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    access_node(int rwx_ok) : _access_test_mode(rwx_ok) {}
};

class perm_node : public fs_node {
    mode_t _must_set = 0;
    mode_t _must_not_set = 0;

public:
    // Each bit in `must set` and `must unset` reduce 10% success rate.
    double success_rate() const noexcept override {
        return ::pow(0.9, static_cast<double>(
            ::__builtin_popcount(_must_set | (_must_not_set << 16))
        ));
    }
    double cost() const noexcept override { return 1.2e-5; }
    fs_node* clone() const override {
        return new perm_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;
};

class username_node : public fs_node {
    uid_t _targ;
    bool _is_group;
    bool _match_invalid; // For -nouser -nogroup

public:
    double success_rate() const noexcept override { return 0.4; }
    double cost() const noexcept override { return 1.2e-5; }
    fs_node* clone() const override {
        return new username_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    username_node(bool group, bool match_invalid) 
        : _targ(-1), _is_group(group), _match_invalid(match_invalid) {}
};

// CONTENT
class content_strstr_node : public fs_node {
    str_t pattern;
    bool _blocked;
    bool _allow_binary;
    bool _icase;

public:
    double success_rate() const noexcept override { return 1.0; }
    double cost() const noexcept override { return 1e-4; }
    fs_node* clone() const override {
        return new content_strstr_node(*this);
    }

    tribool_bad apply(fs_data_iter& it) override;
    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    content_strstr_node(bool block, bool bin, bool icase)
        : _blocked(block), _allow_binary(bin), _icase(icase) {}
};

class content_regex_node : public fs_node {
    std::shared_ptr<pcre2_code> _re;
    std::shared_ptr<pcre2_match_data> _match_dat;
    bool _blocked;
    bool _allow_binary;
    bool _icase;

public:
    double success_rate() const noexcept override { return 1.0; }
    double cost() const noexcept override { return 5e-4; }
    fs_node* clone() const override {
        return new content_regex_node(*this);
    }

    tribool_bad apply(fs_data_iter& it) override;
    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    content_regex_node(bool block, bool bin, bool icase)
        : _match_dat(pcre2_match_data_create(1, nullptr), pcre2_match_data_free)
        , _blocked(block), _allow_binary(bin), _icase(icase) {}
};

#ifdef ORIE_NEED_SELINUX
class selcontext_node : public fs_node {
    std::array<char_t, 256 / sizeof(char_t)> _pattern;

public:
    double success_rate() const noexcept override { return 0.2; }
    double cost() const noexcept override {
        return 1e-5;
    }
    fs_node* clone() const override {
        return new selcontext_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;
};
#endif 

// MODIFIERS
/**
 *  @brief Test whether the parent directory matches preds following it.
 * Takes No params, but can be stacked multiple times
 * @note Since files in a same directory may be passed sequentially,
 * a cache exists to save recently matched items.
 * @note Therefore in @code -updir -exec ... @endcode, the @c -exec
 * will run at @a least once and at @a most its child count times.  
 * @warning All nodes in @c -updir is executed synchorously 
 */
class updir_node : public fs_mod_node {
    // Circular array queue caching recently judged results
    std::array<std::pair<fs_data_record, bool>, 8> _last_done_q;
    size_t _last_idx;
    // Lock for the queue.
    std::mutex _last_done_mut;

    double cost() const noexcept override { return prev_cost / 8.0 + 2e-7; }
    fs_node* clone() const override {
        return new updir_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    tribool_bad apply(fs_data_iter& it) override;

    updir_node(const updir_node& r)
        : _last_done_q(r._last_done_q), _last_idx(r._last_idx) {}
    updir_node& operator=(const updir_node& r) {
        if (this != &r) {
            this->~updir_node();
            new (this) updir_node(r);
        }
        return *this;
    }
};

/**
 * @brief True if the count of children satisfying the modifier's following pred
 * it falls inside a specified range. Always false on non-directories.
 * Has two @a optional parameters.
 * @param [+-]NUM1 True if the dir has more(+), less(-) or exactly(no +-) @c NUM1.
 * @param [+-]NUM2 Same as the previous one. @c +NUM1 and @c -NUM2 specifies a range.
 * @note @c +0 is automatically specified by default.
 * @note The range is @b NOT inclusive on both sides.
 */
class downdir_node : public fs_mod_node {
    size_t _max_cnt = 0;
    size_t _min_cnt = 0;

public:
    double cost() const noexcept override { return prev_cost * 8.0 + 5e-8; }
    fs_node* clone() const override {
        return new downdir_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    tribool_bad apply(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;
};

struct pathmod_node : public fs_mod_node {
    double cost() const noexcept override { return prev_cost + 1e-7; }
    fs_node* clone() const override {
        return new pathmod_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    tribool_bad apply(fs_data_iter& it) override;
};

} // namespace pred_tree
} // namespace orie
