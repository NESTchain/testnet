set(WASM_CONTRACT_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_contract.cpp
)

set(WASM_CONTRACT_INCLUDE
	${CMAKE_CURRENT_SOURCE_DIR}/include/wasm_contract.hpp
)

add_library( wasm_contract
	${WASM_CONTRACT_SRC}
	${WASM_CONTRACT_INCLUDE}
)

set(WASM_JIT_DIR "wasm-jit")

target_include_directories( wasm_contract
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/wasm-jit/Include"
)


add_subdirectory(${WASM_JIT_DIR})

