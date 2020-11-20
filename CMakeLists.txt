cmake_minimum_required(VERSION 3.1...3.16)
project(AAsterisk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# OpenCV 
find_package(OpenCV)
#
message(${OpenCV_LIBS})
include_directories( 
 include
${OpenCV_INCLUDE_DIRS} 
)

add_executable(aasterisk AAsterisk.cpp)
target_link_libraries( aasterisk ${OpenCV_LIBS} )