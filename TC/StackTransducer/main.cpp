#include <iostream>
#include <fstream>

#include "StackTransducer.h"

using namespace std;

int main(){
    ifstream f("data.in");
    StackTransducer transducer(f);

    ifstream input("input.in");

    string inputString;
    input >> inputString;

    ofstream g("data.out");
    transducer.run(inputString,g);

    return 0;
}