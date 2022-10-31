#pragma once
#include <cmath>
#include <cstring>
#include <cctype>
#include <stdexcept>
#include <string>

namespace orie {

/** @brief Copy at most nmemb null-terminated characters of arbitrary type to dest
 * @tparam des_t a type assignable from type @c src_t
 * @param [out] dest at least @p nmemb @em +1 bytes pre-allocated buffer
 * @param [in] src string of any length. Can be nullptr, in which case
 * nothing is copied
 * @param nmemb Maximum characters copied, @em not including terminating NULL.
 * @return dest */
template<class des_t, class src_t, typename = std::enable_if_t<
    std::is_assignable_v<des_t&, src_t> > >
des_t* strncpy(des_t* dest, const src_t* src, size_t nmemb) 
    noexcept(std::is_nothrow_assignable_v<des_t&, src_t>) {
    if (!dest || !src)
        return dest;
    size_t i = 0;
    while (i < nmemb && src[i])
        dest[i] = static_cast<des_t>(src[i]), ++i;
    dest[i] = des_t();
    return dest;
}

template<> inline
char* strncpy<char, char>(char* d, const char* s, size_t n) noexcept {
    size_t size_cp = ::strnlen(s, n);
    ::memcpy(d, s, size_cp);
    d[size_cp] = '\0';
    return d;
}
template<> inline
wchar_t* strncpy<wchar_t, wchar_t>(wchar_t* d, const wchar_t* s, size_t n) noexcept {
    size_t size_cp = ::wcsnlen(s, n);
    ::memcpy(d, s, size_cp * sizeof(wchar_t));
    d[size_cp] = L'\0';
    return d;
}

/** @brief Copy null-terminated string of arbitrary type to dest
 * @tparam des_t a type assignable from type @p src_t
 * @param [out] dest pre-allocated buffer at least as long as @p src
 * @param [in] src string of any length. Can be nullptr, in which case
 * nothing is copied
 * @return dest */
template<class des_t, class src_t, typename = std::enable_if_t<
    std::is_assignable_v<des_t&, src_t> >>
des_t* strcpy(des_t* dest, const src_t* src) 
    noexcept(std::is_nothrow_assignable_v<des_t&, src_t>) {
    return orie::strncpy<des_t, src_t>(dest, src, ~size_t());
}

template<> inline
char* strcpy<char, char>(char* dest, const char* src) noexcept {
    return ::strcpy(dest, src);
}
template<> inline
wchar_t* strcpy<wchar_t, wchar_t>(wchar_t* dest, const wchar_t* src) noexcept {
    return ::wcscpy(dest, src);
}
/** @brief Length of a string of arbitrary type
 * @param [in] src string of any length. Can be nullptr, in which case
 * 0 is returned
 * @return Length of @p src */
template<typename char_t,
    typename = std::enable_if_t<std::is_default_constructible_v<
        typename std::pointer_traits<char_t*>::element_type
    >>>
size_t strlen(const char_t* src) noexcept {
    if (!src) return 0;
    size_t res = size_t();
    using elem_t = typename std::pointer_traits<char_t>::element_type;
    while (src[res] != elem_t())
        res++;
    return res;
}
template<> inline
size_t strlen<char>(const char* s) noexcept {
    return ::strlen(s);
}
template<> inline
size_t strlen<wchar_t>(const wchar_t* s) noexcept {
    return ::wcslen(s);
}

template<class des_t, class src_t>
des_t* strncat(des_t* dest, const src_t* src, size_t nmemb) noexcept {
    if (!dest) return nullptr;
    else if (!src) return nullptr;
    orie::strncpy<des_t, src_t>(
        dest + orie::strlen<des_t>(dest), src, nmemb
    );
    return dest;
}
template<> inline
char* strncat<char, char>(char* d, const char* s, size_t n) noexcept {
    return ::strncat(d, s, n);
}
template<> inline
wchar_t* strncat<wchar_t, wchar_t>(wchar_t* d, const wchar_t* s, size_t n) noexcept {
    return ::wcsncat(d, s, n);
}

template<class des_t, class src_t>
des_t* strcat(des_t* dest, const src_t* src) noexcept {
    return orie::strncat<des_t, src_t>(dest, src, ~size_t());
}

template<> inline
char* strcat<char, char>(char* d, const char* s) noexcept {
    return ::strcat(d, s);
}
template<> inline
wchar_t* strcat<wchar_t, wchar_t>(wchar_t* d, const wchar_t* s) noexcept {
    return ::wcscat(d, s);
}

template <typename src1_t, typename src2_t>
int strcmp(const src1_t* src1, const src2_t* src2) noexcept {
    // Same string or both nullptr
    if (src1 == src2) return 0;
    // One nullptr
    if (src1 == nullptr) return -1;
    if (src2 == nullptr) return 1;

    // All not nullptr
    while (*src1 && *src2) {
        if (*src1 > *src2)
            return 1;
        else if (*src1 < *src2)
            return -1;
        ++src1; ++src2;
    }
    // At least one reached null-terminator here
    if (*src1 != src1_t())
        return 1;
    else if (*src2 != src2_t())
        return -1;
    return 0;
}

template <> inline
int strcmp<char, char>(const char* src1, const char* src2) noexcept {
    return ::strcmp(src1, src2);
}
template <> inline
int strcmp<wchar_t, wchar_t>(const wchar_t* src1, const wchar_t* src2) noexcept {
    return ::wcscmp(src1, src2);
}

// Get a naive copy of a std::basic_string of another char type.
// No custom trait/allocator :(
template <class src_t, class des_t = char>
std::basic_string<des_t> xxstrcpy(std::basic_string_view<src_t> src) {
    std::basic_string<des_t> res;
    res.reserve(src.size());
    for (src_t ch : src)
        res.push_back(static_cast<des_t>(ch));
    return res;
}

/** @brief Convert a number to string
 * @tparam cal_t an arithmetic type. Either integers or floating points
 * @param [out] buf pre-allocated buffer to which the converted string write.
 * @param base base of the output string, defaulted to 10
 * @param digits Only used in floating-point types. Number of decimal digits written to buf.
 * @return @b JUST FOUND SOME BUGS DO NOT USE RETURN VALUE */
template<class cal_t, class char_t> 
char_t* to_char_t(char_t* buf, cal_t num, unsigned base = 10,
[[maybe_unused]] unsigned digits = 7) noexcept
{
    auto to_digit_val = [](cal_t ch) -> cal_t {
        if (ch >= 10) return static_cast<cal_t>(87 + ch);
        else return static_cast<cal_t>(48 + ch);
    };
    if (num == 0) {
        *buf = 48;
        return buf;
    } else if (num < 0)
        num = -num, *buf++ = 45;
    char_t* const ret = buf;
    if constexpr (std::is_integral<cal_t>::value) {
        char_t* bbuf = buf;
        while (num) {
            *buf++ = to_digit_val(num % base);
            num /= base;
        }
        buf--;
        while (bbuf < buf){
            auto tmp = *buf;
            *buf = *bbuf;
            *bbuf = tmp;
            bbuf++, buf--;
        }
    } else {
        int in_dig = std::log2(num) / std::log2(base);
        if (in_dig < 0) in_dig--;
        long long val = num * std::pow(base, digits - in_dig - 1);
        buf = ret + digits;
        while (buf != ret + 1) {
            *buf-- = to_digit_val(val % base);
            val /= base;
        }
        ret[1] = 46, ret[0] = to_digit_val(val); 
        if (in_dig != 0) {
            ret[digits + 1] = 101;
            to_char_t(ret + digits + 2, in_dig, base);
        }
    }
    return ret;
}

/*! @brief Parse a cstring from @p begin to @p end
* @param [in] begin The beginning of a string representing a number. 
* @param [in] end The end of a string representing a number. 
* @param base Base of the number the string represents, defaulted to 10.
* @param [out] res Result to save to.
* @return Length consumed in @c src
* @throw @c std::out_of_range when quoations do not bound 
* @note @p end is nullable (when @p begin is null-terminated) but @p begin is not */
template<class cal_t, class char_t> const char_t* 
from_char_t(const char_t* beg, const char_t* const end, cal_t& res, unsigned base = 10) noexcept
{
    auto get_digit_val = [](char_t ch, unsigned b) {
        if (ch >= 48 && ch <= 57) ch -= 48;
        else if (ch >= 65 && ch <= 90) ch -= 55;
        else if (ch >= 97 && ch <= 122) ch -= 87;
        if (static_cast<unsigned>(ch) >= b)
            return static_cast<cal_t>(36);
        return static_cast<cal_t>(ch);
    };
    bool is_neg = false, need_exp = false, has_point = false;
    res = cal_t();
    if (*beg == 45) beg++, is_neg = true; //-
    else if (*beg == 43) beg++; //+
    cal_t tmp = get_digit_val(*beg, base);
    while (tmp != 36 && beg != end) {
        (res *= base) += tmp;
        tmp = get_digit_val(*++beg, base);
    }
    [[maybe_unused]] cal_t pivot = 1;
    if (beg != end && *beg == 46) {
        has_point = true;
        tmp = get_digit_val(*++beg, base);
        while (tmp != 36 && beg != end) {
            if constexpr (!std::is_integral<cal_t>::value)
                res += (pivot /= base) * tmp;
            tmp = get_digit_val(*++beg, base);
        }
    }
    if (beg != end && base >= 15 && (beg[-1] == 69 || beg[-1] == 101)) {
        if (has_point) 
            res -= pivot * 15;
        else (res -= 15) /= base;
        if (res <= 1e-5) need_exp = true;
    }
    if (beg != end && base < 15 && (*beg == 69 || *beg == 101))
        need_exp = static_cast<bool>(res), beg++;
    if (need_exp) {
        int exp10;
        beg = from_char_t(beg, end, exp10, base);
        res *= static_cast<cal_t>(std::pow(10, exp10));
    }
    if (is_neg) res = -res;
    return beg;
}

/*! @brief Parse the next token from space-seperated cstring.
 * @param src The string.
 * @param src_size Size of string. @b '\0' has no special meaning.
 * @param buf Buffer at least buf_size bytes long. No NULL is appended.
 * @return Length consumed in @c src and length written to @c buf.
 * @throw @c std::out_of_range when quoations do not bound
 * @retval @c ~size_t(),~size_t() (second value) when buffer is not large enough */
template<class char_t>
std::pair<size_t, size_t>
next_token(const char_t* src, size_t src_size,
           char_t* buf, size_t buf_size)
{
    size_t src_at = 0;
    while (src_at < src_size && ::isspace(src[src_at]))
        src_at++; // lstrip

    char_t quote_delim = '\0'; size_t res_at = 0;
    while (quote_delim || (src_at < src_size && !::isspace(src[src_at]))) {
        // If inside a quotation, simply copy what is in the quote
        if (quote_delim != '\0') {
            while (src_at < src_size && res_at < buf_size && src[src_at] != quote_delim) {
                /* if (src[src_at] == '\\') {
                    // Handle '\\' inside quotation
                    if (src_at >= src_size - 1)
                        throw std::out_of_range("Escape at end of command");
                    buf[res_at++] = src[++src_at];
                    ++src_at;
                }
                else */ buf[res_at++] = src[src_at++];
            }
            if (src_at == src_size)
                throw std::out_of_range("Quotations do not bound");
            ++src_at;
            quote_delim = '\0';
            continue;
        }

        if (src_at == src_size)
            break;
        else if (res_at >= buf_size)
            return std::make_pair(~size_t(), ~size_t());
            // throw std::out_of_range("Buffer exhausted");

        switch (src[src_at]) {
        case '\'': case '\"': case '`':
            quote_delim = src[src_at]; break;
        case '\\':
            if (src_at >= src_size - 1)
                throw std::out_of_range("Escape at end of command");
            buf[res_at++] = src[++src_at];
            break;
        default:
            buf[res_at++] = src[src_at];
        }
        ++src_at;
    }

    while (src_at < src_size && ::isspace(src[src_at]))
        src_at++; // rstrip
    return std::make_pair(src_at, res_at);
}

template<class char_t>
std::pair<size_t, std::basic_string<char_t>>
next_token(const char_t* src, size_t src_size) {
    // 15 is the minimum of maximum small string optimization
    // length in major implementations.
    std::basic_string<char_t> res(15, char_t('\0'));
    size_t src_at = next_token(src, src_size, res.data(), res.size()).first;
    while (src_at == res.npos) {
        res.assign(res.size() << 1, '\0');
        src_at = next_token(src, src_size, res.data(), res.size()).first;
    }

    size_t res_len = res.find_last_not_of('\0');
    if (res_len != res.npos)
        res.erase(res_len + 1);
    return std::make_pair(src_at, res);
}

//! @brief Split a std string by the C string provided by spl.
//! @tparam container_t A container supporting `emplace_back` method 
//! (like @c `std::vector` ).
//! @return The container
template <typename container_t, typename char_t = char>
container_t str_split(const std::basic_string<char_t>& str,
                      const char_t* spl) noexcept
{
    container_t res = container_t();
    if (!spl || !spl[0] || str.empty())
        return res;
    size_t pos_before = 0, pos_after;
    const char_t* const _data = str.c_str();
    size_t splen = 0;
    while (spl[++splen]);
    while ((pos_after = str.find(spl, pos_before)) != str.npos) {
        res.emplace_back(_data + pos_before, pos_after - pos_before);
        pos_before = pos_after + splen;
    }
    res.emplace_back(_data + pos_before);
    return res;
}

}
