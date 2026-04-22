#include "engine.h"
#include <algorithm>
#include <cassert>

Engine::Engine(std::string fen) : bd(fen) {}

Engine::Engine(std::string fen, std::vector<std::string> _flags) : bd(fen)
{
    flags = 0b11;
    for (const auto &flag : _flags)
    {
        if (flag == "noprint")
            flags &= 0b10;
        else if (flag == "noclock")
            flags &= 0b01;
    }
}

std::pair<int, std::vector<Move>> Engine::getBest(
    Board &bd, int depth, int alpha = -1000, int beta = 1000)
{
    if (depth == 0) {
        return {bd.eval(), {}};
    }

    std::vector<Move> moves = bd.allMoves();

    if (moves.empty()) {
        return {bd.eval(), {}};
    }

    int best_score = -100000;
    std::vector<Move> b_moves;

    for (const auto &move : moves) {
        if (!bd.movePiece(move)) continue;

        auto [score, c_moves] = getBest(bd, depth - 1, -beta, -alpha);
        score = -score;

        bd.undoMove();

        if (score > best_score) {
            best_score = score;

            b_moves.clear();
            b_moves.push_back(move);
            b_moves.insert(b_moves.end(), c_moves.begin(), c_moves.end());
        }

        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            break; 
        }
    }

    if (best_score == -100000) {
        return {bd.eval(), {}};
    }

    return {best_score, b_moves};
}

std::string Engine::moveAndPrint(Board &bd, const Move &b_move)
{
    bd.movePiece(b_move);
    return Board::descMove(b_move);
}

void Engine::printMoves(Board bd, const std::vector<Move> &b_moves)
{
    int cnt = 1;
    for (size_t i = 0; i < b_moves.size(); i++)
    {
        std::cout << cnt << ": " << moveAndPrint(bd, b_moves[i]) << "\n"
                  << bd << "\n";
        cnt++;
    }
}

void Engine::printResult(Board bd, int score, const std::vector<Move> &b_moves)
{
    std::cout << "best variant: " << score << "\n";
    std::cout << "board:\n"
              << bd;
    Engine::printMoves(bd, b_moves);
}

void Engine::printResult(Board bd, int score, const Move &bmove)
{
    std::cout << "best variant: " << score << "\n";
    std::cout << "first move: " << moveAndPrint(bd, bmove) << "\n";
}

void Engine::findBestVariant(int depth)
{

    clock_t start = clock();

    auto [score, b_moves] = getBest(bd, depth);

    if (flags & 0b01)
        printResult(bd, score, b_moves);
    else
        printResult(bd, score, b_moves[0]);

    clock_t end = clock();
    if (flags & 0b10)
    {
        double elapsed_time = double(end - start) / CLOCKS_PER_SEC;
        std::cout << "time : " << elapsed_time << " s" << "\n";
    }
}