#!/bin/sh
set -x
#make -s clean
make -s algocfg
make -s
./build.sh

# best ranked algos for englishText. only those with all sizes
./select -backup -none simdkr epsm libc shc2 fhc2 qf33 qf34 lhc2 twfr2 qf28 lwfr2 fsbndmq42 libc1 qf26 bsdm3 twfrq2 fsbndmq41 qf43 qf24 hc2 fsbndmq32 twfrq3 fsbndmq31 qf62 sbndm2
# was tvsbs fjs hash8 so sbndm sbndm2 sbndm-bmh fndm sbndmq8 ufndmq2 dbww dbww2 fs-w2 lwfr5 qf43 sbndm-w2 tsa wfr2 twfrq2 twfrq4 wom libc1 musl1 simdkr epsm fhc3

./smart -text all -all -txt
./select -restore

exit 0

# pre-time + search-time
best25_english=<<'EOF'
  1. simdkr — 0.7492
  2. epsm — 0.7600
  3. libc — 0.8108
  4. shc2 — 0.8375
  5. fhc2 — 0.8600
  6. qf33 — 0.8650
  7. qf34 — 0.8667
  8. lhc2 — 0.8817
  9. twfr2 — 0.8833
  10. qf28 — 0.8917
  11. lwfr2 — 0.8992
  12. fsbndmq42 — 0.9000
  13. libc1 — 0.9042
  14. qf26 — 0.9050
  15. bsdm3 — 0.9150
  16. twfrq2 — 0.9150
  17. fsbndmq41 — 0.9158
  18. qf43 — 0.9225
  19. qf24 — 0.9342
  20. hc2 — 0.9350
  21. fsbndmq32 — 0.9358
  22. twfrq3 — 0.9375
  23. fsbndmq31 — 0.9400
  24. qf62 — 0.9417
  25. sbndm2 — 0.9417
EOF

best25_rand64=<<'EOF'
  1. libc — 0.6558
  2. simdkr — 0.6583
  3. fsbndmq21 — 0.7225
  4. fsbndm — 0.7250
  5. shc2 — 0.7425
  6. epsm — 0.7475
  7. sbndmq2 — 0.7483
  8. fs-w8 — 0.7492
  9. fhc2 — 0.7525
  10. sbndm2 — 0.7525
  11. fsbndmq20 — 0.7600
  12. bndmq2 — 0.7608
  13. bsdm2 — 0.7700
  14. lhc2 — 0.7758
  15. twfr2 — 0.7825
  16. qf26 — 0.7850
  17. fs-w6 — 0.7867
  18. shc1 — 0.7917
  19. bxs1 — 0.7942
  20. hc1 — 0.7942
  21. lhc1 — 0.7942
  22. lwfr2 — 0.7942
  23. fsbndm-w2 — 0.7975
  24. libc1 — 0.7975
  25. qf28 — 0.8000
EOF
