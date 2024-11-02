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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef CBMC
#include "timer.h"
#define __CPROVER_loop_invariant(x)
#include "shmids.h"
#endif

#if !defined __AVR__ && !defined CBMC
TIMER *_timer;
double *run_time, // searching time
    *pre_time;    // preprocessing time
double m_run_time, m_pre_time;

#define BEGIN_PREPROCESSING                                                    \
  {                                                                            \
    timer_start(_timer);                                                       \
    start = clock();                                                           \
  }
#define BEGIN_SEARCHING                                                        \
  {                                                                            \
    timer_start(_timer);                                                       \
    start = clock();                                                           \
  }
#define END_PREPROCESSING                                                      \
  {                                                                            \
    timer_stop(_timer);                                                        \
    end = clock();                                                             \
    (*pre_time) = timer_elapsed(_timer) * 1000;                                \
  }
#define END_SEARCHING                                                          \
  {                                                                            \
    timer_stop(_timer);                                                        \
    end = clock();                                                             \
    (*run_time) = timer_elapsed(_timer) * 1000;                                \
  }

/* global variables used for computing preprocessing and searching times */
clock_t start, end;
#else // AVR || CBMC
#define BEGIN_PREPROCESSING
#define BEGIN_SEARCHING
#define END_PREPROCESSING
#define END_SEARCHING
#endif

static inline int search(unsigned char *p, int m, unsigned char *t, int n);

#if defined FUZZ

__AFL_FUZZ_INIT();
#pragma clang optimize off
#pragma GCC optimize("O0")
int main(void) {
  int m, n;
  unsigned char *p = NULL, *t = NULL;
  __AFL_INIT();
  p = __AFL_FUZZ_TESTCASE_BUF;
  n = (rand() % 5000) + 32;
  t = malloc(n + 1);
  for (int i = 0; i < n; i++)
    t[i] = rand() % 256;
  t[n] = '\0';
  while (__AFL_LOOP(10000)) {
    m = __AFL_FUZZ_TESTCASE_LEN;
    if (m > 1 && m < 64)
      search(p, m, t, n);
  }
  free(t);
  return 0;
}

#elif defined CBMC

#include <assert.h>
#ifndef MAX_M
#define MAX_M 8
#endif
#ifndef MAX_N
#define MAX_N 12
#endif
#undef XSIZE
#undef YSIZE
#define XSIZE MAX_M
#define YSIZE MAX_N

/* the brute force algorithm used for comparing occurrences */
static inline int bf_search(unsigned char *x, int m, unsigned char *y, int n) {
  assert(m < MAX_M);
  assert(n < MAX_N);
  int count = 0;
  for (int j = 0; j <= n - m; ++j)
    __CPROVER_loop_invariant(j <= MAX_N - MAX_M)
    __CPROVER_loop_invariant(j >= __CPROVER_loop_entry(j))
  {
    if (memcmp(x, &y[j], m) == 0)
      count++;
    //for (i = 0; i < m && x[i] == y[i + j]; ++i)
    //  ;
    //if (i >= m)
    //  count++;
  }
  return count;
}

unsigned char nondet_uchar();
int nondet_int();

int main(void) {
#define RANDCH(c)                                                              \
  {                                                                            \
    c = nondet_uchar();                                                        \
    __CPROVER_assume(c > 0 && c <= 255);                                       \
  }
  int m = MAX_M; // nondet_int();
#ifdef MIN_M
  __CPROVER_assume(m > MIN_M && m < MAX_M);
#else
  __CPROVER_assume(m > 0 && m < MAX_M);
#endif
  int n = MAX_N; // nondet_int();
  unsigned char P[MAX_M];
  unsigned char T[256];
  for (int i = 0; i < MAX_M; i++)
    __CPROVER_loop_invariant(i < MAX_M)
    RANDCH(P[i]);
  for (int i = 0; i < MAX_N; i++)
    __CPROVER_loop_invariant(i < MAX_N)
    RANDCH(T[i]);
  __CPROVER_input("P", P);
  //__CPROVER_input("m", m);
  __CPROVER_input("T", T);
  //__CPROVER_input("n", n);
  int occ, ref;

#define M_N_LOOP(m, n)                          \
  T[n] = '\0';                                  \
  P[m] = '\0';                                  \
  occ = search(P, m, T, n);                     \
  __CPROVER_output("occ", occ);                 \
  ref = bf_search(P, m, T, n);                  \
  __CPROVER_output("ref", ref);                 \
  __CPROVER_assert(ref == occ, "ref == occ")

  M_N_LOOP(0, MAX_N-1);
  M_N_LOOP(1, MAX_N-1);
  M_N_LOOP(2, MAX_N-1);
  M_N_LOOP(3, MAX_N-1);
  M_N_LOOP(4, MAX_N-1);
  M_N_LOOP(5, MAX_N-1);
  M_N_LOOP(6, MAX_N-1);
  M_N_LOOP(7, MAX_N-1);
  M_N_LOOP(MAX_M, MAX_N-1);
  M_N_LOOP(MAX_M, MAX_N); // includes the 0
  M_N_LOOP(7, 8);
  M_N_LOOP(7, MAX_N-2);

  strcpy(T, "8TOLWd903QRXd4QMPL665XT8dPY0b214VPQMNZNRK0::4bW");
  M_N_LOOP(2, 47);
  M_N_LOOP(4, 47);
  M_N_LOOP(8, 47);
}

#else

int main(int argc, char *argv[]) {
  int m, n;
  unsigned char *p = NULL, *t = NULL;
#ifndef __AVR__
  _timer = (TIMER *)malloc(sizeof(TIMER));
#endif

  if (argc > 1 && strncmp("shared", argv[1], 6) == 0) {
#ifndef HAVE_SHM
    return 1;
#endif
    if (argc < 7) {
      printf("error in input parameter\nseven parameters needed when used with "
             "shared memory\n");
      return 1;
    }
#ifdef HAVE_SHM
    key_t pkey = atoi(argv[2]);   // segment name for the pattern
    m = atoi(argv[3]);            // segment size for the pattern
    key_t tkey = atoi(argv[4]);   // segment name for the text
    n = atoi(argv[5]);            // segment size for the text
    key_t rkey = atoi(argv[6]);   // segment name for the occurrences
    key_t ekey = atoi(argv[7]);   // segment name for the running time
    key_t prekey = atoi(argv[8]); // preprocessing running time
    int *result;

    if (!(p = shmretrieve(shm_P, pkey, m)))
      return 1;
    if (!(t = shmretrieve(shm_T, tkey, n)))
      return 1;
    if (!(result = shmretrieve(shm_r, rkey, 4)))
      return 1;
    if (!(run_time = shmretrieve(shm_e, ekey, 8)))
      return 1;
    if (!(pre_time = shmretrieve(shm_pre, prekey, 8)))
      return 1;
    //fprintf(stderr, "%s\n", argv[0]);

    *result = search(p, m, t, n);
    return 0;
#endif
  } else {

    if (argc < 5) {
      printf("error in input parameter\nfour parameters needed in standard "
             "mode\n");
      return 1;
    }
    size_t lp = strlen(argv[1]);
    size_t lt = strlen(argv[3]);
    // for SSE code, esp. ssecp for int128 mu
#define PAD_16(x) (((x) + 15) & ~15)
    p = (unsigned char *)calloc(PAD_16(lp + 1), 1);
    //NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.strcpy)
    strcpy((char *)p, argv[1]);
    m = atoi(argv[2]);
    // we always guarantee zero-termination (+1)
    // and boyer-moore requires space at the end of t. (+m: tunbm)
    t = (unsigned char *)calloc(PAD_16(lt + m + 1), 1);
    //NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.strcpy)
    strcpy((char *)t, argv[3]);
    n = atoi(argv[4]);
    if (m > (int)lp)
      fprintf(stderr, "Invalid 2nd arg m=%d, should be <= %u\n", m,
              (unsigned)lp);
    if (n > (int)lt)
      fprintf(stderr, "Invalid 4nd arg n=%d, should be <= %u\n", n,
              (unsigned)lt);
    pre_time = &m_pre_time;
    run_time = &m_run_time;

    int occ = search(p, m, t, n);

    printf("pre_time: %f\nrun_time: %f\n", *pre_time, *run_time);
    printf("found %d occurrences\n", occ);
    free(p);
    free(t);
    return occ;
  }
}

#endif
