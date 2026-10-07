#include "engine.h"
#include <stdexcept>

int main(int argc, char* argv[])
{
    // flags: noprint, noclock
    std::vector<std::string> flags(argv + 1, argv + argc);

    // 6k1/3b3r/1p1p4/p1n2p2/1PPNpP1q/P3Q1p1/1R1RB1P1/5K2 b - - 0 1 //mate in 5
    // 2bB4/N2k1N2/3P1P2/4p3/1R3p1P/Q6p/6p1/K3R3 w - - 0 1 //mate in 3
    try
    {
        std::string fen;
        std::cout << "Enter FEN: ";
        std::getline(std::cin, fen);
        std::cout << "Enter depth: ";
        std::string depth_str;
        std::getline(std::cin, depth_str);
        int depth = std::stoi(depth_str);

        Engine engine(fen, flags);
        engine.findBestVariant(depth);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
