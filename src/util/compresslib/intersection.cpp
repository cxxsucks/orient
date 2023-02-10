/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */

#include <orient/util/compresslib/intersection.h>

namespace compressionLib {

/**
 * More or less from
 * http://highlyscalable.wordpress.com/2012/06/05/fast-intersection-sorted-lists-sse/
 */
const static __m128i shuffle_mask[16] = {
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 7, 6, 5, 4),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 11, 10, 9, 8),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 11, 10, 9, 8, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 11, 10, 9, 8, 7, 6, 5, 4),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 15, 14, 13, 12),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 15, 14, 13, 12, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 15, 14, 13, 12, 7, 6, 5, 4),
    _mm_set_epi8(15, 14, 13, 12, 15, 14, 13, 12, 7, 6, 5, 4, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 15, 14, 13, 12, 11, 10, 9, 8),
    _mm_set_epi8(15, 14, 13, 12, 15, 14, 13, 12, 11, 10, 9, 8, 3, 2, 1, 0),
    _mm_set_epi8(15, 14, 13, 12, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4),
    _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
};
// precomputed dictionary

/**
 * Version optimized by D. Lemire
 * starting from
 * http://highlyscalable.wordpress.com/2012/06/05/fast-intersection-sorted-lists-sse/
 *
 * The main difference is that we break the data dependency and maximizes
 * superscalar execution.
 *
 * It is not safe for out to be either A or B.
 */
size_t i32_intersect(const uint32_t *A, const size_t s_a,
                      const uint32_t *B, const size_t s_b, uint32_t *out) 
{
  assert(out != A);
  assert(out != B);
  const uint32_t *const initout(out);
  size_t i_a = 0, i_b = 0;
  const static uint32_t cyclic_shift1 = _MM_SHUFFLE(0, 3, 2, 1);
  const static uint32_t cyclic_shift2 = _MM_SHUFFLE(1, 0, 3, 2);
  const static uint32_t cyclic_shift3 = _MM_SHUFFLE(2, 1, 0, 3);

  // trim lengths to be a multiple of 4
  size_t st_a = (s_a / 4) * 4;
  size_t st_b = (s_b / 4) * 4;
  if (i_a < st_a && i_b < st_b) {
    __m128i v_a, v_b;
    v_a = MM_LOAD_SI_128((__m128i *)&A[i_a]);
    v_b = MM_LOAD_SI_128((__m128i *)&B[i_b]);
    while (true) {
      const __m128i cmp_mask1 =
          _mm_cmpeq_epi32(v_a, v_b); // pairwise comparison
      const __m128i cmp_mask2 = _mm_cmpeq_epi32(
          v_a, _mm_shuffle_epi32(v_b, cyclic_shift1)); // again...
      __m128i cmp_mask = _mm_or_si128(cmp_mask1, cmp_mask2);
      const __m128i cmp_mask3 = _mm_cmpeq_epi32(
          v_a, _mm_shuffle_epi32(v_b, cyclic_shift2)); // and again...
      cmp_mask = _mm_or_si128(cmp_mask, cmp_mask3);
      const __m128i cmp_mask4 = _mm_cmpeq_epi32(
          v_a, _mm_shuffle_epi32(v_b, cyclic_shift3)); // and again.
      cmp_mask = _mm_or_si128(cmp_mask, cmp_mask4);
      // convert the 128-bit mask to the 4-bit mask
      const int mask = _mm_movemask_ps(*reinterpret_cast<__m128 *>(&cmp_mask));
      // copy out common elements
      const __m128i p = _mm_shuffle_epi8(v_a, shuffle_mask[mask]);

      _mm_storeu_si128((__m128i *)out, p);
      out += _mm_popcnt_u32(mask); // a number of elements is a weight of the mask

      const uint32_t a_max = A[i_a + 3];
      if (a_max <= B[i_b + 3]) {
        i_a += 4;
        if (i_a >= st_a)
          break;
        v_a = MM_LOAD_SI_128((__m128i *)&A[i_a]);
      }
      if (a_max >= B[i_b + 3]) {
        i_b += 4;
        if (i_b >= st_b)
          break;
        v_b = MM_LOAD_SI_128((__m128i *)&B[i_b]);
      }
    }
  }

  // intersect the tail using scalar intersection
  while (i_a < s_a && i_b < s_b) {
    if (A[i_a] < B[i_b]) {
      i_a++;
    } else if (B[i_b] < A[i_a]) {
      i_b++;
    } else {
      *out++ = B[i_b];
      i_a++;
      i_b++;
    }
  }

  return out - initout;
}

} // namespace SIMDCompressionLib
