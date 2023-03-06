extern "C" {
#include <unistd.h>
}
#include <orient/util/arr2d.hpp>
#include <algorithm>

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
    std::system_error(std::make_error_code(static_cast<std::errc>(errno)))

void arr2d_reader::move_file(orie::str_t path) {
    // On Unix, moving or deleting a file when still open is possible
    if (0 != rename(_map_path.c_str(), path.c_str()))
        THROW_SYS_ERROR;
    _map_path = std::move(path);

    // On Windows, a file MUST be closed before move or deletion.
    // A lock must be introduced to avoid accessing during the gap when
    // the file is unmapped.
}

void arr2d_reader::clear() {
    if (_mapped_sz == 0)
        return;
    if (_map_descriptor >= 0)
        close(_map_descriptor);
    unlink(_map_path.c_str());
    _map_descriptor = open(_map_path.c_str(), O_RDONLY | O_CREAT, 0600);
    if (_map_descriptor == -1)
        THROW_SYS_ERROR;
    refresh();
}

void arr2d_reader::refresh() {
    const uint32_t* old_dat = _mapped_data;
    struct stat stbuf;
    if (fstat(_map_descriptor, &stbuf) != 0)
        goto fail;

    if (stbuf.st_size == 0) {
        static const uint32_t dummy[2] = {0, 0};
        _mapped_data = dummy; // Fake page with 0 lines and no next page
        _mapped_sz = 0;
        return;
    }

    _mapped_data = static_cast<const uint32_t*>(
        mmap(nullptr, static_cast<size_t>(stbuf.st_size),
             PROT_READ, MAP_SHARED, _map_descriptor, 0)
    );
    if (_mapped_data == MAP_FAILED)
        goto fail;
    // Here is a window in which _mapped_sz is not equal to actual mapped
    // size but as long as the file is appended (with arr2d_writer), the
    // first `_mapped_sz` bytes are valid so no undefined behavior happen.

    if (_mapped_sz != 0) // Previous map was not empty
        if (munmap(const_cast<uint32_t*>(old_dat), _mapped_sz) != 0)
            goto fail;
    _mapped_sz = static_cast<size_t>(stbuf.st_size);
    return;
fail:
    _mapped_data = old_dat;
    THROW_SYS_ERROR;
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

    // Move the cached page and position to current page
    if (__likely(cache_page_idx == page))
        return cache_page_offset;

    if (cache_page_idx > page)
        cache_page_idx = (cache_page_offset = 0);
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

    // cache_page_offset now points at beginning of required page
    // Write back cache
    _cache_mut.lock();
    _cache_page_idx = cache_page_idx;
    _cache_page_offset = cache_page_offset;
    _cache_mut.unlock();
    return cache_page_offset;
}

std::pair<const uint32_t*, uint32_t>
arr2d_reader::line_data(size_t line, size_t page) const noexcept {
    uint32_t page_off = page_offset(page);
    if (page_off == ~uint32_t())  // Outbound page
        return std::make_pair(nullptr, ~uint32_t() - 1);
    else if (_mapped_data[page_off] <= line) // Outbound Line
        return std::make_pair(nullptr, ~uint32_t());
    return std::make_pair(
        // TODO: EXPLAIN THIS MONSTROSITY!!!
        _mapped_data + page_off + _mapped_data[page_off + line + 1] + 1,
        _mapped_data[page_off + line + 2] - _mapped_data[page_off + line + 1] - 1
    );
}

uint32_t arr2d_reader::uncmprs_size(size_t line, size_t page) const noexcept {
    const uint32_t* lineptr = line_data(line, page).first;
    if (lineptr == nullptr)
        return ~uint32_t();
    return lineptr[-1];
}

uint32_t arr2d_intersect::next_intersect(size_t redundancy) {
    if (_reader == nullptr || _lines_to_query.empty())
        return ~uint32_t();

    if (!_cur_page_res.empty()) 
        goto retrive;
    while (_cur_page_res.empty()) {
        // Sort the numbers by number of elements in current page ascending
        std::sort(_lines_to_query.begin(), _lines_to_query.end(), [this] (uint32_t l, uint32_t r) {
            return _reader->line_data(l, _next_page_idx).second <
                   _reader->line_data(r, _next_page_idx).second;
        });
        _lines_to_query.erase(std::unique(
            _lines_to_query.begin(), _lines_to_query.end()
        ), _lines_to_query.end());

        // Decompress the `_lines_to_query[0]`th line in current page
        auto [lineptr, cmprs_sz] =
            _reader->line_data(_lines_to_query[0], _next_page_idx);
        if (cmprs_sz >= ~uint32_t() - 1) {
            _cur_page_res.clear();
            if (cmprs_sz == ~uint32_t() - 1)
                return ~uint32_t();
            ++_next_page_idx;
            continue;
        }
        uint32_t uncmprs_sz = lineptr[-1];

        _cur_page_res.resize(uncmprs_sz + 4);
        size_t decode_sz = _cur_page_res.size();
        // decodeArray do not change input data
        _codec.decodeArray(lineptr, cmprs_sz,
                           _cur_page_res.data(), decode_sz);
        ASSERT(decode_sz == _cur_page_res.size() - 4, "Decode first line error");
        ASSERT(decode_sz == uncmprs_sz, "Decode first line error");
        _cur_page_res.resize(decode_sz);

        std::vector<uint32_t> decode_buf, next_res;
        for (size_t i = 1; i < _lines_to_query.size() &&
             _cur_page_res.size() > redundancy; ++i)
        {
            std::tie(lineptr, cmprs_sz) = 
                _reader->line_data(_lines_to_query[i], _next_page_idx);
            if (lineptr == nullptr) {
                _cur_page_res.clear();
                continue;
            }
            uncmprs_sz = lineptr[-1];

            // Prepare space for uncompression and intersection
            decode_buf.resize(uncmprs_sz + 4);
            next_res.resize(_cur_page_res.size() + 4);

            // Decode next line
            decode_sz = decode_buf.size();
            _codec.decodeArray(lineptr, cmprs_sz,
                               decode_buf.data(), decode_sz);
            ASSERT(decode_sz == uncmprs_sz, "Decode error");
            // Intersect with previous results
            size_t inters_sz = compressionLib::i32_intersect(
                decode_buf.data(), decode_sz,
                _cur_page_res.data(), _cur_page_res.size(), next_res.data());
            next_res.resize(inters_sz);

            // Swap preserves buffer allocated
            std::swap(next_res, _cur_page_res);
            // _cur_page_res should be in descending order, so back()s
            // return ascending values
        }
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

    while (r->line_data(0, q._next_page_idx).second != ~uint32_t() - 1) {
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

