#include "StackTransducer.h"

using namespace std;

bool operator<(StackTransducer::Transition a,StackTransducer::Transition b){
    auto t1=tie(a.state,a.inputChar,a.stackChar);
    auto t2=tie(b.state,b.inputChar,b.stackChar);

    return t1<t2;
}

ostream &operator <<(ostream &out,const StackTransducer::Transition &trans){
    out << "Trans: " << trans.state<<","<<trans.inputChar<<","<<trans.stackChar;
    return out;
}

ostream &operator <<(ostream &out,const StackTransducer::TransitionResult &trans){
    out << "TransRes: " << trans.state<<","<<trans.stackString<<","<<trans.outputString;
    return out;
}

ostream &operator <<(ostream &out,const StackTransducer::Description &desc){
    string input = StackTransducer::emptyToLambda(desc.input);
    string stack = StackTransducer::emptyToLambda(desc.stack);
    string output = StackTransducer::emptyToLambda(desc.output);
    
    out << "Trans: " << desc.state<<","<<input<<","<<stack<<","<<output;
    return out;
}

string StackTransducer::removeSubstrings(string str, string pattern){
    if(pattern == ""){
        return str;
    }
    
    for(auto i = str.find(pattern); i != string::npos; i=str.find(pattern)){
        str.erase(i,pattern.length());
    }

    return str;
}

string StackTransducer::emptyToLambda(string str,char lambda){
    if(str.length()>0){
        return str;
    } else {
        return string(1,lambda);
    }
}


StackTransducer::StackTransducer(istream &in){
    in>>this->acceptance;
    in>>this->initialState;

    int nrTrans;
    in>>nrTrans;
    while(nrTrans--){
        Transition trans;
        string nothing;
        TransitionResult res;

        in>>trans.state>>trans.inputChar>>trans.stackChar;
        in>>nothing;
        in>>res.state>>res.stackString>>res.outputString;

        res.stackString = StackTransducer::removeSubstrings(res.stackString,string(1,LAMBDA_SYMBOL));
        res.outputString = StackTransducer::removeSubstrings(res.outputString,string(1,LAMBDA_SYMBOL));

        this->delta[trans].push_back(res);
    }

    if(this->acceptance == 'F'){
        int nrFinal;
        in>>nrFinal;
        while(nrFinal--){
            int state;
            in>>state;
            this->finalStates.insert(state);
        }
    }
}
bool StackTransducer::acceptFinalState(const Description &desc){
    if(desc.input.length()==0 && this->acceptance=='F' && this->finalStates.count(desc.state)==1){
        return true;
    } else {
        return false;
    }
}

bool StackTransducer::acceptEmptyStack(const Description &desc){
    if(desc.input.length()==0 && this->acceptance=='S' && desc.stack.length()==0){
        return true;
    } else {
        return false;
    }
}

bool StackTransducer::acceptState(const Description &desc){
    return this->acceptFinalState(desc) || this->acceptEmptyStack(desc);
}

int StackTransducer::expand(const QueElem&elem, queue<QueElem>&Q,bool withLambda){
    Description curr = elem.desc;

    if(curr.stack.length()==0 || (curr.input.length()==0 && withLambda==false)){
        return 0;
    }

    Transition trans;
    trans.state=curr.state;
    trans.inputChar=(withLambda)?LAMBDA_SYMBOL:curr.input[0];
    trans.stackChar=curr.stack.back();

    int newElems=0;
    for(TransitionResult res:this->delta[trans]){
        Description newDesc;
        newDesc.state=res.state;
        newDesc.input=(withLambda)?curr.input:curr.input.substr(1);

        string newStack = curr.stack;
        newStack.pop_back();
        newStack += res.stackString;
        newDesc.stack=newStack;

        newDesc.output = curr.output + res.outputString;

        QueElem newElem;
        newElem.desc = newDesc;

        newElem.path=elem.path;
        newElem.path.push_back(curr);

        Q.push(newElem);
        newElems++;
    }
    return newElems;
}


void StackTransducer::run(string input, ostream &out, int maxRes){
    input = StackTransducer::removeSubstrings(input,string(1,LAMBDA_SYMBOL));

    queue<QueElem>Q;
    string initialStack = string(1,StackTransducer::STACK_BASE);
    Description desc = {this->initialState,input,initialStack,""};
    Q.push({desc,{}});

    int cntRes=0;

    while(!Q.empty()){
        QueElem elem=Q.front();
        Q.pop();
        Description desc = elem.desc;

        if(this->acceptState(desc)){
            this->write(input,elem,out);

            cntRes++;
            if(cntRes==maxRes){
                out<<"Stopped after printing "<<maxRes<<" results\n";
                return;
            }
        }

        expand(elem,Q,false);
        expand(elem,Q,true);
    }

    if(cntRes==0){
        out<<"The input is rejected\n";
    } else {
        out<<"The transducer has found "<<cntRes<<" results\n";
    }
}

void StackTransducer::write(string input,QueElem elem, ostream &out){
    input = StackTransducer::emptyToLambda(input);
    out<<input<<'\n';

    string output=elem.desc.output;
    output=StackTransducer::emptyToLambda(output);
    out<<output<<'\n';

    out<<"Path:\n";
    for(auto it:elem.path){
        out<<it<<'\n';
    }

    out<<'\n';
}