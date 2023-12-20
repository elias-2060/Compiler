//
// Created by elias on 17/12/2023.
//
#include "lrParser.cpp"

class Compiler {
private:
    map<string, pair<bool, string>> symbolTable;
    Node* ast;
    string output= ".text\n";
    string dataOuput = ".data\n";
    map<string, int> stack;
    int stackPointer = 0;
    int dataCounter = 0;
    int registerNumberInt = 0;
    int registerNumberFloat = 0;

public:
    Compiler(const map<string, pair<bool, string>>& s, Node* tree){
        symbolTable = s;
        ast = tree;
    }
    void convertToMips(){
        int size = 8 + symbolTable.size() * 4;
        output += "\tmain:\n";

        // Create stack frame
        output += "\t\t addiu $sp, $sp, -" + to_string(size) + "\n";
        output += "\t\t sw $ra, " + to_string(size - 4) + "($sp)\n";
        output += "\t\t sw $fp, " + to_string(size - 8) + "($sp)\n";
        output += "\t\t move $fp, $sp\n";
        output += "\n";

        stackPointer = size - 12;

        // Generate code for each statement
        generateCode(ast);

        // Deconstruct stack frame
        output += "\n";
        output += "\t\t move $sp, $fp\n";
        output += "\t\t lw $ra, " + to_string(size - 4) + "($sp)\n";
        output += "\t\t lw $fp, " + to_string(size - 8) + "($sp)\n";
        output += "\t\t addiu $sp, $sp, " + to_string(size) + "\n";
        output += "\t\t j exit\n";

        // Exit
        output += "\n";
        output += "\texit:\n";
        output += "\t\t li $v0, 10\n";
        output += "\t\t syscall\n";

        // Print mips code
        printCode();
    }

    void printCode(){
        ofstream file("output.asm");
        file << dataOuput << endl;
        file << output;
    }

    void operation(Node* root, const string& type){
        Node* left = root->children[0];
        Node* right = root->children[2];
        if (left->value == "opAddOrSub" or left->value == "opMultOrDiv"){
            operation(left, type);
        }
        if (right->value == "opAddOrSub" or right->value == "opMultOrDiv"){
            operation(right, type);
        }
        if (left->value == "int" and (right->value == "identifier" or right->value == "opUnary")) {
            bool unary = false;
            if (right->value == "opUnary"){
                if (right->children[0]->value == "-"){
                    unary = true;
                }
            }
            string idName = right->children[0]->value;
            if (right->value == "opUnary")
                idName = right->children[1]->children[0]->value;
            else
                idName = right->children[0]->value;
            string idType = symbolTable[idName].second;
            int offset = stack[idName];
            if (type == "int") {
                int value = stoi(left->children[0]->value);
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt+=1;
                if (idType == "int") {
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);

                } else {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat+=1;
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                }
            } else if (type == "float") {
                float value = stoi(left->children[0]->value);
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat+= 1;
                if (idType == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt+=1;
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);

                } else {
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";;
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                }
            }
        }
        if (right->value == "int" and (left->value == "identifier" or left->value == "opUnary")) {
            bool unary = false;
            if (left->value == "opUnary"){
                if (left->children[0]->value == "-"){
                    unary = true;
                }
            }
            string idName = left->children[0]->value;
            if (left->value == "opUnary")
                idName = left->children[1]->children[0]->value;
            else
                idName = left->children[0]->value;
            string idType = symbolTable[idName].second;
            int offset = stack[idName];
            if (type == "int") {
                int value = stoi(right->children[0]->value);
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt+=1;
                if (idType == "int") {
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);

                } else {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat+=1;
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                }
            } else if (type == "float") {
                float value = stoi(right->children[0]->value);
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat+= 1;
                if (idType == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt+=1;
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);

                } else {
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                }
            }
        }
        if (left->value == "float" and (right->value == "identifier" or right->value == "opUnary")) {
            bool unary = false;
            if (right->value == "opUnary"){
                if (right->children[0]->value == "-"){
                    unary = true;
                }
            }
            string idName = right->children[0]->value;
            if (right->value == "opUnary")
                idName = right->children[1]->children[0]->value;
            else
                idName = right->children[0]->value;
            string idType = symbolTable[idName].second;
            int offset = stack[idName];
            if (type == "int") {
                int value = stof(left->children[0]->value);
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt+=1;
                if (idType == "int") {
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);

                } else {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat+=1;
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                }
            } else if (type == "float") {
                float value = stof(left->children[0]->value);
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat+= 1;
                if (idType == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt+=1;
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);

                } else {
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                }
            }
        }
        if (right->value == "float" and (left->value == "identifier" or left->value == "opUnary")) {
            bool unary = false;
            if (left->value == "opUnary"){
                if (left->children[0]->value == "-"){
                    unary = true;
                }
            }
            string idName = left->children[0]->value;
            if (left->value == "opUnary")
                idName = left->children[1]->children[0]->value;
            else
                idName = left->children[0]->value;
            string idType = symbolTable[idName].second;
            int offset = stack[idName];
            if (type == "int") {
                int value = stof(right->children[0]->value);
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt+=1;
                if (idType == "int") {
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + to_string(value) + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);

                } else {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat+=1;
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t addiu " + reg + ", " + reg + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                }
            } else if (type == "float") {
                float value = stof(right->children[0]->value);
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat+= 1;
                if (idType == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt+=1;
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);

                } else {
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    string extraReg = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + extraReg + ", data" + to_string(dataCounter) + "\n";
                    dataCounter += 1;
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + extraReg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node* node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                }
            }
        }
        else if ((left->value == "identifier" or left->value == "opUnary") and (right->value == "identifier" or right->value == "opUnary")){
            bool unaryLeft = false;
            bool unaryRight = false;
            if (left->value == "opUnary"){
                if (left->children[0]->value == "-"){
                    unaryLeft = true;
                }
            }
            if (right->value == "opUnary"){
                if (right->children[0]->value == "-"){
                    unaryRight = true;
                }
            }
            string idLeft;
            if (left->value == "opUnary")
                idLeft = left->children[1]->children[0]->value;
            else
                idLeft = left->children[0]->value;
            string idRight;
            if (right->value == "opUnary")
                idRight = right->children[1]->children[0]->value;
            else
                idRight = right->children[0]->value;
            string idTypeLeft = symbolTable[idLeft].second;
            string idTypeRight = symbolTable[idRight].second;
            int offsetLeft = stack[idLeft];
            int offsetRight = stack[idRight];
            if (type == "int") {
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt += 1;
                string reg2 = "$t" + to_string(registerNumberInt);
                registerNumberInt += 1;
                if (idTypeLeft == "int" and idTypeRight == "int") {
                    output += "\t\t lw " + reg + ", " + to_string(offsetLeft) + "($fp)\n";
                    if (unaryLeft)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t lw " + reg2 + ", " + to_string(offsetRight) + "($fp)\n";
                    if (unaryRight)
                        output += "\t\t negu " + reg2 + ", " + reg2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + reg2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = reg;
                    root->children.push_back(node);

                } else if (idTypeLeft == "float" and idTypeRight == "float") {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    string regFloat2 = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t l.s " + regFloat + ", " + to_string(offsetLeft) + "($fp)\n";
                    output += "\t\t l.s " + regFloat2 + ", " + to_string(offsetRight) + "($fp)\n";
                    if (unaryLeft)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    if (unaryRight)
                        output += "\t\t neg.s " + regFloat2 + ", " + regFloat2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                } else if (idTypeLeft == "int" and idTypeRight == "float") {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t lw " + reg + ", " + to_string(offsetLeft) + "($fp)\n";
                    if (unaryLeft)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t l.s " + regFloat + ", " + to_string(offsetRight) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg2 + ", " + regFloat + "\n";
                    if (unaryRight)
                        output += "\t\t negu " + reg2 + ", " + reg2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + reg2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                }
                else if (idTypeLeft == "float" and idTypeRight == "int") {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat += 1;
                    output += "\t\t lw " + reg + ", " + to_string(offsetRight) + "($fp)\n";
                    if (unaryRight)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t l.s " + regFloat + ", " + to_string(offsetLeft) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg2 + ", " + regFloat + "\n";
                    if (unaryLeft)
                        output += "\t\t negu " + reg2 + ", " + reg2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + reg + ", " + reg + ", " + reg2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + reg + ", " + reg + ", " + reg2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = reg;
                    root->children.push_back(node);
                }
            }
            else{
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat += 1;
                string regFloat2 = "$f" + to_string(registerNumberFloat);
                registerNumberFloat += 1;
                if (idTypeLeft == "int" and idTypeRight == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt += 1;
                    string reg2 = "$t" + to_string(registerNumberInt);
                    registerNumberInt += 1;
                    output += "\t\t lw " + reg + ", " + to_string(offsetLeft) + "($fp)\n";
                    if (unaryLeft)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t lw " + reg2 + ", " + to_string(offsetRight) + "($fp)\n";
                    if (unaryRight)
                        output += "\t\t negu " + reg2 + ", " + reg2 + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t mtc1 " + reg2 + ", " + regFloat2 + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat2 + ", " + regFloat2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);

                } else if (idTypeLeft == "float" and idTypeRight == "float") {
                    output += "\t\t l.s " + regFloat + ", " + to_string(offsetLeft) + "($fp)\n";
                    output += "\t\t l.s " + regFloat2 + ", " + to_string(offsetRight) + "($fp)\n";;
                    if (unaryLeft)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    if (unaryRight)
                        output += "\t\t neg.s " + regFloat2 + ", " + regFloat2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                } else if (idTypeLeft == "int" and idTypeRight == "float") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt += 1;
                    output += "\t\t lw " + reg + ", " + to_string(offsetLeft) + "($fp)\n";
                    if (unaryLeft)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t l.s " + regFloat + ", " + to_string(offsetRight) + "($fp)\n";
                    if (unaryRight)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat2 + "\n";
                    output += "\t\t cvt.s.w " + regFloat2 + ", " + regFloat2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                }
                else if (idTypeLeft == "float" and idTypeRight == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt += 1;
                    output += "\t\t lw " + reg + ", " + to_string(offsetRight) + "($fp)\n";
                    if (unaryRight)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t l.s " + regFloat + ", " + to_string(offsetLeft) + "($fp)\n";
                    if (unaryLeft)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat2 + "\n";
                    output += "\t\t cvt.s.w " + regFloat2 + ", " + regFloat2 + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + regFloat + ", " + regFloat + ", " + regFloat2 + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = regFloat;
                    root->children.push_back(node);
                }
            }
        }
        else if (left->value == "int" and right->value == "register") {
            if (type == "int") {
                int value = stoi(left->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t addiu " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = right->children[0]->value;
                root->children.push_back(node);
            } else {
                float value = stoi(left->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t add.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = right->children[0]->value;
                root->children.push_back(node);
            }
        }
        else if (left->value == "register" and right->value == "int") {
            if (type == "int") {
                int value = stoi(right->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t addiu " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = left->children[0]->value;
                root->children.push_back(node);
            } else {
                float value = stoi(right->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t add.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = left->children[0]->value;
                root->children.push_back(node);
            }
        }
        else if (left->value == "float" and right->value == "register") {
            if (type == "int") {
                int value = stof(left->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t addiu " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = right->children[0]->value;
                root->children.push_back(node);
            } else {
                float value = stof(left->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t add.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = right->children[0]->value;
                root->children.push_back(node);
            }
        }
        else if (left->value == "register" and right->value == "float") {
            if (type == "int") {
                int value = stof(right->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t addiu " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = left->children[0]->value;
                root->children.push_back(node);
            } else {
                float value = stof(right->children[0]->value);
                if (root->children[1]->value == "+")
                    output += "\t\t add.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                            to_string(value) + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              to_string(value) + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = left->children[0]->value;
                root->children.push_back(node);
            }
        }
        if (right->value == "register" and (left->value == "identifier" or left->value == "opUnary")) {
            bool unary = false;
            if (left->value == "opUnary"){
                if (left->children[0]->value == "-"){
                    unary = true;
                }
            }
            string idName = left->children[0]->value;
            if (left->value == "opUnary")
                idName = left->children[1]->children[0]->value;
            else
                idName = left->children[0]->value;
            string idType = symbolTable[idName].second;
            int offset = stack[idName];
            if (type == "int") {
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt+=1;
                if (idType == "int") {
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = right->children[0]->value;
                    root->children.push_back(node);

                } else {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat+=1;
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  reg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = right->children[0]->value;
                    root->children.push_back(node);
                }
            } else if (type == "float") {
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat+=1;
                if (idType == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt+=1;
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";

                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                regFloat + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                regFloat + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                regFloat + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                regFloat + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                regFloat + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = right->children[0]->value;
                    root->children.push_back(node);

                } else {
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + right->children[0]->value + ", " + right->children[0]->value + ", " +
                                  regFloat + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = left->children[0]->value;
                    root->children.push_back(node);
                }
            }
        }
        if (left->value == "register" and (right->value == "identifier" or right->value == "opUnary")) {
            bool unary = false;
            if (right->value == "opUnary"){
                if (right->children[0]->value == "-"){
                    unary = true;
                }
            }
            string idName = right->children[0]->value;
            if (right->value == "opUnary")
                idName = right->children[1]->children[0]->value;
            else
                idName = right->children[0]->value;
            string idType = symbolTable[idName].second;
            int offset = stack[idName];
            if (type == "int") {
                string reg = "$t" + to_string(registerNumberInt);
                registerNumberInt+=1;
                if (idType == "int") {
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = left->children[0]->value;
                    root->children.push_back(node);

                } else {
                    string regFloat = "$f" + to_string(registerNumberFloat);
                    registerNumberFloat+=1;
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    output += "\t\t cvt.w.s " + regFloat + ", " + regFloat + "\n";
                    output += "\t\t mfc1 " + reg + ", " + regFloat + "\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  reg + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = left->children[0]->value;
                    root->children.push_back(node);
                }
            } else if (type == "float") {
                string regFloat = "$f" + to_string(registerNumberFloat);
                registerNumberFloat+=1;
                if (idType == "int") {
                    string reg = "$t" + to_string(registerNumberInt);
                    registerNumberInt+=1;
                    output += "\t\t lw "  + reg + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t negu " + reg + ", " + reg + "\n";
                    output += "\t\t mtc1 " + reg + ", " + regFloat + "\n";
                    output += "\t\t cvt.s.w " + regFloat + ", " + regFloat + "\n";

                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = left->children[0]->value;
                    root->children.push_back(node);

                } else {
                    output += "\t\t l.s "  + regFloat + ", " + to_string(offset) + "($fp)\n";
                    if (unary)
                        output += "\t\t neg.s " + regFloat + ", " + regFloat + "\n";
                    if (root->children[1]->value == "+")
                        output += "\t\t add.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "-")
                        output += "\t\t sub.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "*")
                        output += "\t\t mul.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "/")
                        output += "\t\t div.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    else if (root->children[1]->value == "%")
                        output += "\t\t rem.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                                  regFloat + "\n";
                    root->value = "register";
                    root->children.clear();
                    Node *node = new Node();
                    node->value = left->children[0]->value;
                    root->children.push_back(node);
                }
            }
        }
        else if (left->value == "register" and right->value == "register"){
            if (type == "int") {
                if (root->children[1]->value == "+")
                    output += "\t\t add " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                            right->children[0]->value + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = left->children[0]->value;
                root->children.push_back(node);
            } else {
                if (root->children[1]->value == "+")
                    output += "\t\t add.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                else if (root->children[1]->value == "-")
                    output += "\t\t sub.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                            right->children[0]->value + "\n";
                else if (root->children[1]->value == "*")
                    output += "\t\t mul.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                else if (root->children[1]->value == "/")
                    output += "\t\t div.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                else if (root->children[1]->value == "%")
                    output += "\t\t rem.s " + left->children[0]->value + ", " + left->children[0]->value + ", " +
                              right->children[0]->value + "\n";
                root->value = "register";
                root->children.clear();
                Node *node = new Node();
                node->value = left->children[0]->value;
                root->children.push_back(node);
            }
        }
    }

    void generateCode(Node* node){
        if (node->value == "definition"){
            string varName;
            Node* initValue;
            if (node->children.size() == 4){
                varName = node->children[1]->value;
                initValue = node->children[3];
            }
            else{
                varName = node->children[2]->value;
                initValue = node->children[4];
            }
            string type = symbolTable[varName].second;
            if (initValue->value == "opAddOrSub" or initValue->value == "opMultOrDiv"){
                operation(initValue, type);
                if (type == "int")
                    output += "\t\t sw " + initValue->children[0]->value + ", " + to_string(stackPointer) + "($fp)\n";
                else
                    output += "\t\t swc1 " + initValue->children[0]->value + ", " + to_string(stackPointer) + "($fp)\n";
                stack[varName] = stackPointer;
                stackPointer -= 4;
            }
            else if (initValue->value == "opUnary"){
                string idName = initValue->children[1]->children[0]->value;
                string idType = symbolTable[idName].second;
                int offset = stack[idName];
                if (type == "int") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset) + "($fp)\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t negu $t0, $t0\n";
                        output += "\t\t sw $t0, " + to_string(stackPointer) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset) + "($fp)\n";
                        output += "\t\t cvt.w.s $f0, $f0\n";
                        output += "\t\t mfc1 $t0, $f0\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t negu $t0, $t0\n";
                        output += "\t\t sw $t0, " + to_string(stackPointer) + "($fp)\n";
                    }
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                } else if (type == "float") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset) + "($fp)\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t negu $t0, $t0\n";
                        output += "\t\t mtc1 $t0, $f0\n";
                        output += "\t\t cvt.s.w $f0, $f0\n";
                        output += "\t\t swc1 $f0, " + to_string(stackPointer) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset) + "($fp)\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t neg.s $f0, $f0\n";
                        output += "\t\t swc1 $f0, " + to_string(stackPointer) + "($fp)\n";
                    }
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                }
            }
            else if (initValue->value == "identifier"){
                string idName = initValue->children[0]->value;
                string idType = symbolTable[idName].second;
                int offset = stack[idName];
                if (type == "int") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset) + "($fp)\n";
                        output += "\t\t sw $t0, " + to_string(stackPointer) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset) + "($fp)\n";
                        output += "\t\t cvt.w.s $f0, $f0\n";
                        output += "\t\t mfc1 $t0, $f0\n";
                        output += "\t\t sw $t0, " + to_string(stackPointer) + "($fp)\n";
                    }
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                } else if (type == "float") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset) + "($fp)\n";
                        output += "\t\t mtc1 $t0, $f0\n";
                        output += "\t\t cvt.s.w $f0, $f0\n";
                        output += "\t\t swc1 $f0, " + to_string(stackPointer) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset) + "($fp)\n";
                        output += "\t\t swc1 $f0, " + to_string(stackPointer) + "($fp)\n";
                    }
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                }
            }
            else{
                if (type == "int"){
                    int value;
                    if (initValue->value == "int") {
                        value = stoi(initValue->children[0]->value);
                    }
                    else{
                        value = stof(initValue->children[0]->value);
                    }
                    output += "\t\t li $t0, " + to_string(value) + "\n";
                    output += "\t\t sw $t0, " + to_string(stackPointer) + "($fp)\n";
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                }
                else if (type == "float"){
                    float value;
                    if (initValue->value == "float")
                        value = stof(initValue->children[0]->value);
                    else
                        value = stoi(initValue->children[0]->value);
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    output += "\t\t l.s $f0, data" + to_string(dataCounter) + "\n";
                    output += "\t\t swc1 $f0, " + to_string(stackPointer) + "($fp)\n";
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                    dataCounter++;
                }
                else if (type == "char"){
                    char value = initValue->children[0]->value[1];
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .byte '" + value + "'\n";
                    output += "\t\t lb $t0, data"  + to_string(dataCounter) + "\n";
                    output += "\t\t sb $t0, " + to_string(stackPointer) + "($fp)\n";
                    stack[varName] = stackPointer;
                    stackPointer -= 4;
                    dataCounter++;
                }
            }
        }
        else if (node->value == "assignment"){
            string varName = node->children[0]->value;
            int offset = stack[varName];
            string type = symbolTable[varName].second;
            Node* initValue = node->children[2];
            if (initValue->value == "opAddOrSub" or initValue->value == "opMultOrDiv"){
                operation(initValue, type);
                if (type == "int")
                    output += "\t\t sw " + initValue->children[0]->value + ", " + to_string(offset) + "($fp)\n";
                else
                    output += "\t\t swc1 " + initValue->children[0]->value + ", " + to_string(offset) + "($fp)\n";
            }
            else if (initValue->value == "opUnary"){
                string idName = initValue->children[1]->children[0]->value;
                string idType = symbolTable[idName].second;
                int offset2 = stack[idName];
                if (type == "int") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset2) + "($fp)\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t negu $t0, $t0\n";
                        output += "\t\t sw $t0, " + to_string(offset) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset2) + "($fp)\n";
                        output += "\t\t cvt.w.s $f0, $f0\n";
                        output += "\t\t mfc1 $t0, $f0\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t negu $t0, $t0\n";
                        output += "\t\t sw $t0, " + to_string(offset) + "($fp)\n";
                    }
                } else if (type == "float") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset2) + "($fp)\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t negu $t0, $t0\n";
                        output += "\t\t mtc1 $t0, $f0\n";
                        output += "\t\t cvt.s.w $f0, $f0\n";
                        output += "\t\t swc1 $f0, " + to_string(offset) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset2) + "($fp)\n";
                        if (initValue->children[0]->value == "-")
                            output += "\t\t neg.s $f0, $f0\n";
                        output += "\t\t swc1 $f0, " + to_string(offset) + "($fp)\n";
                    }
                }
            }
            else if (initValue->value == "identifier"){
                string idName = initValue->children[0]->value;
                string idType = symbolTable[idName].second;
                int offset2 = stack[idName];
                if (type == "int") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset2) + "($fp)\n";
                        output += "\t\t sw $t0, " + to_string(offset) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset2) + "($fp)\n";
                        output += "\t\t cvt.w.s $f0, $f0\n";
                        output += "\t\t mfc1 $t0, $f0\n";
                        output += "\t\t sw $t0, " + to_string(offset) + "($fp)\n";
                    }
                } else if (type == "float") {
                    if (idType == "int") {
                        output += "\t\t lw $t0, " + to_string(offset2) + "($fp)\n";
                        output += "\t\t mtc1 $t0, $f0\n";
                        output += "\t\t cvt.s.w $f0, $f0\n";
                        output += "\t\t swc1 $f0, " + to_string(offset) + "($fp)\n";
                    } else {
                        output += "\t\t l.s $f0, " + to_string(offset2) + "($fp)\n";
                        output += "\t\t swc1 $f0, " + to_string(offset) + "($fp)\n";
                    }
                }
            }
            else {
                if (type == "int") {
                    int value;
                    if (initValue->value == "int") {
                        value = stoi(initValue->children[0]->value);
                    } else {
                        value = stof(initValue->children[0]->value);
                    }
                    output += "\t\t li $t0, " + to_string(value) + "\n";
                    output += "\t\t sw $t0, " + to_string(offset) + "($fp)\n";
                } else if (type == "float") {
                    float value;
                    if (initValue->value == "float")
                        value = stof(initValue->children[0]->value);
                    else
                        value = stoi(initValue->children[0]->value);
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .float " + to_string(value) + "\n";
                    output += "\t\t l.s $f0, data" + to_string(dataCounter) + "\n";
                    output += "\t\t swc1 $f0, " + to_string(offset) + "($fp)\n";
                    dataCounter++;
                } else if (type == "char") {
                    char value = initValue->children[0]->value[1];
                    dataOuput += "\t\t data" + to_string(dataCounter) + ": .byte '" + value + "'\n";
                    output += "\t\t lb $t0, data" + to_string(dataCounter) + "\n";
                    output += "\t\t sb $t0, " + to_string(offset) + "($fp)\n";
                    dataCounter++;
                }
            }

        }
        else if (node->value == "declaration"){
            string varname = node->children[1]->value;
            stack[varname] = stackPointer;
            stackPointer -= 4;
        }

        else{
            for (auto child : node->children){
                generateCode(child);
            }
        }
    }

};
