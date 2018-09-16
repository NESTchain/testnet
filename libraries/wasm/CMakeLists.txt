set(WASM_CONTRACT_SRC
	src/wasm_contract.cpp
	src/wasm_interface.cpp
	src/wavm.cpp
)

set(WASM_CONTRACT_INCLUDE
	include/wasm_contract.hpp
	include/common.hpp
	include/runtime_interface.hpp
	include/wasm_interface.hpp
	include/wavm.hpp
)

add_library( wasm_contract
	${WASM_CONTRACT_SRC}
	${WASM_CONTRACT_INCLUDE}
)

set(WASM_JIT_DIR "wasm-jit")

target_include_directories( wasm_contract
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/wasm-jit/Include"
	PUBLIC "${CMAKE_SOURCE_DIR}/libraries/chain/include"
	PUBLIC "${CMAKE_SOURCE_DIR}/libraries/fc/include"
	PUBLIC "${CMAKE_SOURCE_DIR}/libraries/db/include"
)


add_subdirectory(${WASM_JIT_DIR})

