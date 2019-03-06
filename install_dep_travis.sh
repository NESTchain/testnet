#!/bin/sh

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
	cd ~ && wget https://e.iotee.io/db-18.1.25.tgz
	tar zxf db-18.1.25.tgz

	cd ~/db-18.1.25/build_unix && sudo make install
	mkdir ~/opt  && cd ~/opt && wget https://e.iotee.io/wasm.tgz
	tar zxf wasm.tgz

	cd ~/opt && wget https://e.iotee.io/boost_1_69_0.tgz
	tar zxf boost_1_69_0.tgz
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
	brew install berkeley-db doxygen llvm@7

	mkdir ~/opt  && cd ~/opt && wget https://e.iotee.io/wasm_osx.tgz
	tar zxf wasm_osx.tgz

	cd ~/opt && wget https://e.iotee.io/boost_1_69_0_osx.tgz
	tar zxf boost_1_69_0_osx.tgz
fi
