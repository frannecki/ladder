#!/bin/sh
rm ./*/*.out

export INCLUDE_DIR="-I`pwd`/../ladder/include -I/usr/local/include"
export LIB_DIR="-L`pwd`/../ladder/build -L/usr/local/lib -L/usr/local/lib64 -lladder"

for sub in client
do
    cd ${sub}
    g++ -o test_${sub}.out test_${sub}.cpp ${INCLUDE_DIR} ${LIB_DIR} -std=c++11
    cd ..
done

source ./envrc
