#pragma once
#include <orient/fs/predef.hpp>
#include <zstd.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace orie {
namespace dmp {

// At most 255 chunks of raw data with at most 8 chunks in memory
// Others are stored in a file
class file_mem_chunk {
private:
    // Index of cache containing the chunk, >=8 for none
    // 256-size heap-alloced array
    uint8_t* _chunkid_to_cacheid;
    // Presum of size in bytes of each chunk
    // 256-size heap-alloced array
    size_t* _chunk_size_presum;

    // Compression Context
    ZSTD_CCtx* _cctx;
    // Decompression Context
    ZSTD_DCtx* _dctx;

    // Content of each cache
    std::vector<std::byte> _cached_dat[8];
    // Chunk index of each cache
    uint8_t _cacheid_to_chunkid[8];

public:
    // Final block is not in file yet and can be freely modified
    std::vector<std::byte> _unplaced_dat;

private:
    // For reader-writer synchonization
    size_t _reader_count;
    std::mutex _writer_mut, _cnt_mut;

    // File to save memory not in cache
    str_t _saving_path;
    uint8_t _chunk_num, // Amount of total chunks in file
            _next_overwrite; // Index of cache to get overwritten next (FIFO)
    const uint8_t _cache_num; // Amount of cache blocks

    void __writer_lock() noexcept; // `finish_visit` is writer unlock

public:
    // TODO: 0 is too generic
    static constexpr size_t magic_num = 0;
    // Remove the memory file on destruction
    bool _rmfile_on_dtor;

    // Final block is not in file yet and can be freely modified
    // Modifications on this block is NOT thread safe to any functions
    std::vector<std::byte>& chunk_to_add() noexcept { return _unplaced_dat; };
    size_t chunk_count() const noexcept { return _chunk_num; }
    size_t chunk_size(uint8_t at) const;
    const str_t& saving_path() const noexcept { return _saving_path; }

    // Write the chunk returned in `chunk_to_add` to file, making it unchangable.
    // Empty chunk will not be added (no-op in that case)
    // Note that destructor also calls this function.
    void add_last_chunk();

    // Visit the `at`-th byte of `chunk_idx`-th block
    // The pointer is always valid before calling `finish_visit`
    // If exception is thrown (out of bound), no need for `finisg_visit`
    const std::byte* start_visit(uint8_t chunk_idx, size_t at);
    // Each `start_visit` must call one and only one `finish_visit`
    void finish_visit() noexcept;

    // Move the memory file to `fpath`.
    // Throw runtime error if no permission, and the file is NOT moved
    void move_file(const char_t* fpath);
    // Reset memory file and cache.
    void clear();

    // Throw runtime error if no permission
    file_mem_chunk(sv_t fpath, uint8_t cached_chunk_cnt,
                   bool empty, bool rmfile_on_destroy);
    ~file_mem_chunk();
    file_mem_chunk(const file_mem_chunk&) = delete;
    file_mem_chunk(file_mem_chunk&&) = delete;
};

// To make sure start_visit and finish_visit are coupled,
// use this RAII visitor
// class file_mem_visitor {
//     const std::byte* _visiting;
//     file_mem_chunk* _chunk;

// public:
//     file_mem_visitor(file_mem_chunk&)
// };


} // namespace dmp
} // namespace orie
