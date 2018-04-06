cmake_minimum_required(VERSION 3.5.0)
project(clustering)
include_directories(include)

#Set debug flag
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")


#Can manually add the sources using the set command as follows:
set(SOURCES clustering.cpp ) 
list(APPEND SOURCES cpp/graph.cpp)
list(APPEND SOURCES cpp/mat.cpp)
list(APPEND SOURCES cpp/util.cpp)
list(APPEND SOURCES cpp/ap.cpp)
list(APPEND SOURCES cpp/sparse_ap.cpp)

add_executable(clustering ${SOURCES})

#Set name of executable
set_target_properties(clustering PROPERTIES OUTPUT_NAME ap.out PREFIX "")

target_compile_features(clustering PRIVATE cxx_range_for)