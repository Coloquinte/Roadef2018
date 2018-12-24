
cmake_minimum_required(VERSION 2.8)
PROJECT(ROADEF2018)

SET(CMAKE_CXX_FLAGS "-Wall -std=c++14 -g")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -march=native")

FIND_PACKAGE(Boost COMPONENTS system filesystem program_options unit_test_framework REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE_DIRECTORIES(${ROADEF2018_SOURCE_DIR}/include)

SET(TEST_TIME 3 CACHE STRING "Run time for each test (seconds)")

SET (SOURCES
  src/problem.cpp
  src/solution.cpp
  src/io_problem.cpp
  src/solution_checker.cpp
  src/sequence_packer.cpp
  src/plate_packer.cpp
  src/cut_packer.cpp
  src/row_packer.cpp
  src/packer.cpp
  src/packer_front.cpp
  src/ordering_heuristic.cpp
  src/solver.cpp
  src/params.cpp
  src/move.cpp
  src/merger_move.cpp
  src/main.cpp
  src/row_merger.cpp
  src/cut_merger.cpp
  src/merger.cpp
)

add_executable(gcut ${SOURCES})
target_link_libraries(gcut
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
)

enable_testing()
foreach (i RANGE 1 20)
  ADD_TEST(BENCH_A${i} gcut --batch ${ROADEF2018_SOURCE_DIR}/dataset/A${i}_batch.csv --defects ${ROADEF2018_SOURCE_DIR}/dataset/A${i}_defects.csv --time ${TEST_TIME} -o A${i}_solution.csv)
endforeach(i)

