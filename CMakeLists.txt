cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
project(sysCall VERSION 0.1.0 LANGUAGES CXX)
option(sysCall_BUILD_TESTS "Build BBS-Mallocator tests" OFF)
option(sysCall_RUN_BENCHMARKS "Run BBS-MALLOCATOR benchmarks" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_executable(sysCall
        src/main.cpp
        src/sysCall.cpp
        src/Tokenizer.cpp
        src/Parser.cpp
        src/Visitor.cpp
        src/SemanticAnalyser.cpp
        src/IrGenertor.cpp
        src/AsmPrinter.cpp
)
target_include_directories(sysCall
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

if(sysCall_BUILD_TESTS)
  include(CTest)
  enable_testing()

  add_executable(sysCall_test
        tests/test.cpp
    )

  add_test(NAME RunBBSMallocatorTests COMMAND sysCall_test)
endif()

if(sysCall_RUN_BENCHMARKS)
  add_executable(sysCall_benchmark
    benchmarks/benchmark.cpp
  )
endif()

