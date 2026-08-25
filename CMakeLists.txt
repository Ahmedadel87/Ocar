cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
project(Ocar VERSION 0.1.0 LANGUAGES CXX)

add_executable(Ocar
        src/main.cpp
        src/Ocar.cpp
        src/Tokenizer.cpp
        src/Parser.cpp
        src/Visitor.cpp
        src/SemanticAnalyser.cpp
        src/IrGenerator.cpp
        src/IrPrinter.cpp
        src/Preprocessor.cpp
)
target_include_directories(Ocar
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
target_compile_features(Ocar PRIVATE cxx_std_20)
target_compile_definitions(Ocar PRIVATE
    OCAR_STDLIB_PATH="${CMAKE_SOURCE_DIR}/stdlib"
)