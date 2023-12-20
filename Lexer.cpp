//
// Created by elias on 26/11/2023.
//
#include <iostream>
#include <regex>
#include <vector>
#include <stack>
#include <queue>
#include <fstream>
#include "json.hpp"
#include "iomanip"
#include <map>
using namespace std;
using json = nlohmann::json;

// Token types
enum TokenType {
    ID, KEYWORD, CHAR, INT, FLOAT, PLUS, MINUS, MULTIPLY, DIVIDE, EQUAL, REMINDER,
    SEMICOLON, CONST, OPENPARENT, CLOSINGPARENT, DOLLAR
};

// Token structure
struct Token {
    TokenType type;
    string lexeme;
    string stringType;
};

class Lexer{
private:
    queue<Token> tokens;
    string errors;
public:
    explicit Lexer(const string& input) {
        size_t pos = 0;
        while (pos < input.length()) {
            char currentChar = input[pos];

            // Skip whitespace
            if (currentChar == ' ' || currentChar == '\t' || currentChar == '\n') {
                pos++;
                continue;
            }

            // Identifier or keyword
            if (isalpha(currentChar) || currentChar == '_') {
                size_t start = pos;
                while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
                    pos++;
                }
                string lexeme = input.substr(start, pos - start);

                // Check if it's a keyword
                if (lexeme == "int" || lexeme == "float" || lexeme == "char"){
                    tokens.push({KEYWORD, lexeme, "KEYWORD"});
                }
                else if(lexeme == "const"){
                    tokens.push({CONST, lexeme, "CONST"});
                }
                else {
                    tokens.push({ID, lexeme, "ID"});
                }
            }
                // Integers and Floats
            else if (isdigit(currentChar) || currentChar == '.') {
                size_t start = pos;
                bool isFloat = false;

                while (pos < input.length() && (isdigit(input[pos]) || input[pos] == '.')) {
                    if (input[pos] == '.') {
                        isFloat = true;
                    }
                    pos++;
                }

                string lexeme = input.substr(start, pos - start);
                if (isFloat) {
                    tokens.push({FLOAT, lexeme, "FLOAT"});
                } else {
                    tokens.push({INT, lexeme, "INT"});
                }
            }
                // Chars
            else if (currentChar == '\'') {
                pos++;
                if (pos < input.length()) {
                    std::string lexeme = "'";
                    lexeme += input[pos];
                    lexeme += "'";
                    tokens.push({CHAR, lexeme, "CHAR"});
                    pos++;
                    if (pos < input.length() && input[pos] == '\'') {
                        pos++;
                    } else {
                        errors += "syntaxError: Expected closing single quote for character literal at position " +
                                  to_string(pos) + "\n";
                    }
                } else {
                    errors += "syntaxError: Unexpected end of input after single quote for character literal at position " +
                              to_string(pos) + "\n";
                }
            }
                // Operators and semicolon
            else {
                switch (currentChar) {
                    case '+':
                        tokens.push({PLUS, "+", "PLUS"});
                        break;
                    case '-':
                        tokens.push({MINUS, "-", "MINUS"});
                        break;
                    case '*':
                        tokens.push({MULTIPLY, "*", "MULTIPLY"});
                        break;
                    case '/':
                        tokens.push({DIVIDE, "/", "DIVIDE"});
                        break;
                    case '%':
                        tokens.push({REMINDER, "%", "REMINDER"});
                        break;
                    case ';':
                        tokens.push({SEMICOLON, ";", "SEMICOLON"});
                        break;
                    case '=':
                        tokens.push({EQUAL, "=", "EQUAL"});
                        break;
                    case '(':
                        tokens.push({OPENPARENT, "(", "OPENPARENT"});
                        break;
                    case ')':
                        tokens.push({CLOSINGPARENT, ")", "CLOSINGPARENT"});
                        break;
                    default:
                        errors += "syntaxError: Unrecognized character at position " + to_string(pos) + "\n";
                        pos++;
                        break;
                }
                pos++;
            }
        }
        tokens.push({DOLLAR, "$", "DOLLAR"});
    }

    string getErrors(){
        return errors;
    }

    queue<Token> getTokens(){
        return tokens;
    }
};