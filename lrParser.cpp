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

class Parser {
private:
    stack<int> parseStack;
    vector<vector<string>> parseTable;
    vector<pair<string, vector<string>>> rules;
    CST root;
public:
    Parser(){
        parseStack.push(0);
        initRules();
        initParseTable();
    }

    void initParseTable() {
        parseTable = {};
    }

    void initRules() {
        pair<string, vector<string>> rule1 = {"expr", {"opAnd", "SEMICOLON"}};
        pair<string, vector<string>> rule2 = {"expr", {"opAnd", "SEMICOLON", "expr"}};
        pair<string, vector<string>> rule3 = {"expr", {"variableDefinition", "expr"}};
        pair<string, vector<string>> rule4 = {"expr", {"variableDeclaration", "SEMICOLON", "expr"}};
        pair<string, vector<string>> rule5 = {"expr", {"assignmentStatement", "SEMICOLON", "expr"}};
        pair<string, vector<string>> rule6 = {"expr", {"ε"}}; // epsilon
        pair<string, vector<string>> rule7 = {"opAnd", {"opAnd", "AND", "opOr"}};
        pair<string, vector<string>> rule8 = {"opAnd", {"opOr"}};
        pair<string, vector<string>> rule9 = {"opOr", {"opOr", "OR", "opCompare"}};
        pair<string, vector<string>> rule10 = {"opOr", {"opCompare"}};
        pair<string, vector<string>> rule11 = {"opCompare", {"opCompare", "ISEQUAL", "opAddOrSub"}};
        pair<string, vector<string>> rule12 = {"opCompare", {"opCompare", "SET", "opAddOrSub"}};
        pair<string, vector<string>> rule13 = {"opCompare", {"opCompare", "GET", "opAddOrSub"}};
        pair<string, vector<string>> rule14 = {"opCompare", {"opCompare", "NET", "opAddOrSub"}};
        pair<string, vector<string>> rule15 = {"opCompare", {"opCompare", "ST", "opAddOrSub"}};
        pair<string, vector<string>> rule16 = {"opCompare", {"opCompare", "GT", "opAddOrSub"}};
        pair<string, vector<string>> rule17 = {"opCompare", {"opAddOrSub"}};
        pair<string, vector<string>> rule18 = {"opAddOrSub", {"opAddOrSub", "PLUS", "opMultOrDiv"}};
        pair<string, vector<string>> rule19 = {"opAddOrSub", {"opAddOrSub", "MINUS", "opMultOrDiv"}};
        pair<string, vector<string>> rule20 = {"opAddOrSub", {"opMultOrDiv"}};
        pair<string, vector<string>> rule21 = {"opMultOrDiv", {"opMultOrDiv", "MULTIPLY", "opUnary"}};
        pair<string, vector<string>> rule22 = {"opMultOrDiv", {"opMultOrDiv", "DIVIDE", "opUnary"}};
        pair<string, vector<string>> rule23 = {"opMultOrDiv", {"opMultOrDiv", "REMINDER", "opUnary"}};
        pair<string, vector<string>> rule24 = {"opMultOrDiv", {"opUnary"}};
        pair<string, vector<string>> rule25 = {"opUnary", {"PLUS", "brackets"}};
        pair<string, vector<string>> rule26 = {"opUnary", {"MINUS", "brackets"}};
        pair<string, vector<string>> rule27 = {"opUnary", {"NOT", "brackets"}};
        pair<string, vector<string>> rule28 = {"opUnary", {"brackets"}};
        pair<string, vector<string>> rule29 = {"brackets", {"OPENPARENT", "opAnd", "CLOSINGPARENT"}};
        pair<string, vector<string>> rule30 = {"brackets", {"dataTypes"}};
        pair<string, vector<string>> rule31 = {"variableDefinition", {"variableDeclaration", "EQUAL", "opAnd", "SEMICOLON"}};
        pair<string, vector<string>> rule32 = {"variableDeclaration", {"constWord", "nameIdentifier"}};
        pair<string, vector<string>> rule33 = {"assignmentStatement", {"ID", "EQUAL", "opAddOrSub"}};
        pair<string, vector<string>> rule34 = {"constWord", {"CONST", "reservedWord"}};
        pair<string, vector<string>> rule35 = {"constWord", {"reservedWord"}};
        pair<string, vector<string>> rule36 = {"reservedWord", {"KEYWORD"}};
        pair<string, vector<string>> rule37 = {"dataTypes", {"INT"}};
        pair<string, vector<string>> rule38 = {"dataTypes", {"FLOAT"}};
        pair<string, vector<string>> rule39 = {"dataTypes", {"CHAR"}};
        pair<string, vector<string>> rule40 = {"dataTypes", {"ID"}};
        rules = {rule1, rule2, rule3, rule4, rule5, rule6, rule7, rule8, rule9, rule10,
                 rule11, rule12, rule13, rule14, rule15, rule16, rule17, rule18, rule19, rule20,
                 rule21, rule22, rule23, rule24, rule25, rule26, rule27, rule28, rule29, rule30,
                 rule31, rule32, rule33, rule34, rule35, rule36, rule37, rule38, rule39, rule40};
    }

    int getPopCount(int reducedRule) {
        return rules[reducedRule - 1].second.size();
    }

    int getReducedSymbol(int reducedRule) {
        string symbol = rules[reducedRule - 1].first;

        if (symbol == "expr") {
            return 25;
        } else if (symbol == "opAnd") {
            return 26;
        } else if (symbol == "opOr") {
            return 27;
        } else if (symbol == "opCompare") {
            return 28;
        } else if (symbol == "opAddOrSub") {
            return 29;
        } else if (symbol == "opMultOrDiv") {
            return 30;
        } else if (symbol == "opUnary") {
            return 31;
        } else if (symbol == "brackets") {
            return 32;
        }else if (symbol == "variableDefinition") {
            return 33;
        }else if (symbol == "variableDeclaration") {
            return 34;
        }else if (symbol == "assignmentStatement") {
            return 35;
        }else if (symbol == "constWord") {
            return 36;
        }else if (symbol == "reservedWord") {
            return 37;
        }else if (symbol == "dataTypes") {
            return 38;
        }else {
            cerr << "Unknown symbol: " << symbol << endl;
            return -1;
        }
    }
    static int getSymbol(const Token& token) {
        if (token.type == ID) {
            return 0;
        } else if (token.type == KEYWORD) {
            return 1;
        } else if (token.type == CHAR) {
            return 2;
        } else if (token.type == INT) {
            return 3;
        } else if (token.type == FLOAT) {
            return 4;
        } else if (token.type == PLUS) {
            return 5;
        } else if (token.type == MINUS) {
            return 6;
        } else if (token.type == MULTIPLY) {
            return 7;
        }else if (token.type == DIVIDE) {
            return 8;
        }else if (token.type == EQUAL) {
            return 9;
        }else if (token.type == REMINDER) {
            return 10;
        }else if (token.type == AND) {
            return 11;
        }else if (token.type == OR) {
            return 12;
        }else if (token.type == ISEQUAL) {
            return 13;
        }else if (token.type == ST) {
            return 14;
        }else if (token.type == GT) {
            return 15;
        }else if (token.type == SET) {
            return 16;
        }else if (token.type == GET) {
            return 17;
        }else if (token.type == NOT) {
            return 18;
        }else if (token.type == NET) {
            return 19;
        }else if (token.type == SEMICOLON) {
            return 20;
        }else if (token.type == CONST) {
            return 21;
        }else if (token.type == OPENPARENT) {
            return 22;
        }else if (token.type == CLOSINGPARENT) {
            return 23;
        }else {
            cerr << "Unknown token type: " << token.type << endl;
            return -1;
        }
    }

    bool parse(queue<Token> inputQueue) {
        while (!inputQueue.empty()) {
            int currentState = parseStack.top();
            int inputSymbol = getSymbol(inputQueue.front());
            string action = parseTable[currentState][inputSymbol];

            if (action[0] == 'S') {
                // Shift
                parseStack.push(static_cast<int>(action[1]));
                inputQueue.pop();
            } else if (action[0] == 'R') {
                // Reduce
                int rule = static_cast<int>(action[1]);
                int popCount = getPopCount(rule);
                for (int i = 0; i < 2 * popCount; ++i) {
                    parseStack.pop();
                }
                int newState = stoi(parseTable[parseStack.top()][getReducedSymbol(rule)]);
                parseStack.push(getReducedSymbol(rule));
                parseStack.push(newState);
            } else if (action == "acc") {
                // Accept
                return true;
            } else {
                // Error
                //handleParsingError();
                break;
            }
        }
        return false;
    }
};

