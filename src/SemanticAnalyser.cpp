#include "../include/SemanticAnalyser.h"
#include "../include/ErrorHandler.h"
#include <algorithm>
#include <iostream>

template <typename T> using uq = std::unique_ptr<T>;

// == HELPERS ==
std::unique_ptr<ScopeBlock> SemanticAnalyser::hand_over_AST() {
    return std::move(ast);
}
void SemanticAnalyser::load_ast(uq<ScopeBlock> ast_) {
    ast = std::move(ast_);
}
void SemanticAnalyser::analyse() {
    enter_scope();

    ast->accept(*this);
    exit_scope();
}
void SemanticAnalyser::semaPanic(const std::string& msg, SourceLocation src) {
    if (src.row == -1) {
        panic("[SEMANTIC ANALYSIS PANIC] " + msg);
        return;
    }

    panic("[SEMANTIC ANALYSIS PANIC] " + msg + " [AT " + std::to_string(src.row) + ":" +
          std::to_string(src.column) + "]");
}
ExpressionInfo SemanticAnalyser::analyseExpression(Expression* expr) {
    if (auto lt = dynamic_cast<StringLiteral*>(expr)) {
        return ExpressionInfo(false, true);
    } else if (auto lt = dynamic_cast<IntegerLiteral*>(expr)) {
        return ExpressionInfo(false, true);
    } else if (auto lt = dynamic_cast<FloatLiteral*>(expr)) {
        return ExpressionInfo(false, true);
    } else if (auto lt = dynamic_cast<BooleanLiteral*>(expr)) {
        return ExpressionInfo(false, true);
    } else if (auto lt = dynamic_cast<VoidLiteral*>(expr)) {
        return ExpressionInfo(false, true);
    } else if (auto var = dynamic_cast<VariableReference*>(expr)) {
        return ExpressionInfo(true, false);
    } else if (auto func = dynamic_cast<RoutineCallExpr*>(expr)) {
        const Symbol* symbol = getSymbol(func->identifier);
        if (symbol == nullptr)
            semaPanic("undeclared function \"" + func->identifier + "\"", func->location);
        return ExpressionInfo(false, true);
    } else if (auto bexpr = dynamic_cast<BinaryExpression*>(expr)) {
        auto leftInfo = analyseExpression(bexpr->left.get());
        auto rightInfo = analyseExpression(bexpr->right.get());

        return ExpressionInfo(false, true);
    } else if (auto uxpr = dynamic_cast<UnaryExpression*>(expr)) {
        uxpr->value->accept(*this);
        if ((uxpr->op == UnaryOperation::DECREMENT || uxpr->op == UnaryOperation::INCREMENT ||
             uxpr->op == UnaryOperation::NEGATE)) {
            semaPanic("cannot decrement/increment/negate non-integer/float value");
        }
        return ExpressionInfo(false, true);
    } else {
        semaPanic("invalid expression", expr->location);
    }
    return ExpressionInfo();
}

void SemanticAnalyser::enter_scope() {
    if (stack.size() > 0)
        stack.push_back(std::make_unique<Scope>(stack.back().get()));
    else
        stack.push_back(std::make_unique<Scope>(nullptr));
}
void SemanticAnalyser::exit_scope() {
    stack.pop_back();
}
void SemanticAnalyser::addSymbol(const Symbol& symbol) {
    stack.back()->symbols[symbol.identifier] = std::make_unique<Symbol>(symbol);
}
bool SemanticAnalyser::symbolExists(const std::string& identifier) const {
    Scope* scp = stack.back().get();
    while (scp != nullptr) {
        if (scp->symbols.contains(identifier))
            return true;

        scp = scp->parent;
    }
    return false;
}
const Symbol* SemanticAnalyser::getSymbol(const std::string& identifier) const {
    Scope* scp = stack.back().get();
    while (scp != nullptr) {
        if (scp->symbols.contains(identifier))
            return scp->symbols[identifier].get();

        scp = scp->parent;
    }
    return nullptr;
}
void SemanticAnalyser::removeSymbol(const std::string& identifier) {
    Scope* scp = stack.back().get();
    while (scp != nullptr) {
        if (scp->symbols.contains(identifier))
            scp->symbols.erase(identifier);
        scp = scp->parent;
    }
    semaPanic("Internal, cannot erase symbol \"" + identifier + "\"; it does not exist");
}

void SemanticAnalyser::push_active_memory(const std::string& memname, const std::string& varname) {
    activeMemory.push_back(std::pair<std::string, std::string>(memname, varname));
}
void SemanticAnalyser::remove_active_memory(const std::string& varname) {
    auto it = std::find_if(activeMemory.begin(), activeMemory.end(),
                           [&](const auto& p) { return p.second == varname; });

    if (it != activeMemory.end()) {
        activeMemory.erase(it);
    } else { // ehhh idk what to do here so let's just throw
        semaPanic("internal, cannot remove variable \"" + varname +
                  "\" from active memory; it does not exist");
    }
}
std::string& SemanticAnalyser::find_in_active_memory(const std::string& memname) {
    auto it = std::find_if(activeMemory.begin(), activeMemory.end(),
                           [&](const auto& p) { return p.first == memname; });

    if (it != activeMemory.end())
        return it->second;
    else
        return emptystring;
}
std::string& SemanticAnalyser::find_memory_by_varname(const std::string& varname) {
    auto it = std::find_if(activeMemory.begin(), activeMemory.end(),
                           [&](const auto& p) { return p.second == varname; });

    if (it != activeMemory.end())
        return it->first;
    else
        return emptystring;
}

// == VISIT ==
void SemanticAnalyser::visit(ScopeBlock& node) {
    enter_scope();
    for (auto& child : node.children) {
        child->accept(*this);
    }
    exit_scope();
}
void SemanticAnalyser::visit(StringLiteral& node) {
    // literally what could go wrong in a string literal (famous last words)
}
void SemanticAnalyser::visit(FloatLiteral& node) {}
void SemanticAnalyser::visit(IntegerLiteral& node) {}
void SemanticAnalyser::visit(BooleanLiteral& node) {}
void SemanticAnalyser::visit(VoidLiteral& node) {}
void SemanticAnalyser::visit(BinaryExpression& node) {
    analyseExpression(&node);
}
void SemanticAnalyser::visit(RoutineCallExpr& node) {
    if (!symbolExists(node.identifier)) {
        semaPanic("cannot reference function \"" + node.identifier + "\"; it does not exist.",
                  node.location);
    }
    if (getSymbol(node.identifier)->kind != SymbolKind::Routine) {
        semaPanic("\"" + node.identifier + "\" is used as a function even though it is a variable",
                  node.location);
    }

    auto paramCount = getSymbol(node.identifier)->paramCount;
    if (node.args.size() < paramCount) {
        semaPanic("less arguments than requested");
    }
    if (node.args.size() > paramCount) {
        semaPanic("more arguments than requested");
    }
}
void SemanticAnalyser::visit(VariableReference& node) {
    if (!symbolExists(node.identifier)) {
        semaPanic("cannot reference variable \"" + node.identifier + "\"; it does not exist.",
                  node.location);
    }
    if (getSymbol(node.identifier)->kind != SymbolKind::Variable) {
        semaPanic("\"" + node.identifier + "\" is used as a variable even though it is a function",
                  node.location);
    }
    analyseExpression(&node);
}
void SemanticAnalyser::visit(VariableReassignment& node) {
    if (!symbolExists(node.identifier)) {
        semaPanic("cannot reassign variable \"" + node.identifier + "\"; it does not exist.",
                  node.location);
    }
    if (getSymbol(node.identifier)->kind != SymbolKind::Variable) {
        semaPanic("\"" + node.identifier + "\" is used as a variable even though it is a function",
                  node.location);
    }
    std::string memname = find_memory_by_varname(node.identifier);
    if (memname.empty()) {
        semaPanic("Could not resolve memory name for identifier \"" + node.identifier + "\"",
                  node.location);
    }
    node.memory = memname;
}
void SemanticAnalyser::visit(VariableDefinition& node) {
    std::string& othervar = find_in_active_memory(node.memory);
    if (!othervar.empty()) {
        semaPanic("cannot reassign memory \"" + node.memory + "\" to \"" + node.identifier +
                  "\"; it is already taken by \"" + othervar + "\"");
    }
    push_active_memory(node.memory, node.identifier);
    addSymbol(Symbol(node.identifier, SymbolKind::Variable));
}
void SemanticAnalyser::visit(UnaryExpression& node) {
    node.value->accept(*this);
}
void SemanticAnalyser::visit(RoutineCallStmt& node) {
    if (!symbolExists(node.identifier)) {
        semaPanic("cannot reference function \"" + node.identifier + "\"; it does not exist.",
                  node.location);
    }
    if (getSymbol(node.identifier)->kind != SymbolKind::Routine) {
        semaPanic("\"" + node.identifier + "\" is used as a function even though it is a variable",
                  node.location);
    }

    auto paramCount = getSymbol(node.identifier)->paramCount;
    if (node.args.size() < paramCount) {
        semaPanic("less arguments than requested");
    }
    if (node.args.size() > paramCount) {
        semaPanic("more arguments than requested");
    }
}
void SemanticAnalyser::visit(RoutineDefinition& node) {
    addSymbol(Symbol(node.identifier, SymbolKind::Routine));
    node.scope->accept(*this);
}
void SemanticAnalyser::visit(SectionDefinition& node) {
    addSymbol(Symbol(node.identifier, SymbolKind::Routine));
    node.scope->accept(*this);
}
void SemanticAnalyser::visit(Global& node) {
    if (!symbolExists(node.identifier))
        semaPanic("cannot make identifier \"" + node.identifier + "\" global; it is not declared",
                  node.location);

    if (getSymbol(node.identifier)->kind != SymbolKind::Routine) {
        semaPanic("cannot use 'global' on symbol \"" + node.identifier + "\"; it is not a routine");
    }
}
void SemanticAnalyser::visit(RoutineDeclaration& node) {
    addSymbol(Symbol(node.identifier, SymbolKind::Routine));
}
void SemanticAnalyser::visit(AsmInstruction& node) {}
void SemanticAnalyser::visit(DeleteVariable& node) {
    if (!symbolExists(node.identifier))
        semaPanic("cannot delete \"" + node.identifier + "\"; it is not declared");

    removeSymbol(node.identifier);

    if (getSymbol(node.identifier)->kind != SymbolKind::Variable)
        return; // if it's not a variable, it's not in active memory

    auto it = std::find_if(activeMemory.begin(), activeMemory.end(),
                           [&](const auto& p) { return p.second == node.identifier; });

    if (it != activeMemory.end())
        activeMemory.erase(it);
    else
        semaPanic("Internal, cannot erase memory \"" + it->first + "\" occupied by \"" +
                  it->second + "\"");
}
