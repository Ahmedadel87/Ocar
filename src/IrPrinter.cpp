#include "../include/IrPrinter.h"

namespace casmlang {
void IrPrinter::print_indent(std::ostream& stream) {
    for (int i = 0; i < indent; i++) {
        stream << "    ";
    }
}
void IrPrinter::dispatch_print(Ir* instr, std::ostream& stream) {
    if (!instr)
        panic("Internal, cannot choose print dispatcher for instruction 0x00");

    if (auto cst = dynamic_cast<IrMovStmt*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrLabelStmt*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrRetStmt*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrRtnCall*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrGlobal*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrIntLit*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrSection*>(instr))
        instr_print(cst, stream);
    else if (auto cst = dynamic_cast<IrAsm*>(instr))
        instr_print(cst, stream);
    else
        panic("Internal, invalid print dispatch for instruction");
}

void IrPrinter::instr_print(IrMovStmt* ins, std::ostream& stream) {
    print_indent(stream);
    stream << "mov ";
    stream << ins->memname;
    stream << ", ";
    dispatch_print(ins->value.get(), stream);
}
void IrPrinter::instr_print(IrLabelStmt* ins, std::ostream& stream) {
    indent = 0;
    print_indent(stream);
    stream << ins->labeltext << ":";
    indent++;
}
void IrPrinter::instr_print(IrRetStmt* ins, std::ostream& stream) {
    print_indent(stream);
    stream << "ret";
    indent--;
}
void IrPrinter::instr_print(IrRtnCall* ins, std::ostream& stream) {
    print_indent(stream);
    stream << "call " << ins->rtnname;
}
void IrPrinter::instr_print(IrGlobal* ins, std::ostream& stream) {
    print_indent(stream);
    stream << "global " << ins->identifier;
}
void IrPrinter::instr_print(IrSection* ins, std::ostream& stream) {
    print_indent(stream);
    stream << "section ." << ins->text;
    indent++;
}
void IrPrinter::instr_print(IrAsm* ins, std::ostream& stream) {
    print_indent(stream);
    stream << ins->instruction << " ; direct conversion";
}

void IrPrinter::instr_print(IrIntLit* ins, std::ostream& stream) {
    stream << ins->number;
}

void IrPrinter::load_ir(std::vector<std::unique_ptr<Ir>> ir_) {
    ir = std::move(ir_);
}
void IrPrinter::print(std::ostream& stream) {
    for (auto& instr : ir) {
        dispatch_print(instr.get(), stream);
        stream << "\n";
    }
}
} // namespace casmlang