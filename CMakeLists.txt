cmake_minimum_required (VERSION 2.6)
project (JHCOMMON)

include_directories(${JHCOMMON_SOURCE_DIR}/include)
link_directories(${JHCOMMON_BINARY_DIR}/src)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X specific code
   add_definitions(-DPLATFORM_DARWIN)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

add_subdirectory(src)
add_subdirectory(test)



