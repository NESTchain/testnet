# μNEST的Core组件编译说明

作者：μNEST开发组，http://iotee.io



目前只支持在Linux、macOS上编译。

## Ubuntu 16.04/18.04 LTS (64-bit)

### 安装Clang 4.0

```
sudo apt-get install software-properties-common
sudo apt-add-repository "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-4.0 main"
sudo apt-get update
sudo apt-get install clang-4.0 lldb-4.0 libclang-4.0-dev
```

### 安装依赖的包

```
sudo apt-get install libboost-all-dev python-dev libbz2-dev  libssl-dev openssl libreadline-dev autoconf libtool git ntp doxygen libc++-dev libcurl4-openssl-dev
```

### 安装CMake 3.12

```
cd /tmp && wget https://cmake.org/files/v3.12/cmake-3.12.4-Linux-x86_64.sh
sudo mkdir -p /opt/cmake && chmod +x /tmp/cmake-3.12.4-Linux-x86_64.sh
sudo bash /tmp/cmake-3.12.4-Linux-x86_64.sh --prefix=/opt/cmake --skip-license
sudo ln -sfT /opt/cmake/bin/cmake /usr/local/bin/cmake
```

装完后，需要关掉并重新打开Ubuntu终端窗口，否则CMake可能会报错“could not find CMAKE_ROOT”。

### 安装带WASM组件的LLVM 4.0

安装到~/opt/wasm。

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

### 安装Berkeley DB

需要在Oracle网站注册帐号并登录后下载源码包。

http://download.oracle.com/otn/berkeley-db/db-18.1.25.tar.gz

```
tar -xzvf db-18.1.25.tar.gz
cd db-18.1.25/build_unix
../dist/configure --prefix=/usr --enable-cxx
make
sudo make install
```

### 安装Boost 1.67

安装到~/opt/boost。Boost版本不能低于1.67。

```
sudo ln -s /usr/bin/clang-4.0   /usr/bin/clang
sudo ln -s /usr/bin/clang++-4.0 /usr/bin/clang++

export BOOST_ROOT=~/opt/boost

cd ~ && wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz -O  boost_1_67_0.tar.gz
tar -zxvf boost_1_67_0.tar.gz && cd boost_1_67_0 && chmod +x bootstrap.sh
./bootstrap.sh --with-toolset=clang --prefix=${BOOST_ROOT}
./b2 -j 4 stage release
./b2 install --prefix=${BOOST_ROOT}
```

### 编译μNEST的Core

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （其中XXX是要编译的分支的名称，比如master或develop等）
git submodule update --init --recursive

export WASM_ROOT=~/opt/wasm
export BOOST_ROOT=~/opt/boost
export C_COMPILER=clang-4.0
export CXX_COMPILER=clang++-4.0

mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DOPENSSL_ROOT_DIR=/usr/include/openssl \
     -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DCMAKE_C_COMPILER="${C_COMPILER}" \
     -DOPENSSL_INCLUDE_DIR=/usr/include/openssl -DOPENSSL_LIBRARIES=/usr/lib/openssh -DBOOST_ROOT=${BOOST_ROOT} -DCMAKE_BUILD_TYPE=Release ..
make -j4
```

## macOS

### 安装XCode

在macOS的App Store中安装。参考：<https://guide.macports.org/#installing.xcode>。

如果XCode的命令行工具无法正常运行，请登录https://developer.apple.com/download/more下载XCode命令行工具的安装包重新安装。

### 安装Homebrew包管理器

在macOS的terminal窗口中执行如下命令安装。参考：[https://brew.sh/。](https://brew.sh/%E3%80%82)

```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

### 安装依赖的包

Boost只能用1.57~1.65.1版本，OpenSSL只能用1.0.1、1.0.2的版本。

```
brew update
brew install boost@1.57 cmake git openssl autoconf automake berkeley-db libtool llvm@4 doxygen wget
```

安装后，在macOS的terminal窗口中执行命令ls -l /usr/local/opt/boost，能看到符号链接/usr/local/opt/boost具体指向的是Boost的哪个版本。符号链接/usr/local/opt/openssl也类似。如果这两个符号链接指向的不是我们所要求的版本，则可以在后续的CMake的命令行中不使用这两个符号链接，而是使用带具体版本号的的Boost、OpenSSL目录。

### 安装带WASM组件的LLVM 4.0

安装到~/opt/wasm目录下。

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_40 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

### 编译μNEST Core的源码

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （其中XXX是要编译的分支的名称，比如master或develop等）
git submodule update --init --recursive

export WASM_ROOT=~/opt/wasm
export C_COMPILER=/usr/local/Cellar/llvm@4/4.0.1/bin/clang-4.0
export CXX_COMPILER=/usr/local/Cellar/llvm@4/4.0.1/bin/clang++

mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DCMAKE_C_COMPILER="${C_COMPILER}" -DBOOST_ROOT=/usr/local/opt/boost@1.57 -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include -DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib -DCMAKE_BUILD_TYPE=Release ..
make -j 4
```

## 搭建testnet

请参考bitshares的文档：http://docs.bitshares.org/testnet/private-testnet.html



## 参考资料

https://github.com/bitshares/bitshares-core/wiki/BUILD_UBUNTU

https://github.com/bitshares/bitshares-core/wiki/Building-on-OS-X

