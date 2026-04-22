#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <iostream>

enum Collision {
    OUT_OF_B = -1,
    NO_COLL,
    COLL,
    OPP
};

struct Coords {
    int x;
    int y;
};

struct Nmove {
    Coords from;
    Coords to;
};

typedef std::pair<Nmove, Nmove> Smove;

typedef std::variant<Nmove, Smove> Move;

struct UndoNmove {
    Coords from;
    char from_field;
    Coords to;
    char to_field;
    unsigned int castles;
    Coords enpass;
};

typedef std::pair<UndoNmove, UndoNmove> UndoSmove;

typedef std::variant<UndoNmove, UndoSmove> UndoMove;

class Board {
private:
    const std::map<char, int> VALUES = {{'r', 5}, {'n', 3}, {'b', 3}, {'q', 9}, {'p', 1}, {'k', 0}};

    char arr[64]; // [y*8 + x]
    unsigned int castles; // 0b1000: white ks, 0b0100 white qs, 0b0010 black ks, 0b0001 black qs
    Coords enpass; // (x, y)
    bool on_move; // true: white, false: black

    std::vector<UndoMove> undo_stack;
    std::vector<bool> undo_special;

    std::vector<std::string> splitFen(const std::string &str);
    char getField(int x, int y);
    Collision isCollision(int x, int y);
    void setField(int x, int y, char piece);
    bool getColor(int x, int y);
    
    std::vector<Nmove> pMoves(Coords from);
    std::vector<Nmove> nMoves(Coords from);
    std::vector<Nmove> bMoves(Coords from);
    std::vector<Nmove> rMoves(Coords from);
    std::vector<Nmove> qMoves(Coords from);
    std::vector<Nmove> kMoves(Coords from);
    
    bool nChecking(Coords from);
    bool bChecking(Coords from, char piece = 'b');
    bool rChecking(Coords from, char piece = 'r');
    bool qChecking(Coords from);
    bool pChecking(Coords from);
    
    Coords getKingOnMove();
    
    std::vector<Nmove> normalMoves();
    std::vector<Smove> specialMoves();
    
    void undo(const UndoNmove *undo_nmove);
    
    bool smovePiece(const Smove *smove);
    bool nmovePiece(const Nmove *nmove);

    
    static std::string descNmove(const Nmove *nmv);
    static std::string descSmove(const Smove *smv);

public:
    Board(std::string fen = "");
    Board(const Board &other);
    
    friend std::ostream &operator<<(std::ostream &os, Board &bd);

    static std::string descField(Coords coords);
    static std::string descMove(const Move &move);
    void readFen(std::string fen);
    bool onMove();
    std::vector<Move> allMoves();
    std::vector<Nmove> getMoves(Coords from);

    bool isCheck(Coords from = {-1, -1});
    bool isMate();
    bool isStaleMate();
    bool movePiece(const Move &move);
    bool undoMove();
    int getScore();
    int eval();
};

#endif // BOARD_H