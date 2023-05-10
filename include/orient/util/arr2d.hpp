#pragma once
#include <orient/fs/predef.hpp>
#include <orient/util/compresslib/intersection.h>
#include <shared_mutex>

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

// A view on a 2-dimensional 32bit integers in ascending order
// created by `arr2d_writer`. 
class arr2d_reader {
public:
    bool _rmfile_on_dtor;
private:
#ifdef _WIN32
    HANDLE _map_descriptor;
#else
    int _map_descriptor;
#endif // _WIN32

    orie::str_t _map_path;
    const uint32_t* _mapped_data;
    size_t _mapped_sz;

    // Cache the starting point of a page for repeated access
    mutable uint32_t _cache_page_idx;
    mutable uint32_t _cache_page_offset;
    mutable std::mutex _cache_mut;
    mutable std::shared_mutex _access_mut;

    // Tuple of offset to compressed data, compressed and decompressed size
    // [0, -1, -1] if page is out of bound
    // [0, 0, 0] if line is out of bound but page is not
    std::tuple<size_t, uint32_t, uint32_t>
    raw_line_data(size_t line, size_t page) const noexcept;
    // Get the offset of `page` in the file.
    // Matching it against `~uint32_t()` is useful in determining whether
    // a page exists or not.
    uint32_t page_offset(size_t page) const noexcept;

public:
    // Write decompressed data to out and total size (in 4 bytes).
    // Return -1 if page is out of bound.
    // If retval > outsz - 4, nothing is written or decompressed and a larger
    // buffer is required.
    size_t line_data(compressionLib::fastPForCodec& decomper, uint32_t* out,
                     size_t outsz, size_t line, size_t page) const;
    // Write decompressed data to a vector.
    // Return false if page is out of bound, and out will be empty.
    bool line_data(compressionLib::fastPForCodec& decomper,
                   std::vector<uint32_t>& out, size_t line, size_t page) const;

    // ~uint32_t() if page is out of bound, 0 if line is empty or out of bound
    // no throw because out of bound is common :)
    uint32_t uncmprs_size(size_t line, size_t page) const noexcept;
    const orie::str_t& saving_path() const noexcept { return _map_path; }

    // Move the database file elsewhere. 
    // Move, clear and refresh cannot mutually run concurrently,
    // but they can run with data query functions.
    void move_file(orie::str_t path);
    // Call this after some numbers are appended with
    // `arr2d_writer::append_to_file` to maniefst recent changes
    // Move, clear and refresh cannot mutually run concurrently,
    // but they can run with data query functions.
    void refresh();
    // Clear all elements.
    // Move, clear and refresh cannot mutually run concurrently,
    // but they can run with data query functions.
    void clear();
    // Close the file containing the array. The reader will be viewing an
    // empty array after close. Call to `refresh` reopens the original array.
    // Implemented by `munmap` and `close`
    void close() noexcept;

    // Throws system error if open failed
    arr2d_reader(orie::str_t arr_file_path);
    // Map again instead of `memcpy`
    arr2d_reader(const arr2d_reader& r) : arr2d_reader(r._map_path) {}
    arr2d_reader& operator=(const arr2d_reader& r) {
        if (&r != this) {
            this->~arr2d_reader();
            new (this) arr2d_reader(r);
        }
        return *this;
    }
    // munmap, close and remove file if `_rmfile_on_dtor` is set
    ~arr2d_reader() noexcept;
};

// THREAD UNSAFE semi-lazy invert index queryer
// Create multiple queries for thread safety
// Can query both intersection and frequency (for fuzzy search)
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
    // ~uint32_t when finished, i.e., no more integersappearing more than
    // `min_freq` times in `_lines_to_query`
    uint32_t next_frequent(uint32_t min_freq);
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
