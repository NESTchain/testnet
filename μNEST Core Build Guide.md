# μNEST Core Build Guide

Author: μNEST dev team,  http://iotee.io



Currently we only support compilation on Linux and macOS platform.

## Ubuntu 16.04/18.04 LTS (64-bit)

### Install Clang 7.0.1+

Download the pre-built binaries from official site: http://releases.llvm.org, extract it and add its bin directory  to the front of PATH.

```
cd ~
wget http://releases.llvm.org/7.0.1/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
tar zxvf clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
export PATH=~/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04/bin:$PATH
```

### Install Dependencies

```
sudo apt-get install libboost-all-dev python-dev libbz2-dev  libssl-dev openssl libreadline-dev autoconf libtool git ntp doxygen libc++-dev libcurl4-openssl-dev
```

### Install CMake 3.12

```
cd /tmp && wget https://cmake.org/files/v3.12/cmake-3.12.4-Linux-x86_64.sh
sudo mkdir -p /opt/cmake && chmod +x /tmp/cmake-3.12.4-Linux-x86_64.sh
sudo bash /tmp/cmake-3.12.4-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -sfT /opt/cmake/bin/cmake /usr/local/bin/cmake
```

Close current terminal and reopen a new terminal, or CMake may report error "could not find CMAKE_ROOT".

### Install LLVM 7.0.1+ with WASM feature

Install it to ~/opt/wasm.

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_ENABLE_RTTI=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DLLVM_TARGETS_TO_BUILD="X86" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

Add ~/opt/wasm/bin to the front of PATH. From now on we will use our customized LLVM, instead of official pre-built LLVM.

```
export PATH=~/opt/wasm/bin:$PATH
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

### Install Boost 1.69

Install it to ~/opt/boost。Boost version >= 1.69.

```
export BOOST_ROOT=~/opt/boost

cd ~ && wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz -O  boost_1_69_0.tar.gz
tar -zxvf boost_1_69_0.tar.gz && cd boost_1_69_0 && chmod +x bootstrap.sh
./bootstrap.sh --with-toolset=clang --prefix=${BOOST_ROOT}
./b2 -j 4 stage release
./b2 install --prefix=${BOOST_ROOT}
```

### Build μNEST Core

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （XXX is desired branch name，e.g. master or develop etc.）
git submodule update --init --recursive
cd contracts/musl/upstream && git checkout eosio

export WASM_ROOT=~/opt/wasm
export BOOST_ROOT=~/opt/boost
export C_COMPILER=clang
export CXX_COMPILER=clang++

cd ~/testnet && mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DOPENSSL_ROOT_DIR=/usr/include/openssl \
     -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DCMAKE_C_COMPILER="${C_COMPILER}" \
     -DOPENSSL_INCLUDE_DIR=/usr/include/openssl -DBOOST_ROOT=${BOOST_ROOT} -DCMAKE_BUILD_TYPE=Release ..
make -j4
```



## macOS

### Install XCode

Install in macOS App Store. Refer to <https://guide.macports.org/#installing.xcode>.

If XCode command line tools(e.g. xcrun etc.) can't run, please login to https://developer.apple.com/download/more to download XCode command line tools *.dmg package and re-install it.

### Install Homebrew

Run the following command in macOS terminal. Refer to <https://brew.sh/>.

```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

### Install Dependencies

Supported OpenSSL version: 1.0.1, 1.0.2. Currently we will get LLVM 7.0.1 with "llvm@7", add its bin directory  to the front of PATH.

```
brew update
brew install cmake git openssl autoconf automake berkeley-db libtool llvm@7 doxygen wget

export PATH=/usr/local/Cellar/llvm/7.0.1/bin:$PATH
```

After installation, run "ls -la /usr/local/opt/openssl" in macOS terminal  to see which OpenSSL version  the symbolic link /usr/local/opt/openssl is linked to. In case it is not the desired version, we can specify desired OpenSSL directory with specific version number in the following CMake command line. For example,  we specify "/usr/local/opt/openssl@1.0" instead of "/usr/local/opt/openssl".

### Install LLVM 7.0.1+ with WASM feature

Install it into ~/opt/wasm.

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_ENABLE_RTTI=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

### Install Boost 1.69

Install it to ~/opt/boost.

```
export BOOST_ROOT=~/opt/boost

cd ~ && wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz -O  boost_1_69_0.tar.gz
tar -zxvf boost_1_69_0.tar.gz && cd boost_1_69_0 && chmod +x bootstrap.sh

./bootstrap.sh --prefix=${BOOST_ROOT}
./b2 -j 4 --buildtype=complete install --prefix=${BOOST_ROOT} toolset=clang cxxflags="-arch x86_64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -I/usr/local/opt/llvm@7/include" linkflags="-arch x86_64  -L/usr/local/opt/llvm@7/lib"
```

### Build μNEST Core

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （XXX is desired branch name，e.g. master or develop etc.）
git submodule update --init --recursive
cd contracts/musl/upstream && git checkout eosio

export WASM_ROOT=~/opt/wasm
export BOOST_ROOT=~/opt/boost
export C_COMPILER=clang
export CXX_COMPILER=clang++

cd ~/testnet && mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
	-DCMAKE_C_COMPILER="${C_COMPILER}" -DBOOST_ROOT=${BOOST_ROOT} \
	-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
	-DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include \
	-DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib \
	-DCMAKE_BUILD_TYPE=Release ..
make -j 4
```

## Setup testnet

Currently please refer to BitShares official doc: http://docs.bitshares.org/testnet/private-testnet.html



## Reference

https://github.com/bitshares/bitshares-core/wiki/BUILD_UBUNTU

https://github.com/bitshares/bitshares-core/wiki/Building-on-OS-X
