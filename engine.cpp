#include "engine.h"
#include <cassert>

std::pair<int, std::vector<BestMove>> getBest(Board &bd, int depth)
{
    if (depth == 0)
    {
        return {bd.getScore(), {}};
    }
    std::vector<Move> moves = bd.allMoves();
    std::vector<Smove> smoves = bd.specialMoves();
    bool on_move = bd.onMove();

    int best = on_move ? -1000 : 1000;
    std::vector<BestMove> b_moves = {};

    for (const auto &move : moves)
    {
        bool res = bd.movePiece(move);
        if (!res)
            continue;

        auto [val, c_moves] = getBest(bd, depth - 1);
        assert(bd.undoMove());

        if (on_move)
        {
            if (val >= best)
            {
                best = val;
                b_moves = c_moves;
                b_moves.push_back(move);
                if (best >= 1000)
                    return {best, b_moves};
            }
        }
        else
        {
            if (val <= best)
            {
                best = val;
                b_moves = c_moves;
                b_moves.push_back(move);
                if (best <= -1000)
                    return {best, b_moves};
            }
        }
    }

    for (const auto &smove : smoves)
    {
        bool res = bd.smovePiece(smove);
        if (!res)
            continue;

        auto [val, c_moves] = getBest(bd, depth - 1);
        assert(bd.undoSmove());

        if (on_move)
        {
            if (val >= best)
            {
                best = val;
                b_moves = c_moves;
                b_moves.push_back(smove);
                if (best >= 1000)
                    return {best, b_moves};
            }
        }
        else
        {
            if (val <= best)
            {
                best = val;
                b_moves = c_moves;
                b_moves.push_back(smove);
                if (best <= -1000)
                    return {best, b_moves};
            }
        }
    }
    return {best, b_moves};
}

std::string descField(Coords coords){
    auto [x, y] = coords;
    return std::string("") + ((char)('A'+ x)) + std::to_string(8-y);
}

std::string descSmove(Smove smv){
    auto [first, second] = smv;
    if(second.from.x == -1){
        switch (second.from.y)
        {
        case 0:
            return descField(first.from) + " to rook";
        case 1:
            return descField(first.from) + " to knight";
        case 2:
            return descField(first.from) + " to bishop";
        case 3:
            return descField(first.from) + " to queen";
        default:
            return "";
        }    
    }
    if(second.to.x == -1){
        return "enpass";
    }
    if(second.from.x == 7){
        return "ks castle";
    }
    if(second.from.x == 0){
        return "qs castle";
    }
    return "";
}

void printResult(const std::vector<BestMove> &b_moves, int val, Board bd)
{
    std::cout << "best variant: " << val << "\n";
    std::cout << "board:\n" << bd;
    int cnt = 1;
    for (int i = b_moves.size() - 1; i >= 0; i--)
    {
        std::string desc = "";
        if (std::holds_alternative<Move>(b_moves[i]))
        {
            Move mv = std::get<Move>(b_moves[i]);
            bd.movePiece(mv);
            desc = descField(mv.from) + " -> " + descField(mv.to);
        }
        else
        {
            Smove smv = std::get<Smove>(b_moves[i]);
            bd.smovePiece(smv);
            desc = descSmove(smv);
        }
        std::cout << cnt << ": " << desc << "\n" << bd;
        cnt ++;
    }
}

void findBestVariant(std::string fen, int depth){
    Board bd(fen);
    
    clock_t start = clock();

    auto [val, b_moves] = getBest(bd, depth);
    printResult(b_moves, val, bd);

    clock_t end = clock();
    double elapsed_time = double(end - start) / CLOCKS_PER_SEC;
    std::cout << "time : " << elapsed_time << " s" << "\n";
}