file(GLOB HEADERS "include/graphene/market_history/*.hpp")

if(WIN32)
	INCLUDE_DIRECTORIES($ENV{BDB_INCLUDE_DIR})
endif(WIN32)

add_library( graphene_market_history 
             market_history_plugin.cpp
           )

if(WIN32)
	target_link_libraries( graphene_market_history graphene_chain graphene_app )
else(WIN32)
	target_link_libraries( graphene_market_history graphene_chain graphene_app db_cxx)
endif(WIN32)

target_include_directories( graphene_market_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( market_history_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   graphene_market_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/graphene/market_history" )

