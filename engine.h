#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <variant>
#include <utility>
#include <vector>
#include <ctime>
#include <iostream>
#include "board.h"


typedef std::variant<Move, Smove> BestMove;

class Engine{
    private:
        Board bd;
        unsigned int flags; // print clock, print moves
        std::pair<int, std::vector<BestMove>> getBest(Board &bd, int depth);
        std::string descField(Coords coords);
        std::string descSmove(Smove smv);
        std::string doMove(Board &bd, const BestMove &b_move);
        void printMoves(Board bd, const std::vector<BestMove> &b_moves);
        void printResult(Board bd, int val, const BestMove &b_move);
        void printResult(Board bd, int val, const std::vector<BestMove> &b_moves);
    public:
        Engine(std::string fen);
        Engine(std::string fen, std::vector<std::string> flags);
        void findBestVariant(int depth);
};

#endif //ENGINE_H