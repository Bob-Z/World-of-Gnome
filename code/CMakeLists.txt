cmake_minimum_required(VERSION 2.8)

project(World_of_Gnome)
set(VERSION 0.0.0)

enable_language(CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -Wall")

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

add_subdirectory(sdl_item)
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
