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
using namespace std;
using json = nlohmann::json;

// Token types
enum TokenType {
    ID, KEYWORD, CHAR, INT, FLOAT, PLUS, MINUS, MULTIPLY, DIVIDE, EQUAL, REMINDER,
    SEMICOLON, CONST, OPENPARENT, CLOSINGPARENT
};

// Token structure
struct Token {
    TokenType type;
    string lexeme;
};

class Lexer{
private:
    queue<Token> tokens;
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
                if (lexeme == "int" || lexeme == "float" || lexeme == "char" || lexeme == "string"){
                    tokens.push({KEYWORD, lexeme});
                }
                else if(lexeme == "const"){
                    tokens.push({CONST, lexeme});
                }
                else {
                    tokens.push({ID, lexeme});
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
                    tokens.push({FLOAT, lexeme});
                } else {
                    tokens.push({INT, lexeme});
                }
            }
            // Chars
            else if (currentChar == '\'') {
                pos++;
                if (pos < input.length()) {
                    std::string lexeme = "'";
                    lexeme += input[pos];
                    lexeme += "'";
                    tokens.push({CHAR, lexeme});
                    pos++;
                    if (pos < input.length() && input[pos] == '\'') {
                        pos++;
                    } else {
                        cerr << "syntaxError: Expected closing single quote for character literal at position " << pos << endl;
                    }
                } else {
                    cerr << "syntaxError: Unexpected end of input after single quote for character literal at position " << pos << endl;
                }
            }
            // Operators and semicolon
            else {
                switch (currentChar) {
                    case '+':
                        tokens.push({PLUS, "+"});
                        break;
                    case '-':
                        tokens.push({MINUS, "-"});
                        break;
                    case '*':
                        tokens.push({MULTIPLY, "*"});
                        break;
                    case '/':
                        tokens.push({DIVIDE, "/"});
                        break;
                    case '%':
                        tokens.push({REMINDER, "%"});
                        break;
                    case ';':
                        tokens.push({SEMICOLON, ";"});
                        break;
                    case '=':
                        tokens.push({EQUAL, "="});
                        break;
                    case '(':
                        tokens.push({OPENPARENT, "("});
                        break;
                    case ')':
                        tokens.push({CLOSINGPARENT, ")"});
                        break;
                    default:
                        cerr << "syntaxError: Unrecognized character at position " << pos << endl;
                        pos++;
                        break;
                }
                pos++;
            }
        }
    }

    queue<Token> getTokens(){
        return tokens;
    }
};
