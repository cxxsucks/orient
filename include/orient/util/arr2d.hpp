#pragma once
#include <mutex>
#include <orient/fs/predef.hpp>
#include <orient/util/compresslib/intersection.h>

struct arr2d_writer {
    orie::str_t _saving_path;
    std::vector<std::vector<uint32_t>> _data_pending;

    void add_int(size_t row, uint32_t val);
    // Throw runtime error if write failed; data pending unchanged
    void append_pending_to_file();

    // DO NOT throw if path is invalid; only throws on appending failure
    arr2d_writer(orie::str_t saving_path) noexcept
        : _saving_path(std::move(saving_path)) {}
};

class arr2d_reader {
#ifdef _WIN32
    HANDLE _map_descriptor;
#else
    int _map_descriptor;
#endif // _WIN32
    orie::str_t _map_path;
    const uint32_t* _mapped_data;
    size_t _mapped_sz;

    // Cache the starting point of a page for repeated access
    mutable uint32_t _cache_page_idx = 0;
    mutable uint32_t _cache_page_offset = 0;
    mutable std::mutex _cache_mut;

    uint32_t page_offset(size_t page) const noexcept;

public:
    // Get both compressed data and its compressed size in 4 bytes.
    // [nullptr, ~uint32_t()] if line is out of bound but page is not
    // [nullptr, ~uint32_t() - 1] if page is out of bound
    // res.first[-1] is uncompressed size in 4 bytes :)
    std::pair<const uint32_t*, uint32_t>
    line_data(size_t line, size_t page) const noexcept;
    // ~uint32_t() if page or line is out of bound
    // no throw because out ogf bound is common :)
    uint32_t uncmprs_size(size_t line, size_t page) const noexcept;

    // Move the database file elsewhere. Thread safe but poorly implemented
    void move_file(orie::str_t path);
    // void refresh();

    // Throws system error if open failed
    arr2d_reader(orie::str_t invidx_path);
    // munmap and close
    ~arr2d_reader() noexcept;
};

// THREAD UNSAFE semi-lazy invert index queryer
// Create multiple queries for thread safety
class arr2d_intersect {
    const arr2d_reader* _reader;
    // Lazily evaluate results, one page at a time
    std::vector<uint32_t> _cur_page_res;
    compressionLib::fastPForCodec _codec;
    uint32_t _next_page_idx;

public:
    // Modify it before any call to next_intersect or before rewind
    // otherwise the behavior is undefined.
    std::vector<uint32_t> _lines_to_query;

    // ~uint32_t when finished, i.e., no more intersections
    uint32_t next_intersect(size_t redundancy = 0);
    void rewind() noexcept { _next_page_idx = 0; _cur_page_res.clear(); }

    // Reader is nullable and can be set later
    void set_reader(const arr2d_reader *reader) noexcept {
        _reader = reader;
        rewind();
    }
    const arr2d_reader* reader() const noexcept { return _reader; }

    // Reader is nullable and can be set later
    arr2d_intersect(const arr2d_reader* reader = nullptr)
        : _reader(reader), _next_page_idx(0) {}

    static std::vector<uint32_t>
    decompress_entire_line(uint32_t line, const arr2d_reader* reader);
};
