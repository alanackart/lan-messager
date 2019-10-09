cmake_minimum_required(VERSION 3.7)
project(lan-messager)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -pthread -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}   -pthread -std=c++0x")
else()
message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


find_package(Boost  COMPONENTS system REQUIRED)
# find_package(Boost 1.55.0 COMPONENTS filesystem system REQUIRED)
# include_directories(
#         "../../include"
#         "../../third/msgpack/include"
# )

add_executable(sender ../sender.cpp)
target_link_libraries(sender ${Boost_LIBRARIES})
add_executable(receiver ../receiver.cpp)
target_link_libraries( receiver ${Boost_LIBRARIES})
