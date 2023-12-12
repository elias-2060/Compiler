//
// Created by elias on 28/11/2023.
//
#include "Lexer.cpp"

typedef pair<string, vector<string>> Rule;
typedef pair<Rule, int> LRitem;

struct Node {
    string value;
    vector<Node*> children;

    // Function to recursively print the tree in DOT format
    void printDOT(ofstream& out) const {
        // Print the current node
        out << "  " << this << " [label=\"" << value << "\"];" << std::endl;

        // Print edges to children
        for (const auto& child : children) {
            out << "  " << this << " -> " << child << ";" << std::endl;
            child->printDOT(out);  // Recursive call for children
        }
    }
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
    Node* cst;
public:
    explicit Parser(const string& CFG){
        parseStack.push(0);
        initRules(CFG);
        rowSize = variables.size() + terminals.size();
        initFolow();
        initParseTable();
    }
    Node* getCST(){
        return cst;
    }

    void initFolow(){
        Rule follow0 = {"expr", {"$"}};
        Rule follow1 = {"opAddOrSub", {"PLUS", "MINUS", "SEMICOLON", "CLOSINGPARENT"}};
        Rule follow2 = {"opMultOrDiv", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow3 = {"opUnary", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow4 = {"brackets", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow5 = {"dataType", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow6 = {"identifier", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow7 = {"int", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow8 = {"float", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow9 = {"char", {"PLUS", "MINUS", "SEMICOLON", "DIVIDE", "MULTIPLY", "REMINDER", "CLOSINGPARENT"}};
        Rule follow10 = {"assignment", {"SEMICOLON"}};
        Rule follow11 = {"declaration", {"SEMICOLON"}};
        Rule follow12 = {"definition", {"SEMICOLON"}};
        follows = {follow0, follow1, follow2, follow3, follow4, follow5, follow6, follow7, follow8, follow9, follow10,
                   follow11, follow12};
    }

    void doClosure(vector<LRitem>& v, vector<string>& done){
        vector<LRitem> temp;
        for (auto & i : v) {
            bool check = false;
            string currSymbol;
            if (i.second != i.first.second.size()) {
                currSymbol = i.first.second[i.second];
                bool checkDone = false;
                for (const auto & j : done) {
                    if (currSymbol == j) {
                        checkDone = true;
                        break;
                    }
                }
                if (!checkDone) {
                    for (const auto &variable: variables) {
                        if (currSymbol == variable) {
                            check = true;
                            break;
                        }
                    }
                }
            }
            if (check){
                for (auto & rule : rules) {
                    if (rule.first == currSymbol){
                        LRitem item;
                        if (rule.second[0] == "epsilon") {
                            item = {rule, 1};
                        } else
                            item = {rule, 0};
                        temp.push_back(item);
                    }
                }
            }
            done.push_back(currSymbol);
        }
        if (!temp.empty()) {
            doClosure(temp, done);
        }
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
            vector <string> done;
            doClosure(currStateRules, done);
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
                                currStateRule.second++;
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
        for (auto & i : parseTable) {
            for (const auto & j : i) {
                if (!j.empty())
                    cout << j << " ";
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

        string start = j["Start"];
        string augmentedStart = start + "'";
        rules.push_back({augmentedStart, {start}});
        for(auto & production : productions){
            string head = production["head"];
            vector<string> body = production["body"];
            Rule rule = {head, body};
            rules.push_back(rule);
        }
        startRule = rules[0];
        variables.push_back(augmentedStart);
        for(const auto & i : j["Variables"])
            variables.push_back(i);
        for(const auto & i : j["Terminals"])
            terminals.push_back(i);
    }

    int getPopCount(int reducedRule) {
        int count = rules[reducedRule].second.size();

        return count;
    }

    int getReducedSymbol(int reducedRule) {
        string symbol = rules[reducedRule].first;
        for (int i = 0; i < variables.size(); ++i) {
            if (symbol == variables[i])
                return i + terminals.size();
        }
        cerr << "Unknown symbol: " << symbol << endl;
    }

    int getSymbol(const Token& token) {
        string tokenType = token.stringType;
        if (tokenType == "DOLLAR"){
            return terminals.size();
        }
        else{
            for (int i = 0; i < terminals.size(); ++i) {
                if (tokenType == terminals[i])
                    return i;
            }
            cerr << "Unknown token type: " << tokenType << endl;
        }
    }

    bool parse(queue<Token> inputQueue) {
        stack<Node*> cstStack;  // Stack to track CST nodes
        while (!inputQueue.empty()) {
            int currentState = parseStack.top();
            int inputSymbol = getSymbol(inputQueue.front());
            string action = parseTable[currentState][inputSymbol];
            if (action[0] == 'S') {
                // Shift
                int intAction;
                if (action.size() == 2) {
                    intAction = action[1] - '0';
                } else {
                    intAction = stoi(action.substr(1));
                }
                parseStack.push(intAction);
                // Create a CST node for the terminal
                Node* terminalNode = new Node{inputQueue.front().lexeme};
                cstStack.push(terminalNode);

                inputQueue.pop();
            } else if (action[0] == 'R') {
                // Reduce
                int rule;
                if (action.size() == 2){
                    rule = action[1] - '0';
                }
                else{
                    rule = stoi(action.substr(1));
                }
                // Create a CST node for the non-terminal
                Node* nonTerminalNode = new Node{rules[rule].first};

                // Pop the corresponding number of states and nodes from the stacks
                int popCount = getPopCount(rule);
                for (int i = 0; i < popCount; ++i) {
                    parseStack.pop();
                    nonTerminalNode->children.push_back(cstStack.top());
                    cstStack.pop();
                }
                reverse(nonTerminalNode->children.begin(), nonTerminalNode->children.end());

                // Determine the new state after reduction
                int newState = stoi(parseTable[parseStack.top()][getReducedSymbol(rule)]);
                parseStack.push(newState);

                // Push the new non-terminal node onto the CST stack
                cstStack.push(nonTerminalNode);
            } else if (action == "acc") {
                // Retrieve the root of the CST
                cst = cstStack.top();
                // Accept
                return true;
            }else if (action.empty()){
                return false;
            }else {
                // Error
                cout << "Error in state " << currentState << " for symbol " << inputQueue.front().lexeme << endl;
                break;
            }
        }
        return false;
    }
    string addNodes(Node* node){
        string result = to_string(reinterpret_cast<uintptr_t>(node)) + "[label=\"" + node->value + "\"] \n";
        for (auto& child : node->children) {
            result += addNodes(child);
        }
        return result;
    }

    string addConnections(Node* node){
        string connections;
        for (auto & i : node->children) {
            connections += to_string(reinterpret_cast<uintptr_t>(node)) + " -- " + to_string(reinterpret_cast<uintptr_t>(i)) + "\n";
            connections += addConnections(i);
        }
        return connections;
    }
    void printTree(Node* root, const string& filename) {
        string graph = "graph ast { \n" + addNodes(root) + addConnections(root) + "}";
        ofstream file(filename);
        file << graph;
    }
};