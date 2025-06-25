#include "engine.h"

using namespace std;

int readInt()
{
    string tmp;
    getline(cin, tmp);
    return stoi(tmp);
}

int main()
{
    // 2bB4/N2k1N2/3P1P2/4p3/1R3p1P/Q6p/6p1/K3R3 w - - 0 1
    string fen;
    getline(cin, fen);
    int depth = readInt();
    findBestVariant(fen, depth);
    return 0;
}