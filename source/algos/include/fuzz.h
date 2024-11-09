__AFL_FUZZ_INIT();
#include <string.h>
#include <unistd.h>
#pragma clang optimize off
#pragma GCC optimize("O0")
int main(void) {
  int m, n;
  unsigned char *p = NULL, *t = NULL;
  char procname[80];
  char fn[80];
  FILE *f;

#ifdef PRETTY_RANDCH
#define RANDCH(c)                                                              \
  do {                                                                         \
    c = (rand() % 74) + '0';                                                   \
  } while (!isalnum(c))
#else
#define RANDCH(c) c = rand() % 256
#endif

  __AFL_INIT();
  p = __AFL_FUZZ_TESTCASE_BUF;
  n = (rand() % 120) + 32;
  t = malloc(n + 1);
  for (int i = 0; i < n; i++)
    RANDCH(t[i]);
  t[n] = '\0';

  // TODO dump only on crash, with the crash id
  snprintf(fn, sizeof(fn), "/proc/%d/comm", getpid());
  f = fopen(fn, "r");
  fgets(procname, sizeof(procname), f);
  fclose(f);
  procname[strcspn(procname, "\n")] = 0;
  snprintf(fn, sizeof(fn), "fuzz/%s/default/data.t", procname);
  f = fopen(fn, "w");
  fwrite(t, n, 1, f);
  fclose(f);

  const int min_m =
#if MIN_M
    MIN_M
#else
    2
#endif
    ;
  const int max_m =
#if MAX_M
    MAX_M
#else
    63
#endif
    ;

  while (__AFL_LOOP(10000)) {
    m = __AFL_FUZZ_TESTCASE_LEN;
    if (m > n)
      continue;
    if (m >= min_m && m <= max_m)
      search(p, m, t, n);
  }
  free(t);
  return 0;
}
