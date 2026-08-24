#pragma once
#include "Common.h"
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

template <typename T> using uq = std::unique_ptr<T>;

enum class SymbolKind { Variable, Routine };
struct Symbol {
    std::string identifier;
    SymbolKind kind;
    int paramCount = 0;

    Symbol(const std::string& identifier_, SymbolKind kind_, int paramCount_ = 0)
        : identifier(identifier_), kind(kind_), paramCount(paramCount_) {}
};

class SemanticAnalyser : public Visitor {
    std::string emptystring = ""; // referenced later
    uq<ScopeBlock> ast = nullptr;
    struct Scope {
        Scope* parent = nullptr;
        std::unordered_map<std::string, uq<Symbol>> symbols;

        explicit Scope(Scope* parent_) : parent(parent_) {}
    };
    std::vector<uq<Scope>> stack;
    std::vector<std::pair<std::string, std::string>> activeMemory; // mem name -> var name

    void enter_scope();
    void exit_scope();
    void addSymbol(const Symbol& symbol);
    void removeSymbol(const std::string& identifier);
    void semaPanic(const std::string& msg, SourceLocation src = SourceLocation());

    void push_active_memory(const std::string& memname, const std::string& varname);
    void remove_active_memory(const std::string& varname);
    std::string& find_memory_by_varname(const std::string& varname);
    std::string& find_in_active_memory(const std::string& memname);

    bool symbolExists(const std::string& identifier) const;
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
    void visit(RoutineCallExpr& node) override;
    void visit(VariableReference& node) override;
    void visit(UnaryExpression& node) override;
    void visit(VariableReassignment& node) override;
    void visit(RoutineCallStmt& node) override;
    void visit(RoutineDefinition& node) override;
    void visit(SectionDefinition& node) override;
    void visit(Global& node) override;
    void visit(RoutineDeclaration& node) override;
    void visit(AsmInstruction& node) override;
    void visit(DeleteSymbol& node) override;
    void visit(FreeMemory& node) override;
    void visit(IfStatement& node) override;

    std::unique_ptr<ScopeBlock> hand_over_AST();
    void load_ast(uq<ScopeBlock> ast_);
    void analyse();
};