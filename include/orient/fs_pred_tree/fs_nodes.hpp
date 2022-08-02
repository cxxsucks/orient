#pragma once
#include <orient/pred_tree/node.hpp>
#include <orient/fs/data_iter.hpp>

#include <fstream>
#include <string>
#include <mutex>

namespace orie {
#ifdef _WIN32
    #define PCRE2_CODE_UNIT_WIDTH 16
    using sv_t = std::wstring_view;
    using str_t = std::wstring;
    using char_t = wchar_t;
#else
    #define PCRE2_CODE_UNIT_WIDTH 8
    using sv_t = std::string_view;
    using str_t = std::string;
    using char_t = char;
#endif
}

extern "C" {
#include <pcre2.h>
}

namespace orie {
namespace pred_tree {

using fs_node = node<fs_data_iter, sv_t>;
using fs_mod_node = mod_base_node<fs_data_iter, sv_t>;

// PRED: -name -iname -strstr -istrstr
// ARG: --icase --full STR
class strstr_glob_node;
// PRED: -regex -iregex -bregex -ibregex ARG: --icase --full STR
class regex_node;
// PRED: -type
// TODO: Currently only file, directory, link
class type_node;
// PRED: -size -[amc][(time)(min)(newer)] -[ug]id -ino -depth
// ARGS: --margin NUM -- [+-]NUM_OR_COMPARE_FILE[cbkMGT]
class num_node;
// PRED: -empty
struct empty_node;
// PRED: -prune
struct prune_node;
// PRED: -print -fprint
// TODO: -printf -fprintf -ls -fls
class print_node;
// TODO: -exec -execdir -ok -okdir
class exec_node;
// TODO: -delete
class del_node;

// Content Matching Nodes
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

class strstr_glob_node : public fs_node {
    std::array<char_t, 252 / sizeof(char_t)> _pattern;
    bool _is_glob;
    bool _is_full;
    bool _is_exact;
    bool _is_icase;
    // Total 256B

public:
    double success_rate() const noexcept override { return 0.1; }
    double cost() const noexcept override {
        return _is_glob ? 2.5e-7 : 1e-7;
    }
    fs_node* clone() const override {
        return new strstr_glob_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    strstr_glob_node(bool is_glob, bool full = false, 
                     bool exact = false, bool icase = false) 
        : _is_glob(is_glob) , _is_full(full) 
        , _is_exact(exact), _is_icase(icase) { }
};

class regex_node : public fs_node {
    // Need a custom deleter
    std::shared_ptr<pcre2_code> _re;
    std::shared_ptr<pcre2_match_data> _match_dat;
    bool _is_full = false;
    bool _is_exact = false;
    bool _is_icase = false;

public:
    double success_rate() const noexcept override { return 0.1; }
    double cost() const noexcept override { return 1e-6; }
    fs_node* clone() const override {
        return new regex_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    regex_node(bool full = false, bool exact = false, bool icase = false) 
        : _match_dat(pcre2_match_data_create(1, nullptr), pcre2_match_data_free)
        , _is_full(full) , _is_exact(exact), _is_icase(icase) { }
};

class type_node : public fs_node {
    std::array<char_t, 8> _permitted;

public:
    double success_rate() const noexcept override;
    double cost() const noexcept override { return 2e-8; }
    fs_node* clone() const override {
        return new type_node(*this);
    }

    bool apply_blocked(fs_data_iter& it) override; 
    bool next_param(sv_t param) override;

    type_node(sv_t param);
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
    bool next_param(sv_t param) override;
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
    print_node(char_t split) : _format(str_t(FS_TEXT("%p")) + split) {}
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

}
}
