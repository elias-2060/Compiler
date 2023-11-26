//
// Created by elias on 26/11/2023.
//
#include "Lexer.cpp"

int main (){
    // Example usage
    string sourceCode = "int x = 42; y = x + 10;";
    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.getTokens();

    // Print tokens
    for (const Token& token : tokens) {
        std::cout << "Type: " << token.type << ", Lexeme: " << token.lexeme << std::endl;
    }

    return 0;
}