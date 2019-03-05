#!/bin/sh

cmake -DWASM_ROOT=$HOME/opt/wasm -DCMAKE_CXX_COMPILER=$HOME/opt/wasm/bin/clang++ -DCMAKE_C_COMPILER=$HOME/opt/wasm/bin/clang -DOPENSSL_ROOT_DIR=/usr/include/openssl -DOPENSSL_INCLUDE_DIR=/usr/include/openssl -DBOOST_ROOT=~/opt/boost -DCMAKE_BUILD_TYPE=Release ..
make -j2 gxc-abigen
