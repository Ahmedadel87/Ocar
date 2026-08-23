#pragma once
#include "Common.h"
#include "IrGenerator.h"

namespace casmlang {
class IrPrinter {
private:
    std::ostream& stream;
    int indent = 0;
    void print_indent();
    void dispatch_print(Ir* instr);

    void instr_print(IrMovStmt* ins);
    void instr_print(IrLabelStmt* ins);
    void instr_print(IrRetStmt* ins);
    void instr_print(IrRtnCall* ins);
    void instr_print(IrGlobal* ins);

public:
    IrPrinter(std::ostream& stream_) : stream(stream_) {}

    void load_ir(std::vector<std::unique_ptr<Ir>> ir);
    void print();
};
} // namespace casmlang