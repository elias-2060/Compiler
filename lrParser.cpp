//
// Created by elias on 28/11/2023.
//
#include "Lexer.cpp"

class Node {
private:
    string value;
public:
    explicit Node(string value) {
        this->value = value;
    }

    string getValue() {
        return value;
    }
};

class CST {
private:
    Node* node;
    Node* parent;
    vector<Node*> children;
};

