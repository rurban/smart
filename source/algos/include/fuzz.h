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
