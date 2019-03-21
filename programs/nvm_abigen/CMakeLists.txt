cmake_minimum_required( VERSION 2.8.12 )
set(SOURCES main.cpp)

find_package(LLVM REQUIRED CONFIG)
if (${LLVM_PACKAGE_VERSION} VERSION_LESS "7.0")
    error("LLVM version must be >= 7.0")
endif()

link_directories(${LLVM_LIBRARY_DIR})

add_executable(nvm-abigen ${SOURCES})

set( CMAKE_CXX_STANDARD 14 )

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

target_link_libraries(nvm-abigen abi_generator)


install( TARGETS
   nvm-abigen

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
