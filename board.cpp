#include "board.h"
#include <sstream>
#include <algorithm>
#include <cassert>

std::vector<std::string> Board::splitFen(const std::string &str)
{
    std::istringstream sin(str);
    std::vector<std::string> splitted;
    std::string part;
    while (sin >> part)
    {
        splitted.push_back(part);
    }
    return splitted;
}

/**
 * @brief get field with X-Y-coordinates
 * standard range: x: 0=A, 7=H; y: 0=8, 7=1;
 * special values: (-1, 0): rook, (-1, 1): knight, (-1, 2): bishop, (-1, 3): queen
 * (-1, -1): out of board
 * @param x X-coordinate
 * @param y Y-coordinate
 * @return value of field '\0' - empty field
 */
char Board::getField(int x, int y)
{
    if (x == -1)
    {
        switch (y)
        {
        case 0:
            return on_move ? 'R' : 'r';
        case 1:
            return on_move ? 'N' : 'n';
        case 2:
            return on_move ? 'B' : 'b';
        case 3:
            return on_move ? 'Q' : 'q';
        default:
            return ' ';
        }
    }
    if (std::min(x, y) < 0 || std::max(x, y) > 7)
    {
        return ' ';
    }
    return arr[y * 8 + x];
}

/**
 * @brief checks if there is a collision
 * @param x X-coordinate
 * @param y Y-coordinate
 * @return 0: no collision, 1: collision, 2: opponent's piece, -1: out of board
 */
Collision Board::isCollision(int x, int y)
{
    if (std::min(x, y) < 0 || std::max(x, y) > 7)
        return OUT_OF_B;
    char piece = getField(x, y);
    if (piece == '\0')
        return NO_COLL;
    bool color = isupper(piece);
    if (color == on_move)
        return COLL;
    else
        return OPP;
}

/**
 * @brief set field
 * x: 0=A, 7=H; y: 0=8, 7=1
 * @param x X-coordinate
 * @param y Y-coordinate
 * @param piece piece to set
 */
void Board::setField(int x, int y, char piece)
{
    if (std::min(x, y) < 0 || std::max(x, y) > 7)
        return;
    arr[y * 8 + x] = piece;
}

bool Board::getColor(int x, int y)
{
    return isupper(getField(x, y));
}

std::vector<Move> Board::pMoves(Coords from)
{
    std::vector<Move> moves;
    auto [x, y] = from;
    int dir = on_move ? -1 : 1;
    int start = on_move ? 6 : 1;
    int end = 7 - start;
    if (y != end && isCollision(x, y + dir) == NO_COLL)
        moves.push_back({from, {x, y + dir}});
    if (isCollision(x + 1, y + dir) == OPP)
        moves.push_back({from, {x + 1, y + dir}});
    if (isCollision(x - 1, y + dir) == OPP)
        moves.push_back({from, {x - 1, y + dir}});
    if (y == start && isCollision(x, y + 2 * dir) == NO_COLL)
        moves.push_back({from, {x, y + 2 * dir}});

    return moves;
}
std::vector<Move> Board::nMoves(Coords from)
{
    std::vector<Move> moves;
    auto [x, y] = from;
    const std::vector<Coords> knight_moves = {
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

    for (const auto &move : knight_moves)
    {
        int x1 = x + move.x;
        int y1 = y + move.y;
        char coll = isCollision(x1, y1);
        if (coll == OUT_OF_B || coll == COLL)
            continue;

        moves.push_back({from, {x1, y1}});
    }

    return moves;
}
std::vector<Move> Board::bMoves(Coords from)
{
    std::vector<Move> moves;
    auto [x, y] = from;

    int x1 = x + 1, y1 = y + 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        x1++;
        y1++;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    x1 = x - 1, y1 = y - 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        x1--;
        y1--;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    x1 = x + 1, y1 = y - 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        x1++;
        y1--;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    x1 = x - 1, y1 = y + 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        x1--;
        y1++;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    return moves;
}
std::vector<Move> Board::rMoves(Coords from)
{
    std::vector<Move> moves;
    auto [x, y] = from;

    int x1 = x, y1 = y + 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        y1++;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    x1 = x, y1 = y - 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        y1--;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    x1 = x + 1, y1 = y;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        x1++;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    x1 = x - 1, y1 = y;
    while (isCollision(x1, y1) == NO_COLL)
    {
        moves.push_back({from, {x1, y1}});
        x1--;
    }
    if (isCollision(x1, y1) == OPP)
        moves.push_back({from, {x1, y1}});

    return moves;
}
std::vector<Move> Board::qMoves(Coords from)
{
    std::vector<Move> moves = bMoves(from);
    std::vector<Move> new_moves = rMoves(from);
    moves.insert(moves.end(), new_moves.begin(), new_moves.end());
    return moves;
}
std::vector<Move> Board::kMoves(Coords from)
{
    std::vector<Move> moves;
    auto [x, y] = from;

    if (isCollision(x + 1, y + 1) == NO_COLL || isCollision(x + 1, y + 1) == OPP)
        moves.push_back({from, {x + 1, y + 1}});
    if (isCollision(x, y + 1) == NO_COLL || isCollision(x, y + 1) == OPP)
        moves.push_back({from, {x, y + 1}});
    if (isCollision(x - 1, y + 1) == NO_COLL || isCollision(x - 1, y + 1) == OPP)
        moves.push_back({from, {x - 1, y + 1}});

    if (isCollision(x + 1, y) == NO_COLL || isCollision(x + 1, y) == OPP)
        moves.push_back({from, {x + 1, y}});
    if (isCollision(x - 1, y) == NO_COLL || isCollision(x - 1, y) == OPP)
        moves.push_back({from, {x - 1, y}});

    if (isCollision(x + 1, y - 1) == NO_COLL || isCollision(x + 1, y - 1) == OPP)
        moves.push_back({from, {x + 1, y - 1}});
    if (isCollision(x, y - 1) == NO_COLL || isCollision(x, y - 1) == OPP)
        moves.push_back({from, {x, y - 1}});
    if (isCollision(x - 1, y - 1) == NO_COLL || isCollision(x - 1, y - 1) == OPP)
        moves.push_back({from, {x - 1, y - 1}});

    return moves;
}

bool Board::nChecking(Coords from)
{
    auto [x, y] = from;
    const std::vector<Coords> knight_moves = {
        {1, 2}, {1, -2}, {-1, 2}, {-1, -2}, {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

    for (const auto &move : knight_moves)
    {
        int x1 = x + move.x;
        int y1 = y + move.y;

        if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == 'n')
            return true;
    }
    return false;
}

bool Board::bChecking(Coords from, char piece)
{
    auto [x, y] = from;

    int x1 = x + 1, y1 = y + 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        x1++;
        y1++;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    x1 = x - 1, y1 = y - 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        x1--;
        y1--;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    x1 = x + 1, y1 = y - 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        x1++;
        y1--;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    x1 = x - 1, y1 = y + 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        x1--;
        y1++;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    return false;
}
bool Board::rChecking(Coords from, char piece)
{
    auto [x, y] = from;

    int x1 = x, y1 = y + 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        y1++;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    x1 = x, y1 = y - 1;
    while (isCollision(x1, y1) == NO_COLL)
    {
        y1--;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    x1 = x + 1, y1 = y;
    while (isCollision(x1, y1) == NO_COLL)
    {
        x1++;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    x1 = x - 1, y1 = y;
    while (isCollision(x1, y1) == NO_COLL)
    {
        x1--;
    }
    if (isCollision(x1, y1) == OPP && tolower(getField(x1, y1)) == piece)
        return true;

    return false;
}
bool Board::qChecking(Coords from)
{
    return bChecking(from, 'q') || rChecking(from, 'q');
}
bool Board::pChecking(Coords from)
{
    auto [x, y] = from;
    int dir = on_move ? -1 : 1;
    if (isCollision(x + 1, y + dir) == OPP && tolower(getField(x + 1, y + dir)) == 'p')
        return true;
    if (isCollision(x - 1, y + dir) == OPP && tolower(getField(x - 1, y + dir)) == 'p')
        return true;
    return false;
}

Coords Board::getKingOnMove()
{
    int x = -1, y = -1;
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            if (tolower(getField(i, j)) == 'k' && getColor(i, j) == on_move)
            {
                x = i;
                y = j;
                break;
            }
        }
        if (x != -1 && y != -1)
            break;
    }

    return {x, y};
}

Board::Board(std::string fen)
{
    if (fen == "")
    {
        fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
    }
    readFen(fen);
}

Board::Board(const Board &other)
{
    for (size_t i = 0; i < 64; i++)
    {
        arr[i] = other.arr[i];
    }
    castles = other.castles;
    enpass = other.enpass;

    on_move = other.on_move;
}

std::ostream &operator<<(std::ostream &os, Board &bd)
{
    os << "   |  ";
    for (size_t i = 0; i < 8; i++)
    {
        os << char('A' + i) << " ";
    }
    os << "\n-----------------------\n";
    for (size_t i = 0; i < 8; i++)
    {
        os << 8 - i << "  |  ";
        for (size_t j = 0; j < 8; j++)
        {
            char piece = bd.getField(j, i);
            os << (piece ? piece : ' ') << " ";
        }
        os << "\n";
    }

    os << "-----------------------\n";
    os << "move: " << (bd.on_move ? "white" : "black") << "\n";
    std::string tmp =
        ((bd.castles & 0b1000) > 0 ? std::string("K") : "") +
        ((bd.castles & 0b0100) > 0 ? std::string("Q") : "") +
        ((bd.castles & 0b0010) > 0 ? std::string("k") : "") +
        ((bd.castles & 0b0001) > 0 ? std::string("q") : "");

    os << "castles : " << tmp << "\n";
    os << "enpass : (" << bd.enpass.x << ", " << bd.enpass.y << ")\n";
    os << "score: " << bd.getScore() << "\n";
    return os;
}

void Board::readFen(std::string fen)
{
    for (int i = 0; i < 64; i++)
    {
        arr[i] = '\0';
    }
    castles = 0;
    enpass = {-1, -1};

    int pos = 0;

    std::vector<std::string> fenParts = splitFen(fen);
    if (fenParts.size() < 4)
    {
        throw std::runtime_error("Invalid fen!");
    }

    std::string fen_mv = fenParts[1];
    std::string fen_cs = fenParts[2];
    std::string fen_bd = fenParts[0];
    std::string fen_en = fenParts[3];

    on_move = fen_mv[0] == 'w';

    for (const char &piece : fen_bd)
    {
        if (isdigit(piece))
            pos += atoi(&piece);
        else
        {
            if (piece == '/')
                continue;
            arr[pos] = piece;
            pos++;
        }
    }

    if (count(fen_cs.begin(), fen_cs.end(), 'K') == 1)
        castles = castles | 0b1000;
    if (count(fen_cs.begin(), fen_cs.end(), 'Q') == 1)
        castles = castles | 0b0100;
    if (count(fen_cs.begin(), fen_cs.end(), 'k') == 1)
        castles = castles | 0b0010;
    if (count(fen_cs.begin(), fen_cs.end(), 'q') == 1)
        castles = castles | 0b0001;

    if (fen_en.length() == 2)
    {
        int x = (int)fen_en[0] - (int)'a';
        int y = atoi(&fen_en[1]);
        enpass = {x, y};
    }
    return;
}

bool Board::onMove()
{
    return on_move;
}

std::vector<Move> Board::getMoves(Coords from)
{
    auto [x, y] = from;
    char piece = getField(x, y);

    if (piece == '\0' || getColor(x, y) != on_move)
        return {};

    piece = tolower(piece);
    switch (piece)
    {
    case 'p':
        return pMoves(from);
    case 'n':
        return nMoves(from);
    case 'b':
        return bMoves(from);
    case 'r':
        return rMoves(from);
    case 'q':
        return qMoves(from);
    case 'k':
        return kMoves(from);
    default:
        return {};
    }
}

std::vector<Move> Board::allMoves()
{
    std::vector<Move> moves;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (getColor(i, j) != on_move)
                continue;
            std::vector<Move> new_moves = getMoves({i, j});
            moves.insert(moves.end(), new_moves.begin(), new_moves.end());
        }
    }
    return moves;
}

std::vector<Smove> Board::specialMoves()
{
    std::vector<std::pair<Move, Move>> moves;

    // check if pawns can transform
    int dir = on_move ? -1 : 1;
    int end = on_move ? 1 : 6;
    char pawn_pattern = on_move ? 'P' : 'p';
    for (int x = 0; x < 8; x++)
    {
        if (getField(x, end) == pawn_pattern)
        {
            for (int i = 0; i < 4; i++)
            {
                moves.push_back({{{x, end}, {x, end + dir}}, {{-1, i}, {x, end + dir}}});
            }
        }
    }

    // check for enpassant
    auto [x, y] = enpass;
    int pass_line = on_move ? 3 : 4;
    if (x >= 0 && y >= 0)
    {
        if (getField(x - 1, pass_line))
            moves.push_back({{{x - 1, pass_line}, enpass}, {{x, pass_line}, {-1, -1}}});
        if (getField(x + 1, pass_line))
            moves.push_back({{{x + 1, pass_line}, enpass}, {{x, pass_line}, {-1, -1}}});
    }

    // check for castling
    int king_line = on_move ? 7 : 0;

    if (castles & (0b10 << (on_move ? 2 : 0)))
    {
        if (getField(5, king_line) == '\0' && getField(6, king_line) == '\0' &&
            (!isCheck({4, king_line}) && !isCheck({5, king_line}) && !isCheck({6, king_line})))
        {

            moves.push_back({{{4, king_line}, {6, king_line}}, {{7, king_line}, {5, king_line}}});
        }
    }

    if (castles & (0b01 << (on_move ? 2 : 0)))
    {
        if (getField(1, king_line) == '\0' && getField(2, king_line) == '\0' && getField(3, king_line) == '\0' &&
            (!isCheck({1, king_line}) && !isCheck({2, king_line}) && !isCheck({3, king_line}) && !isCheck({4, king_line})))
        {
            moves.push_back({{{4, king_line}, {2, king_line}}, {{0, king_line}, {3, king_line}}});
        }
    }

    return moves;
}

bool Board::isCheck(Coords from)
{
    if (from.x == -1)
    {
        from = getKingOnMove();
    }
    return bChecking(from) || rChecking(from) || nChecking(from) || pChecking(from) || qChecking(from);
}

bool Board::isMate()
{
    if (!isCheck())
        return false;
    std::vector<Move> moves = allMoves();
    bool found = false;
    for (const auto &move : moves)
    {
        found = movePiece(move);
        if (found)
        {
            assert(undoMove());
            return false;
        }
    }
    return true;
}

bool Board::isStaleMate()
{
    if (isCheck())
        return false;

    std::vector<Move> moves = allMoves();
    bool found = false;
    for (const auto &move : moves)
    {
        found = movePiece(move);
        if (found)
        {
            assert(undoMove());
            return false;
        }
    }
    return true;
}

bool Board::movePiece(const Move &move)
{
    auto [from, to] = move;
    UndoNode undo;
    undo.from = from;
    undo.from_field = getField(from.x, from.y);
    undo.to = to;
    undo.to_field = getField(to.x, to.y);
    undo.castles = castles;
    undo.enpass = enpass;

    auto [x1, y1] = from;
    char piece = getField(x1, y1);
    setField(x1, y1, '\0');
    auto [x2, y2] = to;
    setField(x2, y2, piece);

    if (tolower(piece) == 'k')
    {
        if (on_move)
            castles = castles & 0b0011;
        else
            castles = castles & 0b1100;
    }
    if (tolower(piece) == 'r')
    {
        if (x1 == 7){
            if (on_move)
                castles = castles & 0b0111;
            else
                castles = castles & 0b1101;
        }
        else if (x1 == 0){
            if (on_move)
                castles = castles & 0b1011;
            else
                castles = castles & 0b1110;
        }
    }
    if (tolower(piece) == 'p' && abs(y2 - y1) == 2)
    {
        enpass = {x1, (y1 + y2) / 2};
    }

    if (isCheck())
    {
        castles = undo.castles;
        enpass = undo.enpass;
        setField(undo.to.x, undo.to.y, undo.to_field);
        setField(undo.from.x, undo.from.y, undo.from_field);
        return false;
    }

    undo_stack.push_back(undo);
    on_move = !on_move;
    return true;
}

bool Board::smovePiece(const Smove &smove)
{
    auto [move1, move2] = smove;

    if (!movePiece(move1))
        return false;
    on_move = !on_move;
    if (!movePiece(move2))
        return false;
    return true;
}

bool Board::undoMove()
{
    if (undo_stack.size() == 0)
        return false;
    UndoNode undo = undo_stack.back();
    undo_stack.pop_back();
    castles = undo.castles;
    enpass = undo.enpass;
    setField(undo.to.x, undo.to.y, undo.to_field);
    setField(undo.from.x, undo.from.y, undo.from_field);
    on_move = !on_move;
    return true;
}

bool Board::undoSmove()
{
    if (undo_stack.size() < 2)
        return false;
    undoMove();
    on_move = !on_move;
    undoMove();
    return true;
}

int Board::getScore()
{
    if (isMate())
    {
        return on_move ? -1000 : 1000;
    }
    else if (isStaleMate())
    {
        return 0;
    }
    int white = 0;
    int black = 0;
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            char piece = getField(i, j);
            if (VALUES.find(tolower(piece)) == VALUES.end())
                continue;
            if (islower(piece))
                black += VALUES.at(tolower(piece));
            else
                white += VALUES.at(tolower(piece));
        }
    }
    return white - black;
}