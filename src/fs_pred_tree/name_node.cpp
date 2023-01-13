#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <orient/fs/predef.hpp>

namespace orie {
namespace pred_tree {

glob_node::glob_node(bool full, bool lname, bool icase)
    : _is_fullpath(full), _is_lname(lname), _is_icase(icase) 
{ ::memset(_pattern.data(), 0, sizeof(_pattern)); }

bool glob_node::apply_blocked(fs_data_iter& it) {
    if (_pattern[0] == '\0')
        throw orie::pred_tree::uninitialized_node(NATIVE_SV("--name"));
    if (_is_lname && it.file_type() != link_tag)
        return false; // Not a link 

    if (_is_fullpath && !_is_lname) 
        // FNM_CASEFOLD is a GNU extension :(
        return orie::glob_match(_pattern.data(), it.path().c_str(), _is_icase);

    if (_is_lname) {
        char_t linkat_path[path_max] = {};
#ifdef _WIN32
        ssize_t re_len = orie::realpath(it.path().c_str(),
                                        linkat_path, path_max);
#else
        // readlink(2) is what find(1) uses
        ssize_t re_len = ::readlink(it.path().c_str(), linkat_path, path_max);
#endif
        if (re_len <= 0)
            return true;
        return orie::glob_match(_pattern.data(), linkat_path, _is_icase);
    } else {
        // Strings matched by fnmatch(3) must be NULL-terminated
        char_t name_buf[path_max];
        sv_t name_sv = it.basename().substr(0, path_max - 1);
        ::memcpy(name_buf, name_sv.data(), name_sv.size() * sizeof(char_t));
        name_buf[name_sv.size()] = '\0';
        return orie::glob_match(_pattern.data(), name_buf, _is_icase);
    }
    // Unreachable
}

bool glob_node::next_param(sv_t param) {
    if (_pattern[0] != '\0')
        return false;

    if (param.substr(0, 2) == NATIVE_SV("--")) {
        if (param == NATIVE_SV("--full"))
            _is_fullpath = true;
        else if (param == NATIVE_SV("--ignore-case"))
            _is_icase = true;
        else if (param == NATIVE_SV("--readlink"))
            _is_lname = true;
        else throw invalid_param_name(param, NATIVE_SV("--name"));
        return true;
    }

    if (param.size() > 252 / sizeof(char_t) - 1) {
        NATIVE_STDERR << NATIVE_PATH("Current implementation only supports "
            "a pattern of at most 252 bytes :(\n");
        param = param.substr(0, 252 / sizeof(char_t) - 1);
    }
    ::memcpy(_pattern.data(), param.data(), param.size() * sizeof(char_t));
    return true;
}

bool strstr_node::apply_blocked(fs_data_iter& it) {
    if (_pattern[0] == '\0')
        throw orie::pred_tree::uninitialized_node(NATIVE_SV("--name"));
    if (_is_lname && it.file_type() != link_tag)
        return false; // Not a symlink 

    // Get the target string_view for matching
    sv_t haystack;
    char_t linkat_path[path_max];
    if (_is_lname) {
#ifdef _WIN32
        ssize_t re_len = orie::realpath(it.path().c_str(),
                                        linkat_path, path_max);
#else
        ssize_t re_len = ::readlink(it.path().c_str(), linkat_path, path_max);
#endif
        if (re_len <= 0)
            return true;
        haystack = sv_t(linkat_path, re_len);
    } 
    else if (_is_fullpath)
        haystack = it.path();
    else haystack = it.basename();
    // And its ignore-case counterpart
    icase_sv_t icase_hs(haystack.data(), haystack.size());

    return _is_icase ?
        icase_hs.find(_pattern.data()) != sv_t::npos :
        haystack.find(_pattern.data()) != sv_t::npos;
}

bool regex_node::apply_blocked(fs_data_iter& it) {
    if (_re == nullptr)
        throw uninitialized_node(NATIVE_SV("-bregex"));
    if (_is_lname && it.file_type() != link_tag)
        return false; // Not a link 

    PCRE2_SPTR re_ptr; ssize_t re_len;
    char_t linkat_path[path_max];
    if (!_is_full && !_is_lname) {
        // Just match basename
        sv_t basename = it.record().file_name_view();
        re_ptr = reinterpret_cast<PCRE2_SPTR>(basename.data());
        re_len = basename.size();
    } else if (_is_full) {
        re_ptr = reinterpret_cast<PCRE2_SPTR>(it.path().c_str());
        re_len = it.path().size();
    } else {
        re_ptr = reinterpret_cast<PCRE2_SPTR>(linkat_path);
#ifdef _WIN32
        re_len = orie::realpath(it.path().c_str(),
                                linkat_path, path_max);
#else
        re_len = ::readlink(it.path().c_str(), linkat_path, path_max);
#endif
        if (re_len <= 0)
            return true;
    }

    return 0 < pcre2_match(
        _re.get(), re_ptr, static_cast<size_t>(re_len), 0,
        _is_exact ? PCRE2_ANCHORED | PCRE2_ENDANCHORED : 0,
        _match_dat.get(), nullptr
    );
}

bool regex_node::next_param(sv_t param) {
    if (_re != nullptr)
        return false;

    if (param.substr(0, 2) == NATIVE_SV("--")) {
        if (param == NATIVE_SV("--full"))
            _is_full = true;
        else if (param == NATIVE_SV("--exact"))
            _is_exact = true;
        else if (param == NATIVE_SV("--ignore-case"))
            _is_icase = true;
        else if (param == NATIVE_SV("--readlink"))
            _is_lname = true;
        else throw invalid_param_name(param, NATIVE_SV("-bregex"));
        return true;
    }

    int errcode; PCRE2_SIZE erroffset;
    _re.reset(pcre2_compile(
        // char -> unsigned char
        reinterpret_cast<PCRE2_SPTR>(param.data()), param.size(), 
        _is_icase ? PCRE2_CASELESS : 0,
        &errcode, &erroffset, nullptr
    ), pcre2_code_free);

    if (_re == nullptr) {
            PCRE2_UCHAR errbuf[128];
            int msg_len = pcre2_get_error_message(errcode, errbuf, 128);
            if constexpr (sizeof(PCRE2_UCHAR) == 1)
                throw std::runtime_error(reinterpret_cast<char*>(errbuf));
            else
                throw std::runtime_error(orie::xxstrcpy(
                    std::basic_string_view(errbuf, msg_len)
                ));
    }
    return true;
}

bool fuzz_node::apply_blocked(fs_data_iter& it) {
    if (!_matcher.has_value())
        throw orie::pred_tree::uninitialized_node(NATIVE_SV("-fuzz"));

    // TODO: readlink
    sv_t to_match = _is_full ? sv_t(it.path()) : it.basename();
    if (to_match.size() < _min_haystack_len)
        return false;
    return _matcher.value().similarity(to_match, _cutoff) > _cutoff;
}

bool fuzz_node::next_param(sv_t param) {
    if (_matcher.has_value())
        return false;

    if (param.substr(0, 2) == NATIVE_SV("--")) {
        if (param == NATIVE_SV("--full"))
            _is_full = true;
        else if (param == NATIVE_SV("--cutoff")) 
            _next_cutoff = true;
        else if (param == NATIVE_SV("--readlink"))
            _is_lname = true;
        else throw invalid_param_name(param, NATIVE_SV("-fuzz"));
        return true;
    }

    if (_next_cutoff) {
        uint64_t targ;
        const char_t* __beg = param.data(),
            *__end = __beg + param.size(),
            *__numend = orie::from_char_t(__beg, __end, targ);
        if (__numend != __end) 
            throw pred_tree::not_a_number(param);
        if (targ > 100)
            throw pred_tree::invalid_param_name(param, NATIVE_SV(
                "--cutoff; must in 0~99"
            ));
        _cutoff = static_cast<double>(targ);
        _next_cutoff = false;
    } else {
        _matcher.emplace(param);
        _min_haystack_len = (param.size() >> 1) + 1;
    } 
    return true;
}

double type_node::success_rate() const noexcept {
    for (size_t i = 0; i < 8; ++i)
        if (_permitted[i] == orie::file_tag)
            return 0.9;
    return 0.1;
}

bool type_node::apply_blocked(fs_data_iter& it) {
    auto ty = it.file_type();
    for (size_t i = 0; i < 8 && _permitted[i] != unknown_tag; ++i)
        if (_permitted[i] == ty)
            return true;
    return false;
}

bool type_node::next_param(sv_t param) {
    if (_permitted[0] != orie::unknown_tag)
        return false;
    size_t i = 0;
    for (char_t ch : param) {
        switch (ch) {
        case file_tag: case dir_tag: case link_tag:
        case fifo_tag: case blk_tag: case sock_tag:
        case char_tag:
            _permitted[i++] = static_cast<category_tag>(ch);
            break;
        }
        if (i >= 8) break;
    }
    if (i == 0) 
        throw invalid_param_name(param, sv_t(NATIVE_PATH("-type")));
    return true;
}

}
}
