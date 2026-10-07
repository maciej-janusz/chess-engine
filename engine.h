#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <utility>
#include <vector>
#include <ctime>
#include <iostream>
#include "board.h"

class Engine{
    private:
        Board bd;
        unsigned int flags = 0b11; // bit 0: print the whole variant, bit 1: print the clock
        std::pair<int, std::vector<Move>> getBest(Board &bd, int depth, int alpha = -1000, int beta = 1000);
        static std::string moveAndPrint(Board &bd, const Move &b_move);
        static void printMoves(Board bd, const std::vector<Move> &b_moves);
        static void printResult(Board bd, int val, const Move &b_move);
        static void printResult(Board bd, int val, const std::vector<Move> &b_moves);
    public:
        Engine(std::string fen);
        Engine(std::string fen, std::vector<std::string> flags);
        void findBestVariant(int depth);
};

#endif //ENGINE_H