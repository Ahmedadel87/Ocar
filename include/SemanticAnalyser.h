#pragma once
#include "Common.h"
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

template <typename T> using uq = std::unique_ptr<T>;

enum class SymbolKind { Variable, Function };
struct Symbol {
    std::string identifier;
    SymbolKind kind;
    VariableType type;
    std::vector<VariableType> parameters;

    Symbol(const std::string& identifier_, SymbolKind kind_, VariableType type_,
           std::vector<VariableType> parameters_)
        : identifier(identifier_), kind(kind_), type(type_), parameters(parameters_) {}
};

class SemanticAnalyser : public Visitor {
    uq<ScopeBlock> ast = nullptr;
    struct Scope {
        Scope* parent = nullptr;
        std::unordered_map<std::string, uq<Symbol>> symbols;

        explicit Scope(Scope* parent_) : parent(parent_) {}
    };
    std::vector<uq<Scope>> stack;

    void enter_scope();
    void exit_scope();
    void addSymbol(const Symbol& symbol);
    void semaPanic(const std::string& msg, SourceLocation src = SourceLocation());

    bool commandIncluded(const std::string& cmd);
    bool commandExists(const std::string& cmd);
    bool symbolExists(const std::string& identifier) const;
    VariableType getSymbolType(const std::string& identifier) const;
    const Symbol* getSymbol(const std::string& identifier) const;

    ExpressionInfo analyseExpression(Expression* expr);

public:
    void visit(ScopeBlock& node) override;
    void visit(StringLiteral& node) override;
    void visit(FloatLiteral& node) override;
    void visit(IntegerLiteral& node) override;
    void visit(BooleanLiteral& node) override;
    void visit(VoidLiteral& node) override;
    void visit(VariableDefinition& node) override;
    void visit(BinaryExpression& node) override;
    void visit(IfStatement& node) override;
    void visit(FunctionCallExpr& node) override;
    void visit(VariableReference& node) override;
    void visit(UnaryExpression& node) override;
    void visit(VariableReassignment& node) override;
    void visit(FunctionCallStmt& node) override;
    void visit(ElseIfStatement& node) override;
    void visit(ElseStatement& node) override;
    void visit(ForLoop& node) override;
    void visit(FunctionDefinition& node) override;

    std::unique_ptr<ScopeBlock> hand_over_AST();
    void load_ast(uq<ScopeBlock> ast_);
    void analyse();
};