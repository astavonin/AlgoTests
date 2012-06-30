cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER g++-mp-4.7)
ADD_DEFINITIONS("-std=c++11")

project(AlgoTests)

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/build/bin
  CACHE PATH
   "Single Directory for all"
  )

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/build/bin
  CACHE PATH
   "Single Directory for all"
  )

SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/build/lib
  CACHE PATH
   "Single Directory for all"
  )

SET(AlgoTests_source main.cpp support.cpp)
SET(SortChecker_source sort_checker.cpp)

ADD_EXECUTABLE(algo_tests ${AlgoTests_source})
ADD_EXECUTABLE(sort_checker ${SortChecker_source})

