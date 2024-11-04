#!/bin/sh
id="$1"
if [ -z "$id" ] || [ ! -f "$id" ]; then
    echo arg $id not found. usage: fuzz-repro.sh fuzz/ac/default/crashes/id:000000,sig:11,src:000017,time:25,execs:1557,op:havoc,rep:2
    exit 1
fi

default="$(dirname $(dirname "$id"))"
algo="$(basename $(dirname "$default"))"
data="$default/data.t"

if [ -z "$data" ] || [ ! -f "$data" ]; then
    echo data.t $data not found in fuzz/$algo/default/data.t
fi

make -s SANITIZE=1 test-asan bin/asan/$algo
echo ./test-asan $algo --files "$id" "$data"
./test-asan $algo --files "$id" "$data" && rm -i -- "$id"

