#pragma once
#include "IrGenerator.h"
#include "IrPrinter.h"
#include "Parser.h"
#include "SemanticAnalyser.h"
#include "Tokenizer.h"

#include <iostream>
#include <string>
#include <vector>

class Compiler {
private:
    ocarlang::Tokenizer tokenizer;
    Parser parser;
    SemanticAnalyser sema;
    ocarlang::IrGenerator irgen;
    ocarlang::IrPrinter printer;

public:
    void compile(const std::string& code, std::ostream& stream = std::cout);
    std::vector<std::string> get_asm();
};
