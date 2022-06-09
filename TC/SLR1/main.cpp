#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>

using namespace std;

const char LAMBDA = '_';
const char END_SYMBOL = '#';
const char START_SYMBOL = '$';

// STRUCTS

struct Production{
    int id;
    char lft;
    string rgt;

    friend ostream& operator <<(ostream &out,const Production&x){
        string rgt = x.rgt;
        if(x.rgt == ""){
            rgt = string(1,LAMBDA);
        }
        out << x.id << ": " << x.lft << "->" << rgt;
        return out;
    }
};

struct LR0Item {
    int prodId;
    char lft;
    string leftDot;
    string rightDot;

    friend ostream& operator <<(ostream &out,const LR0Item &x){
        out << x.prodId << " " << x.lft << "->" << x.leftDot << "." << x.rightDot;
        return out;
    }

    friend bool operator == (const LR0Item &a, const LR0Item &b){
        auto t1 = tie(a.prodId, a.lft, a.leftDot, a.rightDot);
        auto t2 = tie(b.prodId, b.lft, b.leftDot, b.rightDot);
        return t1 == t2;
    }

    friend bool operator < (const LR0Item &a, const LR0Item &b){
        auto t1 = tie(a.prodId, a.lft, a.leftDot, a.rightDot);
        auto t2 = tie(b.prodId, b.lft, b.leftDot, b.rightDot);
        return t1 < t2;
    }
};

struct Action{
    int id;
    string type;

    Action(){
        id = -1;
        type = "ERROR";
    }

    friend ostream& operator << (ostream &out, const Action&x){
        if(x.type == "SHIFT"){
            out << "shift " << x.id;
        } else if(x.type == "REDUCE"){
            out << "reduce " << x.id;
        } else if(x.type == "ACCEPT"){
            out << "accept";
        } else {
            out << "ERROR";
        }

        return out;
    }
};

//DATA
int n;
char S;
vector<Production> allProds;
set<char> terminals, nonTerminals;

map<char, vector<Production> >prod;
map<char, set<string> >first, follow;

map<set<LR0Item>, int> stateToId;
map<int, set<LR0Item> >idToState;

map<int, map<char, int> > transition;
map<int, map<char, Action> >actionTable;
map<int, map<char, int> >gotoTable;


//UTILITY FUNCTIONS
bool isNonTerminal(char c){
    return ('A' <= c && c <= 'Z') || c == START_SYMBOL;
}

bool isTerminal(char c){
    return !isNonTerminal(c);
}

bool hasNonTerminal(string str){
    for(auto it : str){
        if(isNonTerminal(it)){
            return true;
        }
    }
    return false;
}

string removeSubstrings(string str, string pattern){
    if(pattern == ""){
        return str;
    }
    
    for(auto i = str.find(pattern); i != string::npos; i=str.find(pattern)){
        str.erase(i,pattern.length());
    }

    return str;
}

string removeLambda(string str){
    return removeSubstrings(str, string(1,LAMBDA));
}

void buildTerms(string str){
    for(auto it:str){
        if(isTerminal(it)){
            terminals.insert(it);
        } else {
            nonTerminals.insert(it);
        }
    }
}

void addTerms(char c){
    buildTerms(string(1, c));
}

template<typename T>
bool mergeSets(set<T>&a, set<T>&b){
    bool res = false;
    for(const T& e : b){
        if(a.count(e) == 0){
            a.insert(e);
            res = true;
        }
    }

    return res;
}

set<string> getFirst(const string&str, map<char, set<string> >&fncOfLast){
    set<string> res;
    bool nullable = true;
    int i = 0;

    for(const char&c : str){
        set<string> currSet;
        if(i < str.size() - 1){
            currSet = first[c];
        } else {
            currSet = fncOfLast[c];
        }

        for(string s : currSet){
            if(s != ""){
                res.insert(s);
            }
        }

        bool currNull = (currSet.count("")==0);
        if(currNull){
            nullable = false;
            break;
        }

        i++;
    }
    if(nullable){
        res.insert("");
    }
    return res;
}


void buildFirst(){
    for(char c : terminals){
        first[c].insert(string(1, c));
    }
    for(Production p : allProds){
        if(hasNonTerminal(p.rgt) == false){
            first[p.lft].insert(p.rgt.substr(0, 1));
        }
    }

    bool running = true;
    while(running){
        running = false;
        for(Production p : allProds){
            if(hasNonTerminal(p.rgt) == false){
                continue;
            }
            set<string> aux = getFirst(p.rgt, first);
            if(mergeSets(first[p.lft], aux)){
                running = true;
            }
        }
    }
}


void buildFollow(){
    follow[S].insert(string(1, END_SYMBOL));

    bool running = true;
    while(running){
        running = false;
        for(Production p : allProds){
            for(int i=0; i<p.rgt.size(); i++){
                char c = p.rgt[i];

                if(isNonTerminal(c)){
                    string suff = p.rgt.substr(i+1);
                    set<string> aux = getFirst(suff + string(1, p.lft), follow);
                    if(mergeSets(follow[c], aux)){
                        running = true;
                    }
                }
            }
        }
    }
}


set<LR0Item> getClosure(set<LR0Item> s){
    queue<LR0Item> que;
    set<LR0Item> res = s;

    for(auto &it : s){
        que.push(it);
    }
    while(!que.empty()){
        auto nod = que.front();
        que.pop();

        if(nod.rightDot == ""){
            continue;
        }

        char ch = nod.rightDot[0];
        if(isNonTerminal(ch)){
            for(Production p : prod[ch]){
                LR0Item item;
                item.prodId = p.id;
                item.lft = p.lft;
                item.leftDot = "";
                item.rightDot = p.rgt;

                if(res.count(item) == 0){
                    res.insert(item);
                    que.push(item);
                }
            }
        }
    }

    return res;
}


set<LR0Item> getNextState(set<LR0Item> state, char ch){
    set<LR0Item> nxt;
    for(LR0Item it : state){
        if(it.rightDot.substr(0, 1) == string(1, ch)){
            LR0Item newIt;
            newIt.prodId = it.prodId;
            newIt.lft = it.lft;
            newIt.leftDot = it.leftDot + string(1, ch);
            newIt.rightDot = it.rightDot.substr(1);

            nxt.insert(newIt);
        }
    }

    return getClosure(nxt);
}

void buildTransTable(){
    LR0Item added;
    added.prodId = 0;
    added.lft = START_SYMBOL;
    added.leftDot = "";
    added.rightDot = string(1, S);

    set<LR0Item> state1 = getClosure({added});
    int cnt = 1;
    idToState[cnt] = state1;
    stateToId[state1] = cnt;
    cnt++;

    queue<set<LR0Item> >que;
    que.push(state1);

    while(!que.empty()){
        set<LR0Item> nod = que.front();
        que.pop();

        set<char> symbols = terminals;
        mergeSets(symbols, nonTerminals);
        for(const char &c:symbols){
            set<LR0Item> nxt = getNextState(nod, c);

            if(nxt == set<LR0Item>()){
                continue;
            }

            if(stateToId[nxt] == 0){
                idToState[cnt] = nxt;
                stateToId[nxt] = cnt;
                cnt++;
                que.push(nxt);
            }

            transition[stateToId[nod]][c] = stateToId[nxt];
        }
    }
}


bool addAction(int id, char c, Action act){
    if(actionTable[id].count(c) > 0 && actionTable[id][c].id != act.id && actionTable[id][c].type != act.type){
        return true;
    } else {
        actionTable[id][c] = act;
        return false;
    }
}


bool buildActionGoto(){
    for(auto it:idToState){
        int currId = it.first;
        set<LR0Item>&state = it.second;

        for(LR0Item item : state){
            if(item.rightDot != "" && isTerminal(item.rightDot[0])){
                Action act;
                act.type = "SHIFT";
                act.id = transition[currId][item.rightDot[0]];

                bool fail = addAction(currId, item.rightDot[0], act);
                if(fail){
                    return false;
                }
            } else if(item.prodId != 0 && item.rightDot == ""){
                Action act;
                act.type = "REDUCE";
                act.id = item.prodId;
                char lft = allProds[act.id].lft;

                for(string str:follow[lft]){
                    if(str == ""){
                        continue;
                    }
                    bool fail = addAction(currId, str[0], act);
                    if(fail){
                        return false;
                    }
                }
            } else if(item.prodId==0 && item.rightDot==""){
                Action act;
                act.type = "ACCEPT";

                bool fail = addAction(currId, END_SYMBOL, act);
                if(fail){
                    return false;
                }
            }
        }

        for(char nonterm : nonTerminals){
            if(transition[currId].count(nonterm) > 0){
                gotoTable[currId][nonterm] = transition[currId][nonterm];
            }
        }
    }

    return true;
}


//DEBUGGING
void printActionTable(ostream &out){
    for(auto it : idToState){
        int currId = it.first;

        for(char c : terminals){
            out<<"action["<<currId<<"]["<<c<<"] = "<<actionTable[currId][c] <<'\n';
        }
    }
    out<<"\n\n";
}


pair<bool,vector<int> >runWord(string str, ostream &out){
    out << "Cuvantul " << str << '\n';
    str += END_SYMBOL;

    vector<int> stk;
    vector<int> productions;
    stk.push_back(1);

    while(true){
        out << "Stiva: ";
        for(auto it : stk){
            out << it << " ";
        }
        out << "w: " << str << '\n';
        int currState = stk.back();
        char c = str[0];

        Action act = actionTable[currState][c];
        if(act.type == "ACCEPT"){
            reverse(productions.begin(), productions.end());
            return {true, productions};
        } else if(act.type == "ERROR"){
            reverse(productions.begin(), productions.end());
            return {false, productions};
        } else if(act.type == "SHIFT"){
            stk.push_back(act.id);
            str = str.substr(1);
        } else if(act.type == "REDUCE"){
            Production p = allProds[act.id];
            int lenProd = p.rgt.size();

            for(int i=0; i<lenProd; i++){
                stk.pop_back();
            }

            if(gotoTable[stk.back()][p.lft] == 0){
                reverse(productions.begin(), productions.end());
                return {false, productions};
            }
            stk.push_back(gotoTable[stk.back()][p.lft]);
            productions.push_back(p.id);
        }
    }
}


int main(){

    ifstream in("input.in");
    ofstream out("output.out");

    in >> n >> S;
    Production firstprod;
    firstprod.id = 0;
    firstprod.lft = START_SYMBOL;
    firstprod.rgt = string(1, S);

    allProds.push_back(firstprod);
    prod[firstprod.lft].push_back(firstprod);

    addTerms(START_SYMBOL);
    addTerms(END_SYMBOL);

    int cnt = 0;
    for(int i=0; i<n; i++){
        Production p;
        cnt++;
        p.id = cnt;
        in >> p.lft >> p.rgt;
        p.rgt = removeLambda(p.rgt);

        allProds.push_back(p);
        prod[p.lft].push_back(p);

        addTerms(p.lft);
        buildTerms(p.rgt);
    }

    buildFirst();
    buildFollow();
    buildTransTable();

    bool isSLR = buildActionGoto();
    if(isSLR == false){
        out << "Gramatica nu este SLR1\n";
        return 0;
    }

    //printActionTable(out);

    out << "Gramatica este SLR!\n";
    int cntWords;
    in >> cntWords;
    for(int nr=0; nr<cntWords; nr++){
        string str;
        in >> str;
        string newStr = removeLambda(str);

        auto res = runWord(newStr, out);
        if(res.first){
            out << "Cuvantul " << str << " este acceptat prin\n";
            for(auto prodId : res.second){
                out << allProds[prodId] << '\n';
            }
            out << '\n';
        } else {
            out << "Cuvantul " << str << "nu este acceptat \n";
        }
    }

    return 0;
}