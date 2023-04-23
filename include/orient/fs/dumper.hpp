#pragma once
#include <orient/fs/trigram.hpp>
#include <orient/util/fifo_thpool.hpp>
#include <orient/util/file_mem_chunk.hpp>

namespace orie {
namespace dmp {

// Low-level filesystem database writer
// All members can be directly modified
class dumper {
public:
    // TODO: Changeable in conf file
    // Minimum size of each chunk. Actual size will be a bit larger.
    static constexpr size_t chunk_size_hint = 2000000;
    // Number of chunks loaded into memory
    static constexpr uint8_t cached_chunk_cnt = 8;
    // Number of files in a batch
    static constexpr uint8_t nfile_in_batch = 24;

    // The path to start scanning filesystem
    str_t _root_path;
    // The paths not to scan
    // All path must be absolute (start with slash) and have no extra slash
    // or they will be made so during database rebuild.
    std::vector<str_t> _pruned_paths;
    // The paths where read operations would not be concurrent
    // Usually mount points of spinninf disks
    // All path must be absolute (start with slash) and have no extra slash
    // or they will be made so during database rebuild.
    std::vector<str_t> _noconcur_paths;
    fifo_thpool& _pool;

private:
    // The filesystem database
    // Outside the class, its _unplaced_dat field MUST NOT BE
    // MODIFIED and shall REMAIN EMPTY
    file_mem_chunk _index;
    arr2d_reader _invidx;

    // Map from batch subscript to the batch's position in forward index
    // (inverted index to forward index)
    std::vector<uint32_t> _pos_of_batches; // In-chunk position
    std::vector<uint32_t> _chunk_of_batches; // Chunk id

public:
    dumper(sv_t database_path, fifo_thpool& pool);
    void rebuild_database();
    void move_file(str_t path);

    // The position `batch`th batch is at, in its chunk in forward index
    uint32_t in_chunk_pos_of_batch(size_t batch) const noexcept {
        return _pos_of_batches[batch];
    }
    // The chunk `batch`th batch is at, in forward index
    uint32_t chunk_of_batch(size_t batch) const noexcept {
        return _chunk_of_batches[batch];
    }

    size_t batch_count() const noexcept { return _pos_of_batches.size(); }
    size_t chunk_count() const noexcept { return _index.chunk_count(); }
    const str_t& fwdidx_path() const noexcept { return _index.saving_path(); }
    const str_t& invidx_path() const noexcept { return _invidx.saving_path(); }

    // TODO: Refactor file_mem_chunk to auto manage file memory visit
    // Calls _fwdidx->start_visit()
    const std::byte* start_visit(uint32_t chunk, size_t in_chunk_pos) {
        return _index.start_visit(chunk, in_chunk_pos);
    }
    // Calls _fwdidx->finish_visit()
    void finish_visit() { _index.finish_visit(); }

    void set_remove_on_destroy(bool on) noexcept {
        _index._rmfile_on_dtor = on;
        _invidx._rmfile_on_dtor = on;
    }

    // Reset a trigram query object so that it queries data dumped
    // by this dumper object. Calls `query.reset_reader(&_invidx)`.
    void to_query_of_this_index(trigram_query& query) const noexcept {
        query.reset_reader(&_invidx);
    }

private:
    // Simple std::find
    bool is_pruned(const str_t& fullp);
    bool is_noconcur(const str_t& fullp);

    // Basename of child dirs and basenames of child files
    // dir_fullpath may change inside, but remain unchanged on return
    typedef std::pair<std::vector<str_t>, std::vector<str_t>> dir_info_t;
    dir_info_t fetch_dir_info(str_t& dir_fullpath);

    // Helper function that serilizes a directory whose contents
    // are already read into `info`
    size_t dump_one(const str_t& fullpath, size_t basename_len, arr2d_writer& w,
                    const dir_info_t& info, size_t nth_file);
    // fullpath may change inside, but remain unchanged on return
    size_t dump_concur(str_t& fullpath, size_t basename_len, arr2d_writer& w,
                       const dir_info_t& info, size_t nth_file);
    // fullpath may change inside, but remain unchanged on return
    size_t dump_noconcur(str_t& fullpath, size_t basename_len, arr2d_writer& w,
                         const dir_info_t& info, size_t nth_file);
};

}
}
