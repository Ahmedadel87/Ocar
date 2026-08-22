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
    stream << "Type(" << (int)node.type << ")\n";
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
void PrettyPrinter::visit(IfStatement& node) {
    printIndent();

    stream << "IfStatement\n";
    indent++;
    node.condition->accept(*this);
    node.block->accept(*this);
    if (node.nextStatement != nullptr)
        node.nextStatement->accept(*this);
    indent--;
}
void PrettyPrinter::visit(FunctionCallExpr& node) {
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
void PrettyPrinter::visit(FunctionCallStmt& node) {
    printIndent();
    stream << "FunctionCallStatement " << node.identifier << "\n";

    indent++;
    for (auto& arg : node.args) {
        arg->accept(*this);
    }
    indent--;
}
void PrettyPrinter::visit(ElseIfStatement& node) {
    printIndent();

    stream << "ElseIfStatement\n";
    indent++;
    node.condition->accept(*this);
    node.block->accept(*this);
    if (node.nextStatement != nullptr)
        node.nextStatement->accept(*this);
    indent--;
}
void PrettyPrinter::visit(ElseStatement& node) {
    printIndent();
    stream << "ElseStatement\n";
    indent++;
    node.block->accept(*this);
    indent--;
}
void PrettyPrinter::visit(ForLoop& node) {
    printIndent();
    stream << "ForLoop\n";
    indent++;
    node.definition->accept(*this);
    node.condition->accept(*this);
    node.then_do->accept(*this);
    node.scope->accept(*this);
    indent--;
}
void PrettyPrinter::visit(FunctionDefinition& node) {
    printIndent();
    stream << "FunctionDefinition " << node.identifier << "\n";
    indent++;
    node.scope->accept(*this);
    indent--;
}