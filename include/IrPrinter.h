#pragma once
#include "Common.h"
#include "IrGenerator.h"

namespace casmlang {
class IrPrinter {
private:
    std::ostream& stream;
    std::vector<std::unique_ptr<Ir>> ir;
    int indent = 0;
    void print_indent();
    void dispatch_print(Ir* instr); // yes, yes, a dispatcher function isn't the best practice but
                                    // like who really cares it's just a bunch of if statements, way
                                    // more readable than operator overloads

    void instr_print(IrMovStmt* ins);
    void instr_print(IrLabelStmt* ins);
    void instr_print(IrRetStmt* ins);
    void instr_print(IrRtnCall* ins);
    void instr_print(IrGlobal* ins);

public:
    IrPrinter(std::ostream& stream_) : stream(stream_) {}

    void load_ir(std::vector<std::unique_ptr<Ir>> ir_);
    void print();
};
} // namespace casmlang