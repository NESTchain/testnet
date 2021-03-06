if(WIN32)
	include_directories($ENV{BDB_INCLUDE_DIR})
endif(WIN32)

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling witness_node with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

add_executable( witness_node main.cpp )

# We have to link against graphene_debug_witness because deficiency in our API infrastructure doesn't allow plugins to be fully abstracted #246

if(WIN32)

target_link_libraries( witness_node

PRIVATE graphene_app graphene_delayed_node graphene_account_history graphene_elasticsearch graphene_market_history graphene_grouped_orders graphene_witness graphene_chain graphene_debug_witness graphene_egenesis_full graphene_snapshot graphene_es_objects fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS}
$ENV{BDB_LIB_DIR}/libdb181.lib
)

else(WIN32)

target_link_libraries( witness_node

PRIVATE graphene_app graphene_delayed_node graphene_account_history graphene_elasticsearch graphene_market_history graphene_grouped_orders graphene_witness graphene_chain graphene_debug_witness graphene_egenesis_full graphene_snapshot graphene_es_objects fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS}
db_cxx
)

endif(WIN32)

install( TARGETS
   witness_node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
