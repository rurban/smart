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

  __AFL_INIT();
  p = __AFL_FUZZ_TESTCASE_BUF;
  n = (rand() % 5000) + 32;
  t = malloc(n + 1);
  for (int i = 0; i < n; i++)
    t[i] = rand() % 256;
  t[n] = '\0';

#if 1
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
#endif

  while (__AFL_LOOP(10000)) {
    m = __AFL_FUZZ_TESTCASE_LEN;
    if (m > 1 && m < 64)
      search(p, m, t, n);
  }
  free(t);
  return 0;
}
