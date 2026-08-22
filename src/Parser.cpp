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

    if (check(TokenType::KeywordVartype)) {
        if (peek(2).type == TokenType::LParen) {
            return parseFunctionDefinition();
        }
        return parseVariableDeclaration();
    } else if (check(TokenType::KeywordIf)) {
        return parseIfStatement();
    } else if (check(TokenType::Identifier)) {
        if (peek(1).type == TokenType::LParen || peek(1).type == TokenType::Period) {
            return parseFunctionCallStmt();
        } else {
            return parseVariableReassignment();
        }
    } else if (check(TokenType::Hashtag)) {
        parsePreword();
        return nullptr;
    } else if (check(TokenType::KeywordFor)) {
        return parseForLoop();
    } else {
        parserPanic("invalid token \"" + peek().lexeme + "\"", peek().location);
        return nullptr;
    }
}
std::unique_ptr<VariableDefinition> Parser::parseVariableDeclaration() {
    Token tkn = eat(TokenType::KeywordVartype, "expected variable type for variable declaration");

    std::string type_string = tkn.lexeme;
    VariableType type = VariableType::VOID;

    if (type_string == "int")
        type = VariableType::INT;
    else if (type_string == "string")
        type = VariableType::STRING;
    else if (type_string == "float")
        type = VariableType::FLOAT;
    else if (type_string == "bool")
        type = VariableType::BOOL;
    else if (type_string == "auto")
        type = VariableType::VOID;
    else
        parserPanic("invalid type \"" + type_string + "\"", tkn.location);

    std::string identifier =
        eat(TokenType::Identifier, "expected identifier for variable declaration").lexeme;

    eat(TokenType::Equal, "expected equals sign for variable declaration");

    std::unique_ptr<Expression> value = parseExpression();
    eat(TokenType::Semicolon, "expected semi colon after variable declaration");

    return std::make_unique<VariableDefinition>(tkn.location, type, identifier, std::move(value));
}
std::unique_ptr<IfStatement> Parser::parseIfStatement() {
    SourceLocation lct = eat(TokenType::KeywordIf, "expected if keyword for if statement").location;
    eat(TokenType::LParen, "expected '(' after if statement");
    std::unique_ptr<Expression> condition = parseExpression();

    eat(TokenType::RParen, "expected ')' after if statement condition");

    std::unique_ptr<ScopeBlock> scope = parseScope(true);
    std::unique_ptr<ConditionalStatement> nextStmt = nullptr;
    if (check(TokenType::KeywordElse)) {
        if (peek(1).type == TokenType::KeywordIf) {
            nextStmt = parseElseIfStatement();
        } else {
            nextStmt = parseElseStatement();
        }
    }

    return std::make_unique<IfStatement>(lct, std::move(condition), std::move(scope),
                                         std::move(nextStmt));
}
std::unique_ptr<FunctionCallExpr> Parser::parseFunctionCallExpr() {
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

    return std::make_unique<FunctionCallExpr>(tkn.location, identifier, std::move(args));
}
std::unique_ptr<VariableReassignment> Parser::parseVariableReassignment() {
    Token tkn = eat(TokenType::Identifier, "expected identifier for variable reassignment");
    std::string identifier = tkn.lexeme;

    eat(TokenType::Equal, "expected equal sign after identifier in variable reassignment");
    std::unique_ptr<VariableReassignment> ptr =
        std::make_unique<VariableReassignment>(tkn.location, identifier, parseExpression());

    eat(TokenType::Semicolon, "expected semicolon after statement");
    return std::move(ptr);
}
void Parser::parsePreword() {
    eat(TokenType::Hashtag, "expected hashtag for preword");
    eat(TokenType::PrewordInclude, "expected include for preword");
    eat(TokenType::LessThan, "expected < before include name for preword");
    std::string includeName =
        eat(TokenType::Identifier, "expected include name in preword command").lexeme;
    eat(TokenType::GreaterThan, "expected > after include name in preword");
    includes.push_back(includeName);
}
std::unique_ptr<FunctionCallStmt> Parser::parseFunctionCallStmt() {
    Token tkn = eat(TokenType::Identifier, "expected identifier for function call");
    std::string identifier = tkn.lexeme;

    while (match(TokenType::Period)) {
        identifier += " ";
        identifier +=
            eat(TokenType::Identifier, "expected identifier continuation after period").lexeme;
    }

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

        eat(TokenType::Comma, "expected comma between function arguments");
    }

    eat(TokenType::Semicolon, "expected semi colon after function call");
    return std::make_unique<FunctionCallStmt>(tkn.location, identifier, std::move(args));
}
std::unique_ptr<ElseIfStatement> Parser::parseElseIfStatement() {
    auto lct =
        eat(TokenType::KeywordElse, "expected keyword 'else' for else if statement").location;
    eat(TokenType::KeywordIf, "expected 'if' keyword for else if statement");
    eat(TokenType::LParen, "expected '(' after if statement");
    std::unique_ptr<Expression> condition = parseExpression();

    eat(TokenType::RParen, "expected ')' after if statement condition");

    std::unique_ptr<ScopeBlock> scope = parseScope(true);
    std::unique_ptr<ConditionalStatement> nextStmt = nullptr;
    if (check(TokenType::KeywordElse)) {
        if (peek(1).type == TokenType::KeywordIf) {
            nextStmt = parseElseIfStatement();
        } else {
            nextStmt = parseElseStatement();
        }
    }

    return std::make_unique<ElseIfStatement>(lct, std::move(condition), std::move(scope),
                                             std::move(nextStmt));
}
std::unique_ptr<ElseStatement> Parser::parseElseStatement() {
    auto lct = eat(TokenType::KeywordElse, "expected 'else' keyword for else").location;
    std::unique_ptr<ScopeBlock> scope = parseScope(true);
    return std::make_unique<ElseStatement>(lct, std::move(scope));
}
std::unique_ptr<ForLoop> Parser::parseForLoop() {
    eat(TokenType::KeywordFor, "expected 'for' keyword for for loop");
    eat(TokenType::LParen, "expected '(' after 'for' for for loop");
    auto def = parseVariableDeclaration();
    auto cond = parseExpression();
    eat(TokenType::Semicolon);
    auto stmt = parseStatement();
    eat(TokenType::RParen, "expected ')' after 'for' for for loop");

    auto scope = parseScope(true);
    return std::make_unique<ForLoop>(std::move(def), std::move(cond), std::move(stmt),
                                     std::move(scope));
}
std::unique_ptr<FunctionDefinition> Parser::parseFunctionDefinition() {
    Token tkn = eat(TokenType::KeywordVartype, "expected variable type for function definition");

    std::string type_string = tkn.lexeme;
    VariableType type = VariableType::VOID;

    if (type_string == "int")
        type = VariableType::INT;
    else if (type_string == "string")
        type = VariableType::STRING;
    else if (type_string == "float")
        type = VariableType::FLOAT;
    else if (type_string == "bool")
        type = VariableType::BOOL;
    else if (type_string == "auto")
        type = VariableType::VOID;
    else if (type_string == "void")
        type = VariableType::VOID;
    else
        parserPanic("invalid type \"" + type_string + "\"", tkn.location);

    std::string identifier = eat(TokenType::Identifier, "expected function identifier").lexeme;
    eat(TokenType::LParen, "expected '(' after identifier for function definition");
    eat(TokenType::RParen, "expected ')' after identifier for function definition");
    auto scope = parseScope(true);

    return std::make_unique<FunctionDefinition>(identifier, std::move(scope));
}

// == EXPRESSION PARSERS ==

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseLogicalOr();
}
std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto tkn = peek();
    auto node = parseLogicalAnd();
    while (match(TokenType::DoubleStraightLine)) {
        auto right = parseLogicalAnd();
        node = std::make_unique<BinaryExpression>(tkn.location, std::move(node),
                                                  BinaryOperation::LOGICAL_OR, std::move(right));
    }
    return node;
}
std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto tkn = peek();
    auto node = parseEquality();
    while (match(TokenType::DoubleAmpersand)) {
        auto right = parseEquality();
        node = std::make_unique<BinaryExpression>(tkn.location, std::move(node),
                                                  BinaryOperation::LOGICAL_AND, std::move(right));
    }
    return node;
}
std::unique_ptr<Expression> Parser::parseEquality() {
    auto tkn = peek();
    auto node = parseRelational();
    while (check(TokenType::EqualEqual) || check(TokenType::NotEqual)) {
        BinaryOperation op =
            check(TokenType::EqualEqual) ? BinaryOperation::EQUALS : BinaryOperation::NOT_EQUALS;
        advance();

        auto right = parseRelational();
        node =
            std::make_unique<BinaryExpression>(tkn.location, std::move(node), op, std::move(right));
    }
    return node;
}
std::unique_ptr<Expression> Parser::parseRelational() {
    auto tkn = peek();
    auto node = parseAddition();
    while (check(TokenType::GreaterThan) || check(TokenType::GreaterEqual) ||
           check(TokenType::LessThan) || check(TokenType::LessEqual)) {
        BinaryOperation op;
        switch (peek().type) {
            case TokenType::GreaterThan:
                op = BinaryOperation::GREATER_THAN;
                break;
            case TokenType::GreaterEqual:
                op = BinaryOperation::GREATER_THAN_OR_EQUAL;
                break;
            case TokenType::LessThan:
                op = BinaryOperation::LESS_THAN;
                break;
            case TokenType::LessEqual:
                op = BinaryOperation::LESS_THAN_OR_EQUAL;
                break;

            default:
                parserPanic("invalid operation type \"" + peek().lexeme + "\"", peek().location);
                break;
        }
        advance();

        auto right = parseAddition();
        node =
            std::make_unique<BinaryExpression>(tkn.location, std::move(node), op, std::move(right));
    }
    return node;
}
std::unique_ptr<Expression> Parser::parseAddition() {
    auto tkn = peek();
    auto node = parseMultiplication();
    while (check(TokenType::Plus) || check(TokenType::Minus)) {
        BinaryOperation op =
            check(TokenType::Plus) ? BinaryOperation::ADD : BinaryOperation::SUBTRACT;
        advance();

        auto right = parseMultiplication();

        // wrap the current expresion with another one for a nice tree structure
        node =
            std::make_unique<BinaryExpression>(tkn.location, std::move(node), op, std::move(right));
    }
    return node;
}
std::unique_ptr<Expression> Parser::parseMultiplication() {
    auto tkn = peek();
    auto node = parseUnary();
    while (check(TokenType::Star) || check(TokenType::Slash)) {
        BinaryOperation op =
            check(TokenType::Star) ? BinaryOperation::MULTIPLY : BinaryOperation::DIVIDE;
        advance();

        auto right = parseUnary();

        node =
            std::make_unique<BinaryExpression>(tkn.location, std::move(node), op, std::move(right));
    }

    return node;
}
std::unique_ptr<Expression> Parser::parseUnary() {
    auto tkn = peek();

    if (match(TokenType::Exclamation)) {
        return std::make_unique<UnaryExpression>(tkn.location, parseUnary(),
                                                 UnaryOperation::COMPLEMENT);
    }
    if (match(TokenType::PlusPlus)) {
        return std::make_unique<UnaryExpression>(tkn.location, parseUnary(),
                                                 UnaryOperation::INCREMENT);
    }
    if (match(TokenType::MinusMinus)) {
        return std::make_unique<UnaryExpression>(tkn.location, parseUnary(),
                                                 UnaryOperation::DECREMENT);
    }
    if (match(TokenType::Minus)) {
        return std::make_unique<UnaryExpression>(tkn.location, parseUnary(),
                                                 UnaryOperation::NEGATE);
    }

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
        if (match(TokenType::Ampersand)) { // reflection operator
            return std::make_unique<StringLiteral>(tkn.location, identifier);
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
