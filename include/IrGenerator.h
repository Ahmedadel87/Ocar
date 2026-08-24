#pragma once
#include "Common.h"
#include "ErrorHandler.h"
#include <memory>
#include <ostream>
#include <unordered_map>

namespace casmlang {
const std::unordered_map<ComparativeConditions, std::string> condMap = {
    {ComparativeConditions::EQ, "je"}, {ComparativeConditions::NEQ, "jne"},
    {ComparativeConditions::GT, "jg"}, {ComparativeConditions::GTEQ, "jge"},
    {ComparativeConditions::LT, "jl"}, {ComparativeConditions::LTEQ, "jle"}};

class Ir {
public:
    virtual ~Ir() = default;
    virtual void print(std::ostream& stream) const = 0;
};
class IrExpr : public Ir {
public:
    virtual ~IrExpr() = default;
    virtual void print(std::ostream& stream) const = 0;
};
class IrLit : public IrExpr {
public:
    virtual ~IrLit() = default;
    virtual void print(std::ostream& stream) const = 0;
};
class IrStringLit : public IrExpr {
public:
    std::string value;

    IrStringLit(const std::string& value_) : value(value_) {}
    void print(std::ostream& stream) const override {
        stream << "\"" << value << "\"";
    }
};
class IrIntLit : public IrExpr {
public:
    int number;

    IrIntLit(int number_) : number(number_) {}
    void print(std::ostream& stream) const override {
        stream << "(int)" << number;
    }
};

class IrStmt : public Ir {
public:
    virtual ~IrStmt() = default;
};
class IrMovStmt : public IrStmt {
public:
    std::string memname;
    std::unique_ptr<IrExpr> value;

    IrMovStmt(const std::string& memname_, std::unique_ptr<IrExpr> value_)
        : memname(memname_), value(std::move(value_)) {}
    void print(std::ostream& stream) const override {
        stream << "mov ";
        value->print(stream);
        stream << " -> " << memname;
    }
};
class IrLabelStmt : public IrStmt {
public:
    std::string labeltext;
    IrLabelStmt(const std::string& labeltext_) : labeltext(labeltext_) {}

    void print(std::ostream& stream) const override {
        stream << labeltext << ":";
    }
};
class IrRetStmt : public IrStmt {
public:
    void print(std::ostream& stream) const override {
        stream << "ret";
    }
};
class IrRtnCall : public IrStmt {
public:
    std::string rtnname;
    IrRtnCall(const std::string& rtnname_) : rtnname(rtnname_) {}

    void print(std::ostream& stream) const override {
        stream << "call " << rtnname;
    }
};
class IrGlobal : public IrStmt {
public:
    std::string identifier;

    IrGlobal(const std::string& identifier_) : identifier(identifier_) {}
    void print(std::ostream& stream) const override {
        stream << "global " << identifier;
    }
};
class IrSection : public IrStmt {
public:
    std::string text;
    IrSection(const std::string& text_) : text(text_) {}

    void print(std::ostream& stream) const override {
        stream << "section ." << text;
    }
};
class IrAsm : public IrStmt {
public:
    std::string instruction;

    IrAsm(const std::string& instruction_) : instruction(instruction_) {}

    void print(std::ostream& stream) const override {
        stream << "@" << instruction;
    }
};
class IrJmp : public IrStmt {
public:
    std::string name = "jmp";
    std::string location;

    IrJmp(const std::string& location_, const std::string& name_ = "jmp")
        : name(name_), location(location_) {}

    void print(std::ostream& stream) const override {
        stream << name << " " << location;
    }
};

class IrGenerator : public Visitor {
private:
    int labelCounter = 0;
    std::unique_ptr<ScopeBlock> entry_point;
    std::vector<std::unique_ptr<Ir>> ir;
    std::unique_ptr<Ir> currentNode;
    std::string generateLabel();

    template <typename T> std::unique_ptr<T> get_current_as(const std::string& err = "") {
        if (!currentNode.get())
            panic("Internal, cannot cast currentNode as it is nullptr");

        std::string errmsg;
        errmsg = err;
        if (err.empty())
            errmsg = "Internal, invalid cast for currentNode";

        auto* base = currentNode.get();
        auto cur_T = dynamic_cast<T*>(base);
        if (!cur_T)
            panic(errmsg);
        currentNode.release();
        std::unique_ptr<T> derived(cur_T);

        return derived;
    }

public:
    void load_ast(std::unique_ptr<ScopeBlock> ast);
    std::vector<std::unique_ptr<Ir>> give_ir();

    void generate_ir();
    void print_ir(std::ostream& stream);

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
};

} // namespace casmlang
