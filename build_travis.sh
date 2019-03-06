#!/bin/bash

cd $TRAVIS_BUILD_DIR/contracts/musl/upstream && git checkout eosio
cd $TRAVIS_BUILD_DIR && mkdir -p build && cd build

if [[ $TRAVIS_OS_NAME == linux ]]; then
	cmake -DWASM_ROOT=$HOME/opt/wasm -DLLVM_DIR=$HOME/opt/wasm/lib/cmake/llvm -DCMAKE_CXX_COMPILER=$HOME/opt/wasm/bin/clang++ -DCMAKE_C_COMPILER=$HOME/opt/wasm/bin/clang -DOPENSSL_ROOT_DIR=/usr/include/openssl -DOPENSSL_INCLUDE_DIR=/usr/include/openssl -DBOOST_ROOT=$HOME/opt/boost -DCMAKE_BUILD_TYPE=Release ..
elif [[ $TRAVIS_OS_NAME == osx ]]; then
	cmake -DWASM_ROOT=$HOME/opt/wasm -DLLVM_DIR=$HOME/opt/wasm/lib/cmake/llvm -DCMAKE_CXX_COMPILER=/usr/local/Cellar/llvm/7.0.1/bin/clang++ -DCMAKE_C_COMPILER=/usr/local/Cellar/llvm/7.0.1/bin/clang -DBOOST_ROOT=$HOME/opt/boost -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib -DCMAKE_BUILD_TYPE=Release ..
fi

make -j4 witness_node
