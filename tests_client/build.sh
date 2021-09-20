#!/bin/bash

export INCLUDE_DIR="-I`pwd`/../ladder_client/include -I`pwd`/../ladder/include -I/usr/local/include"
export LIB_DIR="-L`pwd`/../ladder_client/build -lladder_client -L`pwd`/../ladder/build -lladder -L/usr/local/lib -lpthread -L/usr/local/lib64 -lprotobuf"

SUBS=$@

if [ $# -eq 0 ]; then
    SUBS="client mass_clients timer tcp_client"
fi

for sub in $SUBS
do
    cd ${sub}
    echo "Building test_${sub}"
    g++ -o test_${sub}.out test_${sub}.cpp ${INCLUDE_DIR} ${LIB_DIR} -std=c++11 -g
    cd ..
done

source ./envrc
