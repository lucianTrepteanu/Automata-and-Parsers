#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <queue>

using namespace std;

class StackTransducer{
public:
    struct Transition{
        int state;
        char inputChar;
        char stackChar;

        friend bool operator<(Transition a,Transition b);

        friend ostream& operator << (ostream &out, const Transition &trans);
    };

    struct TransitionResult{
        int state;
        string stackString;
        string outputString;

        friend ostream& operator << (ostream &out, const TransitionResult& trans);
    };

    struct Description{
        int state;
        string input,stack,output;

        friend ostream &operator << (ostream &out, const Description &desc);
    };

    static const char STACK_BASE = '_';
    static const char LAMBDA_SYMBOL = '$';

private:
    struct QueElem{
        StackTransducer::Description desc;
        vector<Description> path;
    };

    int initialState;
    char acceptance;
    map<Transition,vector<TransitionResult> >delta;
    set<int> finalStates;

    int expand(const QueElem&, queue<QueElem>&,bool);
    void write(string input, QueElem elem, ostream&out);

public:
    StackTransducer(istream&);
    bool acceptFinalState(const Description&desc);
    bool acceptEmptyStack(const Description&desc);
    bool acceptState(const Description&desc);

    void run(string input,ostream &out,int maxResults=10);

    static string removeSubstrings(string str,string pattern);
    static string emptyToLambda(string, char lambda=StackTransducer::LAMBDA_SYMBOL);
};