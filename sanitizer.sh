#!/bin/sh
make SANITIZE=1
echo test all, not just the working algos
for t in `./algocfg good` `./algocfg good 0`
do
    ./test-asan "$t"
done
echo now test the broken asan algos with some full smart test
make -s algocfg
for t in `./algocfg good 0`
do
    ./test-asan "$t" rand32
done
