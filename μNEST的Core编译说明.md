# μNEST的Core组件编译说明

作者：μNEST开发组，http://iotee.io



目前只支持在Linux、macOS上编译。

## Ubuntu 16.04/18.04 LTS (64-bit)

### 下载LLVM官方预编译的Clang/LLVM 7.0.1+

从Clang官网http://releases.llvm.org下载预编译的对应平台的包，解压，然后将解压后得到的bin目录设置到PATH环境变量的最前面。

```
cd ~
wget http://releases.llvm.org/7.0.1/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz
tar zxvf clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz

export PATH=~/clang+llvm-7.0.1-x86_64-linux-gnu-ubuntu-16.04/bin:$PATH
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

### 从源码编译安装带WASM组件的LLVM 7.0.1+

安装到~/opt/wasm。此处要编译其WASM组件，并打开RTTI。LLVM官方预编译的版本没有我们需要的这两个特性。

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_ENABLE_RTTI=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DLLVM_TARGETS_TO_BUILD="X86" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

然后将产物的路径设置到PATH环境变量的最前面（这点很重要）。自此，我们不再使用官方预编译的LLVM版本，而是始终使用我们自己编译的LLVM版本。

```
export PATH=~/opt/wasm/bin:$PATH
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

### 安装Boost 1.69

安装到~/opt/boost。Boost版本不低于1.69。

```
export BOOST_ROOT=~/opt/boost

cd ~ && wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz -O  boost_1_69_0.tar.gz
tar -zxvf boost_1_69_0.tar.gz && cd boost_1_69_0 && chmod +x bootstrap.sh
./bootstrap.sh --with-toolset=clang --prefix=${BOOST_ROOT}
./b2 -j 4 stage release
./b2 install --prefix=${BOOST_ROOT}
```

### 编译μNEST的Core

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （其中XXX是要编译的分支的名称，比如master或develop等）
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

### 安装XCode

在macOS的App Store中安装。参考：<https://guide.macports.org/#installing.xcode>。

如果XCode的命令行工具比如xcrun无法正常运行，请登录https://developer.apple.com/download/more下载XCode命令行工具的dmg安装包重新安装。

### 安装Homebrew包管理器

在macOS的terminal窗口中执行如下命令安装。参考：[https://brew.sh/。](https://brew.sh/%E3%80%82)

```
/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
```

### 安装依赖的包

OpenSSL版本为1.0.1, 1.0.2。目前通过参数"llvm@7"下载到的LLVM版本为7.0.1，将其bin目录设置到PATH环境变量的最前面。

```
brew update
brew install cmake git openssl autoconf automake berkeley-db libtool llvm@7 doxygen wget

export PATH=/usr/local/Cellar/llvm/7.0.1/bin:$PATH
```

安装后，在macOS的terminal窗口中执行命令ls -l /usr/local/opt/openssl，能看到符号链接/usr/local/opt/openssl具体指向的是OpenSSL的哪个版本。如果这个符号链接指向的不是我们所要求的版本，则可以在后续传给CMake的命令行中不使用这个符号链接，而是使用带具体版本号的的OpenSSL目录。

### 安装带WASM组件的LLVM 7.0.1+

安装到~/opt/wasm目录下。

```
mkdir  ~/wasm-compiler && cd ~/wasm-compiler
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/llvm.git
cd llvm/tools
git clone --depth 1 --single-branch --branch release_70 https://github.com/llvm-mirror/clang.git
cd .. && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=~/opt/wasm -DLLVM_ENABLE_RTTI=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DLLVM_TARGETS_TO_BUILD= -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=WebAssembly -DCMAKE_BUILD_TYPE=Release ..
make -j4 install
```

### 安装Boost 1.69

安装到~/opt/boost目录下。

```
export BOOST_ROOT=~/opt/boost

cd ~ && wget https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz -O  boost_1_69_0.tar.gz
tar -zxvf boost_1_69_0.tar.gz && cd boost_1_69_0 && chmod +x bootstrap.sh

./bootstrap.sh --prefix=${BOOST_ROOT}
./b2 -j 4 --buildtype=complete install --prefix=${BOOST_ROOT} toolset=clang cxxflags="-arch x86_64 -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -I/usr/local/opt/llvm@7/include" linkflags="-arch x86_64  -L/usr/local/opt/llvm@7/lib"
```

### 编译μNEST Core的源码

```
cd ~ && git clone https://github.com/NESTchain/testnet.git
cd testnet && git checkout XXX （其中XXX是要编译的分支的名称，比如master或develop等）
git submodule update --init --recursive
cd contracts/musl/upstream && git checkout eosio

export WASM_ROOT=~/opt/wasm
export BOOST_ROOT=~/opt/boost
export C_COMPILER=clang
export CXX_COMPILER=clang++

cd ~/testnet && mkdir -p build &&  cd build
cmake -DWASM_ROOT=${WASM_ROOT} -DLLVM_DIR="${WASM_ROOT}/lib/cmake/llvm" \
    -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" -DCMAKE_C_COMPILER="${C_COMPILER}" \
    -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl \
	-DOPENSSL_INCLUDE_DIR=/usr/local/opt/openssl/include \
	-DOPENSSL_LIBRARIES=/usr/local/opt/openssl/lib \
    -DBOOST_ROOT=${BOOST_ROOT} \
	-DCMAKE_BUILD_TYPE=Release ..
make -j 4
```

## 搭建testnet

请参考bitshares的文档：http://docs.bitshares.org/testnet/private-testnet.html



## 参考资料

https://github.com/bitshares/bitshares-core/wiki/BUILD_UBUNTU

https://github.com/bitshares/bitshares-core/wiki/Building-on-OS-X

