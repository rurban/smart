#!/bin/sh
set -x
#make -s clean
make algocfg
make -s

./select -backup -all

(sleep 2s; ./kill-tests.sh)&
./smart -text all -all -txt

./select -restore
