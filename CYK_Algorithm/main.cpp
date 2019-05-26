#include <iostream>
#include <Fstream>
#include <vector>
#include <set>
#include <map>
#include <cstring>

using namespace std;

ifstream f("data.in");

class CYK{
public:
    string word;
    string start;
    
    int cntTerminal;
    vector<string> terminals;
    
    int cntNonterminal;
    vector<string> nonterminals;

    int cntProds;
    map<string,vector<string> >productions;
    map<char,int> indexOf;

    friend istream &operator >>(istream &in,CYK &x);

    void createTable(){
        int len=word.length();
        bool dp[len+1][len+1][cntNonterminal+1];
        //set<char> generators[len+1][len+1];
        
        len--;

        memset(dp,false,sizeof(dp));


        for(int i=1;i<=len;i++){
            for(auto it:nonterminals){
                for(auto prod:productions[it]){
                    if(prod[0]==word[i]){
                        dp[1][i][indexOf[it[0]]]=true;
                        //generators[1][i].insert(it[0]);
                    }
                }
            }
        }

        for(int l=2;l<=len;l++){
            for(int s=1;s<=len-l+1;s++){
                for(int p=1;p<=l-1;p++){
                    for(auto it:productions){
                        for(auto it2:it.second){
                            if(it2.length()>1){
                                if(dp[p][s][indexOf[it2[0]]] && dp[l-p][s+p][indexOf[it2[1]]]){
                                    dp[l][s][indexOf[it.first[0]]]=true;
                                    //generators[l][s].insert(it.first[0]);
                                }
                            }
                        }
                    }
                }
            }
        }

        if(dp[len][1][1]){
            cout<<"DA\n";
        } else {
            cout<<"NU\n";
        }

        /*for(int i=1;i<=len;i++){
            for(int j=1;j<=len-i+1;j++){
                cout<<"GENERATORII LA "<<i<<" "<<j<<" sunt: ";
                for(auto it:generators[i][j]){
                    cout<<it<<",";
                }
                cout<<'\n';
            }
        }*/
        for(int k=1;k<=cntNonterminal;k++){
            cout<<k<<"\n";
            for(int i=1;i<=len;i++){
                for(int j=1;j<=len-i+1;j++){
                    cout<<dp[i][j][k]<<" ";
                }
                cout<<'\n';
            }
        }
    }
};

istream &operator>>(istream &in,CYK &cyk){
    in>>cyk.word;
    cyk.word=" "+cyk.word;
    
    in>>cyk.start;
    in>>cyk.cntTerminal;
    for(int i=0;i<cyk.cntTerminal;i++){
        string aux;
        in>>aux;
        cyk.terminals.push_back(aux);
    }
    in>>cyk.cntNonterminal;
    for(int i=0;i<cyk.cntNonterminal;i++){
        string aux;
        in>>aux;
        cyk.nonterminals.push_back(aux);
        cyk.indexOf[aux[0]]=i+1;
    }

    in>>cyk.cntProds;
    for(int i=0;i<cyk.cntProds;i++){
        string first;
        string second;
        in>>first>>second;
        cyk.productions[first].push_back(second);
    }

    return in;
}

int main(){
    CYK cyk;
    f>>cyk;

    for(auto it:cyk.nonterminals){
        cout<<it<<" "<<cyk.indexOf[it[0]]<<'\n';
    }
    
    cyk.createTable();

    return 0;
}