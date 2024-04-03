#!/bin/sh
set -x
make -s clean
make -s

./select -backup -all
./smart -text all -all -txt
./select -restore
