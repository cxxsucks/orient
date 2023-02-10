#pragma once
#include "compositecodec.h"
#include "fastpfor.h"
#include "variablebyte.h"

namespace compressionLib {

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
                      const uint32_t *B, const size_t s_b, uint32_t *out);

using fastPForCodec = CompositeCodec<FastPFor<8, true>, VariableByte<true>>;

} // namespace SIMDCompressionLib
