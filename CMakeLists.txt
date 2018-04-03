
cmake_minimum_required(VERSION 2.8)
PROJECT(ROADEF2018)

SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++14")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Og -Werror -Wno-unknown-pragmas")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG -DBOOST_UBLAS_NDEBUG -fopenmp")

FIND_PACKAGE(Boost COMPONENTS system filesystem program_options unit_test_framework REQUIRED)

INCLUDE_DIRECTORIES(${ROADEF2018_SOURCE_DIR}/include)

option(BUILD_TEST "Build unit tests" ON)

SET (SOURCES
  src/problem.cpp
  src/solution.cpp
  src/io_problem.cpp
  src/solution_checker.cpp
  src/sequence_packer.cpp
  src/solver.cpp
  src/utils.cpp
  src/main.cpp
)

add_executable(cut ${SOURCES})

