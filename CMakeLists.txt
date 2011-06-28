cmake_minimum_required (VERSION 2.6)
project (JHCOMMON)

include_directories(${JHCOMMON_SOURCE_DIR}/include)
link_directories(${JHCOMMON_BINARY_DIR}/src)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X specific code
   add_definitions(-DPLATFORM_DARWIN)
ELSE ()
	set(JHCOM_LIBS dl)
ENDIF ()

message( "Build Type: ${CMAKE_BUILD_TYPE}" )

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
add_definitions(-DJH_VERBOSE_LOGGING)
endif ()

find_package(Threads REQUIRED)

set(JHCOMMON_LIBS jhcommon ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(src)
add_subdirectory(test)



