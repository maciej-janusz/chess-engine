#include "engine.h"

int readInt()
{
    std::string tmp;
    std::getline(std::cin, tmp);
    return stoi(tmp);
}

int main(int argc, char* argv[])
{
    char** begin = argv;
    char** end = argv + argc; 
    std::vector<std::string> args_vector(begin, end);

    // 6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1 //mate in 5
    // 2bB4/N2k1N2/3P1P2/4p3/1R3p1P/Q6p/6p1/K3R3 w - - 0 1 //mate in 3
    std::string fen;
    std::cout << "Enter FEN: ";
    std::getline(std::cin, fen);
    std::cout << "Enter depth: ";
    int depth = readInt();
    Engine engine(fen, args_vector);
    engine.findBestVariant(depth);
    return 0;
}