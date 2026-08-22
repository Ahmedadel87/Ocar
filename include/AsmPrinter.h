#pragma once
#include "IrGenerator.h"

class BashPrinter {
    std::vector<std::unique_ptr<BashStatement>> bash;
    bool use_ugly_flags = true;
    bool use_bc_arithmetic = true;
    int locality_depth = 0;

    void print(BashBinaryExpression& node, std::ostream& stream);
    void print(BashLiteral& node, std::ostream& stream);
    void print(BashFunctionCallExpression& node, std::ostream& stream);
    void print(BashVariableReference& node, std::ostream& stream);
    void print(BashUnaryExpression& node, std::ostream& stream);

    void print(BashAssignment& node, std::ostream& stream);
    void print(BashIfStatement& node, std::ostream& stream);
    void print(BashEndIfStatement& node, std::ostream& stream);
    void print(BashFunctionCallStatement& node, std::ostream& stream);
    void print(BashElseIfStatement& node, std::ostream& stream);
    void print(BashElseStatement& node, std::ostream& stream);
    void print(BashForLoop& node, std::ostream& stream);
    void print(BashDone& node, std::ostream& stream);
    void print(BashFunctionDefinition& node, std::ostream& stream);
    void print(BashFunctionDefinitionEnd& node, std::ostream& stream);

    void print_dispatcher(Bash* node, std::ostream& stream);
    std::string wrap(const std::string& string, WrapType wrap_type);
    std::string get_operation(BinaryOperation operation);

public:
    void load_ir(std::vector<std::unique_ptr<BashStatement>> bash_);
    void print_asm(std::ostream& stream);
};