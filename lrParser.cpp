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
    int rowSize;
    vector<Rule> rules;
    Rule startRule;
    vector<string> variables;
    vector<string> terminals;
    CST root;
public:
    explicit Parser(const string& CFG){
        parseStack.push(0);
        initRules(CFG);
        rowSize = variables.size() + terminals.size();
        initFolow();
        initParseTable();
    }
    void initFolow(){
        Rule follow0 = {"A", {"$", "CLOSINGPARENT"}};
        follows = {follow0};
    }

    void doClosure(vector<LRitem> & v){
        vector<LRitem> temp;
        for (int i = 0; i < v.size(); ++i) {
            bool check = false;
            string currSymbol;
            if (v[i].second != v[i].first.second.size()) {
                currSymbol = v[i].first.second[v[i].second];
                for (const auto &variable: variables) {
                    if (currSymbol == variable) {
                        check = true;
                        break;
                    }
                }
            }
            if (check){
                for (auto & rule : rules) {
                    if (rule.first == currSymbol){
                        LRitem item;
                        if (rule.second[0] == "ε") {
                            item = {rule, 1};
                        } else
                            item = {rule, 0};
                        temp.push_back(item);
                    }
                }
            }
        }
        if (!temp.empty())
            doClosure(temp);
        for (auto & i : temp) {
            bool check = false;
            for (auto & j : v) {
                if (i.first == j.first && i.second == j.second){
                    check = true;
                    break;
                }
            }
            if (!check)
                v.push_back(i);
        }
    }
    int getIndex(const string& element, const string& type){
        if (element == "$")
            return terminals.size();
        else {
            if (type == "terminal") {
                for (int i = 0; i < terminals.size(); ++i) {
                    if (element == terminals[i])
                        return i;
                }
            } else if (type == "variable") {
                for (int i = 0; i < variables.size(); ++i) {
                    if (element == variables[i]) {
                        return i + terminals.size();
                    }
                }
            } else
                cerr << "Unknown type: " << type << endl;
        }
    }
    int getRuleIndex(const Rule& rule){
        auto it = find(rules.begin(), rules.end(), rule);

        // If element was found
        if (it != rules.end()){

            // calculating the index
            // of K
            int index = it - rules.begin();
            return index;
        }
        else {
            // If the element is not
            // present in the vector
            return -1;
        }
    }
    void initParseTable() {
        vector<pair<int ,vector<LRitem>>> states;
        vector<pair<int ,vector<LRitem>>> copyStates;
        LRitem firstItem = {startRule, 0};
        pair<int ,vector<LRitem>> state = {0, {firstItem}};
        states.push_back(state);
        copyStates.push_back(state);
        int currStateIndex = 0;

        while (!states.empty()){
            vector<string> row = {};
            for (int i = 0; i < rowSize; ++i) {
                row.emplace_back("");
            }
            int currState = states.front().first;
            vector<LRitem> currStateRules = states.front().second;
            doClosure(currStateRules);
            vector<pair<string, int>> transitions;
            for (auto & currStateRule : currStateRules){
                int dotIndex = currStateRule.second;

                // Reduce
                if (currStateRule.first.second.size() == dotIndex){
                    string symbol = currStateRule.first.first;
                    int ruleIndex = getRuleIndex(currStateRule.first);
                    if (ruleIndex == 0){
                        int testIndex = getIndex("$", "terminal");
                        if (row[testIndex].empty()) {
                            row[testIndex] = "acc";
                        } else {
                            cerr << "Conflict in state " << currState << " for symbol " << "$" << endl;
                        }
                    }
                    else {
                        for (auto &i: follows) {
                            if (i.first == symbol) {
                                for (const auto &follow: i.second) {
                                    int index = getIndex(follow, "terminal");
                                    if (row[index].empty()) {
                                        row[index] = "R" + to_string(ruleIndex);
                                    } else {
                                        cerr << "Conflict in state " << currState << " for symbol " << follow << endl;
                                    }
                                }
                            }
                        }
                    }
                }
                // Shift or Goto
                else{
                    string symbol = currStateRule.first.second[dotIndex];
                    bool isVar = false;
                    bool isTerm = false;
                    for (const auto & variable : variables) {
                        if (symbol == variable){
                            isVar = true;
                            break;
                        }
                    }
                    for (const auto & terminal : terminals) {
                        if (symbol == terminal){
                            isTerm = true;
                            break;
                        }
                    }
                    bool check = false;
                    int stateIndex;
                    for (const auto & transition : transitions) {
                        if (symbol == transition.first){
                            check = true;
                            stateIndex = transition.second;
                            break;
                        }
                    }
                    if (check){
                        for (auto & s : states) {
                            if (s.first == stateIndex){
                                s.second.push_back(currStateRule);
                                break;
                            }
                        }
                    }else{
                        bool found = false;
                        int tempIndex = 0;
                        for (auto & copyState : copyStates) {
                            LRitem item = {currStateRule.first, currStateRule.second + 1};
                            if (copyState.second.front() == item){
                                found = true;
                                tempIndex = copyState.first;
                                break;
                            }
                        }
                        if (!found)
                            currStateIndex++;
                        if (!found)
                            transitions.emplace_back(symbol, currStateIndex);
                        else
                            transitions.emplace_back(symbol, tempIndex);
                        if (!found) {
                            currStateRule.second++;
                            pair<int, vector<LRitem>> newState = {currStateIndex, {currStateRule}};
                            states.push_back(newState);
                            copyStates.push_back(newState);
                        }
                        int index;
                        if (isVar)
                            index = getIndex(symbol, "variable");
                        else if (isTerm)
                            index = getIndex(symbol, "terminal");
                        else
                            cerr << "Unknown symbol: " << symbol << endl;
                        if (row[index].empty()) {
                            if (isTerm) {
                                if (!found)
                                    row[index] = "S" + to_string(currStateIndex);
                                else
                                    row[index] = "S" + to_string(tempIndex);
                            }
                            else if (isVar) {
                                if (!found)
                                    row[index] = to_string(currStateIndex);
                                else
                                    row[index] = to_string(tempIndex);
                            }
                            else
                                cerr << "Unknown symbol: " << symbol << endl;
                        }else{
                            cerr << "Conflict in state " << currState << " for symbol " << symbol << endl;
                        }
                    }
                }
            }
            states.erase(states.begin());
            parseTable.push_back(row);
        }
    }

    void printTable(){
        for (int i = 0; i < parseTable.size(); ++i) {
            for (int j = 0; j < parseTable[i].size(); ++j) {
                if (!parseTable[i][j].empty())
                    cout << parseTable[i][j] << " ";
                else
                    cout << "- ";
            }
            cout << endl;
        }
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

        if (symbol == "A") {
            return 4;
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
        } else if (token.type == OPENPARENT) {
            return 1;
        } else if (token.type == CLOSINGPARENT) {
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
        }else if (token.type == KEYWORD) {
            return 13;
        }else if (token.type == CHAR) {
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