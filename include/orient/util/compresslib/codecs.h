/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */

#ifndef SIMDCompressionAndIntersection_CODECS_H_
#define SIMDCompressionAndIntersection_CODECS_H_

#include "common.h"
#include "util.h"

namespace compressionLib {

struct NotEnoughStorage : public std::runtime_error {
  size_t required; // number of 32-bit symbols required
  NotEnoughStorage(const size_t req) : runtime_error(""), required(req) {}
};

struct IntegerCODEC {
  /**
   * You specify input and input length, as well as
   * output and output length. nvalue gets modified to
   * reflect how much was used. If the new value of
   * nvalue is more than the original value, we can
   * consider this a buffer overrun.
   *
   * You are responsible for allocating the memory (length
   * for *in and nvalue for *out).
   */
  virtual void encodeArray(uint32_t *in, const size_t length, uint32_t *out,
                           size_t &nvalue) = 0;

  /**
   * Usage is similar to encodeArray except that it returns a pointer
   * incremented from in. In theory it should be in+length. If the
   * returned pointer is less than in+length, then this generally means
   * that the decompression is not finished (some scheme compress
   * the bulk of the data one way, and they then they compress remaining
   * integers using another scheme).
   *
   * As with encodeArray, you need to have length elements allocated
   * for *in and at least nvalue elements allocated for out. The value
   * of the variable nvalue gets updated with the number actually used
   * (if nvalue exceeds the original value, there might be a buffer
   * overrun).
   */
  virtual const uint32_t *decodeArray(const uint32_t *in, const size_t length,
                                      uint32_t *out, size_t &nvalue) = 0;
  virtual ~IntegerCODEC() {}
};

/******************
 * This just copies the data, no compression.
 */
class JustCopy : public IntegerCODEC {
public:
  void encodeArray(uint32_t *in, const size_t length, uint32_t *out,
                   size_t &nvalue) {
    memcpy(out, in, sizeof(uint32_t) * length);
    nvalue = length;
  }
  // like encodeArray, but we don't actually copy
  void fakeencodeArray(const uint32_t * /*in*/, const size_t length,
                       size_t &nvalue) {
    nvalue = length;
  }

  const uint32_t *decodeArray(const uint32_t *in, const size_t length,
                              uint32_t *out, size_t &nvalue) {
    memcpy(out, in, sizeof(uint32_t) * length);
    nvalue = length;
    return in + length;
  }
};

} // namespace compressionLib

#endif /* SIMDCompressionAndIntersection_CODECS_H_ */
