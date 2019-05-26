
#include <bits/stdc++.h>
using namespace std;

const char K_LAMBDA='.';

ifstream f("data.in");
ofstream g("data.out");

class LNFA{
public:
    int nrStates;
    vector<int> states;
    map<int,int> normStates;

    int startState;

    int nrFinal;
    map<int,bool> isFinal;

    int cntSigma;
    vector<char> alphabet;

    int nrTrans;
    map<pair<int,char> ,vector<int> >transitions;
    map<int,vector<int> > lambdaTrans;
    map<int,vector<int> >backLambda;

    map<int,set<int> >backClosure;
    map<int,set<int> >lambdaClosure;

    void readData(int delay){
        f>>nrStates;
        states.resize(nrStates);
        for(int i=0;i<nrStates;i++){
            f>>states[i];
            normStates[states[i]]=delay+i+1;
        }

        f>>cntSigma;
        alphabet.resize(cntSigma);
        for(int i=0;i<cntSigma;i++){
            f>>alphabet[i];
        }

        f>>startState;
        startState=normStates[startState];
        f>>nrFinal;
        for(int i=0;i<nrFinal;i++){
            int x;
            f>>x;
            isFinal[normStates[x]]=true;
        }

        f>>nrTrans;
        for(int i=0;i<nrTrans;i++){
            int state1,state2;
            char letter;

            f>>state1>>letter>>state2;
            state1=normStates[state1];
            state2=normStates[state2];

            if(letter!=K_LAMBDA){
                transitions[{state1,letter}].push_back(state2);    
            } else {
                lambdaTrans[state1].push_back(state2);
                backLambda[state2].push_back(state1);
            }
        }    
    }

    void printData(){
        g<<nrStates<<'\n';
        for(int i=0;i<nrStates;i++){
            if(normStates[states[i]]==0){
                g<<states[i]<<" ";
            } else {
                g<<normStates[states[i]]<<" ";
            }
        }
        g<<'\n';

        g<<cntSigma<<'\n';
        for(int i=0;i<alphabet.size();i++){
            g<<alphabet[i]<<" ";
        }
        g<<'\n';

        g<<startState<<'\n';
        g<<nrFinal<<'\n';
        for(auto it:isFinal){
            g<<it.first<<" ";
        }
        g<<'\n';

        g<<nrTrans<<'\n';
        for(auto it:transitions){
            for(auto it2:it.second)
                g<<it.first.first<<" "<<it.first.second<<" "<<it2<<'\n';
        }
    }

    void dfs(int nod,set<int> &vis,int start){
        lambdaClosure[start].insert(nod);
        vis.insert(nod);
        for(auto it:lambdaTrans[nod])
            if(vis.find(it)==vis.end()){
                dfs(it,vis,start);
            }
    }

    void dfsBack(int nod,set<int> &vis,int start){
        backClosure[start].insert(nod);
        vis.insert(nod);
        for(auto it:backLambda[nod])
            if(vis.find(it)==vis.end()){
                dfsBack(it,vis,start);
            }
    }

    void getClosures(){
        for(int i=1;i<=nrStates;i++){
            lambdaClosure[i].insert(i);
            set<int> vis;
            dfs(i,vis,i);
        }
    }

    void getBackClosures(){
        for(int i=1;i<=nrStates;i++){
            backClosure[i].insert(i);
            set<int> vis;
            dfsBack(i,vis,i);
        }
    }

    void printClosure(int nod){
        for(auto it:lambdaClosure[nod]){
            cout<<it<<" ";
        }
        cout<<'\n';
    }
};


int main(){
    LNFA fa1,fa2;

    fa1.readData(0);
    fa2.readData(fa1.nrStates);

    LNFA concate;
    concate.nrStates=fa1.nrStates+fa2.nrStates;
    for(int i=0;i<fa1.nrStates;i++){
        concate.states.push_back(fa1.normStates[fa1.states[i]]);
    }
    for(int i=0;i<fa2.nrStates;i++){
        concate.states.push_back(fa2.normStates[fa2.states[i]]);
    }

    concate.nrFinal=fa2.nrFinal;
    for(auto it:fa2.isFinal){
        concate.isFinal[it.first]=true;
    }

    concate.startState=fa1.startState;

    concate.cntSigma=fa1.cntSigma+fa2.cntSigma;
    for(int i=0;i<fa1.cntSigma;i++){
        concate.alphabet.push_back(fa1.alphabet[i]);
    }
    for(int i=0;i<fa2.cntSigma;i++){
        concate.alphabet.push_back(fa2.alphabet[i]);
    }

    concate.nrTrans=fa1.nrTrans+fa2.nrTrans;
    for(auto it:fa1.transitions){
        for(auto it2:it.second){
            concate.transitions[{it.first.first,it.first.second}].push_back(it2);
        }
    }
    for(auto it:fa2.transitions){
        for(auto it2:it.second){
            concate.transitions[{it.first.first,it.first.second}].push_back(it2);
        }
    }
    for(auto it:fa1.isFinal){
        concate.lambdaTrans[it.first].push_back(fa2.startState);
    }

    concate.getClosures();
    concate.getBackClosures();
    
    /*for(int i=1;i<=concate.nrStates;i++){
        concate.printClosure(i);
    }*/

    map<pair<set<int>,char> ,set<int> >newTrans;
    
    queue<set<int> > Que;
    set<int> startSet;
    startSet=concate.backClosure[concate.startState];
    for(auto it:concate.lambdaClosure[concate.startState]){
        startSet.insert(it);
    }

    map<set<int>,int> normalise;
	map<int,set<int> >invNormalise;
	int currIdx=0;
    map<set<int>,bool> visited;
    
	visited[startSet]=true;
    Que.push(startSet);
	normalise[startSet]=++currIdx;
	invNormalise[currIdx]=startSet;
	map<pair<int,char>,int> normTrans;
	
	while(!Que.empty()){
        set<int> curr=Que.front();
        Que.pop();

        for(auto letter:concate.alphabet){
			set<int> newState;
            for(auto nod:curr){
                for(auto newNod:concate.lambdaClosure[nod]){
                    for(auto toState:concate.transitions[{newNod,letter}]){
                        for(auto lastLambda:concate.lambdaClosure[toState]){
                            newState.insert(lastLambda);
                        }
                    }
                }
            }
			
			if(newState.size()==0){
				continue;
			}

			newTrans[{curr,letter}]=newState;
			
			if(visited[newState]==false){
				Que.push(newState);
				visited[newState]=true;
				normalise[newState]=++currIdx;
				invNormalise[currIdx]=newState;
			}
			
			normTrans[{normalise[curr],letter}]=normalise[newState];
        }
    }
	
	map<int,bool> reducedFinal;
	for(auto it:normalise){
		for(auto it2:it.first){
			if(concate.isFinal[it2]==true){
				reducedFinal[it.second]=true;
			}
		}
	}
	
    g<<concate.nrStates<<'\n';
    for(auto it:concate.states){
        g<<it<<" ";
    }
    g<<'\n';

    g<<concate.cntSigma<<'\n';
    for(auto it:concate.alphabet){
        g<<it<<" ";
    }
    g<<'\n';
    
    concate.startState=normalise[startSet];
    g<<concate.startState<<'\n';
    
    g<<reducedFinal.size()<<'\n';
    for(auto it:reducedFinal){
        g<<it.first<<" ";
    }
    g<<'\n';
   
    g<<normTrans.size()<<'\n';
	for(auto it:normTrans){
		g<<it.first.first<<" "<<it.first.second<<" "<<it.second<<'\n';
	}

    return 0;
}