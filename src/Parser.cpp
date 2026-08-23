#include "../include/Parser.h"
#include "../include/Common.h"
#include "../include/ErrorHandler.h"
#include "Tokenizer.h"
#include <iostream>

void Parser::load_tokens(std::vector<Token> tkns) {
    tokens = std::move(tkns);
}
std::unique_ptr<ScopeBlock> Parser::hand_over_AST() {
    return std::move(entry_point);
}

void Parser::parse() {
    cursor = 0;
    entry_point = parseScope(false);

    PrettyPrinter pretty(std::cout);
    entry_point->accept(pretty);
}
std::unique_ptr<ScopeBlock> Parser::parseScope(bool require_brackets) {
    std::unique_ptr<ScopeBlock> scope = std::make_unique<ScopeBlock>();

    if (require_brackets)
        eat(TokenType::LBrace, "expected '{' on scope entry");

    while (!isEnd()) {
        auto stmt = parseStatement();
        if (stmt != nullptr) {
            scope->children.push_back(std::move(stmt));
        }

        if (match(TokenType::EndOfFile))
            break;
        if (require_brackets && match(TokenType::RBrace))
            break;
    }
    return std::move(scope);
}

// == PARSE FUNCTIONS ==
std::unique_ptr<Statement> Parser::parseStatement() {
    if (match(TokenType::Semicolon))
        return nullptr;
    else if (check(TokenType::KeywordRegister))
        return parseVariableDeclaration();
    else if (check(TokenType::Identifier)) {
        if (peek(1).type == TokenType::LParen)
            return parseRoutineCallStmt();
        return parseVariableReassignment();
    } else if (check(TokenType::KeywordRoutine))
        return parseRoutineDefinition();
    else {
        parserPanic("invalid token \"" + peek().lexeme + "\"", peek().location);
        return nullptr;
    }
}
std::unique_ptr<VariableDefinition> Parser::parseVariableDeclaration() {
    Token tkn = eat(TokenType::KeywordRegister, "expected memory name for variable declaration");

    std::string type_string = tkn.lexeme;

    std::string identifier =
        eat(TokenType::Identifier, "expected identifier for variable declaration").lexeme;

    eat(TokenType::EqualSign, "expected equals sign for variable declaration");

    std::unique_ptr<Expression> value = parseExpression();
    eat(TokenType::Semicolon, "expected semi colon after variable declaration");

    return std::make_unique<VariableDefinition>(tkn.location, type_string, identifier,
                                                std::move(value));
}
std::unique_ptr<RoutineCallExpr> Parser::parseFunctionCallExpr() {
    Token tkn = eat(TokenType::Identifier, "expected identifier for function call");
    std::string identifier = tkn.lexeme;
    eat(TokenType::LParen, "expected '(' after identifier for function call");
    std::vector<std::unique_ptr<Expression>> args;

    while (!isEnd() && !match(TokenType::RParen)) {
        args.push_back(parseExpression());
        if (match(TokenType::RParen))
            break;
        if (isEnd()) {
            parserPanic("unexpected eof token before closing parenthesis of function call");
            break;
        }

        eat(TokenType::Comma);
    }

    return std::make_unique<RoutineCallExpr>(tkn.location, identifier, std::move(args));
}
std::unique_ptr<VariableReassignment> Parser::parseVariableReassignment() {
    Token tkn = eat(TokenType::Identifier, "expected identifier for variable reassignment");
    std::string identifier = tkn.lexeme;

    eat(TokenType::EqualSign, "expected equal sign after identifier in variable reassignment");
    std::unique_ptr<VariableReassignment> ptr =
        std::make_unique<VariableReassignment>(tkn.location, identifier, parseExpression());

    eat(TokenType::Semicolon, "expected semicolon after statement");
    return std::move(ptr);
}
std::unique_ptr<RoutineCallStmt> Parser::parseRoutineCallStmt() {
    Token tkn = eat(TokenType::Identifier, "expected identifier for routine call");
    std::string identifier = tkn.lexeme;

    eat(TokenType::LParen, "expected '(' after identifier for routine call");
    std::vector<std::unique_ptr<Expression>> args;

    while (!isEnd() && !match(TokenType::RParen)) {
        args.push_back(parseExpression());
        if (match(TokenType::RParen))
            break;
        if (isEnd()) {
            parserPanic("unexpected eof token before closing parenthesis of routine call");
            break;
        }

        eat(TokenType::Comma, "expected comma between routine arguments");
    }

    eat(TokenType::Semicolon, "expected semi colon after routine call");
    return std::make_unique<RoutineCallStmt>(tkn.location, identifier, std::move(args));
}

std::unique_ptr<RoutineDefinition> Parser::parseRoutineDefinition() {
    Token tkn = eat(TokenType::KeywordRoutine, "expected keyword 'rtn' for routine definition");

    std::string type_string = tkn.lexeme;

    std::string identifier = eat(TokenType::Identifier, "expected routine identifier").lexeme;
    eat(TokenType::LParen, "expected '(' after identifier for routine definition");
    eat(TokenType::RParen, "expected ')' after identifier for routine definition");
    auto scope = parseScope(true);

    return std::make_unique<RoutineDefinition>(identifier, std::move(scope));
}

// == EXPRESSION PARSERS ==
std::unique_ptr<Expression> Parser::parseExpression() {
    return parseFactor();
}
std::unique_ptr<Expression> Parser::parseFactor() {
    auto tkn = peek();
    if (check(TokenType::LParen)) {
        eat(TokenType::LParen, "expected '('");
        auto expr = parseExpression();
        eat(TokenType::RParen, "expected ')'");
        return std::move(expr);
    } else if (check(TokenType::Identifier)) {
        std::string identifier = eat(TokenType::Identifier, "expected identifier").lexeme;
        if (match(TokenType::LParen)) {
            advance(-2);
            return parseFunctionCallExpr();
        }
        return std::make_unique<VariableReference>(tkn.location, identifier);
    }
    return parseLiteral();
}
std::unique_ptr<Literal> Parser::parseLiteral() {
    auto token = peek();
    advance();

    if (token.type == TokenType::StringLiteral) {
        return std::make_unique<StringLiteral>(token.location, token.lexeme);
    } else if (token.type == TokenType::BoolLiteral &&
               (token.lexeme == "true" || token.lexeme == "false")) {
        return std::make_unique<BooleanLiteral>(token.location, token.lexeme == "true");
    } else if (token.type == TokenType::IntegerLiteral) {
        return std::make_unique<IntegerLiteral>(token.location, std::stoi(token.lexeme));
    } else if (token.type == TokenType::FloatLiteral) {
        return std::make_unique<FloatLiteral>(token.location, std::stof(token.lexeme));
    }

    parserPanic("invalid token " + token.lexeme, token.location);
    return std::unique_ptr<VoidLiteral>();
}
std::unique_ptr<IntegerLiteral> Parser::parseInteger() {
    bool negative = false;
    if (match(TokenType::Minus)) {
        negative = true;
    }

    Token tkn = eat(TokenType::IntegerLiteral, "expected integer literal");
    return std::make_unique<IntegerLiteral>(tkn.location, std::stoi(tkn.lexeme));
}
std::unique_ptr<FloatLiteral> Parser::parseFloat() {
    bool negative = false;
    if (match(TokenType::Minus)) {
        negative = true;
    }

    Token tkn = eat(TokenType::FloatLiteral, "expected float literal");
    return std::make_unique<FloatLiteral>(tkn.location, std::stof(tkn.lexeme));
}
std::unique_ptr<StringLiteral> Parser::parseString() {
    Token tkn = eat(TokenType::StringLiteral, "expected a string literal");
    return std::make_unique<StringLiteral>(tkn.location, tkn.lexeme);
}

// == HELPERS ==
Token& Parser::peek(int offset) {
    if (cursor + offset > tokens.size()) {
        parserPanic("cannot peek into token number " + std::to_string(cursor + offset) +
                    "; such token does not exist.");
    }
    return tokens[cursor + offset];
}
Token& Parser::previous(int offset) {
    if (cursor - offset > tokens.size()) {
        parserPanic("cannot peek into previous token number " + std::to_string(cursor - offset) +
                    "; such token does not exist.");
    }
    return tokens[cursor - offset];
}
bool Parser::isEnd() {
    return check(TokenType::EndOfFile);
}
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    } else {
        return false;
    }
}
bool Parser::check(TokenType type) {
    if (peek().type == type) {
        return true;
    }
    return false;
}
void Parser::advance(int offset) {
    if ((cursor + offset > tokens.size()) && offset > 0) {
        parserPanic("cannot advance to position " + std::to_string(cursor + offset) +
                    "; position is out of bounds.");
    }
    if ((cursor + offset < 0) && offset < 0) {
        parserPanic("cannot advance to position " + std::to_string(cursor + offset) +
                    "; position is out of bounds.");
    }
    cursor += offset;
}
Token Parser::eat(TokenType type, const std::string& msg) {
    if (!check(type))
        parserPanic(msg, peek().location);

    Token t = peek();
    advance();
    return t;
}
void Parser::parserPanic(const std::string& msg, const SourceLocation& src) {
    panic("[PARSER PANIC] " + msg + " [AT " + std::to_string(src.row) + ":" +
          std::to_string(src.column) + "]");
}
