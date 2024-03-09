/*
 * SMART: string matching algorithms research tool.
 * Copyright (C) 2012  Simone Faro and Thierry Lecroq
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * contact the authors at: faro@dmi.unict.it, thierry.lecroq@univ-rouen.fr
 * download the tool at: http://www.dmi.unict.it/~faro/smart/
 *
 * This is an SIMD-optimized implementation of the Karp-Rabin algorithm.
 * https://mattsills.github.io/2024/03/02/rabin-karp/
 */

#include "include/define.h"
#include "include/main.h"
#define REHASH(a, b, h) ((((h) - (a)*d) << 1) + (b))

#if defined __x86_64__ && defined __AVX2__
#include <stdint.h>
#include <inttypes.h>
#include <memory.h>
#include <immintrin.h>

static inline __m256i constant_ymm(uint32_t x) {
    return _mm256_set_epi32(x, x, x, x, x, x, x, x);
}

// Byte extractors
static inline __m256i byte0(__m256i bytes) {
    return _mm256_srai_epi32(_mm256_slli_epi32(bytes, 24), 24);
}

static inline __m256i byte1(__m256i bytes) {
    return _mm256_srai_epi32(_mm256_slli_epi32(bytes, 16), 24);
}

static inline __m256i byte2(__m256i bytes) {
    return _mm256_srai_epi32(_mm256_slli_epi32(bytes, 8), 24);
}

static inline __m256i byte3(__m256i bytes) {
    return _mm256_srai_epi32(bytes, 24);
}

size_t karprabin_rolling4_chunked_streaming_8x4_avx2(const char *data,
                                                     size_t len, size_t N,
                                                     uint32_t B,
                                                     uint32_t target) {
  uint32_t BtoN = 1;
  for (size_t i = 0; i < N; i++) {
    BtoN *= B;
  }

  size_t counter = 0;
#ifdef BETTER_POPCOUNT
  __m256i counts = _mm256_setzero_si256();
#endif
  size_t subblock_size = 4 * N;
  size_t block_size = 8 * subblock_size;

  __m256i targets = constant_ymm(target);
  __m256i bs = constant_ymm(B);
  __m256i bns = constant_ymm(BtoN);
  __m256i offsets =
      _mm256_set_epi32(7 * subblock_size, 6 * subblock_size, 5 * subblock_size,
                       4 * subblock_size, 3 * subblock_size, 2 * subblock_size,
                       subblock_size, 0);

  __m256i hashes = _mm256_setzero_si256();
  const char *block_start = data;
  while ((block_start - data) + block_size + N < len) {
    // Initialize hashes
    hashes = _mm256_setzero_si256();

    for (size_t i = 0; i < N; i++) {
      __m256i as = _mm256_srai_epi32(
          _mm256_slli_epi32(_mm256_i32gather_epi32(
                                (const int *)(block_start + i), offsets, 1),
                            24),
          24);
      hashes = _mm256_add_epi32(_mm256_mullo_epi32(hashes, bs), as);
    }

    uint32_t first = _mm256_extract_epi32(hashes, 0);
    if (first == target) {
      counter++;
    }

    for (size_t i = 0; i < subblock_size; i += 4) {
      // Values to be added in
      __m256i as = _mm256_i32gather_epi32((const int *)(block_start + i + N),
                                          offsets, 1);
      // Values to be dropped off
      __m256i ans =
          _mm256_i32gather_epi32((const int *)(block_start + i), offsets, 1);

      // Value 0
      hashes = _mm256_sub_epi32(
          _mm256_add_epi32(_mm256_mullo_epi32(hashes, bs), byte0(as)),
          _mm256_mullo_epi32(byte0(ans), bns));
#ifdef BETTER_POPCOUNT
      matches = mm256_cmpeq_epi32(hashes, targets);
      counts = _mm256_sub_epi32(counts, matches);
#else
      counter += __builtin_popcount(_mm256_cmpeq_epi32_mask(hashes, targets));
#endif
      // Value 1
      hashes = _mm256_sub_epi32(
          _mm256_add_epi32(_mm256_mullo_epi32(hashes, bs), byte1(as)),
          _mm256_mullo_epi32(byte1(ans), bns));
#ifdef BETTER_POPCOUNT
      matches = mm256_cmpeq_epi32(hashes, targets);
      counts = _mm256_sub_epi32(counts, matches);
#else
      counter += __builtin_popcount(_mm256_cmpeq_epi32_mask(hashes, targets));
#endif
      // Value 2
      hashes = _mm256_sub_epi32(
          _mm256_add_epi32(_mm256_mullo_epi32(hashes, bs), byte2(as)),
          _mm256_mullo_epi32(byte2(ans), bns));
#ifdef BETTER_POPCOUNT
      matches = mm256_cmpeq_epi32(hashes, targets);
      counts = _mm256_sub_epi32(counts, matches);
#else
      counter += __builtin_popcount(_mm256_cmpeq_epi32_mask(hashes, targets));
#endif
      // Value 3
      hashes = _mm256_sub_epi32(
          _mm256_add_epi32(_mm256_mullo_epi32(hashes, bs), byte3(as)),
          _mm256_mullo_epi32(byte3(ans), bns));
#ifdef BETTER_POPCOUNT
      matches = mm256_cmpeq_epi32(hashes, targets);
      counts = _mm256_sub_epi32(counts, matches);
#else
      counter += __builtin_popcount(_mm256_cmpeq_epi32_mask(hashes, targets));
#endif
    }

#ifdef BETTER_POPCOUNT
    counter += _mm256_extract_epi32(counts, 0)
            + _mm256_extract_epi32(counts, 1);
            + _mm256_extract_epi32(counts, 2);
            + _mm256_extract_epi32(counts, 3);
#endif
    block_start += block_size + 1;
  }

  // Deal with what's left over
  size_t last_end = (block_start - data) + N - 1;
  uint32_t hash = _mm256_extract_epi32(hashes, 7);
  for (size_t i = last_end; i < len; i++) {
    hash = hash * B + data[i] - BtoN * data[i - N];
    if (hash == target) {
      counter++;
    }
  }

  return counter;
}
#endif

// FIXME #if defined __x86_64__ && defined __AVX2__
int search(unsigned char *x, int m, unsigned char *y, int n) {
  unsigned int d, hx, hy;
  int i, j, count;

  BEGIN_PREPROCESSING
  count = 0;
  /* Preprocessing */
  for (d = i = 1; i < m; ++i)
    d = (d << 1);

  for (hy = hx = i = 0; i < m; ++i) {
    hx = ((hx << 1) + x[i]);
    hy = ((hy << 1) + y[i]);
  }
  END_PREPROCESSING

  /* Searching */
  BEGIN_SEARCHING
  j = 0;
  while (j <= n - m) {
    if (hx == hy && memcmp(x, y + j, m) == 0)
      OUTPUT(j);
    hy = REHASH(y[j], y[j + m], hy);
    ++j;
  }
  END_SEARCHING
  return count;
}
