//
// Created by elias on 26/11/2023.
//
#include "Lexer.cpp"
#include <fstream>

string inputFile(const string& fileName){
    while (true)
    {
        ifstream input(fileName.c_str());
        if (input){
            stringstream buffer;
            buffer << input.rdbuf();
            return buffer.str();
        }
        cout << "\nInvalid file name or path";
    }
}

int main (){
    string sourceCode = inputFile("test.txt");
    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.getTokens();

    // Print tokens
    for (const Token& token : tokens) {
        std::cout << "Type: " << token.type << ", Lexeme: " << token.lexeme << std::endl;
    }

    return 0;
}