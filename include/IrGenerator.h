#pragma once
#include "Common.h"
#include <memory>

namespace casmlang {
class Ir {
public:
    virtual ~Ir() = default;
};
class IrExpr : public Ir {
public:
    virtual ~IrExpr() = default;
};
class IrLit : public IrExpr {
public:
    virtual ~IrLit() = default;
};

class IrStmt : public Ir {
public:
    virtual ~IrStmt() = default;
};

class IrGenerator : public Visitor {
private:
    std::unique_ptr<ScopeBlock> entry_point;
    std::vector<std::unique_ptr<Ir>> ir;

public:
    void load_ast(std::unique_ptr<ScopeBlock>& ast);
    std::vector<std::unique_ptr<Ir>> give_ir();

    void generate_ir();

    void visit(ScopeBlock& node) override;
    void visit(StringLiteral& node) override;
    void visit(FloatLiteral& node) override;
    void visit(IntegerLiteral& node) override;
    void visit(BooleanLiteral& node) override;
    void visit(VoidLiteral& node) override;
    void visit(VariableDefinition& node) override;
    void visit(BinaryExpression& node) override;
    void visit(FunctionCallExpr& node) override;
    void visit(VariableReference& node) override;
    void visit(UnaryExpression& node) override;
    void visit(VariableReassignment& node) override;
    void visit(FunctionCallStmt& node) override;
    void visit(FunctionDefinition& node) override;
};

} // namespace casmlang
