# μNEST Core Build Guide

Author: μNEST dev team,  http://iotee.io



Currently we only support compilation on Linux and macOS platform.

## Ubuntu 16.04/18.04 LTS (64-bit)

### Install Clang 4.0

```
sudo apt-get install software-properties-common
sudo apt-add-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main"
sudo apt-get update
sudo apt-get install clang-4.0 lldb-4.0 libclang-4.0-dev
```

### Install Dependencies

```
sudo apt-get install libboost-all-dev python-dev libbz2-dev  libssl-dev openssl libreadline-dev autoconf libtool git ntp doxygen libc++-dev libcurl4-openssl-dev
```

The Boost which ships with 了Ubuntu 16.04 LTS (64-bit) is version 1.58, so it is not necessary to build Boost by yourself and not necessary to set Boost environment variable.

### Install CMake 3.12

```
cd /tmp && wget https://cmake.org/files/v3.12/cmake-3.12.4-Linux-x86_64.sh
sudo mkdir -p /opt/cmake && chmod +x /tmp/cmake-3.12.4-Linux-x86_64.sh
sudo bash /tmp/cmake-3.12.4-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -sfT /opt/cmake/bin/cmake /usr/local/bin/cmake
```

Close current terminal and reopen a new terminal, or CMake may report error "could not find CMAKE_ROOT".

### Install LLVM 4.0 with WASM feature

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

### Install Berkeley DB

Sign up in Oracle web site and login to download the source code package:

http://download.oracle.com/otn/berkeley-db/db-18.1.25.tar.gz

```
tar -xzvf db-18.1.25.tar.gz
cd db-18.1.25/build_unix
../dist/configure --prefix=/usr --enable-cxx
make
sudo make install
```

### Build μNEST Core

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （XXX is desired branch name，e.g. master or develop etc.）
git submodule update --init --recursive

export WASM_ROOT=~/opt/wasm
export C_COMPILER=clang-4.0
export CXX_COMPILER=clang++-4.0

mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DOPENSSL_ROOT_DIR=/usr/include/openssl \
     -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DCMAKE_C_COMPILER="${C_COMPILER}" \
     -DOPENSSL_INCLUDE_DIR=/usr/include/openssl -DOPENSSL_LIBRARIES=/usr/lib/openssh -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

## macOS

### Install XCode

Install in macOS App Store. Refer to <https://guide.macports.org/#installing.xcode>.

If XCode command line tools can't run, please login to https://developer.apple.com/download/more to download XCode command line tools package and re-install it.

### Install Homebrew

Run the following command in macOS terminal. Refer to <https://brew.sh/>.

```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

### Install Dependencies

Supported Boost version: 1.57~1.65.1, supported OpenSSL version: 1.0.1, 1.0.2.

```
brew update
brew install boost@1.57 cmake git openssl autoconf automake berkeley-db libtool llvm@4 doxygen wget
```

After installation, run "ls -la /usr/local/opt/boost" in macOS terminal  to see which Boost version  the symbolic link /usr/local/opt/boost is linked to. Do the same with "ls -la "/usr/local/opt/openssl". In case they are not the desired versions, we can specify desired Boost/OpenSSL directory with specific version number in the following CMake command line. For example,  we specify "/usr/local/opt/boost@1.57" instead of "/usr/local/opt/boost".

### Install LLVM 4.0 with WASM feature

Install it into ~/opt/wasm.

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

### Build μNEST Core

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （XXX is desired branch name，e.g. master or develop etc.）
git submodule update --init --recursive

export WASM_ROOT=~/opt/wasm
export C_COMPILER=/usr/local/Cellar/llvm@4/4.0.1/bin/clang-4.0
export CXX_COMPILER=/usr/local/Cellar/llvm@4/4.0.1/bin/clang++

mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DCMAKE_C_COMPILER="${C_COMPILER}" -DBOOST_ROOT=/usr/local/opt/boost@1.57 -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib -DCMAKE_BUILD_TYPE=Release ..
make -j 4
```

## Setup testnet

Currently please refer to BitShares official doc: http://docs.bitshares.org/testnet/private-testnet.html



## Reference

https://github.com/bitshares/bitshares-core/wiki/BUILD_UBUNTU

https://github.com/bitshares/bitshares-core/wiki/Building-on-OS-X
