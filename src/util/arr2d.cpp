extern "C" {
#include <unistd.h>
}
#include <orient/util/arr2d.hpp>
#include <algorithm>
#include <mutex>
// Return values of `munmap(2)` are NOT checked because:
// https://stackoverflow.com/questions/22779556/linux-error-from-munmap

#ifdef _WIN32
#define fopen _wfopen
#endif

void arr2d_writer::add_int(size_t row, uint32_t val) {
    if (row >= _data_pending.size())
        _data_pending.insert(_data_pending.cend(), row - _data_pending.size() + 1,
                             std::vector<uint32_t>());

    // Check if the value is already placed, ensuring the arrays are ascending
    if (row <= 1 || _data_pending[row].empty() || _data_pending[row].back() < val)
        _data_pending[row].push_back(val);
}

void arr2d_writer::append_pending_to_file() {
    FILE* fp = fopen(_saving_path.c_str(), "rb+");
    if (fp == nullptr) {
        if ((fp = fopen(_saving_path.c_str(), "w")) == nullptr)
            throw std::runtime_error("Open failed: " + _saving_path);
        fclose(fp);
        if ((fp = fopen(_saving_path.c_str(), "rb+")) == nullptr)
            throw std::runtime_error("Open failed: " + _saving_path);
    }
    fseek(fp, 0, SEEK_END);

    // Write number of rows; only std::bad_alloc exception since then
    uint32_t towrite = static_cast<uint32_t>(_data_pending.size());
    fwrite(&towrite, sizeof(uint32_t), 1, fp);
    long hdr_off = ftell(fp); // Offset to header
    // Fill header (each line's offset) with 0
    towrite = 0;
    for (size_t i = 0; i < 1 + _data_pending.size(); ++i)
        fwrite(&towrite, sizeof(uint32_t), 1, fp);

    compressionLib::fastPForCodec codec;
    std::vector<uint32_t> cmprs_buf;
    // Offset to line data; +2 for num of rows and next page start
    uint32_t line_off = 2 + _data_pending.size();
    for (size_t idx = 0; idx < _data_pending.size(); ++idx) {
        std::vector<uint32_t>& d = _data_pending[idx];
        towrite = static_cast<uint32_t>(d.size());
        // Write Uncompressed integer count first
        fwrite(&towrite, sizeof(uint32_t), 1, fp);

        // Compress and Write compressed data
        cmprs_buf.resize(d.size() + 128);
        size_t cmprs_len = cmprs_buf.size();
        codec.encodeArray(d.data(), d.size(), cmprs_buf.data(), cmprs_len);
        fwrite(cmprs_buf.data(), sizeof(uint32_t), cmprs_len, fp);
        d.clear();

        // In header section, write offset to line data 
        fseek(fp, hdr_off, SEEK_SET);
        fwrite(&line_off, sizeof(uint32_t), 1, fp);
        hdr_off += sizeof(uint32_t);
        line_off += 1 + cmprs_len;
        fseek(fp, 0, SEEK_END);
    }

    // Write one-past-end pos of all data (start of next page) in header
    fseek(fp, hdr_off, SEEK_SET);
    fwrite(&line_off, sizeof(uint32_t), 1, fp);
    fclose(fp);
}

#define THROW_SYS_ERROR throw \
    std::system_error(errno, std::system_category())

void arr2d_reader::move_file(orie::str_t path) {
#ifdef _WIN32
    // On Windows, a file MUST be closed before move or deletion.
    // A lock must be introduced to avoid accessing during the gap when
    // the file is unmapped.
    std::unique_lock __lck(_access_mut);
    // Unmap original data
    if (_mapped_sz != 0) {
        UnmapViewOfFile(_mapped_data);
        static const uint32_t dummy[2] = {0, 0};
        _mapped_sz = 0;
        _mapped_data = dummy;
    }

    // Close file
    if (_mapped_descriptor != INVALID_HANDLE_VALUE) {
        CloseHandle(_mapped_descriptor);
        _mapped_descriptor = INVALID_HANDLE_VALUE;
    }

    if (MoveFileExW(_map_path.c_str(), path.c_str(),
                    MOVEFILE_REPLACE_EXISTING) == 0)
    {
        refresh(); 
        THROW_SYS_ERROR;
    }
    _map_path = std::move(path);
    refresh();

#else
    // On Unix, moving or deleting a file when still open is possible
    if (0 != rename(_map_path.c_str(), path.c_str()))
        THROW_SYS_ERROR;
    _map_path = std::move(path);
#endif
}

void arr2d_reader::clear() {
    if (_mapped_sz == 0)
        return;
#ifdef _WIN32
    std::unique_lock __lck(_access_mut);
    // Unmap original data
    if (_mapped_sz != 0) 
        UnmapViewOfFile(_mapped_data);
    static const uint32_t dummy[2] = {0, 0};
    _mapped_sz = 0;
    _mapped_data = dummy;

    // Close file
    if (_map_handle != INVALID_HANDLE_VALUE) {
        if (CloseHandle(_map_handle) == 0)
            THROW_SYS_ERROR;
        _map_handle = INVALID_HANDLE_VALUE;
    }

    // Delete the file at _map_path
    if (DeleteFileW(_map_path.c_str()) == 0) {
        refresh();
        THROW_SYS_ERROR;
    }

    // Create and open an empty file at _map_path
    _map_handle = CreateFileA(_map_path.c_str(), GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                              nullptr);
    if (_map_handle == INVALID_HANDLE_VALUE)
        THROW_SYS_ERROR;

#else
    if (_map_descriptor >= 0)
        close(_map_descriptor);
    unlink(_map_path.c_str());
    _map_descriptor = open(_map_path.c_str(), O_RDONLY | O_CREAT, 0600);
    if (_map_descriptor == -1)
        THROW_SYS_ERROR;
    refresh();
#endif
}

void arr2d_reader::refresh() {
    const uint32_t* old_dat = _mapped_data;
    size_t old_sz = _mapped_sz;
#ifdef _WIN32
    struct _stat64 stbuf;
    if (_fstat64(_fileno(_map_handle), &stbuf) != 0)
        THROW_SYS_ERROR

    if (stbuf.st_size == 0) {
        static const uint32_t dummy[2] = {0, 0};
        _mapped_sz = 0;
        _mapped_data = dummy;
        if (_mapped_sz != 0)
            UnmapViewOfFile(const_cast<uint32_t*>(old_dat));
        return;
    }

    HANDLE map_obj = CreateFileMappingW(_map_handle, NULL, PAGE_READONLY,
                                        0, 0, NULL);
    if (map_obj == nullptr)
        THROW_SYS_ERROR;
    _mapped_data = static_cast<const uint32_t*>(
        MapViewOfFile(map_obj, FILE_MAP_READ, 0, 0, 0));
    if (_mapped_data == nullptr)
        THROW_SYS_ERROR;

    if (_mapped_sz != 0) // Previous map was not empty
        UnmapViewOfFile(const_cast<uint32_t*>(old_dat));
    _mapped_sz = static_cast<size_t>(stbuf.st_size);
    CloseHandle(map_obj);

#else
    struct stat stbuf;
    if (fstat(_map_descriptor, &stbuf) != 0)
        THROW_SYS_ERROR;

    std::unique_lock __lck(_access_mut);
    if (stbuf.st_size == 0) {
        // Fake page with 0 lines and no next page
        static const uint32_t dummy[2] = {0, 0};
        _mapped_sz = 0;
        // The first 0 bytes are definitely correct so no undefined behaviors
        _mapped_data = dummy; 
        if (old_sz != 0) // Previous map was not empty
            munmap(const_cast<uint32_t*>(old_dat), old_sz);
        return;
    }

    _mapped_data = static_cast<const uint32_t*>(
        mmap(nullptr, static_cast<size_t>(stbuf.st_size),
             PROT_READ, MAP_SHARED, _map_descriptor, 0)
    );
    if (_mapped_data == MAP_FAILED) {
        _mapped_data = old_dat;
        THROW_SYS_ERROR;
    }

    if (_mapped_sz != 0) // Previous map was not empty
        munmap(const_cast<uint32_t*>(old_dat), _mapped_sz);
    _mapped_sz = static_cast<size_t>(stbuf.st_size);
#endif
}

arr2d_reader::arr2d_reader(orie::str_t fpath)
    : _rmfile_on_dtor(false), _map_path(std::move(fpath))
    , _mapped_data(nullptr) , _mapped_sz(0)
    , _cache_page_idx(0), _cache_page_offset(0)
{
    _map_descriptor = open(_map_path.c_str(), O_RDONLY | O_CREAT, 0600);
    if (_map_descriptor == -1)
        THROW_SYS_ERROR;
    refresh();
}

arr2d_reader::~arr2d_reader() noexcept {
    if (_mapped_sz != 0)
        munmap(const_cast<uint32_t*>(_mapped_data), _mapped_sz);
    if (_map_descriptor >= 0)
        close(_map_descriptor);
    if (_rmfile_on_dtor)
        unlink(_map_path.c_str());
}

uint32_t arr2d_reader::page_offset(size_t page) const noexcept {
    // Copy shared cache to local stack to reduce locking
    _cache_mut.lock();
    uint32_t cache_page_idx = _cache_page_idx;
    uint32_t cache_page_offset = _cache_page_offset;
    _cache_mut.unlock();
    // If the cached page offset is what we want, don't bother visiting data
    // Repeated access to a same page is th fastest
    if (__likely(cache_page_idx == page))
        return cache_page_offset;
    // If the cached page is smaller than what we want, iterate from cached
    // page instead of the beginning.
    if (cache_page_idx > page)
        cache_page_idx = (cache_page_offset = 0);

    {
    std::shared_lock __lck(_access_mut);
    while (cache_page_idx != page) {
        // Lines this page +1
        uint32_t nlinep1 = _mapped_data[cache_page_offset] + 1;
        // Offset of beginning of next page
        uint32_t next_off = _mapped_data[cache_page_offset + nlinep1]
                            + cache_page_offset;
        if (next_off >= _mapped_sz / sizeof(uint32_t))
            return ~uint32_t();
        ++cache_page_idx;
        cache_page_offset = next_off;
    }
    }

    // cache_page_offset now points at beginning of required page
    // Write back cache
    _cache_mut.lock();
    _cache_page_idx = cache_page_idx;
    _cache_page_offset = cache_page_offset;
    _cache_mut.unlock();
    return cache_page_offset;
}

std::tuple<const uint32_t*, uint32_t, uint32_t>
arr2d_reader::raw_line_data(size_t line, size_t page) const noexcept {
    uint32_t page_off = page_offset(page);
    if (page_off == ~uint32_t())  // Outbound page
        return std::make_tuple(nullptr, ~uint32_t(), ~uint32_t());
    else if (_mapped_data[page_off] <= line) // Outbound Line
        return std::make_tuple(nullptr, 0, 0);

    uint32_t line_off = page_off + line + 1;
    std::shared_lock __lck(_access_mut);
    return std::make_tuple(
        // TODO: EXPLAIN THIS MONSTROSITY!!!
        _mapped_data + page_off + _mapped_data[line_off] + 1,
        _mapped_data[line_off + 1] - _mapped_data[line_off] - 1,
        _mapped_data[page_off + _mapped_data[line_off]]
    );
}

uint32_t arr2d_reader::uncmprs_size(size_t line, size_t page) const noexcept {
    return std::get<2>(raw_line_data(line, page));
}

size_t arr2d_reader::line_data(compressionLib::fastPForCodec &co, uint32_t *out,
                               size_t outsz, size_t line, size_t page) const
{
    auto [lineptr, cmprs_sz, uncmprs_sz] = raw_line_data(line, page);
    if (cmprs_sz == ~uint32_t()) // Requested page is out of bound
        return ~size_t();
    // Do not run decompression if the line has nothing.
    // Not necessary, but faster for empty lines.
    else if (uncmprs_sz == 0)
        return 0;

    // TODO: Segfault if `lineptr` is invalid due to remapping
    // Solve by returning {ptr, cmprs_sz, uncmprs_sz} tuple in raw_line_data
    if (outsz + 4 >= uncmprs_sz)
        return uncmprs_sz; // Not enough space
    co.decodeArray(lineptr, cmprs_sz, out, outsz);
    ASSERT(outsz == uncmprs_sz,
           "arr2d error in uncompressing line to buffer");
    return uncmprs_sz;
}

bool arr2d_reader::line_data(compressionLib::fastPForCodec &co,
    std::vector<uint32_t> &out, size_t line, size_t page) const
{
    auto [lineptr, cmprs_sz, uncmprs_sz] = raw_line_data(line, page);
    if (cmprs_sz == ~uint32_t()) // Requested page is out of bound
        return false;
    // Do not run decompression if the line has nothing.
    // Not necessary, but faster for empty lines.
    else if (uncmprs_sz == 0) {
        out.clear();
        return true;
    }

    out.resize(uncmprs_sz + 4);
    size_t buf_sz = out.size();
    co.decodeArray(lineptr, cmprs_sz, out.data(), buf_sz);
    ASSERT(uncmprs_sz == buf_sz,
           "arr2d error in uncompressing line to buffer");
    out.resize(uncmprs_sz);
    return true;
}

uint32_t arr2d_intersect::next_intersect(size_t redundancy) {
    if (_reader == nullptr || _lines_to_query.empty())
        return ~uint32_t();
    if (!_cur_page_res.empty()) 
        goto retrive;

    while (_cur_page_res.empty()) {
        // Sort the numbers by number of elements in current page ascending
        std::sort(_lines_to_query.begin(), _lines_to_query.end(),
                  [this](uint32_t l, uint32_t r) {
                      return _reader->uncmprs_size(l, _next_page_idx) <
                             _reader->uncmprs_size(r, _next_page_idx);
                  });
        _lines_to_query.erase(std::unique(
            _lines_to_query.begin(), _lines_to_query.end()
        ), _lines_to_query.end());

        // Decompress the `_lines_to_query[0]`th line in current page
        if (!_reader->line_data(_codec, _cur_page_res, _lines_to_query[0],
                                _next_page_idx))
            return ~uint32_t(); // No more pages; intersection finished
        if (_cur_page_res.empty()) {
            // Trying to intersect an empty line; no result in this page
            ++_next_page_idx;
            continue;
        }

        std::vector<uint32_t> decode_buf, next_res;
        for (size_t i = 1; i < _lines_to_query.size() &&
             _cur_page_res.size() > redundancy; ++i)
        {
            // Decode next line
            _reader->line_data(_codec, decode_buf, _lines_to_query[i],
                               _next_page_idx); // Always returns true

            // Prepare space for intersection
            next_res.resize(_cur_page_res.size() + 4);
            // Intersect with previous results
            size_t inters_sz = compressionLib::i32_intersect(
                decode_buf.data(), decode_buf.size(),
                _cur_page_res.data(), _cur_page_res.size(), next_res.data());
            next_res.resize(inters_sz);

            // Swap preserves buffer allocated
            std::swap(next_res, _cur_page_res);
        }
        // _cur_page_res should be in descending order, so back()s
        // return ascending values
        ++_next_page_idx;
    }
    std::reverse(_cur_page_res.begin(), _cur_page_res.end());

retrive:
    uint32_t res = _cur_page_res.back();
    _cur_page_res.pop_back();
    return res;
}

std::vector<uint32_t>
arr2d_intersect::decompress_entire_line(uint32_t line, const arr2d_reader* r) {
    std::vector<uint32_t> res;
    if (r == nullptr)
        return res;
    arr2d_intersect q(r);
    q._lines_to_query.assign({line});

    while (r->uncmprs_size(line, q._next_page_idx) != ~uint32_t()) {
        uint32_t first = q.next_intersect();
        if (first != ~uint32_t()) {
            res.push_back(first);
            res.insert(res.cend(), q._cur_page_res.crbegin(),
                       q._cur_page_res.crend());
            q._cur_page_res.clear();
        }
    }
    return res;
}

// A simple uint-to-uint flat map for obtaining frequency data
// in fuzzy matching. Difference between max and min key must be within 1M.
class flatmap_bad {
    size_t min_key;
    std::vector<uint32_t> values;

public:
    uint32_t& operator[](size_t at);
    void clear() noexcept { values.clear(); }
    void get_frequent_nums(uint32_t threshold, std::vector<uint32_t>& dest) const;
    flatmap_bad() : min_key(0) { values.reserve(200000); }
};

uint32_t &flatmap_bad::operator[](size_t at) {
    if (__unlikely(values.empty()))
        min_key = at > 16384 ? at - 16384 : 0;
    if (__unlikely(at < min_key)) {
        size_t n_ins = min_key - at;
        n_ins += (at > 16384 ? at - 16384 : 0);
        ASSERT(n_ins + values.size() < 1200000,
               "Batch span in a page too large: " << n_ins + values.size());
        values.insert(values.cbegin(), n_ins, 0);
        min_key -= n_ins;
    }
    if (__unlikely(at >= min_key + values.size())) {
        values.resize(at - min_key + 100000);
        ASSERT(values.size() < 1200000,
               "Batch span in a page too large: " << values.size());
    }
    return values[at - min_key];
}

void flatmap_bad::get_frequent_nums(uint32_t threshold,
                                    std::vector<uint32_t> &dest) const
{
    // Write to `dest` in descending order
    for (size_t i = values.size() - 1; i != ~size_t(); --i) {
        if (values[i] >= threshold)
            dest.push_back(static_cast<uint32_t>(i + min_key));
    }
}

uint32_t arr2d_intersect::next_frequent(uint32_t min_freq) {
    if (_reader == nullptr || _lines_to_query.empty())
        return ~uint32_t();
    if (__likely(!_cur_page_res.empty()))
        goto retrive;

{   std::vector<uint32_t> decode_buf;
    flatmap_bad cur_page_freqs;
    while (_cur_page_res.empty()) {
        for (uint32_t line : _lines_to_query) {
            // Uncompress this line and +1 frequency for each elem in it
            if (!_reader->line_data(_codec, decode_buf, line,
                                    _next_page_idx))
                return ~uint32_t(); // No more pages
            for (uint32_t elem : decode_buf)
                ++cur_page_freqs[elem];
        }
        ++_next_page_idx;
        cur_page_freqs.get_frequent_nums(min_freq, _cur_page_res);
        cur_page_freqs.clear();
    }
}

retrive:
    uint32_t res = _cur_page_res.back();
    _cur_page_res.pop_back();
    return res;
}
