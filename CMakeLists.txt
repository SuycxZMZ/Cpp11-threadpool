# Set the minimum version of CMake that can be used
cmake_minimum_required(VERSION 3.12)

#project name
project(CPP11-THREADPOOL)

#add compile options
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

add_compile_options("-Wall" "-pthread")

#set CMAKE_BUILD_TYPE
set( CMAKE_BUILD_TYPE Debug )

# set output binary path 
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

############################################################
# Create a library
############################################################

#Generate the static library from the library sources
include_directories(/include)

add_library( thread_pool_lib STATIC src/thread_pool.cpp src/work_steal_queue.cpp )

target_include_directories( thread_pool_lib PUBLIC ${PROJECT_SOURCE_DIR}/include )

############################################################
# Create an executable
############################################################

# Add an executable with the above sources
add_executable( main src/main.cpp )

# link the new main target with the thread_pool_lib target
target_link_libraries( main thread_pool_lib )
