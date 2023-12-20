//
// Created by elias on 26/11/2023.
//
#include "compiler.cpp"

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

    // Create parser
    Parser parser("CFG.json");
    //parser.printTable();

    // Parse de tokens
    parser.parse(tokens);

    // Get the CST
    Node* cst = parser.getCST();
    // Print the CST in dot formaat
    parser.printTree(cst, "CST.dot");

    // Create the AST
    Node* ast = parser.getAST();

    // Print the AST in dot formaat
    parser.printTree(ast, "AST.dot");

    // Initialize the symbol table and check for errors
    parser.initSymbolTable(ast);
    //parser.printSymbolTable();

    // Do constant folding
    parser.constantFolding(ast);

    // Print the AST after constant folding in dot formaat
    parser.printTree(ast, "AST2.dot");

    // Create the compiler
    Compiler compiler(parser.getSymbolTable(), ast);

    // Convert the AST to mips
    compiler.convertToMips();

    return 0;
}