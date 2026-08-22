cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
project(CasmLang VERSION 0.1.0 LANGUAGES CXX)
option(CasmLang_BUILD_TESTS "Build BBS-Mallocator tests" OFF)
option(CasmLang_RUN_BENCHMARKS "Run BBS-MALLOCATOR benchmarks" OFF)

add_executable(CasmLang
        src/main.cpp
        src/CasmLang.cpp
        src/Tokenizer.cpp
        src/Parser.cpp
        src/Visitor.cpp
        src/SemanticAnalyser.cpp
)
target_include_directories(CasmLang
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
target_compile_features(CasmLang PRIVATE cxx_std_20)

if(CasmLang_BUILD_TESTS)
  include(CTest)
  enable_testing()

  add_executable(CasmLang_test
        tests/test.cpp
    )

  add_test(NAME RunBBSMallocatorTests COMMAND CasmLang_test)
endif()

if(CasmLang_RUN_BENCHMARKS)
  add_executable(CasmLang_benchmark
    benchmarks/benchmark.cpp
  )
endif()

