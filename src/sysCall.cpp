#include "../include/sysCall.h"
#include <iostream>
#include <string>

void Compiler::compile(const std::string& code, std::ostream& stream) {
    tokenizer.tokenize(code);

    parser.load_tokens(std::move(tokenizer.get_tokens()));
    parser.parse();

    sema.load_ast(parser.hand_over_AST());
    sema.analyse();

    irGen.load_ast(sema.hand_over_AST());
    irGen.generate_bash_ir();

    printer.load_ir(irGen.get_ir());
    printer.print_asm(stream);
}
