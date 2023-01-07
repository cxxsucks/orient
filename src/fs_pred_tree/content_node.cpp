#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <cassert>
#include <variant>

#ifndef _WIN32
extern "C" {
#include <sys/mman.h>
}
#endif

namespace orie {

#ifndef _WIN32
struct __dummy_unique_fd {
    int _fd;
    operator int() const noexcept { return _fd; }
    __dummy_unique_fd(int fd) noexcept : _fd(fd) {}
    ~__dummy_unique_fd() noexcept { if (_fd > 0) ::close(_fd); }
};
#endif

namespace pred_tree {

class __match_helper {
private:
    std::variant<
        std::pair<sv_t, bool>, // Needle and icase
        pcre2_code*,
        // Cached needle and cutoff similarity
        std::pair<const fuzz_cache&, double> 
    > _needle;
    bool _allow_binary;

    bool _do_fuzz_match(sv_t haystack) const {
        if (haystack.size() < 5)
            return false; // Prevent too short haystacks like "e"
        return std::get<2>(_needle).first.similarity(haystack) >=
               std::get<2>(_needle).second;
    }

    bool _do_regex_match(sv_t haystack, pcre2_match_data* match_dat) const noexcept {
        return 0 < pcre2_match(
            std::get<1>(_needle),
            reinterpret_cast<PCRE2_SPTR>(haystack.data()),
            haystack.size(), 0, 0, match_dat, nullptr
        ); // C function `pcre2_match` is noexcept
    }

    bool _do_strstr_match(sv_t haystack) const noexcept {
        if (std::get<0>(_needle).second) {
            icase_sv_t hs_icase(haystack.data(), haystack.size()),
                       nd_icase(std::get<0>(_needle).first.data(),
                                std::get<0>(_needle).first.size());
            return hs_icase.find(nd_icase) != sv_t::npos;
        }
        else return haystack.find(std::get<0>(_needle).first) != sv_t::npos;
    }

    bool _do_one_match(sv_t haystack, pcre2_match_data* match_dat) const {
        switch (_needle.index()) {
        case 0: return _do_strstr_match(haystack);
        case 1: return _do_regex_match(haystack, match_dat);
        case 2: return _do_fuzz_match(haystack);
        default: std::terminate(); // Unreachable
        }
    }

public:
    __match_helper(sv_t needle, bool icase, bool allow_bin)
        : _needle(std::in_place_index<0>, needle, icase)
        , _allow_binary(allow_bin) {}
    __match_helper(pcre2_code* rgx, bool allow_bin)
        : _needle(rgx), _allow_binary(allow_bin) {}
    __match_helper(const fuzz_cache& nd_cache, double cutoff, bool allow_bin)
        : _needle(std::in_place_index<2>, nd_cache, cutoff)
        , _allow_binary(allow_bin) {}

    // Search for a regex or substring in a file. Optimal when:
    // 1. mmap(2) is slower than a read(2) but faster than 2 reads.
    // 2. Block size is 4096B. (Suboptimal for small files when <4096B)
    bool do_match(const fs_data_iter& it) {
        // Allocate a match data each call to _do_match for better concurrency
        std::unique_ptr<pcre2_match_data, decltype(&pcre2_match_data_free)> match_dat(
            // Only do allocation if the request is actually a regex match.
            _needle.index() == 1 ? pcre2_match_data_create(16, nullptr) : nullptr,
            &pcre2_match_data_free
        );

    #ifdef _WIN32
        // Read files with std::wifstream, which handles utf-8 gracefully.
        // TODO: i18n support is $hit on Win$hit 💩!!!
    #ifdef _MSC_VER
        std::wstring to_open = it.path(); // utf8 only :(
    #else
        std::wstring to_open = orie::xxstrcpy(sv_t(it.path())); // ansi only :(
    #endif
        // Remove the trailing separator that may exist
        if (to_open.empty())
            return false;
        if (to_open.front() == separator)
            to_open = to_open.substr(1);

        // Judge binary
        if (!_allow_binary) {
            char read_buf[4096];
            ::memset(read_buf, -1, 4096); // Fill the buffer with non-NULLs
            std::ifstream bin_ifs(to_open, std::ios_base::binary);
            bin_ifs.read(read_buf, 4096); // `read` does not place NULL
            if (std::char_traits<char>::find(read_buf, bin_ifs.gcount(), '\0'))
                return false;
        }

        // Caveat: may have to open the file twice. Also `std::istream`s are slow :(
        std::unique_ptr<wchar_t[]> read_buf(new wchar_t[16384 + 256]);
        ::memset(read_buf.get(), 0, 16640);
        std::wifstream ifs(to_open);
    #ifdef _MSC_VER
        ifs.imbue(std::locale("en_US.utf8"));
    #endif

        std::streamsize last_read = ifs.read(read_buf.get(), 256).gcount();
        if (!ifs.good()) // At least match once
            return _do_one_match(sv_t(read_buf.get(), 256), match_dat.get());

        do {
            last_read = ifs.read(read_buf.get() + 256, 16384).gcount();
            if (_do_one_match(sv_t(read_buf.get(), last_read + 256),
                                   match_dat.get()))
                return true;
            ::memcpy(read_buf.get(), read_buf.get() + 16384, 256 * sizeof(wchar_t));
        } while (ifs.good());
        return false;

    #else // Unix content match: read and mmap
        __dummy_unique_fd fd = ::open(it.path().c_str(), O_RDONLY);
        if (fd <= 0)
            return false;
        off_t filesz = it.file_size(), scanned = 0;
        auto blksz = it.io_block_size();
        if (blksz > 4096) blksz = 4096;
        char read_buf[4096 + 128];

        // Return false for binary files when bin files are not matched.
        if (!_allow_binary) {
            // A file is seen as binary if '\0' exists in its first 4096B.
            scanned = ::read(fd, read_buf, 4096);
            if (std::char_traits<char>::find(read_buf, scanned, '\0'))
                return false; // Binary file 
            // If the first 4096 bytes contain a match, stop.
            if (_do_one_match(sv_t(read_buf, scanned), match_dat.get()))
                return true;
        }

        if (scanned == filesz)
            return false; // Empty or fully read when judging binary-ness.
        // If the (remaining) file can be read in one block, use read(2)
        if (scanned + blksz >= filesz) {
            ssize_t match_len;
            if (_allow_binary) {
                assert(scanned == 0);
                match_len = ::read(fd, read_buf, blksz);
            } else {
                // First 4096 bytes are already read.
                // Also search previous 128B to avoid missing matches near 4096B.
                ::memcpy(read_buf, read_buf + 4096 - 128, 128);
                match_len = ::read(fd, read_buf + 128, blksz) + 128;
            }
            return _do_one_match(sv_t(read_buf, match_len), match_dat.get());
        }

        // Otherwise, use mmap(2) mapping 64MiB at a time.
        while (scanned < filesz) {
            scanned = std::max(scanned, off_t(4096));
            size_t map_len = std::min(off_t(64 << 20), filesz - scanned);
            // Also map previous 4096B to avoid missing matches across mapped blocks
            map_len += 4096;
            char* mapped = reinterpret_cast<char*>(
                ::mmap(nullptr, map_len, PROT_READ,
                    MAP_PRIVATE, fd, scanned - 4096)
            );
            scanned += (64 << 20);
            bool matched = _do_one_match(sv_t(mapped, map_len), match_dat.get());
            ::munmap(mapped, map_len);
            if (matched)
                return true;
        }
        return false;
    #endif
    }
};

tribool_bad content_regex_node::apply(fs_data_iter& it) {
    if (_re == nullptr)
        throw uninitialized_node(NATIVE_SV("-content-regex"));
    if (it.file_type() != file_tag)
        return tribool_bad::False;
    if (_blocked)
        return apply_blocked(it);
    return tribool_bad::Uncertain;
}

bool content_regex_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return false;
    if (_re == nullptr)
        throw uninitialized_node(NATIVE_SV("-content-regex"));
    return __match_helper(_re.get(), _allow_binary).do_match(it);
}

bool content_regex_node::next_param(sv_t param) {
    if (_re != nullptr)
        return false;
    if (param == NATIVE_SV("--ignore-case")) {
        _icase = true;
    } else if (param == NATIVE_SV("--blocked")) {
        _blocked = true;
    } else if (param == NATIVE_SV("--binary")) {
        _allow_binary = true;
    } else {
        int errcode; PCRE2_SIZE erroffset;
        _re.reset(pcre2_compile(
            // char to uint8 on Unix, wchar_t to uint16 on Windows
            reinterpret_cast<PCRE2_SPTR>(param.data()), param.size(),
            _icase ? PCRE2_CASELESS | PCRE2_MULTILINE : PCRE2_MULTILINE,
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
    }
    return true;
}

tribool_bad content_strstr_node::apply(fs_data_iter& it) {
    if (_pattern.empty())
        throw uninitialized_node(NATIVE_SV("-content-strstr"));
    if (it.file_type() != file_tag)
        return tribool_bad::False;
    if (_blocked)
        return apply_blocked(it);
    return tribool_bad::Uncertain;
}

bool content_strstr_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return false;
    if (_pattern.empty())
        throw uninitialized_node(NATIVE_SV("-content-strstr"));
    return __match_helper(_pattern, _icase, _allow_binary).do_match(it);
}

bool content_strstr_node::next_param(sv_t param) {
    if (!_pattern.empty())
        return false;
    if (param == NATIVE_SV("--ignore-case")) {
        _icase = true;
    } else if (param == NATIVE_SV("--blocked")) {
        _blocked = true;
    } else if (param == NATIVE_SV("--binary")) {
        _allow_binary = true;
    } 
    else _pattern = param;
    return true;
}

tribool_bad content_fuzz_node::apply(fs_data_iter& it) {
    if (!_matcher.has_value())
        throw uninitialized_node(NATIVE_SV("-content-fuzz"));
    if (it.file_type() != file_tag)
        return tribool_bad::False;
    if (_blocked)
        return apply_blocked(it);
    return tribool_bad::Uncertain;
}

bool content_fuzz_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return false;
    if (!_matcher.has_value())
        throw uninitialized_node(NATIVE_SV("-content-fuzz"));
    return __match_helper(_matcher.value(), _cutoff, _allow_binary).do_match(it);
}

bool content_fuzz_node::next_param(sv_t param) {
    if (_matcher.has_value())
        return false;

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
    } else if (param == NATIVE_SV("--blocked")) {
        _blocked = true;
    } else if (param == NATIVE_SV("--binary")) {
        _allow_binary = true;
    } else if (param == NATIVE_SV("--cutoff")) {
        _next_cutoff = true;
    } else 
        _matcher.emplace(param);
    return true;
}

}
}
