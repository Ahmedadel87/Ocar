#!/bin/bash
cmake -B build -DCasmLang_BUILD_TESTS=ON
cmake --build build
./build/CasmLang_test
./build/CasmLang $1
cd ..
