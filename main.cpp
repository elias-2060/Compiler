//
// Created by elias on 26/11/2023.
//
#include "Lexer.cpp"
#include <fstream>
#include "lrParser.cpp"

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
    // Create lexer and convert the input into tokens
    string sourceCode = inputFile("test.txt");
    Lexer lexer(sourceCode);

    // Get the tokens
    queue<Token> tokens = lexer.getTokens();

    // Get the tokens to print
    queue<Token> printTokens = lexer.getTokens();

    // Create parser
    Parser parser;

    // Parse tokens
    parser.parse(tokens);

    // Print tokens
    for (int i = 0; i < printTokens.size(); i++) {
        Token token = printTokens.front();
        cout << "Type: " << token.type << ", Lexeme: " << token.lexeme << endl;
        printTokens.pop();
    }

    return 0;
}