#include "../include/BashGenerator.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <ostream>

void IrGenerator::load_ast(std::unique_ptr<ScopeBlock> ast__) {
    ast = std::move(ast__);
}
void IrGenerator::generate_bash_ir() {
    ast->accept(*this);
}
std::vector<std::unique_ptr<BashStatement>> IrGenerator::get_ir() {
    return std::move(bash_ir);
}
void IrGenerator::genPanic(const std::string& msg, const SourceLocation& src) {
    if (src.row == -1) {
        panic("[BASH GENERATOR PANIC] " + msg);
    } else {
        panic("[BASH GENERATOR PANIC] " + msg + " [" + std::to_string(src.row) + ":" +
              std::to_string(src.column) + "]");
    }
}
std::unique_ptr<BashExpression> IrGenerator::getAsExpression(std::unique_ptr<Bash> bash) {
    auto ptr = dynamic_cast<BashExpression*>(bash.release());
    if (!ptr) {
        genPanic("tried to read bash value as expression, invalid");
    }
    std::unique_ptr<BashExpression> p(ptr);
    return std::move(p);
}

// == VISIT ==
void IrGenerator::visit(ScopeBlock& node) {
    for (auto& child : node.children) {
        child->accept(*this);
    }
}
void IrGenerator::visit(StringLiteral& node) {
    current_node =
        std::move(std::make_unique<BashLiteral>(WrapType::NONE, VariableType::STRING, node.string));
}
void IrGenerator::visit(FloatLiteral& node) {
    current_node = std::move(std::make_unique<BashLiteral>(WrapType::NONE, VariableType::FLOAT,
                                                           std::to_string(node.number)));
}
void IrGenerator::visit(IntegerLiteral& node) {
    current_node = std::move(std::make_unique<BashLiteral>(WrapType::NONE, VariableType::INT,
                                                           std::to_string(node.number)));
}
void IrGenerator::visit(BooleanLiteral& node) {
    current_node = std::move(std::make_unique<BashLiteral>(WrapType::NONE, VariableType::BOOL,
                                                           node.state ? "true" : "false"));
}
void IrGenerator::visit(VoidLiteral& node) {
    current_node =
        std::move(std::make_unique<BashLiteral>(WrapType::NONE, VariableType::VOID, "NONE"));
}
void IrGenerator::visit(VariableDefinition& node) {
    node.value->accept(*this);
    auto value = getAsExpression(std::move(current_node));

    bash_ir.push_back(std::make_unique<BashAssignment>(node.identifier, std::move(value), true));
}
void IrGenerator::visit(BinaryExpression& node) {
    node.left->accept(*this);
    auto left = getAsExpression(std::move(current_node));

    node.right->accept(*this);
    auto right = getAsExpression(std::move(current_node));

    current_node = std::make_unique<BashBinaryExpression>(
        std::move(left), node.op, std::move(right), node.return_type, WrapType::DOUBLE_QUOTE);
}
void IrGenerator::visit(IfStatement& node) {
    node.condition->accept(*this);
    auto condition = getAsExpression(std::move(current_node));

    bash_ir.push_back(std::make_unique<BashIfStatement>(std::move(condition)));

    node.block->accept(*this);

    if (!node.nextStatement)
        bash_ir.push_back(std::make_unique<BashEndIfStatement>());
    else
        node.nextStatement->accept(*this);
}
void IrGenerator::visit(FunctionCallExpr& node) {
    std::replace(node.identifier.begin(), node.identifier.end(), '.', ' ');
    std::vector<std::unique_ptr<BashExpression>> args;

    for (auto& arg : node.args) {
        arg->accept(*this);
        args.push_back(getAsExpression(std::move(current_node)));
    }

    current_node = std::make_unique<BashFunctionCallExpression>(
        WrapType::VARIABLE_WRAP, node.identifier, node.return_type, std::move(args));
}
void IrGenerator::visit(VariableReference& node) {
    current_node = std::make_unique<BashVariableReference>(WrapType::VARIABLE_WRAP,
                                                           node.return_type, node.identifier);
}
void IrGenerator::visit(UnaryExpression& node) {
    node.value->accept(*this);
    auto value = getAsExpression(std::move(current_node));

    current_node = std::make_unique<BashUnaryExpression>(WrapType::NONE, value->type, node.location,
                                                         node.op, std::move(value));
}
void IrGenerator::visit(VariableReassignment& node) {
    node.value->accept(*this);
    auto value = getAsExpression(std::move(current_node));
    bash_ir.push_back(std::make_unique<BashAssignment>(node.identifier, std::move(value), false));
}
void IrGenerator::visit(FunctionCallStmt& node) {
    std::vector<std::unique_ptr<BashExpression>> args;
    for (auto& arg : node.args) {
        arg->accept(*this);
        args.push_back(getAsExpression(std::move(current_node)));
    }

    bash_ir.push_back(
        std::make_unique<BashFunctionCallStatement>(node.identifier, std::move(args)));
}
void IrGenerator::visit(ElseIfStatement& node) {
    node.condition->accept(*this);
    auto condition = getAsExpression(std::move(current_node));

    bash_ir.push_back(std::make_unique<BashElseIfStatement>(std::move(condition)));

    node.block->accept(*this);

    if (!node.nextStatement)
        bash_ir.push_back(std::make_unique<BashEndIfStatement>());
    else
        node.nextStatement->accept(*this);
}
void IrGenerator::visit(ElseStatement& node) {
    bash_ir.push_back(std::make_unique<BashElseStatement>());
    node.block->accept(*this);
    bash_ir.push_back(std::make_unique<BashEndIfStatement>());
}
void IrGenerator::visit(ForLoop& node) {
    node.definition->accept(*this);

    auto bs_asg = std::move(bash_ir.back());
    auto asg = dynamic_cast<BashAssignment*>(bs_asg.get());

    if (!asg) {
        genPanic("expected assignment statement in for loop", node.definition->location);
        return;
    }

    std::unique_ptr<BashAssignment> definition(static_cast<BashAssignment*>(bs_asg.release()));
    bash_ir.pop_back();

    node.condition->accept(*this);

    auto cond = dynamic_cast<BashBinaryExpression*>(current_node.get());
    if (!cond) {
        genPanic("expected binary expression in for loop", node.condition->location);
        return;
    }

    std::unique_ptr<BashBinaryExpression> condition(
        static_cast<BashBinaryExpression*>(current_node.release()));

    node.then_do->accept(*this);

    auto bs_td = std::move(bash_ir.back());
    auto td = dynamic_cast<BashStatement*>(bs_td.get());

    if (!td) {
        genPanic("expected statement in for loop", node.then_do->location);
        return;
    }

    std::unique_ptr<BashStatement> then_do(static_cast<BashStatement*>(bs_td.release()));
    bash_ir.pop_back();

    bash_ir.push_back(std::make_unique<BashForLoop>(std::move(definition), std::move(condition),
                                                    std::move(then_do)));

    node.scope->accept(*this);

    bash_ir.push_back(std::make_unique<BashDone>());
}
void IrGenerator::visit(FunctionDefinition& node) {
    bash_ir.push_back(std::make_unique<BashFunctionDefinition>(node.identifier));
    node.scope->accept(*this);
    bash_ir.push_back(std::make_unique<BashFunctionDefinitionEnd>());
}