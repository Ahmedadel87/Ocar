cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
project(CasmLang VERSION 0.1.0 LANGUAGES CXX)

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