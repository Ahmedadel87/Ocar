#pragma once
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum class TokenType {
    None,

    Identifier,
    StringLiteral,
    IntegerLiteral,
    FloatLiteral,
    CharLiteral,
    BoolLiteral,
    NullLiteral,

    AsmInstruction,

    LParen,
    RParen,
    LBrace,
    RBrace,
    Comma,
    Semicolon,
    EqualSign,
    Minus,
    SnailSign,

    KeywordRegister,
    KeywordRoutine,
    KeywordSection,
    KeywordGlobal,
    KeywordNoreturn,
    KeywordDelete,
    KeywordFree,

    EndOfFile
};
enum class UnaryOperation { COMPLEMENT, INCREMENT, DECREMENT, NEGATE };
enum class BinaryOperation {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    POWER,
    ASSIGN,

    EQUALS,
    NOT_EQUALS,
    GREATER_THAN,
    GREATER_THAN_OR_EQUAL,
    LESS_THAN,
    LESS_THAN_OR_EQUAL,

    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_XOR
};
enum class AssignOperation {
    ASSIGN,
    ADDASSIGN,
    SUBTRACTASSIGN,
    MULTIPLYASSIGN,
    DIVIDEASSIGN,
    POWERASSIGN
};

class SourceLocation {
public:
    int row = -1;
    int column = -1;

    SourceLocation(int r, int c) : row(r), column(c) {}
    SourceLocation() = default;
};
const std::unordered_map<std::string, TokenType> word_table{
    {{"", TokenType::None},
     {"rtn", TokenType::KeywordRoutine},
     {"sect", TokenType::KeywordSection},
     {"global", TokenType::KeywordGlobal},
     {"noret", TokenType::KeywordNoreturn},
     {"delete", TokenType::KeywordDelete},
     {"free", TokenType::KeywordFree},

     {"rax", TokenType::KeywordRegister},
     {"rbx", TokenType::KeywordRegister},
     {"rcx", TokenType::KeywordRegister},
     {"rdx", TokenType::KeywordRegister},
     {"rsi", TokenType::KeywordRegister},
     {"rdi", TokenType::KeywordRegister},
     {"rbp", TokenType::KeywordRegister},
     {"rsp", TokenType::KeywordRegister},
     {"r8", TokenType::KeywordRegister},
     {"r9", TokenType::KeywordRegister},
     {"r10", TokenType::KeywordRegister},
     {"r11", TokenType::KeywordRegister},
     {"r12", TokenType::KeywordRegister},
     {"r13", TokenType::KeywordRegister},
     {"r14", TokenType::KeywordRegister},
     {"r15", TokenType::KeywordRegister},

     {"true", TokenType::BoolLiteral},
     {"false", TokenType::BoolLiteral},

     {"__END_OF_FILE__", TokenType::EndOfFile},
     {"(", TokenType::LParen},
     {")", TokenType::RParen},
     {"{", TokenType::LBrace},
     {"}", TokenType::RBrace},
     {";", TokenType::Semicolon},
     {"-", TokenType::Minus},
     {",", TokenType::Comma},
     {"@", TokenType::SnailSign},
     {"=", TokenType::EqualSign}}};
struct ExpressionInfo {
    bool isLValue = false;
    bool isConstant = false;

    ExpressionInfo(bool isLValue_ = false, bool isConstant_ = false)
        : isLValue(isLValue_), isConstant(isConstant_) {}
};

class Visitor;
class PrettyPrinter;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(Visitor& visitor) = 0;
};
class ScopeBlock : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> children;

    ScopeBlock(std::vector<std::unique_ptr<ASTNode>> children_) : children(std::move(children_)) {}
    ScopeBlock() = default;
    void accept(Visitor& visitor) override;
};

class Expression : public ASTNode {
public:
    SourceLocation location;

    virtual ~Expression() = default;
    virtual void accept(Visitor& visitor) = 0;

    Expression(const SourceLocation& lct) : location(lct) {}
};
class BinaryExpression : public Expression {
public:
    std::unique_ptr<Expression> left;
    BinaryOperation op;
    std::unique_ptr<Expression> right;

    void accept(Visitor& visitor) override;
    BinaryExpression(const SourceLocation& src, std::unique_ptr<Expression> l, BinaryOperation op_,
                     std::unique_ptr<Expression> r)
        : Expression(src), left(std::move(l)), op(op_), right(std::move(r)) {}
};
class RoutineCallExpr : public Expression {
public:
    std::string identifier;
    std::vector<std::unique_ptr<Expression>> args;

    void accept(Visitor& visitor) override;
    RoutineCallExpr(const SourceLocation& src, const std::string& identifier_,
                    std::vector<std::unique_ptr<Expression>> args_)
        : Expression(src), identifier(identifier_), args(std::move(args_)) {}
};
class VariableReference : public Expression {
public:
    std::string identifier;

    void accept(Visitor& visitor) override;
    VariableReference(const SourceLocation& src, const std::string& identifier_)
        : Expression(src), identifier(identifier_) {}
};
class UnaryExpression : public Expression {
public:
    std::unique_ptr<Expression> value;
    UnaryOperation op;

    void accept(Visitor& visitor) override;
    UnaryExpression(const SourceLocation& src, std::unique_ptr<Expression> value_,
                    UnaryOperation op_)
        : Expression(src), value(std::move(value_)), op(op_) {}
};

class Statement : public ASTNode {
public:
    SourceLocation location;

    virtual ~Statement() = default;
    virtual void accept(Visitor& visitor) = 0;

    Statement(const SourceLocation& lct) : location(lct) {}
    Statement() {}
};
class ConditionalStatement : public Statement {
public:
    virtual ~ConditionalStatement() = default;
    virtual void accept(Visitor& visitor) = 0;

    ConditionalStatement(const SourceLocation& lct) : Statement(lct) {}
};
class VariableDefinition : public Statement {
public:
    std::string identifier;
    std::string memory;
    std::unique_ptr<Expression> value;
    void accept(Visitor& visitor) override;

    VariableDefinition(SourceLocation& src, std::string& memory_, const std::string& identifier_,
                       std::unique_ptr<Expression> value_)
        : Statement(src), memory(memory_), identifier(identifier_), value(std::move(value_)) {}
};
class VariableReassignment : public Statement {
public:
    std::string identifier;
    std::string memory = "";
    std::unique_ptr<Expression> value;

    void accept(Visitor& visitor) override;

    VariableReassignment(SourceLocation& src, const std::string& identifier_,
                         std::unique_ptr<Expression> value_)
        : Statement(src), identifier(identifier_), value(std::move(value_)) {}
};
class RoutineCallStmt : public Statement {
public:
    std::string identifier;
    std::vector<std::unique_ptr<Expression>> args;

    void accept(Visitor& visitor) override;

    RoutineCallStmt(SourceLocation& src, const std::string& identifier_,
                    std::vector<std::unique_ptr<Expression>> args_)
        : Statement(src), identifier(identifier_), args(std::move(args_)) {}
};
class RoutineDefinition : public Statement {
public:
    std::string identifier;
    std::unique_ptr<ScopeBlock> scope;
    bool hasRet = true;

    void accept(Visitor& visitor) override;

    RoutineDefinition(const std::string& identifier_, std::unique_ptr<ScopeBlock> scope_ = nullptr,
                      bool hasRet_ = true)
        : identifier(identifier_), scope(std::move(scope_)), hasRet(hasRet_) {}
};
class SectionDefinition : public Statement {
public:
    std::string identifier;
    std::unique_ptr<ScopeBlock> scope;

    void accept(Visitor& visitor) override;

    SectionDefinition(const std::string& identifier_, std::unique_ptr<ScopeBlock> scope_ = nullptr)
        : identifier(identifier_), scope(std::move(scope_)) {}
};
class Global : public Statement {
public:
    std::string identifier;

    void accept(Visitor& visitor) override;

    Global(const std::string& identifier_) : identifier(identifier_) {}
};
class RoutineDeclaration : public Statement {
public:
    std::string identifier;

    RoutineDeclaration(const std::string& identifier_) : identifier(identifier_) {}

    void accept(Visitor& visitor) override;
};
class AsmInstruction : public Statement {
public:
    std::string instruction;
    void accept(Visitor& visitor) override;

    AsmInstruction(const std::string& instruction_) : instruction(instruction_) {}
};
class DeleteSymbol : public Statement {
public:
    std::string identifier;
    void accept(Visitor& visitor) override;

    DeleteSymbol(const std::string& identifier_) : identifier(identifier_) {}
};
class FreeMemory : public Statement {
public:
    std::string memoryName;
    void accept(Visitor& visitor) override;

    FreeMemory(const std::string& memoryName_) : memoryName(memoryName_) {}
};

class Literal : public Expression {
public:
    virtual ~Literal() = default;

    Literal(const SourceLocation& lct) : Expression(lct) {}
    virtual void accept(Visitor& visitor) = 0;
};
class StringLiteral : public Literal {
public:
    std::string string;

    StringLiteral(const SourceLocation& lct, const std::string str = "")
        : Literal(lct), string(str) {}
    void accept(Visitor& visitor) override;
};
class FloatLiteral : public Literal {
public:
    float number;

    FloatLiteral(const SourceLocation& lct, const float number_) : Literal(lct), number(number_) {}
    void accept(Visitor& visitor) override;
};
class IntegerLiteral : public Literal {
public:
    int number;

    IntegerLiteral(const SourceLocation& lct, const float number_)
        : Literal(lct), number(number_) {}
    void accept(Visitor& visitor) override;
};
class BooleanLiteral : public Literal {
public:
    bool state;

    BooleanLiteral(const SourceLocation& lct, const bool state_) : Literal(lct), state(state_) {}
    void accept(Visitor& visitor) override;
};
class VoidLiteral : public Literal {
public:
    void accept(Visitor& visitor) override;
};

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(ScopeBlock& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(FloatLiteral& node) = 0;
    virtual void visit(IntegerLiteral& node) = 0;
    virtual void visit(BooleanLiteral& node) = 0;
    virtual void visit(VoidLiteral& node) = 0;
    virtual void visit(VariableDefinition& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(RoutineCallExpr& node) = 0;
    virtual void visit(VariableReference& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(VariableReassignment& node) = 0;
    virtual void visit(RoutineCallStmt& node) = 0;
    virtual void visit(RoutineDefinition& node) = 0;
    virtual void visit(SectionDefinition& node) = 0;
    virtual void visit(Global& node) = 0;
    virtual void visit(RoutineDeclaration& node) = 0;
    virtual void visit(AsmInstruction& node) = 0;
    virtual void visit(DeleteSymbol& node) = 0;
    virtual void visit(FreeMemory& node) = 0;
};
class PrettyPrinter : public Visitor {
private:
    int indent = 0;
    std::ostream& stream;
    void printIndent();

public:
    PrettyPrinter(std::ostream& stream_) : stream(stream_) {}

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
};

inline void ScopeBlock::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void StringLiteral::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void FloatLiteral::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void IntegerLiteral::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void BooleanLiteral::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void VoidLiteral::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void VariableDefinition::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void BinaryExpression::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void RoutineCallExpr::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void VariableReference::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void UnaryExpression::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void VariableReassignment::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void RoutineCallStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void RoutineDefinition::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void SectionDefinition::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void Global::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void RoutineDeclaration::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void AsmInstruction::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void DeleteSymbol::accept(Visitor& visitor) {
    visitor.visit(*this);
}
inline void FreeMemory::accept(Visitor& visitor) {
    visitor.visit(*this);
}
