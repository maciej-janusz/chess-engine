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

std::pair<int, std::vector<BestMove>> getBest(Board &bd, int depth);
void printResult(const std::vector<BestMove> &b_moves, int val, Board bd);
void findBestVariant(std::string fen, int depth);

#endif // BOARD_AI_H