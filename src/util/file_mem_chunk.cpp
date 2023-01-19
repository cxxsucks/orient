#include <orient/util/file_mem_chunk.hpp>
#include <cassert>

namespace orie {
namespace dmp {

const std::byte* file_mem_chunk::start_visit(uint8_t chunk_idx, size_t at) {
    if (chunk_idx >= _chunk_num)
        throw std::out_of_range("Chunk Index Out of Range");
    uint8_t in_which_cache;

cache_read:
    __writer_lock(); // Cache table is shared (critical section) therefore lock
    // Read cache table. If this chunk is already in, return it
    in_which_cache = _chunkid_to_cacheid[chunk_idx];
    if (in_which_cache >= 8) 
        goto cache_write;
    if (at >= _cached_dat[in_which_cache].size()) {
        finish_visit();
        throw std::out_of_range("Chunk Visit Out of Range");
    }
    return _cached_dat[in_which_cache].data() + at;

cache_write: {
    finish_visit();
    std::lock_guard __lck(_writer_mut);
    // Needed data may already been filled during waiting
    in_which_cache = _chunkid_to_cacheid[chunk_idx];
    if (in_which_cache < 8)
        goto cache_read;

    // Only do cache swapping if not in cache
    size_t beg = _chunk_size_presum[chunk_idx],
            end = _chunk_size_presum[chunk_idx + 1],
            read_sz = end - beg;
    // C-style binary file read
    FILE *fp = fopen(_saving_path.c_str(), "rb"); 
    assert(end != ~size_t() && fp != nullptr);
    // TODO: Magic number
    fseek(fp, sizeof(size_t) * 256 + beg, SEEK_SET);

    // Remove previous cache info
    in_which_cache = (_next_overwrite++ % _cache_num);
    _chunkid_to_cacheid[_cacheid_to_chunkid[in_which_cache]] = ~uint8_t();
    _cacheid_to_chunkid[in_which_cache] = chunk_idx;
    _chunkid_to_cacheid[chunk_idx] = in_which_cache;

    // Read file content and place it to cache
    // TODO: Read file content, decompress and place it to cache
    // TODO: assign may throw when out of memory
    _cached_dat[in_which_cache].assign(read_sz, std::byte());
    fread(_cached_dat[in_which_cache].data(), 1, read_sz, fp);
    fclose(fp);
}
    goto cache_read;
}

void file_mem_chunk::add_last_chunk() {
    if (_unplaced_dat.empty())
        return;
    if (_chunk_num == 255)
        throw std::out_of_range("Max chunk count reached");

    std::lock_guard __lck(_writer_mut);
    // Remove previous cache info
    uint8_t in_which_cache = (_next_overwrite++ % _cache_num);
    _chunkid_to_cacheid[_cacheid_to_chunkid[in_which_cache]] = ~uint8_t();
    _cacheid_to_chunkid[in_which_cache] = _chunk_num;
    _chunkid_to_cacheid[_chunk_num] = in_which_cache;
    _chunk_size_presum[_chunk_num + 1] = 
        _chunk_size_presum[_chunk_num] + _unplaced_dat.size();

    // Synchonize to file by writing data
    // TODO: Compress data and write result to file
    FILE *fp = fopen(_saving_path.c_str(), "rb+");
    assert(fp != nullptr);
    // Write metadata first. TODO: Magic Number
    fseek(fp, (_chunk_num + 1) * sizeof(size_t), SEEK_SET);
    fwrite(_chunk_size_presum + _chunk_num + 1, sizeof(size_t), 1, fp);
    fseek(fp, 0, SEEK_END);
    fwrite(_unplaced_dat.data(), 1, _unplaced_dat.size(), fp);
    fclose(fp);
    ++_chunk_num;

    // Write RAW data to cache
    _cached_dat[in_which_cache] = std::move(_unplaced_dat);
    _unplaced_dat.clear();
}

file_mem_chunk::file_mem_chunk(const char_t* fpath, uint8_t cache_cnt,
                               bool empty, bool rm)
    : _chunkid_to_cacheid(new uint8_t[256 * (1 + sizeof(size_t))])
    , _chunk_size_presum(reinterpret_cast<size_t*>(_chunkid_to_cacheid + 256))
    , _reader_count(0), _saving_path(fpath), _chunk_num(0)
    , _next_overwrite(0), _cache_num(cache_cnt), _rmfile_on_dtor(rm)
{
    static size_t magic_num = 0;
    *reinterpret_cast<uint64_t*>(_cacheid_to_chunkid) = ~uint64_t();
    memset(_chunkid_to_cacheid, -1, 256 * (1 + sizeof(size_t)));
    _chunk_size_presum[0] = 0;

    // Open the file
    FILE* fp = fopen(fpath, empty ? "wb" : "rb");
    if (fp == nullptr)
        throw std::runtime_error("Permission denied opening memory file");

    // No exception in this if statement
    if (empty) {
        // TODO: magic num
        fwrite(_chunk_size_presum, sizeof(size_t), 256, fp);
    } else {
        if (fread(_chunk_size_presum, sizeof(size_t), 256, fp) != 256
            || _chunk_size_presum[0] != magic_num)
            throw std::runtime_error("Not a valid memory file");
        size_t nc = 0;
        while (nc < 256 && _chunk_size_presum[nc] != ~uint64_t())
            ++nc;
        _chunk_num = static_cast<uint8_t>(--nc);
    }
    fclose(fp);
}

void file_mem_chunk::move_file(const char_t* fpath) {
    std::lock_guard __lck(_writer_mut);
#ifdef _WIN32
    if (::MoveFileW(_saving_path.c_str(), fpath) == FALSE)
#else // C89 rename(2)
    if (::rename(_saving_path.c_str(), fpath) != 0)
#endif
        throw std::runtime_error("Permission denied moving memory file");
    _saving_path.assign(fpath);
}

size_t file_mem_chunk::chunk_size(uint8_t at) const {
    if (at >= _chunk_num)
        throw std::out_of_range("Chunk Index Out of Range");
    return _chunk_size_presum[at + 1] - _chunk_size_presum[at];
}

file_mem_chunk::~file_mem_chunk() noexcept {
    std::lock_guard __lck(_writer_mut);
    delete[] _chunkid_to_cacheid;
    if (_rmfile_on_dtor)
#ifdef _WIN32
        ::DeleteFileW(_saving_path.c_str());
#else // unlink(2)
        ::unlink(_saving_path.c_str());
#endif
}

// Reader Begin Reading
void file_mem_chunk::__writer_lock() noexcept {
    std::lock_guard __lck(_cnt_mut);
    if (++_reader_count == 1)
        _writer_mut.lock();
}

// Reader Finish Reading
void file_mem_chunk::finish_visit() noexcept {
    std::lock_guard __lck(_cnt_mut);
    if (--_reader_count == 0)
        _writer_mut.unlock();
}

}
}