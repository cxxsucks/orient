/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */

#include <orient/util/compresslib/integratedbitpacking.h>

namespace compressionLib {

void __integratedfastunpack0(const uint32_t initoffset,
                             const uint32_t *__restrict__,
                             uint32_t *__restrict__ out) {
  for (uint32_t i = 0; i < 32; ++i)
    *(out++) = initoffset;
}
void __integratedfastpack0(const uint32_t, const uint32_t *__restrict__,
                           uint32_t *__restrict__) {}

void __integratedfastunpack32(const uint32_t, const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  for (int k = 0; k < 32; ++k)
    out[k] = in[k]; // no sense in wasting time with deltas
}

void __integratedfastpack32(const uint32_t, const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  for (int k = 0; k < 32; ++k)
    out[k] = in[k]; // no sense in wasting time with deltas
}

void __integratedfastunpack2(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 2);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28) % (1U << 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack3(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 3);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 1)) << (3 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 2)) << (3 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26) % (1U << 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack5(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 5);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 3)) << (5 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 1)) << (5 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 4)) << (5 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 2)) << (5 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack6(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 6);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 4)) << (6 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 2)) << (6 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 4)) << (6 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 2)) << (6 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack7(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 7);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 3)) << (7 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 6)) << (7 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 2)) << (7 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 5)) << (7 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 1)) << (7 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 4)) << (7 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack9(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 9);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 4)) << (9 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 8)) << (9 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 3)) << (9 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 7)) << (9 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 2)) << (9 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 6)) << (9 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 1)) << (9 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 5)) << (9 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack10(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 10);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 8)) << (10 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 6)) << (10 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 4)) << (10 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 2)) << (10 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 8)) << (10 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 6)) << (10 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 4)) << (10 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 2)) << (10 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack11(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 11);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 1)) << (11 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 2)) << (11 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 3)) << (11 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 4)) << (11 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 5)) << (11 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 6)) << (11 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 7)) << (11 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 8)) << (11 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 9)) << (11 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 10)) << (11 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack12(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 12);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 4)) << (12 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 8)) << (12 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 4)) << (12 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 8)) << (12 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 4)) << (12 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 8)) << (12 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 4)) << (12 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 8)) << (12 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack13(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 13);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 7)) << (13 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 1)) << (13 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 8)) << (13 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 2)) << (13 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 9)) << (13 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 3)) << (13 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 10)) << (13 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 4)) << (13 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 11)) << (13 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 5)) << (13 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 12)) << (13 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 6)) << (13 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack14(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 14);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 10)) << (14 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 6)) << (14 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 2)) << (14 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 12)) << (14 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 8)) << (14 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 4)) << (14 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 10)) << (14 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 6)) << (14 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 2)) << (14 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 12)) << (14 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 8)) << (14 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 4)) << (14 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack15(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 15);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 15) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 13)) << (15 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 11)) << (15 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 9)) << (15 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 7)) << (15 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 5)) << (15 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 3)) << (15 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 1)) << (15 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 14)) << (15 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 12)) << (15 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 10)) << (15 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 8)) << (15 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 6)) << (15 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 4)) << (15 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 2)) << (15 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack17(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 17);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 2)) << (17 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 4)) << (17 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 6)) << (17 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 8)) << (17 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 10)) << (17 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 12)) << (17 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 14)) << (17 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 16)) << (17 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 1)) << (17 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 3)) << (17 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 5)) << (17 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 7)) << (17 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 9)) << (17 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 11)) << (17 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 13)) << (17 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13) % (1U << 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 15)) << (17 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack18(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 18);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 4)) << (18 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 8)) << (18 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 12)) << (18 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 16)) << (18 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 2)) << (18 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 6)) << (18 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 10)) << (18 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 14)) << (18 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 4)) << (18 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 8)) << (18 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 12)) << (18 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 16)) << (18 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 2)) << (18 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 6)) << (18 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 10)) << (18 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 14)) << (18 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack19(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 19);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 6)) << (19 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 12)) << (19 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 18)) << (19 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 5)) << (19 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 11)) << (19 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 17)) << (19 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 4)) << (19 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 10)) << (19 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 16)) << (19 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 3)) << (19 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 9)) << (19 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 15)) << (19 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 2)) << (19 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 8)) << (19 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 14)) << (19 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 1)) << (19 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 7)) << (19 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 13)) << (19 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack20(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 20);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 8)) << (20 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 16)) << (20 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 4)) << (20 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 12)) << (20 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 8)) << (20 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 16)) << (20 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 4)) << (20 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 12)) << (20 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 8)) << (20 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 16)) << (20 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 4)) << (20 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 12)) << (20 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 8)) << (20 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 16)) << (20 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 4)) << (20 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 12)) << (20 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack21(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 21);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 10)) << (21 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 20)) << (21 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 9)) << (21 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 19)) << (21 - 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 8)) << (21 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 18)) << (21 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 7)) << (21 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 17)) << (21 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 6)) << (21 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 16)) << (21 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 5)) << (21 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 15)) << (21 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 4)) << (21 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 14)) << (21 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 3)) << (21 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 13)) << (21 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  ++in;
  *out |= (*in % (1U << 2)) << (21 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 12)) << (21 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 1)) << (21 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 11)) << (21 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack22(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 22);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 12)) << (22 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 2)) << (22 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 14)) << (22 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 4)) << (22 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 16)) << (22 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 6)) << (22 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 18)) << (22 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 8)) << (22 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 20)) << (22 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 10)) << (22 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 12)) << (22 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 2)) << (22 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 14)) << (22 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 4)) << (22 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 16)) << (22 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 6)) << (22 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 18)) << (22 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 8)) << (22 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 20)) << (22 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 10)) << (22 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack23(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 23);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 14)) << (23 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 5)) << (23 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 19)) << (23 - 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 10)) << (23 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 1)) << (23 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 15)) << (23 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 6)) << (23 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 20)) << (23 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 11)) << (23 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11);
  ++in;
  *out |= (*in % (1U << 2)) << (23 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 16)) << (23 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 7)) << (23 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 21)) << (23 - 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 12)) << (23 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 3)) << (23 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 17)) << (23 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 8)) << (23 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 22)) << (23 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 13)) << (23 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  ++in;
  *out |= (*in % (1U << 4)) << (23 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 18)) << (23 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 9)) << (23 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack24(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 24);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 16)) << (24 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 8)) << (24 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack25(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 25);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 18)) << (25 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 11)) << (25 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11);
  ++in;
  *out |= (*in % (1U << 4)) << (25 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 22)) << (25 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 15)) << (25 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 8)) << (25 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 1)) << (25 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 19)) << (25 - 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 12)) << (25 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 5)) << (25 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5) % (1U << 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 23)) << (25 - 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 16)) << (25 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 9)) << (25 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9);
  ++in;
  *out |= (*in % (1U << 2)) << (25 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 20)) << (25 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 13)) << (25 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  ++in;
  *out |= (*in % (1U << 6)) << (25 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6) % (1U << 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 24)) << (25 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 17)) << (25 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 10)) << (25 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 3)) << (25 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 21)) << (25 - 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 14)) << (25 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 7)) << (25 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack26(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 26);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 20)) << (26 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 14)) << (26 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 8)) << (26 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 2)) << (26 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 22)) << (26 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 16)) << (26 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 10)) << (26 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 4)) << (26 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 24)) << (26 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 18)) << (26 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 12)) << (26 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 6)) << (26 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 20)) << (26 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 14)) << (26 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 8)) << (26 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 2)) << (26 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 22)) << (26 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 16)) << (26 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 10)) << (26 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 4)) << (26 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 24)) << (26 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 18)) << (26 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 12)) << (26 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 6)) << (26 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack27(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 27);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 22)) << (27 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 17)) << (27 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 12)) << (27 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 7)) << (27 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7);
  ++in;
  *out |= (*in % (1U << 2)) << (27 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 27);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 24)) << (27 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 19)) << (27 - 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 14)) << (27 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 9)) << (27 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9);
  ++in;
  *out |= (*in % (1U << 4)) << (27 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4) % (1U << 27);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 26)) << (27 - 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 21)) << (27 - 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 16)) << (27 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 11)) << (27 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11);
  ++in;
  *out |= (*in % (1U << 6)) << (27 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  ++in;
  *out |= (*in % (1U << 1)) << (27 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 27);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 23)) << (27 - 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 18)) << (27 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 13)) << (27 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  ++in;
  *out |= (*in % (1U << 8)) << (27 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 3)) << (27 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3) % (1U << 27);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 25)) << (27 - 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 20)) << (27 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 15)) << (27 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 10)) << (27 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 5)) << (27 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack28(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 28);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 24)) << (28 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 20)) << (28 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 16)) << (28 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 12)) << (28 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 8)) << (28 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 4)) << (28 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 24)) << (28 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 20)) << (28 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 16)) << (28 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 12)) << (28 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 8)) << (28 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 4)) << (28 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 24)) << (28 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 20)) << (28 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 16)) << (28 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 12)) << (28 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 8)) << (28 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 4)) << (28 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 24)) << (28 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 20)) << (28 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 16)) << (28 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 12)) << (28 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 8)) << (28 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 4)) << (28 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack29(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 29);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 26)) << (29 - 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 23)) << (29 - 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 20)) << (29 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 17)) << (29 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 14)) << (29 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 11)) << (29 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11);
  ++in;
  *out |= (*in % (1U << 8)) << (29 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 5)) << (29 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5);
  ++in;
  *out |= (*in % (1U << 2)) << (29 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2) % (1U << 29);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 28)) << (29 - 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 25)) << (29 - 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 22)) << (29 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 19)) << (29 - 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 16)) << (29 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 13)) << (29 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  ++in;
  *out |= (*in % (1U << 10)) << (29 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 7)) << (29 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7);
  ++in;
  *out |= (*in % (1U << 4)) << (29 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out |= (*in % (1U << 1)) << (29 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1) % (1U << 29);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 27)) << (29 - 27);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 24)) << (29 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 21)) << (29 - 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 18)) << (29 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 15)) << (29 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 12)) << (29 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 9)) << (29 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9);
  ++in;
  *out |= (*in % (1U << 6)) << (29 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  ++in;
  *out |= (*in % (1U << 3)) << (29 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack30(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 30);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 28)) << (30 - 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 26)) << (30 - 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 24)) << (30 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 22)) << (30 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 20)) << (30 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 18)) << (30 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 16)) << (30 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 14)) << (30 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 12)) << (30 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 10)) << (30 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 8)) << (30 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 6)) << (30 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  ++in;
  *out |= (*in % (1U << 4)) << (30 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out |= (*in % (1U << 2)) << (30 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2);
  ++in;
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 0) % (1U << 30);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 28)) << (30 - 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 26)) << (30 - 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 24)) << (30 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 22)) << (30 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 20)) << (30 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 18)) << (30 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 16)) << (30 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 14)) << (30 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 12)) << (30 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 10)) << (30 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 8)) << (30 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 6)) << (30 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  ++in;
  *out |= (*in % (1U << 4)) << (30 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out |= (*in % (1U << 2)) << (30 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack31(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *out = (*in >> 0) % (1U << 31);
  *out += initoffset; // integrated delta decoding
  out++;
  *out = (*in >> 31);
  ++in;
  *out |= (*in % (1U << 30)) << (31 - 30);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 30);
  ++in;
  *out |= (*in % (1U << 29)) << (31 - 29);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 29);
  ++in;
  *out |= (*in % (1U << 28)) << (31 - 28);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 28);
  ++in;
  *out |= (*in % (1U << 27)) << (31 - 27);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 27);
  ++in;
  *out |= (*in % (1U << 26)) << (31 - 26);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 26);
  ++in;
  *out |= (*in % (1U << 25)) << (31 - 25);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 25);
  ++in;
  *out |= (*in % (1U << 24)) << (31 - 24);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 24);
  ++in;
  *out |= (*in % (1U << 23)) << (31 - 23);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 23);
  ++in;
  *out |= (*in % (1U << 22)) << (31 - 22);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 22);
  ++in;
  *out |= (*in % (1U << 21)) << (31 - 21);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 21);
  ++in;
  *out |= (*in % (1U << 20)) << (31 - 20);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 20);
  ++in;
  *out |= (*in % (1U << 19)) << (31 - 19);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 19);
  ++in;
  *out |= (*in % (1U << 18)) << (31 - 18);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 18);
  ++in;
  *out |= (*in % (1U << 17)) << (31 - 17);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 17);
  ++in;
  *out |= (*in % (1U << 16)) << (31 - 16);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 16);
  ++in;
  *out |= (*in % (1U << 15)) << (31 - 15);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 15);
  ++in;
  *out |= (*in % (1U << 14)) << (31 - 14);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 14);
  ++in;
  *out |= (*in % (1U << 13)) << (31 - 13);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 13);
  ++in;
  *out |= (*in % (1U << 12)) << (31 - 12);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 12);
  ++in;
  *out |= (*in % (1U << 11)) << (31 - 11);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 11);
  ++in;
  *out |= (*in % (1U << 10)) << (31 - 10);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 10);
  ++in;
  *out |= (*in % (1U << 9)) << (31 - 9);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 9);
  ++in;
  *out |= (*in % (1U << 8)) << (31 - 8);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 8);
  ++in;
  *out |= (*in % (1U << 7)) << (31 - 7);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 7);
  ++in;
  *out |= (*in % (1U << 6)) << (31 - 6);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 6);
  ++in;
  *out |= (*in % (1U << 5)) << (31 - 5);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 5);
  ++in;
  *out |= (*in % (1U << 4)) << (31 - 4);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 4);
  ++in;
  *out |= (*in % (1U << 3)) << (31 - 3);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 3);
  ++in;
  *out |= (*in % (1U << 2)) << (31 - 2);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 2);
  ++in;
  *out |= (*in % (1U << 1)) << (31 - 1);
  *out += out[-1]; // integrated delta decoding
  out++;
  *out = (*in >> 1);
  *out += out[-1]; // integrated delta decoding
}

void __integratedfastunpack1(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *out = (*in & 1) + initoffset;
  ++out;
  *out = ((*in >> 1) & 1) + out[-1];
  ++out;
  for (uint32_t i = 2; i < 32; i += 1) {
    *out = ((*in >> i) & 1) + out[-1];
    ++i;
    ++out;
    *out = ((*in >> i) & 1) + out[-1];
    ++out;
  }
}

void __integratedfastunpack4(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *(out++) = (*in % (1U << 4)) + initoffset;
  for (uint32_t i = 4; i < 32; i += 4) {
    *out = ((*in >> i) % (1U << 4)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 4) {
    *out = ((*in >> i) % (1U << 4)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 4) {
    *out = ((*in >> i) % (1U << 4)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 4) {
    *out = ((*in >> i) % (1U << 4)) + out[-1];
    ++out;
  }
}

void __integratedfastunpack8(const uint32_t initoffset,
                             const uint32_t *__restrict__ in,
                             uint32_t *__restrict__ out) {
  *(out++) = (*in % (1U << 8)) + initoffset;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 8) {
    *out = ((*in >> i) % (1U << 8)) + out[-1];
    ++out;
  }
}

void __integratedfastunpack16(const uint32_t initoffset,
                              const uint32_t *__restrict__ in,
                              uint32_t *__restrict__ out) {
  *(out++) = (*in % (1U << 16)) + initoffset;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
  ++in;
  for (uint32_t i = 0; i < 32; i += 16) {
    *out = ((*in >> i) % (1U << 16)) + out[-1];
    ++out;
  }
}

void __integratedfastpack2(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 18;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 20;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 22;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 24;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 26;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 28;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 18;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 20;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 22;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 24;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 26;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 2)) << 28;
  ++in;
  *out |= ((*in - in[-1])) << 30;
}

void __integratedfastpack3(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 3;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 9;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 15;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 18;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 21;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 24;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 27;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 3)) >> (3 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 7;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 13;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 19;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 22;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 25;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 28;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 3)) >> (3 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 5;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 11;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 17;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 20;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 23;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 3)) << 26;
  ++in;
  *out |= ((*in - in[-1])) << 29;
}

void __integratedfastpack5(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 5;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 15;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 20;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 25;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 5)) >> (5 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 3;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 13;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 18;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 23;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 5)) >> (5 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 11;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 21;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 26;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 5)) >> (5 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 9;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 19;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 24;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 5)) >> (5 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 7;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 17;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 5)) << 22;
  ++in;
  *out |= ((*in - in[-1])) << 27;
}

void __integratedfastpack6(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 18;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 24;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 6)) >> (6 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 22;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 6)) >> (6 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 18;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 24;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 6)) >> (6 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 22;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 6)) >> (6 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 6)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 26;
}

void __integratedfastpack7(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 7;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 14;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 21;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 7)) >> (7 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 3;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 17;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 24;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 7)) >> (7 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 13;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 7)) >> (7 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 9;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 16;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 23;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 7)) >> (7 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 5;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 19;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 7)) >> (7 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 15;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 22;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 7)) >> (7 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 11;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 7)) << 18;
  ++in;
  *out |= ((*in - in[-1])) << 25;
}

void __integratedfastpack9(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 9;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 18;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 13;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 22;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 17;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 3;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 12;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 21;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 7;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 11;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 15;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 19;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 9)) >> (9 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 5;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 9)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 23;
}

void __integratedfastpack10(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 18;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 10;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 18;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 10)) >> (10 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 10)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 22;
}

void __integratedfastpack11(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 11);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 11;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 13;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 3;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 15;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 5;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 6;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 17;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 7;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 18;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 8;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 19;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 9;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 20;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 11)) >> (11 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 11)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 21;
}

void __integratedfastpack12(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 12)) >> (12 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 12)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 20;
}

void __integratedfastpack13(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 13);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 13;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 7;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 15;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 9;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 3;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 4;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 17;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 11);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 11;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 5;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 18;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 13)) >> (13 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 13)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 19;
}

void __integratedfastpack14(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 14);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 14);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 2;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 14)) >> (14 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 14)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 18;
}

void __integratedfastpack15(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 15);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 15;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 13);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 13;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 11);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 11;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 9;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 7;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 5;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 1;
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 16;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 14);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 15)) >> (15 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 15)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 17;
}

void __integratedfastpack17(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 14);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 14;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 5;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 7;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 9;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 11);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 11;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 13);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 17)) << 13;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 17)) >> (17 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
}

void __integratedfastpack18(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 18)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 18)) >> (18 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
}

void __integratedfastpack19(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 12);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 12;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 5;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 11);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 11;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 9;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 19)) << 7;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 19)) >> (19 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
}

void __integratedfastpack20(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 20)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 20)) >> (20 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
}

void __integratedfastpack21(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 21);
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 10);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 10;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 9);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 9;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 7;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 5;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 21)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 21)) >> (21 - 11);
  ++in;
  *out |= ((*in - in[-1])) << 11;
}

void __integratedfastpack22(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 22)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 22)) >> (22 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
}

void __integratedfastpack23(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 23);
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 5;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 11);
  ++in;
  *out |= ((*in - in[-1])) << 11;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 7);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 7;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 21);
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 8);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 8;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 23)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 23)) >> (23 - 9);
  ++in;
  *out |= ((*in - in[-1])) << 9;
}

void __integratedfastpack24(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 24)) >> (24 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
}

void __integratedfastpack25(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 25);
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 11);
  ++in;
  *out |= ((*in - in[-1])) << 11;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 25)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 25)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 5);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 25)) << 5;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 23);
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 9);
  ++in;
  *out |= ((*in - in[-1])) << 9;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 25)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 6);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 25)) << 6;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 25)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 21);
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 25)) >> (25 - 7);
  ++in;
  *out |= ((*in - in[-1])) << 7;
}

void __integratedfastpack26(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 26)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 26)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 26)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 26)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 26)) >> (26 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
}

void __integratedfastpack27(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 27);
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 7);
  ++in;
  *out |= ((*in - in[-1])) << 7;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 27)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 9);
  ++in;
  *out |= ((*in - in[-1])) << 9;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 4);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 27)) << 4;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 21);
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 11);
  ++in;
  *out |= ((*in - in[-1])) << 11;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 27)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 23);
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 3);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 27)) << 3;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 25);
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 27)) >> (27 - 5);
  ++in;
  *out |= ((*in - in[-1])) << 5;
}

void __integratedfastpack28(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 28)) >> (28 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
}

void __integratedfastpack29(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 29);
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 23);
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 11);
  ++in;
  *out |= ((*in - in[-1])) << 11;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 5);
  ++in;
  *out |= ((*in - in[-1])) << 5;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 2);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 29)) << 2;
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 25);
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 7);
  ++in;
  *out |= ((*in - in[-1])) << 7;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 1);
  ++in;
  *out |= ((*in - in[-1]) % (1U << 29)) << 1;
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 27);
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 21);
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 9);
  ++in;
  *out |= ((*in - in[-1])) << 9;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
  ++out;
  *out = ((*in - in[-1]) % (1U << 29)) >> (29 - 3);
  ++in;
  *out |= ((*in - in[-1])) << 3;
}

void __integratedfastpack30(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 30);
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 2);
  ++in;
  *out |= ((*in - in[-1])) << 2;
  ++out;
  ++in;
  *out = (*in - in[-1]) % (1U << 30);
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  *out = ((*in - in[-1]) % (1U << 30)) >> (30 - 2);
  ++in;
  *out |= ((*in - in[-1])) << 2;
}

void __integratedfastpack31(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = (*in - initoffset) % (1U << 31);
  ++in;
  *out |= ((*in - in[-1])) << 31;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 30);
  ++in;
  *out |= ((*in - in[-1])) << 30;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 29);
  ++in;
  *out |= ((*in - in[-1])) << 29;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 28);
  ++in;
  *out |= ((*in - in[-1])) << 28;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 27);
  ++in;
  *out |= ((*in - in[-1])) << 27;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 26);
  ++in;
  *out |= ((*in - in[-1])) << 26;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 25);
  ++in;
  *out |= ((*in - in[-1])) << 25;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 24);
  ++in;
  *out |= ((*in - in[-1])) << 24;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 23);
  ++in;
  *out |= ((*in - in[-1])) << 23;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 22);
  ++in;
  *out |= ((*in - in[-1])) << 22;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 21);
  ++in;
  *out |= ((*in - in[-1])) << 21;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 20);
  ++in;
  *out |= ((*in - in[-1])) << 20;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 19);
  ++in;
  *out |= ((*in - in[-1])) << 19;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 18);
  ++in;
  *out |= ((*in - in[-1])) << 18;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 17);
  ++in;
  *out |= ((*in - in[-1])) << 17;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 16);
  ++in;
  *out |= ((*in - in[-1])) << 16;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 15);
  ++in;
  *out |= ((*in - in[-1])) << 15;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 14);
  ++in;
  *out |= ((*in - in[-1])) << 14;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 13);
  ++in;
  *out |= ((*in - in[-1])) << 13;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 12);
  ++in;
  *out |= ((*in - in[-1])) << 12;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 11);
  ++in;
  *out |= ((*in - in[-1])) << 11;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 10);
  ++in;
  *out |= ((*in - in[-1])) << 10;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 9);
  ++in;
  *out |= ((*in - in[-1])) << 9;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 8);
  ++in;
  *out |= ((*in - in[-1])) << 8;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 7);
  ++in;
  *out |= ((*in - in[-1])) << 7;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 6);
  ++in;
  *out |= ((*in - in[-1])) << 6;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 5);
  ++in;
  *out |= ((*in - in[-1])) << 5;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 4);
  ++in;
  *out |= ((*in - in[-1])) << 4;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 3);
  ++in;
  *out |= ((*in - in[-1])) << 3;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 2);
  ++in;
  *out |= ((*in - in[-1])) << 2;
  ++out;
  *out = ((*in - in[-1]) % (1U << 31)) >> (31 - 1);
  ++in;
  *out |= ((*in - in[-1])) << 1;
}

/*assumes that integers fit in the prescribed number of bits*/
void __integratedfastpack1(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = *(in++) - initoffset;
  for (uint32_t i = 1; i < 32; i += 1) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
}

/*assumes that integers fit in the prescribed number of bits*/
void __integratedfastpack4(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = *(in++) - initoffset;
  for (uint32_t i = 4; i < 32; i += 4) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 4; i < 32; i += 4) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 4; i < 32; i += 4) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 4; i < 32; i += 4) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
}

/*assumes that integers fit in the prescribed number of bits*/
void __integratedfastpack8(const uint32_t initoffset,
                           const uint32_t *__restrict__ in,
                           uint32_t *__restrict__ out) {
  *out = *(in++) - initoffset;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 8; i < 32; i += 8) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
}

/*assumes that integers fit in the prescribed number of bits*/
void __integratedfastpack16(const uint32_t initoffset,
                            const uint32_t *__restrict__ in,
                            uint32_t *__restrict__ out) {
  *out = *(in++) - initoffset;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
  *out = *in - in[-1];
  ++in;
  for (uint32_t i = 16; i < 32; i += 16) {
    *out |= (*in - in[-1]) << i;
    ++in;
  }
  ++out;
}

} // namespace compressionLib
