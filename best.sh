#!/bin/sh
set -x
#make -s clean
make -s algocfg
make -s

./select -backup -none tvsbs fjs hash8 so sbndm sbndm2 sbndm-bmh fndm sbndmq8 ufndmq2 dbww dbww2 fs-w2 lwfr5 qf43 sbndm-w2 tsa wfr2 twfrq2 twfrq4 wom libc1 musl1 simdkr epsm
./smart -text all -all -txt
./select -restore
