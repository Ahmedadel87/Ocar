#pragma once
#include "Common.h"
#include "ErrorHandler.h"
#include <sstream>
#include <string>
#include <vector>

enum class WrapType {
    NONE,
    SINGLE_QUOTE,
    DOUBLE_QUOTE,
    VARIABLE_WRAP,
    SINGLE_VARIABE_WRAP,
    DOUBLE_VARIABLE_WRAP,
    COMMAND_WRAP,
    SINGLE_COMMAND_WRAP,
    DOUBLE_COMMAND_WRAP
};

class Bash {
public:
    virtual ~Bash() = default;
};
class BashExpression : public Bash {
public:
    virtual ~BashExpression() = default;
    WrapType wrap_type = WrapType::NONE;
    VariableType type;

    BashExpression(WrapType wrap_type_, VariableType type_) : wrap_type(wrap_type_), type(type_) {}
};
class BashBinaryExpression : public BashExpression {
public:
    std::unique_ptr<BashExpression> left;
    std::unique_ptr<BashExpression> right;
    BinaryOperation operation;

    BashBinaryExpression(std::unique_ptr<BashExpression> left_, BinaryOperation op,
                         std::unique_ptr<BashExpression> right_, VariableType type,
                         WrapType wrap_type_ = WrapType::NONE)
        : left(std::move(left_)), right(std::move(right_)), operation(op),
          BashExpression(wrap_type_, type) {}
};
class BashLiteral : public BashExpression {
public:
    std::string data;

    BashLiteral(WrapType wrap_type, VariableType type, const std::string& data_)
        : BashExpression(wrap_type, type), data(data_) {}
};
class BashFunctionCallExpression : public BashExpression {
public:
    std::string identifier;
    std::vector<std::unique_ptr<BashExpression>> args;

    BashFunctionCallExpression(WrapType wrap_type, const std::string& identifier_,
                               VariableType type_,
                               std::vector<std::unique_ptr<BashExpression>> args_)
        : BashExpression(wrap_type, type_), identifier(identifier_), args(std::move(args_)) {}
};
class BashVariableReference : public BashExpression {
public:
    std::string identifier;

    BashVariableReference(WrapType wrap_type, VariableType type, const std::string& identifier_)
        : BashExpression(wrap_type, type), identifier(identifier_) {}
};
class BashUnaryExpression : public BashExpression {
public:
    UnaryOperation operation;
    std::unique_ptr<BashExpression> value;

    BashUnaryExpression(WrapType wrap_type, VariableType var_type, SourceLocation lct,
                        UnaryOperation op, std::unique_ptr<BashExpression> value_)
        : BashExpression(wrap_type, var_type), operation(op), value(std::move(value_)) {}
};

class BashStatement : public Bash {
public:
    virtual ~BashStatement() = default;
};
class BashAssignment : public BashStatement {
public:
    std::string identifier;
    std::unique_ptr<BashExpression> right;
    bool is_definition = false;

    BashAssignment(const std::string& identifier_, std::unique_ptr<BashExpression> right_,
                   bool is_definition_)
        : identifier(identifier_), right(std::move(right_)), is_definition(is_definition_) {}
};
class BashIfStatement : public BashStatement {
public:
    std::unique_ptr<BashExpression> condition;

    BashIfStatement(std::unique_ptr<BashExpression> condition_)
        : condition(std::move(condition_)) {}
};
class BashEndIfStatement : public BashStatement {
public:
    BashEndIfStatement() = default;
};
class BashFunctionCallStatement : public BashStatement {
public:
    std::string identifier;
    std::vector<std::unique_ptr<BashExpression>> args;

    BashFunctionCallStatement(const std::string& identifier_,
                              std::vector<std::unique_ptr<BashExpression>> args_)
        : identifier(identifier_), args(std::move(args_)) {}
};
class BashElseIfStatement : public BashStatement {
public:
    std::unique_ptr<BashExpression> condition;

    BashElseIfStatement(std::unique_ptr<BashExpression> condition_)
        : condition(std::move(condition_)) {}
};
class BashElseStatement : public BashStatement {};
class BashForLoop : public BashStatement {
public:
    std::unique_ptr<BashAssignment> assignment;
    std::unique_ptr<BashBinaryExpression> condition;
    std::unique_ptr<BashStatement> then_do;

    BashForLoop(std::unique_ptr<BashAssignment> assignment_,
                std::unique_ptr<BashBinaryExpression> condition_,
                std::unique_ptr<BashStatement> then_do_)
        : assignment(std::move(assignment_)), condition(std::move(condition_)),
          then_do(std::move(then_do_)) {}
};
class BashDone : public BashStatement {};
class BashFunctionDefinition : public BashStatement {
public:
    std::string identifier;

    BashFunctionDefinition(const std::string& identifier_) : identifier(identifier_) {}
};
class BashFunctionDefinitionEnd : public BashStatement {};

class IrGenerator : public Visitor {
private:
    void genPanic(const std::string& msg, const SourceLocation& src = SourceLocation());

    std::unique_ptr<ScopeBlock> ast;
    std::vector<std::unique_ptr<BashStatement>> bash_ir;
    std::unique_ptr<Bash> current_node;
    ExpressionInfo analyseExpression(Expression* expr);
    std::unique_ptr<BashExpression> getAsExpression(std::unique_ptr<Bash> bash);

public:
    void load_ast(std::unique_ptr<ScopeBlock> ast__);

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

    void generate_bash_ir();
    std::vector<std::unique_ptr<BashStatement>> get_ir();
};