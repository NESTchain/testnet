#if(NOT ENABLE_STATIC_LINKING AND MSVC)
if (MSVC)
        add_definitions("-DDLL_EXPORT=__declspec(dllexport)")
        add_definitions("-DDLL_IMPORT=__declspec(dllimport)")
else()
        add_definitions(-DDLL_EXPORT=)
        add_definitions(-DDLL_IMPORT=)
endif()

set(WASM_CONTRACT_SRC
	src/wasm_contract.cpp
	src/wasm_interface.cpp
	src/wavm.cpp
)

set(WASM_CONTRACT_INCLUDE
	include/action.hpp
	include/apply_context.hpp
	include/common.hpp
	include/controller.hpp
	include/exceptions.hpp
	include/name.hpp
	include/runtime_interface.hpp
	include/transaction_context.hpp
	include/wasm_contract.hpp
	include/wasm_eosio_constraints.hpp
	include/wasm_interface.hpp
	include/wasm_interface_private.hpp
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
	PUBLIC "${CMAKE_SOURCE_DIR}/libraries/softfloat/source/include"
)


add_subdirectory(${WASM_JIT_DIR})

