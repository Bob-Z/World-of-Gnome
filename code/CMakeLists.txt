cmake_minimum_required(VERSION 2.8)
#enable_language(CXX)
enable_language(C)

project(World of Gnome)
set(VERSION 0.0.0)

#find_package(PkgConfig REQUIRED)
#pkg_search_module(....)

#Path
#set(COMMON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common/)
#set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_YPE})
#set(LIBRARY_OUTPUT_PATH ${COMMON_PATH}/lib/${CMAKE_BUILD_YPE})

#Compile
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11"
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")


include_directories( ${COMMON_PATH} )

#add_library(
#	common
#	SHARED
#	${COMMON_PATH}/...
#)


add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
