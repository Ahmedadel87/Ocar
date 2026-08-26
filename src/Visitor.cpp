#include "../include/Common.h"
#include <iostream>

void PrettyPrinter::printIndent() {
    for (int i = 0; i < indent; i++) {
        if (i == indent - 1) {
            stream << "├────";
        } else {
            stream << "│    ";
        }
    }
}
void PrettyPrinter::visit(ScopeBlock& node) {
    printIndent();
    stream << "ScopeBlock" << std::endl;

    indent++;
    for (auto& child : node.children) {
        child->accept(*this);
    }
    indent--;
}
void PrettyPrinter::visit(VariableDefinition& node) {
    printIndent();
    stream << "VariableDefinition\n";

    indent++;
    printIndent();
    stream << "Identifier(" << node.identifier << ")\n";
    printIndent();
    stream << "Memory(" << node.memory << ")\n";
    node.value->accept(*this);
    indent--;
}
void PrettyPrinter::visit(BinaryExpression& node) {
    printIndent();

    stream << "BinaryExpression\n";
    indent++;
    node.left->accept(*this);

    printIndent();
    stream << "Operation(" << (int)node.op << ")\n";

    node.right->accept(*this);
    indent--;
}
void PrettyPrinter::visit(RoutineCallExpr& node) {
    printIndent();
    stream << "FunctionCall " << node.identifier << std::endl;

    indent++;
    for (auto& arg : node.args) {
        arg->accept(*this);
    }
    indent--;
}
void PrettyPrinter::visit(VariableReference& node) {
    printIndent();
    stream << "VariableReference " << node.identifier << std::endl;
}
void PrettyPrinter::visit(IntegerLiteral& node) {
    printIndent();
    stream << "IntegerLiteral(" << node.number << ")\n";
}
void PrettyPrinter::visit(FloatLiteral& node) {
    printIndent();
    stream << "FloatLiteral(" << node.number << ")\n";
}
void PrettyPrinter::visit(StringLiteral& node) {
    printIndent();
    stream << "StringLiteral(\"" << node.string << "\")\n";
}
void PrettyPrinter::visit(BooleanLiteral& node) {
    printIndent();
    stream << "BooleanLiteral(" << (node.state ? "true)\n" : "false)\n");
}
void PrettyPrinter::visit(VoidLiteral& node) {
    printIndent();
    stream << "VoidLiteral";
}
void PrettyPrinter::visit(UnaryExpression& node) {
    printIndent();

    stream << "UnaryExpression " << (int)node.op << "\n";
    indent++;
    node.value->accept(*this);
    indent--;
}
void PrettyPrinter::visit(VariableReassignment& node) {
    printIndent();
    stream << "VariableReassignment " << node.identifier << "\n";

    indent++;
    node.value->accept(*this);
    indent--;
}
void PrettyPrinter::visit(RoutineCallStmt& node) {
    printIndent();
    stream << "FunctionCallStatement " << node.identifier << "\n";

    indent++;
    for (auto& arg : node.args) {
        arg->accept(*this);
    }
    indent--;
}
void PrettyPrinter::visit(RoutineDefinition& node) {
    printIndent();
    stream << "FunctionDefinition " << node.identifier << "\n";
    indent++;
    node.scope->accept(*this);
    indent--;
}
void PrettyPrinter::visit(SectionDefinition& node) {
    printIndent();
    stream << "SectionDefinition " << node.identifier << "\n";
}
void PrettyPrinter::visit(Global& node) {
    printIndent();
    stream << "global " << node.identifier;
}
void PrettyPrinter::visit(RoutineDeclaration& node) {
    printIndent();
    stream << "FunctionDeclaration " << node.identifier;
}
void PrettyPrinter::visit(AsmInstruction& node) {
    printIndent();
    stream << "raw instruction: " << node.instruction;
}
void PrettyPrinter::visit(DeleteSymbol& node) {
    printIndent();
    stream << "delete " << node.identifier;
}
void PrettyPrinter::visit(FreeMemory& node) {
    printIndent();
    stream << "free memory " << node.memoryName;
}
void PrettyPrinter::visit(IfStatement& node) {
    printIndent();
    stream << "if " << (int)node.cond;

    indent++;
    for (auto& child : node.scope->children) {
        child->accept(*this);
    }
    indent--;
}
void PrettyPrinter::visit(Compare& node) {
    printIndent();
    stream << "compare ";
    node.left->accept(*this);
    stream << " & ";
    node.right->accept(*this);
}
void PrettyPrinter::visit(RegisterName& node) {
    printIndent();
    stream << "(reg)" << node.name;
}
void PrettyPrinter::visit(SyscallStatement& node) {
    printIndent();
    stream << "syscall";
}
void PrettyPrinter::visit(ArithmeticOperation& node) {
    printIndent();
    stream << "ArithmeticOperation";

    indent++;
    node.left->accept(*this);

    printIndent();
    stream << (int)node.operation;

    node.right->accept(*this);
    indent--;
}
void PrettyPrinter::visit(RawAssignment& node) {
    stream << "raw assignment: ";
    node.left->accept(*this);
    stream << " = ";
    node.right->accept(*this);
}
void PrettyPrinter::visit(RawLabel& node) {
    stream << "label " << node.labelname;
}
