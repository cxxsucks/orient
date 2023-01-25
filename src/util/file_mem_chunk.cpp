#include <orient/util/file_mem_chunk.hpp>
#ifdef _WIN32
#define fopen _wfopen
#endif

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
    FILE *fp = fopen(_saving_path.c_str(), NATIVE_PATH("rb")); 
    if (fp == nullptr || end == ~size_t())
        throw std::runtime_error("Cannot read database");
    // TODO: Magic number
    fseek(fp, sizeof(size_t) * 256 + beg, SEEK_SET);

    // Remove previous cache info
    in_which_cache = (_next_overwrite++ % _cache_num);
    _chunkid_to_cacheid[_cacheid_to_chunkid[in_which_cache]] = ~uint8_t();
    _cacheid_to_chunkid[in_which_cache] = chunk_idx;
    _chunkid_to_cacheid[chunk_idx] = in_which_cache;

    // TODO: May throw when out of memory
    // Temporary buffer for Decompression
    std::vector<std::byte> cmprs_buf(read_sz, std::byte());
    // Read file data
    if (fread(cmprs_buf.data(), 1, read_sz, fp) != read_sz)
        throw std::runtime_error("Encountered bad data within database");
    fclose(fp);
    
    // Decompress and place it to cache TODO: error handling
    size_t cmprs_sz = ZSTD_getFrameContentSize(cmprs_buf.data(), read_sz);
    _cached_dat[in_which_cache].assign(cmprs_sz, std::byte());
    ZSTD_decompressDCtx(_dctx, _cached_dat[in_which_cache].data(), cmprs_sz,
                        cmprs_buf.data(), cmprs_buf.size());
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

    // Synchonize to file by writing data
    // Move RAW data to cache
    auto& c = _cached_dat[in_which_cache]; // aliase to cache dest
    c = std::move(_unplaced_dat);
    _unplaced_dat.clear();

    // Make `unplaced_dat` temporarily store compressed dat
    _unplaced_dat.assign(ZSTD_compressBound(c.size()), std::byte());
    size_t cmprs_sz = ZSTD_compressCCtx(_cctx,
        _unplaced_dat.data(), _unplaced_dat.size(),
        c.data(), c.size(), 1);
    _chunk_size_presum[_chunk_num + 1] = 
        _chunk_size_presum[_chunk_num] + cmprs_sz;
    if (ZSTD_isError(cmprs_sz))
        throw std::runtime_error(ZSTD_getErrorName(cmprs_sz));

    // Write metadata first. TODO: Magic Number
    FILE *fp = fopen(_saving_path.c_str(), NATIVE_PATH("rb+"));
    if (fp == nullptr)
        throw std::runtime_error("Cannot write database");
    fseek(fp, (_chunk_num + 1) * sizeof(size_t), SEEK_SET);
    fwrite(_chunk_size_presum + _chunk_num + 1, sizeof(size_t), 1, fp);
    fseek(fp, 0, SEEK_END);
    fwrite(_unplaced_dat.data(), 1, cmprs_sz, fp);
    fclose(fp);
    ++_chunk_num;
    _unplaced_dat.clear();
}

file_mem_chunk::file_mem_chunk(sv_t fpath, uint8_t cache_cnt,
                               bool empty, bool rm)
    : _chunkid_to_cacheid(new uint8_t[256 * (1 + sizeof(size_t))])
    , _chunk_size_presum(reinterpret_cast<size_t*>(_chunkid_to_cacheid + 256))
    , _cctx(ZSTD_createCCtx()), _dctx(ZSTD_createDCtx())
    , _reader_count(0), _saving_path(fpath), _chunk_num(0)
    , _next_overwrite(0), _cache_num(cache_cnt), _rmfile_on_dtor(rm)
{
    *reinterpret_cast<uint64_t*>(_cacheid_to_chunkid) = ~uint64_t();
    memset(_chunkid_to_cacheid, -1, 256 * (1 + sizeof(size_t)));
    _chunk_size_presum[0] = 0;

    // Open the file
    FILE* fp = nullptr;
    if (!empty) 
    // If the file does not exist(open failed), create a new one
        if ((fp = fopen(_saving_path.c_str(), NATIVE_PATH("rb"))) == nullptr)
            empty = true;
    if (empty)
        fp = fopen(_saving_path.c_str(), NATIVE_PATH("wb"));
    if (fp == nullptr)
        throw std::runtime_error("Cannot open database");

    // No exception in this if statement
    if (empty) {
        // TODO: magic num
        fwrite(_chunk_size_presum, sizeof(size_t), 256, fp);
    } else {
        if (fread(_chunk_size_presum, sizeof(size_t), 256, fp) != 256
            || _chunk_size_presum[0] != magic_num)
            throw std::runtime_error("Not a valid database");
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
        throw std::runtime_error("Permission denied moving database");
    _saving_path.assign(fpath);
}

size_t file_mem_chunk::chunk_size(uint8_t at) const {
    if (at >= _chunk_num)
        throw std::out_of_range("Chunk Index Out of Range");
    return _chunk_size_presum[at + 1] - _chunk_size_presum[at];
}

void file_mem_chunk::clear() {
    std::lock_guard __lck(_writer_mut);
    FILE* fp = fopen(_saving_path.c_str(), NATIVE_PATH("wb"));
    if (fp == nullptr)
        throw std::runtime_error("Cannot clear database");

    _chunk_num = 0;
    *reinterpret_cast<uint64_t*>(_cacheid_to_chunkid) = ~uint64_t();
    memset(_chunkid_to_cacheid, -1, 256 * (1 + sizeof(size_t)));
    _chunk_size_presum[0] = 0;
    // TODO: magic num
    fwrite(_chunk_size_presum, sizeof(size_t), 256, fp);
    fclose(fp);

    _unplaced_dat.clear();
    for (uint8_t i = 0; i < _cache_num; ++i)
        _cached_dat[i].clear();
}

file_mem_chunk::~file_mem_chunk() {
    std::lock_guard __lck(_writer_mut);
    if (_rmfile_on_dtor)
#ifdef _WIN32
        ::DeleteFileW(_saving_path.c_str());
#else // unlink(2)
        ::unlink(_saving_path.c_str());
#endif
    else 
        add_last_chunk();
    delete[] _chunkid_to_cacheid;
    ZSTD_freeCCtx(_cctx);
    ZSTD_freeDCtx(_dctx);
}

#ifdef _MSC_VER
// Lock/unlock mismatch; intended here
#pragma warning(disable: 26110 26115 26117)
#endif

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

#ifdef _MSC_VER
#pragma warning(default: 26110 26115 26117)
#endif

}
}