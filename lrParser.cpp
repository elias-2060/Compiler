//
// Created by elias on 28/11/2023.
//
#include "Lexer.cpp"

typedef pair<string, vector<string>> Rule;
typedef pair<Rule, int> LRitem;

class Node {
private:
    string value;
public:
    explicit Node(string v) {
        this->value = v;
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
    vector<Rule> closures;
    vector<Rule> follows;
    vector<vector<string>> parseTable;
    vector<Rule> rules;
    Rule startRule;
    vector<string> variables;
    vector<string> terminals;
    CST root;
public:
    explicit Parser(const string& CFG){
        parseStack.push(0);
        initRules(CFG);
        initClosure();
        initFolow();
        initParseTable();

    }
    void initClosure(){
        Rule closure0 = {"expr'", {"expr"}};
        Rule closure1 = {"expr", {"opAddOrSub", "variableDefinition", "variableDeclaration", "assignmentStatement"}};
        Rule closure2 = {"opAddOrSub", {"opMultOrDiv"}};
        Rule closure3 = {"opMultOrDiv", {"opUnary"}};
        Rule closure4 = {"opUnary", {"brackets"}};
        Rule closure5 = {"brackets", {"dataTypes"}};
        Rule closure6 = {"variableDefinition", {"variableDeclaration"}};
        Rule closure7 = {"variableDeclaration", {"constWord"}};
        Rule closure8 = {"assignmentStatement", {}};
        Rule closure9 = {"constWord", {"reservedWord"}};
        Rule closure10 = {"reservedWord", {}};
        Rule closure11 = {"dataTypes", {}};
        closures = {closure0, closure1, closure2, closure3, closure4, closure5, closure6, closure7, closure8, closure9, closure10, closure11};
    }

    void initFolow(){
        Rule follow0 = {"expr", {"$"}};
        Rule follow1 = {"opAddOrSub", {"SEMICOLON", "PLUS", "MINUS", "CLOSINGPARENT"}};
        Rule follow2 = {"opMultOrDiv", {"SEMICOLON", "PLUS", "MINUS", "CLOSINGPARENT", "MULTIPLY", "DIVIDE", "REMINDER"}};
        Rule follow3 = {"opUnary", {"SEMICOLON", "PLUS", "MINUS", "CLOSINGPARENT", "MULTIPLY", "DIVIDE", "REMINDER"}};
        Rule follow4 = {"brackets", {"SEMICOLON", "PLUS", "MINUS", "CLOSINGPARENT", "MULTIPLY", "DIVIDE", "REMINDER"}};
        Rule follow5 = {"variableDefinition", {"PLUS", "MINUS", "OPENPARENT", "INT", "FLOAT", "CHAR", "ID", "CONST", "KEYWORD", "$"}};
        Rule follow6 = {"variableDeclaration", {"SEMICOLON", "EQUAL"}};
        Rule follow7 = {"assignmentStatement", {"SEMICOLON"}};
        Rule follow8 = {"constWord", {"ID"}};
        Rule follow9 = {"reservedWord", {"ID"}};
        Rule follow10 = {"dataTypes", {"SEMICOLON", "PLUS", "MINUS", "CLOSINGPARENT", "MULTIPLY", "DIVIDE", "REMINDER"}};
        follows = {follow0, follow1, follow2, follow3, follow4, follow5, follow6, follow7, follow8, follow9, follow10};
    }
    void initParseTable() {
        int currState = 0;
        pair<int ,vector<LRitem>> states;

        vector<string> row = {};
        vector<LRitem> stateRules = {};
        int dotIndex = 0;
        LRitem firstItem = {startRule, dotIndex};

    }

    void initRules(const string& CFG) {
        ifstream input(CFG);
        json j;
        input >> j;
        json& productions = j["Productions"];

        for(auto & production : productions){
            string head = production["head"];
            vector<string> body = production["body"];
            Rule rule = {head, body};
            rules.push_back(rule);
        }
        startRule = rules[0];
        for(const auto & i : j["Variables"])
            variables.push_back(i);
        for(const auto & i : j["Terminals"])
            terminals.push_back(i);
    }

    int getPopCount(int reducedRule) {
        return rules[reducedRule - 1].second.size();
    }

    int getReducedSymbol(int reducedRule) {
        string symbol = rules[reducedRule - 1].first;

        if (symbol == "expr") {
            return 16;
        }else if (symbol == "opAddOrSub") {
            return 17;
        } else if (symbol == "opMultOrDiv") {
            return 18;
        } else if (symbol == "opUnary") {
            return 19;
        } else if (symbol == "brackets") {
            return 20;
        }else if (symbol == "variableDefinition") {
            return 21;
        }else if (symbol == "variableDeclaration") {
            return 22;
        }else if (symbol == "assignmentStatement") {
            return 23;
        }else if (symbol == "constWord") {
            return 24;
        }else if (symbol == "reservedWord") {
            return 25;
        }else if (symbol == "dataTypes") {
            return 26;
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
        }else if (token.type == SEMICOLON) {
            return 11;
        }else if (token.type == CONST) {
            return 12;
        }else if (token.type == OPENPARENT) {
            return 13;
        }else if (token.type == CLOSINGPARENT) {
            return 14;
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