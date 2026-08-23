#include "../include/CasmLang.h"
#include <iostream>
#include <string>

void Compiler::compile(const std::string& code, std::ostream& stream) {
    tokenizer.tokenize(code);

    parser.load_tokens(std::move(tokenizer.get_tokens()));
    parser.parse();

    sema.load_ast(parser.hand_over_AST());
    sema.analyse();

    irgen.load_ast(sema.hand_over_AST());
    irgen.generate_ir();
    irgen.print_ir(std::cout);
}
