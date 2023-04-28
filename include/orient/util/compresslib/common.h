/**
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 * (c) Daniel Lemire, http://lemire.me/en/
 */
#ifndef SIMDCompressionAndIntersection_COMMON_H_
#define SIMDCompressionAndIntersection_COMMON_H_

extern "C" {
#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
#include <iso646.h>
#include <limits.h>

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <time.h>
}

#include <cmath>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

#include "platform.h"

#ifdef USE_ALIGNED
#define MM_LOAD_SI_128 _mm_load_si128
#define MM_STORE_SI_128 _mm_store_si128
#else
#define MM_LOAD_SI_128 _mm_loadu_si128
#define MM_STORE_SI_128 _mm_storeu_si128
#endif

namespace compressionLib {} // namespace compressionLib

#endif /* SIMDCompressionAndIntersection_COMMON_H_ */
