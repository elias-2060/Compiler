//
// Created by elias on 26/11/2023.
//
#include <iostream>
#include <regex>
#include <vector>
using namespace std;

// Token types
enum TokenType {
    ID, KEYWORD, CHAR, STRING, INT, FLOAT, PLUS, MINUS, MULTIPLY, DIVIDE, EQUAL, REMINDER, AND, OR, ISEQUAL, ST, GT,
    SET, GET, NOT, NET, OPENBRACKETS, CLOSINGBRACKETS, SEMICOLON
};

// Token structure
struct Token {
    TokenType type;
    string lexeme;
};

class Lexer{
private:
    vector<Token> tokens;
public:
    Lexer(const string& input) {
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
                if (lexeme == "int" || lexeme == "float" || lexeme == "char" || lexeme == "void") {
                    tokens.push_back({KEYWORD, lexeme});
                } else {
                    tokens.push_back({ID, lexeme});
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
                    tokens.push_back({FLOAT, lexeme});
                } else {
                    tokens.push_back({INT, lexeme});
                }
            }
            // Chars
            else if (currentChar == '\'') {
                pos++;
                if (pos < input.length()) {
                    std::string lexeme = "'";
                    lexeme += input[pos];
                    lexeme += "'";
                    tokens.push_back({CHAR, lexeme});
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
            // Strings
            else if (currentChar == '\"') {
                pos++; // Consume the opening double quote
                size_t start = pos;
                while (pos < input.length() && input[pos] != '\"') {
                    pos++;
                }
                if (pos < input.length() && input[pos] == '\"') {
                    std::string lexeme = input.substr(start, pos - start);
                    tokens.push_back({STRING, lexeme});
                    pos++; // Consume the closing double quote
                } else {
                    cerr << "syntaxError: Expected closing double quote for string literal starting at position " << start << endl;
                }
            }
            // Operators and semicolon
            else {
                switch (currentChar) {
                    case '+':
                        tokens.push_back({PLUS, "+"});
                        break;
                    case '-':
                        tokens.push_back({MINUS, "-"});
                        break;
                    case '*':
                        tokens.push_back({MULTIPLY, "*"});
                        break;
                    case '/':
                        tokens.push_back({DIVIDE, "/"});
                        break;
                    case '%':
                        tokens.push_back({REMINDER, "%"});
                        break;
                    case ';':
                        tokens.push_back({SEMICOLON, ";"});
                        break;
                    case '=':
                        // Check for "==" operator
                        if (pos + 1 < input.length() && input[pos + 1] == '=') {
                            tokens.push_back({ISEQUAL, "=="});
                            pos++;
                        } else {
                            tokens.push_back({EQUAL, "="});
                        }
                        break;
                    case '<':
                        // Check for "<=" operator
                        if (pos + 1 < input.length() && input[pos + 1] == '=') {
                            tokens.push_back({SET, "<="});
                            pos++;
                        } else {
                            tokens.push_back({ST, "<"});
                        }
                        break;
                    case '>':
                        // Check for ">=" operator
                        if (pos + 1 < input.length() && input[pos + 1] == '=') {
                            tokens.push_back({GET, ">="});
                            pos++;
                        } else {
                            tokens.push_back({GT, ">"});
                        }
                        break;
                    case '!':
                        // Check for "!=" operator
                        if (pos + 1 < input.length() && input[pos + 1] == '=') {
                            tokens.push_back({NET, "!="});
                            pos++;
                        } else {
                            tokens.push_back({NOT, "!"});
                        }
                        break;
                    case '&':
                        // Check for "&&" operator
                        if (pos + 1 < input.length() && input[pos + 1] == '&') {
                            tokens.push_back({AND, "&&"});
                            pos++;
                        } else {
                            cerr << "syntaxError: Expected second '&' for '&&' operator at position " << pos << endl;
                        }
                        break;
                    case '|':
                        // Check for "||" operator
                        if (pos + 1 < input.length() && input[pos + 1] == '|') {
                            tokens.push_back({OR, "||"});
                            pos++;
                        } else {
                            cerr << "syntaxError: Expected second '|' for '||' operator at position " << pos << endl;
                        }
                        break;
                    case '(':
                        tokens.push_back({OPENBRACKETS, "("});
                        break;
                    case ')':
                        tokens.push_back({CLOSINGBRACKETS, ")"});
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

    vector<Token> getTokens(){
        return tokens;
    }
};
