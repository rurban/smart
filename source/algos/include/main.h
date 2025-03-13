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
TIMER *_pre_timer,
      *_run_timer;
double *pre_time, // sum preprocessing time
       *run_time; // sum searching time
double m_run_time, m_pre_time;

#define BEGIN_PREPROCESSING                                                    \
  {                                                                            \
    /* if we have nested PREPROCESSING, halt SEARCH, as with musl1 */          \
    if (_run_timer->start)                                                     \
      timer_stop(_run_timer);                                                  \
    timer_start(_pre_timer);                                                   \
    /*start = clock();*/                                                       \
  }
#define END_PREPROCESSING                                                      \
  {                                                                            \
    timer_stop(_pre_timer);                                                    \
    /*end = clock();*/                                                         \
    (*pre_time) += timer_elapsed(_pre_timer) * 1000;                           \
    /* if we have nested PREPROCESSING, continue SEARCH */                     \
    if (_run_timer->sum)                                                       \
      timer_start(_run_timer);                                                 \
  }
#define BEGIN_SEARCHING                                                        \
  {                                                                            \
    timer_start(_run_timer);                                                   \
    /*start = clock();*/                                                       \
  }
#define END_SEARCHING                                                          \
  {                                                                            \
    timer_stop(_run_timer);                                                    \
    /*end = clock();*/                                                         \
    (*run_time) += timer_elapsed(_run_timer) * 1000;                           \
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

#include "include/fuzz.h"

#elif defined CBMC

#include "include/cbmc.h"

#else

int main(int argc, char *argv[]) {
  int m, n;
  unsigned char *p = NULL, *t = NULL;
#ifndef __AVR__
  _pre_timer = (TIMER *)calloc(1, sizeof(TIMER));
  _run_timer = (TIMER *)calloc(1, sizeof(TIMER));
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
    m = atoi(argv[2]);
    n = atoi(argv[4]);
    //size_t lp = strlen(argv[1]);
    //size_t lt = strlen(argv[3]);
    // for SSE code, esp. ssecp for int128 mu
#define PAD_16(x) (((x) + 15) & ~15)
    p = (unsigned char *)calloc(PAD_16(m + 1), 1);
    memcpy((char *)p, argv[1], m);
    // we always guarantee zero-termination (+1)
    // and boyer-moore requires space at the end of t. (+m: tunbm)
    t = (unsigned char *)calloc(PAD_16(n + m + 1), 1);
    memcpy((char *)t, argv[3], n);
    /*
    if (m > (int)lp)
      fprintf(stderr, "Invalid 2nd arg m=%d, should be <= %u\n", m,
              (unsigned)lp);
    if (n > (int)lt)
      fprintf(stderr, "Invalid 4nd arg n=%d, should be <= %u\n", n,
              (unsigned)lt);
    */
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
