#include "../include/IrPrinter.h"

namespace casmlang {
void IrPrinter::print_indent() {
    for (int i = 0; i < indent; i++) {
        stream << "    ";
    }
}
void IrPrinter::dispatch_print(Ir* instr) {
    if (!instr)
        panic("Internal, cannot choose print dispatcher for instruction 0x00");

    if (auto cst = dynamic_cast<IrMovStmt*>(instr))
        instr_print(cst);
    else if (auto cst = dynamic_cast<IrLabelStmt*>(instr))
        instr_print(cst);
    else if (auto cst = dynamic_cast<IrRetStmt*>(instr))
        instr_print(cst);
    else if (auto cst = dynamic_cast<IrRtnCall*>(instr))
        instr_print(cst);
    else if (auto cst = dynamic_cast<IrGlobal*>(instr))
        instr_print(cst);
    else
        panic("Internal, invalid print dispatch for instruction");
}

void IrPrinter::instr_print(IrMovStmt* ins) {
    print_indent();
    stream << "mov ";
    stream << ins->memname;
    stream << ", ";
    dispatch_print(ins->value.get());
}
void IrPrinter::instr_print(IrLabelStmt* ins) {
    print_indent();
    stream << ins->labeltext << ":";
    indent++;
}
void IrPrinter::instr_print(IrRetStmt* ins) {
    print_indent();
    stream << "ret";
    indent--;
}
void IrPrinter::instr_print(IrRtnCall* ins) {
    print_indent();
    stream << "call " << ins->rtnname;
}
void IrPrinter::instr_print(IrGlobal* ins) {
    print_indent();
    stream << "global " << ins->identifier;
}

void IrPrinter::load_ir(std::vector<std::unique_ptr<Ir>> ir_) {
    ir = std::move(ir_);
}
void IrPrinter::print() {
    for (auto& instr : ir) {
        dispatch_print(instr.get());
        stream << "\n";
    }
}
} // namespace casmlang