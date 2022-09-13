#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <orient/util/charconv_t.hpp>
#include <algorithm>
extern "C" {
#include <pwd.h>
#include <grp.h>
}

namespace orie {
namespace pred_tree {

std::pair<uint64_t, uint64_t>
num_node::__num_consume(sv_t numstr) const {
    // Is it a file?
    uint64_t __res = __path_to_num(numstr);
    if (__res != ~uint64_t()) {
        // As if the entire filename is a number
        numstr = sv_t(); 
    } else {
        const char_t* __beg = numstr.data(),
            *__end = __beg + numstr.size(),
            *__numend = orie::from_char_t(__beg, __end, __res);
        if (__end != __numend && 
        // Permit a unit succeeding the number for -size
            (__end != __numend + 1 || stamp::SIZE != _stm))
            throw not_a_number(numstr);
    }

    // Number done, now the unit
    if (_stm == stamp::SIZE) {
        if (numstr.empty())
            // 512B block by default
            return std::make_pair(__res, 512);
        uint64_t __unit = 1;
        switch (numstr.back()) {
        case 'G': 
            __unit <<= 10; [[fallthrough]];
        case 'M':
            __unit <<= 10; [[fallthrough]];
        case 'k':
            __unit <<= 1; [[fallthrough]];
        case 'b':
            __unit <<= 9; [[fallthrough]];
        case 'c': return std::make_pair(__res, __unit);
        default:
            throw not_a_number(numstr);
        }
    }

    else if (_stm == stamp::AMIN || _stm == stamp::MMIN ||
             _stm == stamp::CMIN)
        // Specify by Minute
        return std::make_pair(__res, 60); 
    else if (_stm == stamp::ATIME || _stm == stamp::MTIME ||
             _stm == stamp::CTIME)
        // Specify by Day
        return std::make_pair(__res, 86400); 
    return std::make_pair(__res, 1);
}

uint64_t num_node::__path_to_num(sv_t path) const noexcept {
    stat_t stbuf;
    if (orie::stat(str_t(path).c_str(), &stbuf) != 0)
        return ~uint64_t();

    time_t now_clock = ::time(nullptr);
    switch (_stm) {
    case stamp::ATIME: 
        return (now_clock - stbuf.st_atime) / 86400;
    case stamp::MTIME: 
        return (now_clock - stbuf.st_mtime) / 86400;
    case stamp::CTIME: 
        return (now_clock - stbuf.st_ctime) / 86400;
    case stamp::AMIN:
        return (now_clock - stbuf.st_atime) / 60;
    case stamp::MMIN:
        return (now_clock - stbuf.st_mtime) / 60;
    case stamp::CMIN:
        return (now_clock - stbuf.st_ctime) / 60;
    case stamp::UID: return stbuf.st_uid;
    case stamp::GID: return stbuf.st_ino;
    case stamp::INODE: return stbuf.st_ino; 
    case stamp::DEPTH:
        return std::count(path.begin(), path.end(), orie::separator);
        // size_t cnt = 0; 
        // for (const char_t c : path)
        //     if (c == orie::separator)
        //         ++cnt;
        // return cnt;
    case stamp::SIZE:
        return stbuf.st_size >> 9;
    }
    std::terminate(); // Unreachable
}

bool num_node::_num_apply(uint64_t n) const noexcept {
    // `find` -size matching is weird. Number being matched is rounded up.
    // Whereas others match [n, n+1), -size matches (n-1, n], e.g.
    // -size 2M matches anything between (1, 2] MiB
    // -mtime 2 matches anything modified between (2, 3] days.
    if (_stm == stamp::SIZE)
        n = n + _unit - 1;
    n /= _unit;

    switch (_comp) {
    case compar::EQUAL: return n == _targ;
    case compar::GE: return n > _targ;
    case compar::LE: return n < _targ;
    }
    std::terminate(); // Unreachable
}

num_node::num_node(stamp stm, compar cmp)
    : _targ(~uint64_t()), _unit(1), _stm(stm), _comp(cmp) { } 
// there was something inside the bracket which later found to be useless :)

bool num_node::apply_blocked(fs_data_iter& it) {
    if (_targ == ~uint64_t())
        throw uninitialized_node(NATIVE_SV("-num"));

    switch(_stm) {
    case stamp::ATIME: 
    case stamp::AMIN:
        return _num_apply((::time(nullptr) - it.atime()));
    case stamp::MTIME: 
    case stamp::MMIN:
        return _num_apply((::time(nullptr) - it.mtime()));
    case stamp::CTIME:
    case stamp::CMIN:
        return _num_apply((::time(nullptr) - it.ctime()));
    case stamp::UID: return _num_apply(it.uid());
    case stamp::GID: return _num_apply(it.gid());
    case stamp::INODE: return _num_apply(it.inode());
    case stamp::DEPTH: return _num_apply(it.depth());
    case stamp::SIZE: return _num_apply(it.file_size());
    }
    return false;
}

bool num_node::next_param(sv_t param) {
    if (_targ != ~uint64_t())
        return false;
    // Throw at null argument like `find`
    if (param.empty())
        throw invalid_param_name(NATIVE_SV("null parameter"), 
                                 NATIVE_SV("-num"));

    if (param.front() == '+') {
        param.remove_prefix(1);
        _comp = compar::GE;
    } else if (param.front() == '-') {
        param.remove_prefix(1);
        _comp = compar::LE;
    }
    if (param.empty())
        throw invalid_param_name(NATIVE_SV("a single '+' or '-'"), 
                                 NATIVE_SV("-num"));

    std::tie(_targ, _unit) = __num_consume(param);
    return true;
}

bool empty_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() == dir_tag) {
        fs_data_record rec = it.record();
        // Non-empty dirs would descend into itself when
        // incremented, resulting in a positive return value.
        return rec.increment() <= 0;
    }
    return it.file_size() == 0;
}

bool access_node::apply_blocked(fs_data_iter& it) {
    if (_access_test_mode == 0)
        throw uninitialized_node(NATIVE_SV("-access"));
    return ::access(it.path().c_str(), _access_test_mode) == 0;
}

bool access_node::next_param(sv_t param) {
    if (param.substr(0, 2) == NATIVE_SV("--")) {
        if (param == NATIVE_SV("--readable"))
            _access_test_mode |= R_OK;
        else if (param == NATIVE_SV("--writable"))
            _access_test_mode |= W_OK;
        else if (param == NATIVE_SV("--executable"))
            _access_test_mode |= X_OK;
        else throw invalid_param_name(param, NATIVE_SV("-access"));
        return true;
    }
    return false;
}

double perm_node::success_rate() const noexcept {
    // Each bit reduce 10% success rate.
    // EXACT has harsher conditions so each bit reduces 15%.
    double prob =  ::pow(_comp == compar::EXACT_SET ? 0.9 : 0.85,
        static_cast<double>(::__builtin_popcount(_targ)));
    // The more bit-ones in ANY_SET, the more likely a file would match.
    return _comp == compar::ANY_SET ? 1 - prob : prob;
} 

bool perm_node::apply_blocked(fs_data_iter& it) {
    if (_targ == mode_t())
        throw uninitialized_node(NATIVE_SV("-perm"));

    // All perm bits and nothing else; -rwsrwsrwt
    mode_t perm_bits = it.mode() & 07777;
    switch (_comp) {
    case compar::ALL_SET: // -perm -000 matches anything
        return (perm_bits & _targ) == _targ;
    case compar::ANY_SET: // -perm /000 matches nothing
        return (perm_bits & _targ) != 0;
    case compar::EXACT_SET:
        return perm_bits == _targ;
    }
    std::terminate(); // Unreachable
}

bool perm_node::next_param(sv_t param) {
    if (_targ != ~mode_t() || param.empty())
        return false;

    // Handle the first character
    if (param.front() == '/')  {
        _comp = compar::ANY_SET;
        param.remove_prefix(1);
    }
    else if (param.front() == '-') {
        _comp = compar::ALL_SET;
        param.remove_prefix(1);
    }

    // Is it in octal form?
    const char_t* __beg = param.data(),
        *__end = __beg + param.size(),
        *__numend = orie::from_char_t(__beg, __end, _targ, 8);
    if (__numend == __end) // `param` is in octal form
        return true;

    // param is in symbolic form
    mode_t _effect_mask = 0;
    _targ = 0;
    for (char_t ch : param) {
        switch (ch) {
        case 'u': 
        // Later 'rwxst' characters before ',' affect user
            _effect_mask |= (S_IRWXU | S_ISUID); break;
        case 'g':
            _effect_mask |= (S_IRWXG | S_ISGID); break;
        case 'o': // Sticky bit has no macro?
            _effect_mask |= (S_IRWXO | 01000); break;
        case 'r':
            _targ |= _effect_mask & (S_IRUSR | S_IRGRP | S_IROTH); break;
        case 'w':
            _targ |= _effect_mask & (S_IWUSR | S_IWGRP | S_IWOTH); break;
        case 'x':
            _targ |= _effect_mask & (S_IXUSR | S_IXGRP | S_IXOTH); break;
        case 's': case 't':
            _targ |= _effect_mask & (S_ISUID | S_ISGID | 01000); break;
        case ',': // ',' marks a new rule set, so reset the ugo mask.
            _effect_mask = 0;
        // All other chars are ignored, to reduce 
        // special cases and if statements.
        }
    }
    return true;
} 

bool username_node::apply_blocked(fs_data_iter& it) {
    if (_targ == ~uid_t())
        throw uninitialized_node("-username");
    return _is_group ?
        it.gid() == _targ : it.uid() == _targ;
}

bool username_node::next_param(sv_t param) {
    if (_targ != ~uid_t())
        return false;

    const char_t* __beg = param.data(),
        *__end = __beg + param.size(),
        *__numend = orie::from_char_t(__beg, __end, _targ);
    if (__numend == __end) // `param` is in numeric form
        return true;

    // Parse the username or groupname
    char name_buf[256];
    param = param.substr(0, 255);
    ::memcpy(name_buf, param.data(), param.size());
    name_buf[param.size()] = '\0';

    if (_is_group) {
        ::group* group = ::getgrnam(name_buf);
        if (group == nullptr) 
            throw not_a_number(param);
        _targ = group->gr_gid;
        return true;
    } else {
        ::passwd* pw = ::getpwnam(name_buf);
        if (pw == nullptr) 
            throw not_a_number(param);
        _targ = pw->pw_uid;
        return true;
    }
}

bool baduser_node::apply_blocked(fs_data_iter& it) {
    uid_t __id = _is_group ? it.gid() : it.uid();
    auto found = std::find_if(
        _recent_query.begin(), _recent_query.end(),
        [__id] (auto& p) { return __id == p.first; }
    );
    if (found != _recent_query.end())
        return found->second;

    std::lock_guard __lk(_getid_mut);
    bool invalid = _is_group ?
        ::getgrgid(__id) == nullptr :
        ::getpwuid(__id) == nullptr;
    _recent_query[_last_at] = std::make_pair(__id, invalid);
    ++_last_at; _last_at &= 15;
    return invalid;
}

#ifdef ORIE_NEED_SELINUX
bool selcontext_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool selcontext_node::next_param(sv_t param) {
    return false;
}
#endif

}
}
