#include <assert.h>
#ifndef MIN_M
#define MIN_M 0
#endif
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

/* for MAX_N > 12 */
static inline int bf_search_large(unsigned char *x, int m, unsigned char *y, int n) {
  assert(m < MAX_M);
  int count = 0;
  for (int j = 0; j <= n - m; ++j)
    __CPROVER_loop_invariant(j <= n - MAX_M)
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

#define M_N_LOOP_LARGE(m, n)                    \
  T[n] = '\0';                                  \
  P[m] = '\0';                                  \
  occ = search(P, m, T, n);                     \
  __CPROVER_output("occ", occ);                 \
  ref = bf_search(P, m, T, n);                  \
  __CPROVER_output("ref", ref);                 \
  __CPROVER_assert(ref == occ, "ref == occ")

#if MIN_M == 0
  M_N_LOOP(0, MAX_N-1);
#endif
#if MIN_M >= 1 || MIN_M == 0
  M_N_LOOP(1, MAX_N-1);
#endif  
#if MIN_M >= 2 || MIN_M == 0
  M_N_LOOP(2, MAX_N-1);
#endif
#if MIN_M >= 3 || MIN_M == 0
  M_N_LOOP(3, MAX_N-1);
#endif
#if MIN_M >= 4 || MIN_M == 0
  M_N_LOOP(4, MAX_N-1);
#endif
#if MIN_M >= 5 || MIN_M == 0
  M_N_LOOP(5, MAX_N-1);
#endif
#if MIN_M >= 6 || MIN_M == 0
  M_N_LOOP(6, MAX_N-1);
#endif
#if MIN_M >= 7 || MIN_M == 0
  M_N_LOOP(7, MAX_N-1);
#endif
#if MIN_M <= MAX_M || MIN_M == 0
  M_N_LOOP(MAX_M, MAX_N-1);
  M_N_LOOP(MAX_M, MAX_N); // includes the 0
#endif
#if MIN_M >= 7 || MIN_M == 0
  M_N_LOOP(7, 8);
  M_N_LOOP(7, MAX_N-2);
#endif

#if MAX_M == 0 || MAX_M > 47
  strcpy(T, "8TOLWd903QRXd4QMPL665XT8dPY0b214VPQMNZNRK0::4bW");
#if MIN_M >= 2 || MIN_M == 0
  M_N_LOOP_LARGE(2, 47);
#endif
#if MIN_M >= 4 || MIN_M == 0
  M_N_LOOP_LARGE(4, 47);
#endif
#if MIN_M >= 8 || MIN_M == 0
  M_N_LOOP_LARGE(8, 47);
#endif
#if MIN_M >= 20 || MIN_M == 0
  M_N_LOOP_LARGE(20, 47);
#endif
#endif
}
