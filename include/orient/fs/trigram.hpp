#pragma once
#include <orient/util/arr2d.hpp>
#include <optional>

namespace orie {
class fs_data_iter;

namespace dmp {

// Simple trigram extraction
size_t strstr_trigram_ext(sv_t name, uint32_t* out, size_t outsz) noexcept;
// Extract trigrams from glob patterns. Does not treat '/' specially
std::pair<size_t, bool> // trigram size and is basename
glob_trigram_ext(sv_t pat, uint32_t* out, const size_t outsz, bool full) noexcept;
std::pair<size_t, bool> // trigram size and is basename
fullpath_trigram_ext(sv_t pat, bool glob, uint32_t* out, size_t outsz);

uint32_t char_to_trigram(uint32_t low, uint32_t mid, uint32_t high) noexcept;
void place_trigram(sv_t name, uint32_t batch, arr2d_writer& w);

class trigram_query {
    arr2d_intersect _query;
    bool _is_full = false;

public:
    // Getter
    bool is_fullpath() const noexcept { return _is_full; }
    size_t trigram_size() const noexcept
        { return _query._lines_to_query.size(); }
    void reset_reader(const arr2d_reader* reader) noexcept {
        _query.set_reader(reader);
    }
    void rewind() noexcept { _query.rewind(); }

    // is_fullpath() will return the same as is_full here
    void reset_strstr_needle(sv_t needle, bool is_full);
    // is_full == true here does not mean is_fullpath() return true
    void reset_glob_needle(sv_t needle, bool is_full);

    uint32_t next_batch_possible() { return _query.next_intersect(1); }
    // uint32_t next_batch_possible(const fs_data_iter& it);
    // void to_next_batch(fs_data_iter& it);

    trigram_query(const arr2d_reader *reader, sv_t needle = sv_t(),
                  bool is_glob = false, bool is_full = false)
        : _query(reader) { is_glob ? reset_strstr_needle(needle, is_full)
                                   : reset_glob_needle(needle, is_full); }
    trigram_query() = default;
    ~trigram_query() = default;
};

} // namespace dmp
} // namespace orie
