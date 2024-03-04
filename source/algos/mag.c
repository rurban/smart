/*
 * SMART: string matching algorithms research tool.
 *
 * IN WORK: conversion to C
 * from https://github.com/rsusik/mag/blob/master/mag.cpp
 */

#include "include/define.h"
#include "include/main.h"
#include <stdint.h>

#define NEW_SIGMA 5

// how many bytes can be compared (pattern_size - q)
int pelem_compar_bytes;
// patterns in text form
uchar patterns2[40000][500];

// data structures
typedef struct pelem {
  uchar *pattern; // shifed pattern
  uchar *op;      // pattern
  int offset;     // offset of shifted pattern
} pelem_;

typedef struct {
  pelem *list; // list of patterns that have the same index (first q-gram)
  int count;   // number of patterns
} plist;
// patterns indexed by first qgram
plist *indexed_patterns = NULL;
// number of patterns
long patterns_number = 0;

// pelem comparator
int pelem_compar(const pelem *a, const pelem *b) {
  return memcmp(a->pattern, b->pattern, pelem_compar_bytes);
}

int CreateMapByHistogramBinPack(uint8_t *t, int n) {
  // Map array creation
  int sigma = 0;
  unsigned long symbols[256];
  unsigned long *symbolsp[256];
  long sum = 0;
  int binSize = n / (NEW_SIGMA - 1);
  for (int i = 0; i < 256; ++i) {
    symbols[i] = 0;
    symbolsp[i] = &(symbols[i]);
  }
  for (long i = 0; i < n; ++i)
    symbols[t[i]]++;

  std::sort(symbolsp, &(symbolsp[255]), CreateMapByHistogram_compar);
  // symbolsp is now most common symbols (the most at first position)

  for (int i = 0; i < 256; ++i)
    map[i] = 0;
  for (int i = 0; i < 256; ++i) {
    sum += *symbolsp[i];
    if (sum >= binSize) {
      sum = sum - binSize;
      *symbolsp[i] = sigma++;
    } else {
      *symbolsp[i] = sigma;
    }
  }

  for (int i = 0; i < 256; ++i) {
    map[i] = symbols[i];
  }

  // for(unsigned int i = 0; i < 256; i++) { printf("map[%c (%d) ] = %d\n",
  // (char)i, i, map[i]); }

  return ++sigma;
}

#define Q 1
#define SIGMA 5
void mag_build_indexed_patterns(void) {
  uint8_t *pattern = 0;
  for (long l = 0; l < pattern_text_size; l += pattern_size) {
    pattern = patterns_text + l;
    memcpy(patterns2[patterns_number], pattern, pattern_size);
    for (int i = 0; i < Q; i++) {
      unsigned long long int index =
          get_<Q, SIGMA>::gram((patterns2[patterns_number] + i), 1);
      indexed_patterns[index].list =
          (pelem *)realloc(indexed_patterns[index].list,
                           sizeof(pelem) * (indexed_patterns[index].count + 1));
      indexed_patterns[index].list[indexed_patterns[index].count].pattern =
          patterns2[patterns_number] + i;
      indexed_patterns[index].list[indexed_patterns[index].count].op =
          patterns2[patterns_number];
      indexed_patterns[index].list[indexed_patterns[index].count].offset = i;
      indexed_patterns[index].count++;
    }
    ++patterns_number;
  }

  for (int i = 0; i < get_super_<Q, SIGMA>::sigma(); ++i) {
    if (indexed_patterns[i].count > 0) {
      qsort(indexed_patterns[i].list, indexed_patterns[i].count, sizeof(pelem),
            (int (*)(const void *, const void *))pelem_compar);
    }
  }
}

int search(unsigned char *x, int m, unsigned char *y, int n) {

#define q_size 1

  /* Preprocessing */
  BEGIN_PREPROCESSING
  sigma = CreateMapByHistogramBinPack(x, n);
  const int super_sigma = sigma;
  /*
        q_size == 1 ? sigma
      : q_size == 2 ? sigma * sigma
      : q_size == 3 ? sigma * sigma * sigma
      : q_size == 4 ? sigma * sigma * sigma * sigma
      : q_size == 5 ? sigma * sigma * sigma * sigma * sigma
      : q_size == 6 ? sigma * sigma * sigma * sigma * sigma * sigma
      : q_size == 7 ? sigma * sigma * sigma * sigma * sigma * sigma * sigma
      : q_size == 8
          ? sigma * sigma * sigma * sigma * sigma * sigma * sigma * sigma
      : q_size == 9 ? sigma * sigma * sigma * sigma * sigma * sigma * sigma *
                          sigma * sigma
      : q_size == 10 ? sigma * sigma * sigma * sigma * sigma * sigma * sigma *
                           sigma * sigma * sigma
                     : 0;
  */

  size_t RAM = super_sigma * sizeof(plist) + 10000 * sizeof(pelem) +
               10000 * pattern_size + text_size;
  if (RAM > 2000000000) {
    fprintf(stderr, "Error: There is not enough RAM memory available ( %zd )\n",
            RAM);
    exit(1);
  }
  if (super_sigma > 100000000 || super_sigma == 0) {
    fprintf(
        stderr,
        "Error: Q is too big for choosen sigma size { q=%d, super_sigma=%d }\n",
        q_size, super_sigma);
    exit(1);
  }

  indexed_patterns = (plist *)calloc(super_sigma, sizeof(plist));

  for (int i = 0; i < super_sigma; ++i) {
    indexed_patterns[i].list = (pelem *)calloc(1, sizeof(pelem));
  }

  mag_build_indexed_patterns() // sigma=5, q_size=1

      for (unsigned int l = 0; l < log2cache; ++l) log2v[l] = ilog2((ldouble)l);
  indexes = (bool *)calloc(text_size, BOOL_SIZE);
  END_PREPROCESSING

  /* Searching */
  BEGIN_SEARCHING
  int *b = (int *)calloc(get_super_<Q, SIGMA>::sigma(), sizeof(int));
  unsigned d, mm;
  int i, j, h, r, foo = U * K * Q, *vt;
  uint8_t *tt;

  if (patt_size > (int)MAX_WORD_SIZE) {
    printf("Error: patt_size > MAX_WORD_SIZE");
    exit(EXIT_FAILURE);
  }

  for (i = 0; i < get_super_<Q, SIGMA>::sigma(); i++)
    b[i] = ~(int)0;

  for (j = h = 0, mm = (int)0; j < K; j++) {
    for (i = 0; i < patt_size / K; i++) {
      for (int z = 0; z < patterns_number; ++z) {
        for (int offset = 0; offset < Q;
             offset++) { // Can be replaced by switch
          b[get_<Q, SIGMA>::gram((&patterns2[z][i * K * Q + j * Q + offset]), 1)]
            &= ~((int)1 << (h));
        }
      }
      ++h;
    }
    for (r = 0; r < U; r++, h++)
      mm = mm | (1U << (h - 1));
    --h;
  }

  if (h > (int)MAX_WORD_SIZE) {
    printf("Error: h=%d > %d\n", h, (int)MAX_WORD_SIZE);
    exit(1);
  }
  for (i = 0; i < get_super_<Q, SIGMA>::sigma(); i++)
    b[i] &= ~(mm & (mm << 1));

  vt = build_verify_table(patt_size, K, U, 1);
  d = ~mm;
  i = text_size;
  tt = text;
  do {
    mag_<U, U, Q, K, SIGMA>::get_d(d, tt, b);

    if ((d & mm) != mm)
      mag_verification<Q, SIGMA>(d, mm, tt - text - K * Q + foo, vt);
    d &= ~mm;
    tt += foo;
  } while ((i -= foo) > 0);

  free(b);
  free(vt);

  for (int i = 0; i < super_sigma; ++i) {
    free(indexed_patterns[i].list);
  }
  free(indexed_patterns);
  free(patterns_text);
  free(text);
  free(indexes);
  END_SEARCHING
  return count;
}
