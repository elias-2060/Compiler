//
// Created by elias on 28/11/2023.
//
#include "Lexer.cpp"
#include <map>

typedef pair<string, vector<string>> Rule;
typedef pair<Rule, int> LRitem;

struct Node {
    string value;
    vector<Node*> children;
    Node* parent;
};

class Parser {
private:
    stack<int> parseStack;
    vector<Rule> closures;
    vector<Rule> follows;
    vector<Rule> followSets;
    vector<vector<string>> parseTable;
    int rowSize;
    vector<Rule> rules;
    Rule startRule;
    vector<string> variables;
    vector<string> terminals;
    Node* cst;
    map<string, pair<bool, string>> symbolTable;
    string errors;
public:
    explicit Parser(const string& CFG){
        parseStack.push(0);
        initRules(CFG);
        rowSize = variables.size() + terminals.size();
        initFolow();
        initParseTable();
    }
    map<string, pair<bool, string>> getSymbolTable(){
        return symbolTable;
    }

    string getErrors(){
        return errors;
    }

    Node* getCST(){
        return cst;
    }

    Node* getAST(){
        createAST(cst);
        removeUselessNodes(cst);
        return cst;
    }

    void constantFolding(Node* root){
        convertChar(root);
        optimizeUnaryOp(root);
        optimizeBinaryOp(root);
    }

    void convertChar(Node* root){
        if (root->value == "char" and root->children.size() == 1){
            Node* parent = root->parent;
            if (parent->value == "definition"){
                string type;
                if (parent->children.size() == 4){
                    type = parent->children[0]->value;
                }
                else{
                    type = parent->children[1]->value;
                }
                if (type != "char"){
                    root->value = type;
                    string value = root->children[0]->value;
                    int ascii = value[1];
                    root->children[0]->value = to_string(ascii);
                }
            }
            else if (parent->value == "assignment"){
                string var = parent->children[0]->value;
                string type = symbolTable[var].second;
                if (type != "char"){
                    root->value = type;
                    string value = root->children[0]->value;
                    int ascii = value[1];
                    root->children[0]->value = to_string(ascii);
                }
            }
            else if (parent->value == "opAddOrSub" or parent->value == "opMultOrDiv" or parent->value == "opUnary"){
                root->value = "int";
                string value = root->children[0]->value;
                int ascii = value[1];
                root->children[0]->value = to_string(ascii);
            }
        }
        else {
            for (auto &i: root->children) {
                convertChar(i);
            }
        }
    }

    void optimizeUnaryOp(Node* root){
        if (root->value == "opUnary"){
            Node* child = root->children[1];
            string type = child->value;
            string operation = root->children[0]->value;
            if (type == "int"){
                int value = stoi(child->children[0]->value);
                if (operation == "-"){
                    value = -value;
                }
                root->value = "int";
                root->children.clear();
                Node* newNode = new Node{to_string(value)};
                newNode->parent = root;
                root->children.push_back(newNode);
            }
            else if (type == "float"){
                float value = stof(child->children[0]->value);
                if (operation == "-"){
                    value = -value;
                }
                root->value = "float";
                root->children.clear();
                Node* newNode = new Node{to_string(value)};
                newNode->parent = root;
                root->children.push_back(newNode);
            }
        }
        for (auto & i : root->children) {
            optimizeUnaryOp(i);
        }
    }
    void optimizeBinaryOp(Node* root){
        if (root->value == "opAddOrSub" or root->value == "opMultOrDiv"){
            Node* left = root->children[0];
            Node* right = root->children[2];
            Node* curr = root->parent;
            string idType;
            bool found = false;
            while(!found){
                if (curr->value == "definition"){
                    found = true;
                    if (curr->children.size() == 4)
                        idType = curr->children[0]->value;
                    else
                        idType = curr->children[1]->value;
                }
                else if (curr->value == "assignment"){
                    found = true;
                    string var = curr->children[0]->value;
                    idType = symbolTable[var].second;
                }
                else if (curr->value == "expr"){
                    found = true;
                    idType = "float";
                }
                else{
                    curr = curr->parent;
                }
            }
            bool possible = true;
            if (left->value == "opAddOrSub" or left->value == "opMultOrDiv"){
                optimizeBinaryOp(left);
                if (left->value != "int" and left->value != "float"){
                    possible = false;
                }
            }
            else if (left->value != "int" and left->value != "float"){
                possible = false;
            }
            if (right->value == "opAddOrSub" or right->value == "opMultOrDiv"){
                optimizeBinaryOp(right);
                if (right->value != "int" and right->value != "float"){
                    possible = false;
                }
            }
            else if (right->value != "int" and right->value != "float"){
                possible = false;
            }
            string operation = root->children[1]->value;
            if (possible) {
                if (left->value == "int" and right->value == "int" and idType == "int") {
                    int leftValue = stoi(left->children[0]->value);
                    int rightValue = stoi(right->children[0]->value);
                    int result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = leftValue % rightValue;
                    }
                    root->value = "int";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }
                if (left->value == "int" and right->value == "int" and idType == "float") {
                    int leftValue = stoi(left->children[0]->value);
                    int rightValue = stoi(right->children[0]->value);
                    float result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = leftValue % rightValue;
                    }
                    root->value = "float";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }else if (left->value == "float" and right->value == "float" and idType == "float") {
                    float leftValue = stof(left->children[0]->value);
                    float rightValue = stof(right->children[0]->value);
                    float result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = fmod(leftValue, rightValue);
                    }
                    root->value = "float";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }else if (left->value == "float" and right->value == "float" and idType == "int") {
                    float leftValue = stof(left->children[0]->value);
                    float rightValue = stof(right->children[0]->value);
                    int result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = fmod(leftValue, rightValue);
                    }
                    root->value = "int";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }else if (left->value == "int" and right->value == "float" and idType == "int"){
                    int leftValue = stoi(left->children[0]->value);
                    float rightValue = stof(right->children[0]->value);
                    int result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = fmod(leftValue, rightValue);
                    }
                    root->value = "int";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }
                else if (left->value == "int" and right->value == "float" and idType == "float"){
                    int leftValue = stoi(left->children[0]->value);
                    float rightValue = stof(right->children[0]->value);
                    float result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = fmod(leftValue, rightValue);
                    }
                    root->value = "float";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }
                else if (left->value == "float" and right->value == "int" and idType == "int"){
                    float leftValue = stof(left->children[0]->value);
                    int rightValue = stoi(right->children[0]->value);
                    int result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = fmod(leftValue, rightValue);
                    }
                    root->value = "int";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }
                else if (left->value == "float" and right->value == "int" and idType == "float"){
                    float leftValue = stof(left->children[0]->value);
                    int rightValue = stoi(right->children[0]->value);
                    float result;
                    if (operation == "+") {
                        result = leftValue + rightValue;
                    } else if (operation == "-") {
                        result = leftValue - rightValue;
                    } else if (operation == "*") {
                        result = leftValue * rightValue;
                    } else if (operation == "/") {
                        result = leftValue / rightValue;
                    } else if (operation == "%") {
                        result = fmod(leftValue, rightValue);
                    }
                    root->value = "float";
                    root->children.clear();
                    Node *newNode = new Node{to_string(result)};
                    newNode->parent = root;
                    root->children.push_back(newNode);
                }
            }
        }
        for (auto & i : root->children) {
            optimizeBinaryOp(i);
        }
    }

    void removeUselessNodes(Node* root){
        if (root != nullptr){
            if (root->value == "opAddOrSub" or root->value == "opMultOrDiv" or root->value == "opUnary"){
                if (root->children.size() == 1){
                    Node* tempNode = root->children[0];
                    Node* parent = root->parent;
                    bool found = false;
                    while(!found){
                        if (tempNode->children.size() != 1)
                            found = true;
                        else if (tempNode->value == "identifier" or tempNode->value == "int" or tempNode->value == "float" or tempNode->value == "char")
                            found = true;
                        else
                            tempNode = tempNode->children[0];
                    }
                    tempNode->parent = parent;
                    int index;
                    for (int i = 0; i < parent->children.size(); ++i) {
                        if (root == parent->children[i]){
                            index = i;
                            break;
                        }
                    }
                    parent->children[index] = tempNode;

                    for (auto & i : tempNode->children) {
                        removeUselessNodes(i);
                    }

                }
                else{
                    for (auto & i : root->children) {
                        removeUselessNodes(i);
                    }
                }
            }
            else{
                for (auto & i : root->children) {
                    removeUselessNodes(i);
                }
            }
        }
    }

    void createAST(Node* root){
        if (root != nullptr){
            if (root->value == ";"){
                for (int i = 0; i < root->parent->children.size(); ++i) {
                    if (root == root->parent->children[i]){
                        root->parent->children.erase(root->parent->children.begin() + i);
                        break;
                    }
                }
            }

            else if (root->value == "brackets") {
                if (root->children.size() == 3) {
                    Node *child = root->children[1];
                    Node *parent = root->parent;
                    int index;
                    for (int i = 0; i < parent->children.size(); ++i) {
                        if (root == parent->children[i]) {
                            index = i;
                            break;
                        }
                    }
                    child->parent = parent;
                    parent->children[index] = child;
                }
            }

            for (auto & i : root->children) {
                createAST(i);
            }
        }
    }

    void initSymbolTable(Node* root) {
        if (root != nullptr) {
            if (root->value == "declaration" or root->value == "definition") {
                if (root->children[0]->value != "const") {
                    string type = root->children[0]->value;
                    string identifier = root->children[1]->value;
                    if (symbolTable.find(identifier) == symbolTable.end()) {
                        symbolTable[identifier] = {false, type};
                    } else {
                        errors += "Variable " + identifier + " is already declared or defined\n";
                    }
                }
                else{
                    string type = root->children[1]->value;
                    string identifier = root->children[2]->value;
                    if (symbolTable.find(identifier) == symbolTable.end()) {
                        symbolTable[identifier] = {true, type};
                    } else {
                        errors += "Variable " + identifier + " is already declared or defined\n";
                    }
                }
            } else if (root->value == "assignment") {
                string identifier = root->children[0]->value;
                if (symbolTable.find(identifier) != symbolTable.end()) {
                    if (symbolTable[identifier].first) {
                        errors += "Cannot assign variable: " + identifier + " which is of type const\n";
                    }
                } else {
                    errors += "Variable " + identifier + " is not declared or defined\n";
                }
            } else if (root->value == "identifier") {
                string identifier = root->children[0]->value;
                if (symbolTable.find(identifier) == symbolTable.end()) {
                    errors += "Variable " + identifier + " is not declared or defined\n";
                }
            }

            // Traverse children
            for (Node *child: root->children) {
                initSymbolTable(child);
            }
        }
    }

    void printSymbolTable() {
        cout << "Symbol Table" << endl;
        cout << "Identifier\t\tType\t\t\tConst" << endl;
        cout << "--------------------------------------------------------" << endl;
        for (auto & i : symbolTable) {
            cout << boolalpha << i.first << "\t\t\t" << i.second.second << "\t\t\t" << i.second.first << endl;
        }
    }

    void initFolow(){
        map<string, int> mp;



        for (int i = 1; i < variables.size(); ++i) {
            mp[variables[i]] = i-1;
        }

        json cfg;
        ifstream input("CFG.json");
        input >> cfg;
        int index = -1;
        for (int i = 0; i < 13; ++i) {
            follows.emplace_back("", std::vector<std::string>());
        }

        for(auto i : cfg["Variables"]){
            pair<string, vector<string>> h;
            vector<string> v;

            h.first = i;
            for(auto j : cfg["Productions"]){
                int counter = 0;
                for(auto k : j["body"]){
                    if(k == i){
                        index = counter + 1;
                    }
                    if(index == j["body"].size() and k != "expr") {
                        for (auto n: follows[mp[j["head"]]].second) {
                            v.push_back(n);
                        }
                    }
                    if(index == counter){
                        v.push_back(k);
                        break;
                    }
                    counter += 1;
                }
                index = -1;
            }
            sort(v.begin(), v.end());
            v.erase( unique(v.begin(), v.end() ), v.end());

            h.second = v;
            follows[mp[i]] = h;
        }
        follows[0].second.push_back("$");
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
                errors += "Unknown type: " + type + "\n";
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
                            errors += "Conflict in state " + to_string(currState) + " for symbol " + "$" + "\n";
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
                                        errors += "Conflict in state " + to_string(currState) + " for symbol " + follow + "\n";
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
                            errors += "Unknown symbol: " + symbol + "\n";
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
                                errors += "Unknown symbol: " + symbol + "\n";
                        }else{
                            errors += "Conflict in state " + to_string(currState) + " for symbol " + symbol + "\n";
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
        errors += "Unknown symbol: " + symbol + "\n";
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
            errors += "Unknown token type: " + tokenType + "\n";
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
                    cstStack.top()->parent = nonTerminalNode;
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
                cstStack.top()->parent = nullptr;
                cst = cstStack.top();
                // Accept
                return true;
            }else if (action.empty()){
                return false;
            }else {
                // Error
                errors += "Error in state " + to_string(currentState) + " for symbol " + inputQueue.front().lexeme + "\n";
                break;
            }
        }
        return false;
    }
    string addNodes(Node* node){
        if (node == nullptr)
            return "";
        string result = to_string(reinterpret_cast<uintptr_t>(node)) + "[label=\"" + node->value + "\"] \n";
        for (auto& child : node->children) {
            result += addNodes(child);
        }
        return result;
    }

    string addConnections(Node* node){
        if (node == nullptr)
            return "";
        string connections;
        for (auto & i : node->children) {
            connections += to_string(reinterpret_cast<uintptr_t>(node)) + " -- " + to_string(reinterpret_cast<uintptr_t>(i)) + "\n";
            connections += addConnections(i);
        }
        return connections;
    }
    void printTree(Node* root, const string& filename) {
        if (root == nullptr)
            return;
        string graph = "graph ast { \n" + addNodes(root) + addConnections(root) + "}";
        ofstream file(filename);
        file << graph;
    }
};