#pragma once
#include <string>
#include <cmath>
#include <cstring>
#include "charconv_t.hpp"

namespace orie {

/*! @brief Replace '%n' in a standard string with a C-style string.
* The '%n' can be anywhere between %0 ~ %2147483646 and do not have to be continous.
* @param [in out] str The string containing "%n"
* @param [in] rep The character to place into "%n" of @p str
* @return Reference to @p str 
* @note The smallest number will be replaced. 
* @example 
* std::string str("Best %1: 69420") 
* args(str, "Number") // str is Best Number: 69420 */
template <typename char_t>
std::basic_string<char_t>& args(std::basic_string<char_t>& str, const char_t* rep) noexcept
{
    int smallest = 2147483647;
    ptrdiff_t pctPos[50]; size_t pctNum = 0;
    const char_t* pbeg = str.c_str(), *pend = pbeg + str.size();
    while(*pbeg)
        if (*pbeg++ == 37) {
            int c;
            from_char_t(pbeg, pend, c, 10);
            if (c > 0 && c < smallest)
                smallest = c, pctNum = 0;
            if (c == smallest)
                pctPos[pctNum++] = pbeg - str.data() - 1;
        }
    if (smallest == 2147483647) return str;
    size_t len = 2;
    while (smallest /= 10) len++;
    size_t splen = 0;
    while (rep[++splen]);
    str.reserve(str.size() + splen * pctNum);
    while(pctNum--)
        str.replace(pctPos[pctNum], len, rep);
    return str; 
}
/*! @brief Replace '%n' in a standard string with a standard string.
* @see C string overload*/
template <typename char_t> inline std::basic_string<char_t>&
args(std::basic_string<char_t>& str, const std::basic_string<char_t>& rep) noexcept {
    return args(str, rep.c_str());
}

/*! @brief Replace '%n' in a standard string with a number.
* The number can be either integer or floting point.
* @tparam ValT Type of the number
* @see C string overload
* @example 
* std::string str("Best Number: %2") 
* args(str, 69420) // str is Best Number: 69420 */
template <typename char_t, typename ValT> inline std::basic_string<char_t>&
args(std::basic_string<char_t>& str, const ValT& rep) noexcept {
    char_t* buf[20] = {};
    return args(str, to_char_t(buf, rep, 10));
}

/*! @brief Replace '%n' in a standard string subsequently with @p varargin.
* @p varargin Arbitrary amount and type of numbers or strings
* @note Equivalent to invocations like `args(args(str, arg2), arg1)`
* @see C string overload
* @example 
* std::string str("Best %1: %2") 
* args(str, "Number", 69420) // str is Best Number: 69420 */
// 
template <typename char_t, typename ValT, typename ...VarArg> std::basic_string<char_t>&
args(std::basic_string<char_t>& str, const ValT& rep, const VarArg&... varargin) noexcept {
    args(str, rep);
    return args(str, varargin...);
}
//! @brief Split a std string by the C string provided by spl.
//! @tparam Cont A container supporting `emplace_back` method (by default @p `std::vector` ).
//! @return The container
template <typename Cont, typename char_t = char>
Cont str_split(const std::basic_string<char_t>& str, const char_t* spl) noexcept
{
    Cont res = Cont();
    if (!spl || !spl[0] || str.empty())
        return res;
    size_t posBefore = 0, posAfter;
    const char_t* const _data = str.c_str();
    size_t splen = 0;
    while (spl[++splen]);
    while ((posAfter = str.find(spl, posBefore)) != str.npos) {
        res.emplace_back(_data + posBefore, posAfter - posBefore);
        posBefore = posAfter + splen;
    }
    res.emplace_back(_data + posBefore);
    return res;
}

}
