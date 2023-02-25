#include <orient/fs/trigram.hpp>

namespace orie {
namespace dmp {

// Simple trigram extraction
size_t strstr_trigram_ext(sv_t name, uint32_t* out, size_t outsz) noexcept {
    if (name.size() <= 2)
        return 0;
    size_t outat = 0;
    for (size_t i = 0; i < name.size() - 2 && outat < outsz; ++i) {
        uint32_t to_add = char_to_trigram(name[i], name[i+1], name[i+2]);
        if (to_add != 0)
            out[outat++] = to_add;
    }
    return outat;
}

// Extract trigrams from glob patterns. Does not treat '/' specially
std::pair<size_t, bool> // trigram size and is basename
glob_trigram_ext(sv_t pat, uint32_t* out, const size_t outsz, bool full) noexcept {
    char_t l = 0, m = 0, h = 0;
    bool escape = false;
    size_t outat = 0, oldsz = 0;

    for (const char_t* p = pat.data(), *p_end = p + pat.size(); p < p_end; ++p) {
        if (!escape) {
            if (*p == char_t('*') || *p == char_t('?') ||
                (*p == char_t(']') && full))
            {
                l = 0; m = 0; h = 0;
                // A lone unpaiered ']' is treated as normal char in basename
                // but in fullpath it discards all results so far to make alloance for
                // patterns like "0[12/34]567", in which case pat == "34]567"
                if (full) {
                    oldsz = *p == char_t(']') ? 0 : outat;
                    outat = 0;
                }
                continue;
            } else if (*p == char_t('[')) {
                const char_t* s = p + 1; // Pointer to chars in bracket
                while (s < p_end && *s != char_t(']'))
                    s += (*s == char_t('\\') ? 2 : 1);
                // Unpaired '[' is treated the same way as unpaired ']'
                if (full || s < p_end) {
                    p = s;
                    l = 0; m = 0; h = 0; continue;
                } // else p remain unchanged and *p == '[' is a normal char
            } else if (*p == char_t('\\')) {
                escape = true;
                continue;
            }
        }

        l = m; m = h; h = *p;
        escape = false;
        if (l != 0 && outat < outsz) {
            uint32_t toadd = char_to_trigram(l, m, h);
            if (toadd != 0)
                out[outat++] = toadd;
        }
    }
    return std::make_pair(outat == 0 ? oldsz : outat, outat != 0);
}

std::pair<size_t, bool> // trigram size and is basename
fullpath_trigram_ext(sv_t pat, bool glob, uint32_t* out, size_t outsz) {
    // Find the name with at least 1 trigram and as to the right as possible
    size_t cur_at = 0, res = 0;
    sv_t res_name;

    while (true) {
        size_t next_at = pat.find(separator, cur_at);
        if (next_at == pat.npos)
            break;

        sv_t cur_name = pat.substr(cur_at, next_at - cur_at);
        size_t cur_res = glob ? glob_trigram_ext(cur_name, out, outsz, true).first
                              : strstr_trigram_ext(cur_name, out, outsz);
        if (cur_res != 0) {
            res = cur_res;
            res_name = cur_name;
        }
        cur_at = next_at + 1; // Skip the separator
    }
    // The above loop stops on reaching final name (basename)

    // The last name (basename) is scanned first because basename search
    // is much faster than full path search.
    sv_t basename = pat.substr(cur_at);
    auto bn_res = glob 
        ? glob_trigram_ext(basename, out, outsz, true)
        : std::make_pair(strstr_trigram_ext(basename, out, outsz), false);
    // As long as basename contains any trigram, it is used, and other
    // names are not scanned at all.
    if (bn_res.first != 0)
        return bn_res;

    // Actual placing of the best name to `out`
    // res = glob ? glob_trigram_ext(res_name, out, outsz, true).first
    //            : strstr_trigram_ext(res_name, out, outsz);
    return std::make_pair(res, false);
}

uint32_t char_to_trigram(uint32_t low, uint32_t mid, uint32_t high) noexcept {
    // 7 6 5 4 3 2 1 0
    //   ^   ^     ^ ^ Byte 1, bit 0~3 of result
    //         ^ ^ ^ ^ Byte 2, bit 4~7 of result
    //   ^   ^ ^ ^     Byte 3, bit 8~11 of result
    return (low & 0b11) | ((low & 0b10000) >> 2) | ((low & 0b1000000) >> 3) |
           ((mid & 0b1111) << 4) |
           ((high & 0b11100) << 6) | ((high & 0b1000000) << 5);
}

void place_trigram(sv_t name, uint32_t batch, arr2d_writer& w) {
    for (size_t i = 2; i < name.size(); ++i) {
        uint32_t to_add = char_to_trigram(name[i-2], name[i-1], name[i]);
        if (to_add != 0)
            w.add_int(to_add, batch);
    }
}

void trigram_query::reset_strstr_needle(sv_t needle, bool is_full) {
    _is_full = is_full;
    auto& lns = _query._lines_to_query;
    lns.resize(32);

    size_t tgr_sz = is_full ?
        fullpath_trigram_ext(needle, false, lns.data(), 32).first :
        strstr_trigram_ext(needle, lns.data(), 32);
    lns.resize(tgr_sz);
    _query.rewind();
}

void trigram_query::reset_glob_needle(sv_t needle, bool is_full) {
    auto& lns = _query._lines_to_query;
    lns.resize(32);

    auto tgr_sz = is_full ?
        fullpath_trigram_ext(needle, true, lns.data(), 32) :
        glob_trigram_ext(needle, lns.data(), 32, false);
    lns.resize(tgr_sz.first);
    _is_full = !tgr_sz.second && tgr_sz.first != 0;
    _query.rewind();
}

} // namespace dmp
} // namespace orie
