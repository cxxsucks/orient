#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <orient/util/charconv_t.hpp>

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
        if (__beg = __numend)
            throw not_a_number(numstr);
    }

    // Number done, now the unit
    if (_stm == stamp::SIZE) {
        if (numstr.empty())
            // 512B block by default
            return std::make_pair(__res, 512);
        uint64_t __unit = 1;
        switch (numstr.front()) {
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
        return std::count(path.begin(), path.end(), orie::seperator);
        // size_t cnt = 0; 
        // for (const char_t c : path)
        //     if (c == orie::seperator)
        //         ++cnt;
        // return cnt;
    case stamp::SIZE:
        return stbuf.st_size >> 9;
    }
    std::terminate(); // Unreachable
}

bool num_node::_num_apply(uint64_t n) const noexcept {
    // `find` -size matching is weird.
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
}

num_node::num_node(stamp stm, compar cmp)
    : _stm(stm), _comp(cmp) { } 
// there was something inside the bracket which later found to be useless :)

bool num_node::apply_blocked(fs_data_iter& it) {
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
    if (_targ > 0)
        return false;
    // Throw at null argument like `find`
    if (param.empty())
        throw invalid_param_name(NATIVE_PATH_SV("null parameter"), 
                                 NATIVE_PATH_SV("-num"));

    if (param.front() == '+') {
        param.remove_prefix(1);
        _comp = compar::GE;
    } else if (param.front() == '-') {
        param.remove_prefix(1);
        _comp = compar::LE;
    }

    std::tie(_targ, _unit) = __num_consume(param);
    return true;
}

bool empty_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool empty_node::next_param(sv_t param) {
    return false;
}

bool access_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool access_node::next_param(sv_t param) {
    return false;
}

bool perm_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool perm_node::next_param(sv_t param) {
    return false;
} 

bool username_node::apply_blocked(fs_data_iter& it) {
    return false;
}

bool username_node::next_param(sv_t param) {
    return false;
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
