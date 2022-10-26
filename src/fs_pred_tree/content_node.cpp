#include <orient/fs_pred_tree/fs_nodes.hpp>
#include <cassert>
extern "C" {
#include <sys/mman.h>
}

namespace orie {

struct _dummy_unique_fd {
    int _fd;
    operator int() const noexcept { return _fd; }
    _dummy_unique_fd(int fd) noexcept : _fd(fd) {}
    ~_dummy_unique_fd() noexcept { if (_fd > 0) ::close(_fd); }
};

// Search for a regex or substring in a file. Optimal when:
// 1. mmap(2) is slower than a read(2) but faster than 2 reads.
// 2. Block size is 4096B. (Suboptimal for small files when <4096B)
static bool _do_match(const fs_data_iter& it, sv_t str_needle, bool icase,
                      bool allow_binary, pcre2_code* re_needle) noexcept
{
    // Allocate a match data each call to _do_match for better concurrency
    std::unique_ptr<pcre2_match_data, decltype(&pcre2_match_data_free)> match_dat(
        // Only do allocation if the request is actually a regex match.
        re_needle != nullptr ? pcre2_match_data_create(8, nullptr) : nullptr,
        &pcre2_match_data_free
    );
    auto _do_regex_match = 
    [&match_dat] (sv_t haystack, pcre2_code* re) noexcept {
        return 0 < pcre2_match(
            re, reinterpret_cast<PCRE2_SPTR>(haystack.data()),
            haystack.size(), 0, 0,
            match_dat.get(), nullptr
        ); // C function `pcre2_match` is noexcept
    };
    auto _do_strstr_match =
    [] (sv_t haystack, sv_t needle, bool icase) noexcept {
        if (icase) {
            icase_sv_t hs_icase(haystack.data(), haystack.size()),
                       nd_icase(needle.data(), needle.size());
            return hs_icase.find(nd_icase) != sv_t::npos;
        }
        else return haystack.find(needle) != sv_t::npos;
    };

    _dummy_unique_fd fd = ::open(it.path().c_str(), O_RDONLY);
    if (fd <= 0)
        return false;
    off_t filesz = it.file_size(), scanned = 0;
    auto blksz = it.io_block_size();
    if (blksz > 4096) blksz = 4096;
    char read_buf[4096 + 128];

    // Return false for binary files when bin files are not matched.
    if (!allow_binary) {
        // A file is seen as binary if '\0' exists in its first 4096B.
        scanned = ::read(fd, read_buf, 4096);
        if (std::char_traits<char>::find(read_buf, scanned, '\0'))
            return false; // Binary file 
        // If the first 4096 bytes contain a match, stop.
        if (re_needle ? _do_regex_match(sv_t(read_buf, scanned), re_needle)
                      : _do_strstr_match(sv_t(read_buf, scanned), str_needle, icase))
            return true;
    }

    if (scanned == filesz)
        return false; // Empty or fully read when judging binary-ness.
    // If the (remaining) file can be read in one block, use read(2)
    if (scanned + blksz >= filesz) {
        ssize_t match_len;
        if (allow_binary) {
            assert(scanned == 0);
            match_len = ::read(fd, read_buf, blksz);
        } else {
            // First 4096 bytes are already read.
            // Also search previous 128B to avoid missing matches near 4096B.
            ::memcpy(read_buf, read_buf + 4096 - 128, 128);
            match_len = ::read(fd, read_buf + 128, blksz) + 128;
        }
        return re_needle != nullptr
            ? _do_regex_match(sv_t(read_buf, match_len), re_needle)
            : _do_strstr_match(sv_t(read_buf, match_len), str_needle, icase);
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
        bool matched = (re_needle 
            ? _do_regex_match(sv_t(mapped, map_len), re_needle)
            : _do_strstr_match(sv_t(mapped, map_len), str_needle, icase));
        ::munmap(mapped, map_len);
        if (matched)
            return true;
    }
    return false;
}

namespace pred_tree {

tribool_bad content_regex_node::apply(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return tribool_bad::False;
    if (_blocked)
        return apply_blocked(it);
    if (_re == nullptr)
        throw uninitialized_node(NATIVE_SV("-content-regex"));
    return tribool_bad::Uncertain;
}

bool content_regex_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return false;
    if (_re == nullptr)
        throw uninitialized_node(NATIVE_SV("-content-regex"));
    return _do_match(it, sv_t(), _icase, _allow_binary, _re.get());
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
            reinterpret_cast<PCRE2_SPTR8>(param.data()), param.size(),
            _icase ? PCRE2_CASELESS : 0,
            &errcode, &erroffset, nullptr
        ), pcre2_code_free);

        if (_re == nullptr) {
            // Locked to char8 as std::exception only accepts char8
            PCRE2_UCHAR8 errbuf[128];
            pcre2_get_error_message_8(errcode, errbuf, 128);
            throw std::runtime_error(reinterpret_cast<char*>(errbuf));
        }
    }
    return true;
}

tribool_bad content_strstr_node::apply(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return tribool_bad::False;
    if (_blocked)
        return apply_blocked(it);
    if (_pattern.empty())
        throw uninitialized_node(NATIVE_SV("-content-strstr"));
    return tribool_bad::Uncertain;
}

bool content_strstr_node::apply_blocked(fs_data_iter& it) {
    if (it.file_type() != file_tag)
        return false;
    if (_pattern.empty())
        throw uninitialized_node(NATIVE_SV("-content-strstr"));
    return _do_match(it, _pattern, _icase, _allow_binary, nullptr);
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

}
}
