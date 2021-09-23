#!/bin/bash

export INCLUDE_DIR="-I`pwd`/../ladder/include -I/usr/local/include -I`pwd`"
export LIB_DIR="-L`pwd`/../ladder/build -L/usr/local/lib -L/usr/local/lib64 -lpthread -lladder -lprotobuf -lz ../proto/tests.pb.cc"

SUBS=$@

if [ $# -eq 0 ]; then
    SUBS="logger server proto_server event_poller"
fi

for sub in $SUBS
do
    cd $sub
    echo "Building test_$sub"
    g++ -o test_${sub}.out test_${sub}.cpp ${INCLUDE_DIR} ${LIB_DIR} -std=c++11
    cd ..
done

source ./envrc
