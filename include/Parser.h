#pragma once
#include "Common.h"
#include "Tokenizer.h"
#include <memory>
#include <vector>

class Parser {
private:
    std::vector<Token> tokens;
    std::unique_ptr<ScopeBlock> entry_point = std::make_unique<ScopeBlock>();

    int cursor = 0;

    Token& peek(int offset = 0);
    Token& previous(int offset = 0);
    Token eat(TokenType type, const std::string& msg = "");
    bool isEnd();
    bool match(TokenType type);
    bool check(TokenType type);
    void advance(int offset = 1);
    void parserPanic(const std::string& msg, const SourceLocation& location = SourceLocation());

    // clang-format off
    std::unique_ptr<Expression> parseExpression();
        std::unique_ptr<Expression> parseLogicalOr();
        std::unique_ptr<Expression> parseLogicalAnd();
        std::unique_ptr<Expression> parseEquality();
        std::unique_ptr<Expression> parseRelational();
        std::unique_ptr<Expression> parseAddition();
        std::unique_ptr<Expression> parseMultiplication();
        std::unique_ptr<Expression> parseUnary();
        std::unique_ptr<Expression> parseFactor();
        std::unique_ptr<Literal> parseLiteral();
            std::unique_ptr<IntegerLiteral> parseInteger();
            std::unique_ptr<FloatLiteral> parseFloat();
            std::unique_ptr<StringLiteral> parseString();

        std::unique_ptr<FunctionCallExpr> parseFunctionCallExpr();
        
    void parsePreword();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<FunctionDefinition> parseFunctionDefinition();
    std::unique_ptr<FunctionCallStmt> parseFunctionCallStmt();
    std::unique_ptr<VariableDefinition> parseVariableDeclaration();
    std::unique_ptr<VariableReassignment> parseVariableReassignment();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<ElseIfStatement> parseElseIfStatement();
    std::unique_ptr<ElseStatement> parseElseStatement();
    std::unique_ptr<ForLoop> parseForLoop();
    std::unique_ptr<ScopeBlock> parseScope(bool require_brackets = true);

    // clang-format on
public:
    void load_tokens(std::vector<Token> tokens_);
    void parse();
    std::unique_ptr<ScopeBlock> hand_over_AST();
};
