#!/bin/sh
set -x
make -s clean
make -s

./select -backup -none tvsbs fjs hash8 so sbndm sbndm2 sbndm-bmh fndm fsbndm sbndmq8 ufndmq2 ufndmq8 kbndm fs-w2 lwfr5 qf43 sbndm-w2 wfr2 twfrq2 twfrq4 wom libc1 musl1 simdkr epsm
./smart -text all -all -txt
./select -restore
