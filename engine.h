#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <variant>
#include <utility>
#include <vector>
#include <ctime>
#include <iostream>
#include "board.h"

class Engine{
    private:
        Board bd;
        unsigned int flags; // print clock, print moves
        std::pair<int, std::vector<Move>> getBest(Board &bd, int depth, int alfa, int beta);
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