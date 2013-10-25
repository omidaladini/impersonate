#!/bin/bash -e

GCC=${1:-gcc}

$GCC -fPIC -DNOMAIN -shared impersonate.c -o impersonate.so -ldl
xxd -i impersonate.so  > impersonate.so.xxd
$GCC -static -fPIC impersonate.c -o impersonate -ldl

rm ./impersonate.so.xxd
rm ./impersonate.so
