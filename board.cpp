#include "board.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

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
 * @return value of field, '\0' - empty field, ' ' - out of the board
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

std::string Board::descField(Coords coords){
    auto [x, y] = coords;
    return std::string("") + ((char)('A'+ x)) + std::to_string(8-y);
}

std::string Board::descSmove(const Smove *smv)
{
    auto [first, second] = *smv;
    if (second.from.x == -1)
    {
        const char *names[] = {"rook", "knight", "bishop", "queen"};
        if (second.from.y < 0 || second.from.y > 3)
            return "";
        return descField(first.from) + " -> " + descField(first.to) + " =" + names[second.from.y];
    }
    if (second.to.x == -1)
    {
        return descField(first.from) + " -> " + descField(first.to) + " enpass";
    }
    if (second.from.x == 7)
    {
        return "ks castle";
    }
    if (second.from.x == 0)
    {
        return "qs castle";
    }
    return "";
}

std::string Board::descNmove(const Nmove *mv){
    return descField(mv->from) + " -> " + descField(mv->to);
}

std::string Board::descMove(const Move &move)
{
    if (std::holds_alternative<Nmove>(move))
    {
        Nmove mv = std::get<Nmove>(move);
        return descNmove(&mv);
    }
    else
    {
        Smove smv = std::get<Smove>(move);
        return descSmove(&smv);
    }
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

std::vector<Nmove> Board::pMoves(Coords from)
{
    std::vector<Nmove> moves;
    auto [x, y] = from;
    int dir = on_move ? -1 : 1;
    int start = on_move ? 6 : 1;
    int end = 7 - start;
    // every move of a pawn on the last-but-one rank is a promotion (see specialMoves)
    if (y == end)
        return moves;
    if (isCollision(x, y + dir) == NO_COLL)
    {
        moves.push_back({from, {x, y + dir}});
        if (y == start && isCollision(x, y + 2 * dir) == NO_COLL)
            moves.push_back({from, {x, y + 2 * dir}});
    }
    if (isCollision(x + 1, y + dir) == OPP)
        moves.push_back({from, {x + 1, y + dir}});
    if (isCollision(x - 1, y + dir) == OPP)
        moves.push_back({from, {x - 1, y + dir}});

    return moves;
}
std::vector<Nmove> Board::nMoves(Coords from)
{
    std::vector<Nmove> moves;
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
std::vector<Nmove> Board::bMoves(Coords from)
{
    std::vector<Nmove> moves;
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
std::vector<Nmove> Board::rMoves(Coords from)
{
    std::vector<Nmove> moves;
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
std::vector<Nmove> Board::qMoves(Coords from)
{
    std::vector<Nmove> moves = bMoves(from);
    std::vector<Nmove> new_moves = rMoves(from);
    moves.insert(moves.end(), new_moves.begin(), new_moves.end());
    return moves;
}
std::vector<Nmove> Board::kMoves(Coords from)
{
    std::vector<Nmove> moves;
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

bool Board::kChecking(Coords from)
{
    auto [x, y] = from;
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            if ((dx != 0 || dy != 0) && isCollision(x + dx, y + dy) == OPP && tolower(getField(x + dx, y + dy)) == 'k')
                return true;
        }
    }
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
    std::string castles_val =
        ((bd.castles & 0b1000) > 0 ? std::string("K") : "") +
        ((bd.castles & 0b0100) > 0 ? std::string("Q") : "") +
        ((bd.castles & 0b0010) > 0 ? std::string("k") : "") +
        ((bd.castles & 0b0001) > 0 ? std::string("q") : "");

    castles_val = castles_val != "" ? castles_val : "-";
    os << "castles: " << castles_val << "\n";
    std::string enpass_val = bd.enpass.x == -1 ? "-" : Board::descField(bd.enpass);
    os << "enpass: " << enpass_val << "\n";
    os << "score: " << bd.getScore() << "\n";
    return os;
}

void Board::readFen(std::string fen)
{
    std::vector<std::string> fenParts = splitFen(fen);
    if (fenParts.size() < 4)
    {
        throw std::runtime_error("Invalid FEN: expected at least 4 fields");
    }
    const std::string &fen_bd = fenParts[0];
    const std::string &fen_mv = fenParts[1];
    const std::string &fen_cs = fenParts[2];
    const std::string &fen_en = fenParts[3];

    // parse into locals first, so a bad FEN leaves the board untouched
    char new_arr[64] = {};
    int x = 0, y = 0, white_kings = 0, black_kings = 0;
    for (char c : fen_bd)
    {
        if (c == '/')
        {
            if (x != 8 || y >= 7)
                throw std::runtime_error("Invalid FEN: wrong rank length");
            x = 0;
            y++;
        }
        else if (c >= '1' && c <= '8')
        {
            x += c - '0';
            if (x > 8)
                throw std::runtime_error("Invalid FEN: rank has more than 8 squares");
        }
        else if (std::string("pnbrqkPNBRQK").find(c) != std::string::npos)
        {
            if (x > 7)
                throw std::runtime_error("Invalid FEN: rank has more than 8 squares");
            if (tolower(c) == 'p' && (y == 0 || y == 7))
                throw std::runtime_error("Invalid FEN: pawn on the first or last rank");
            if (c == 'K')
                white_kings++;
            if (c == 'k')
                black_kings++;
            new_arr[y * 8 + x] = c;
            x++;
        }
        else
        {
            throw std::runtime_error(std::string("Invalid FEN: unexpected character '") + c + "'");
        }
    }
    if (y != 7 || x != 8)
        throw std::runtime_error("Invalid FEN: board must have 8 ranks of 8 squares");
    if (white_kings != 1 || black_kings != 1)
        throw std::runtime_error("Invalid FEN: each side needs exactly one king");

    if (fen_mv != "w" && fen_mv != "b")
        throw std::runtime_error("Invalid FEN: side to move must be 'w' or 'b'");

    unsigned int new_castles = 0;
    if (fen_cs != "-")
    {
        for (char c : fen_cs)
        {
            if (c == 'K')
                new_castles |= 0b1000;
            else if (c == 'Q')
                new_castles |= 0b0100;
            else if (c == 'k')
                new_castles |= 0b0010;
            else if (c == 'q')
                new_castles |= 0b0001;
            else
                throw std::runtime_error("Invalid FEN: bad castling field");
        }
    }

    Coords new_enpass = {-1, -1};
    if (fen_en != "-")
    {
        if (fen_en.length() != 2 || fen_en[0] < 'a' || fen_en[0] > 'h' || (fen_en[1] != '3' && fen_en[1] != '6'))
            throw std::runtime_error("Invalid FEN: bad en passant field");
        new_enpass = {fen_en[0] - 'a', 8 - (fen_en[1] - '0')};
    }

    std::copy(new_arr, new_arr + 64, arr);
    on_move = fen_mv == "w";
    castles = new_castles;
    enpass = new_enpass;
    undo_stack.clear();
}

bool Board::onMove()
{
    return on_move;
}

std::vector<Nmove> Board::getMoves(Coords from)
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

std::vector<Nmove> Board::normalMoves()
{
    std::vector<Nmove> moves;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (getColor(i, j) != on_move)
                continue;
            std::vector<Nmove> new_moves = getMoves({i, j});
            moves.insert(moves.end(), new_moves.begin(), new_moves.end());
        }
    }
    return moves;
}

std::vector<Smove> Board::specialMoves()
{
    std::vector<std::pair<Nmove, Nmove>> moves;

    // promotions: push onto an empty square or capture an opponent's piece
    int dir = on_move ? -1 : 1;
    int end = on_move ? 1 : 6;
    char pawn_pattern = on_move ? 'P' : 'p';
    for (int x = 0; x < 8; x++)
    {
        if (getField(x, end) != pawn_pattern)
            continue;
        for (int dx = -1; dx <= 1; dx++)
        {
            Collision coll = isCollision(x + dx, end + dir);
            if (coll != (dx == 0 ? NO_COLL : OPP))
                continue;
            for (int i = 0; i < 4; i++)
            {
                moves.push_back({{{x, end}, {x + dx, end + dir}}, {{-1, i}, {x + dx, end + dir}}});
            }
        }
    }

    // en passant: the capturing pawn must be ours, the captured one the opponent's
    auto [x, y] = enpass;
    int target_line = on_move ? 2 : 5;
    int pass_line = on_move ? 3 : 4;
    char enemy_pawn = on_move ? 'p' : 'P';
    if (x >= 0 && y == target_line && getField(x, y) == '\0' && getField(x, pass_line) == enemy_pawn)
    {
        for (int i = -1; i <= 1; i += 2)
        {
            if (x + i < 0 || x + i > 7)
                continue;
            if (getField(x + i, pass_line) == pawn_pattern)
                moves.push_back({{{x + i, pass_line}, enpass}, {{x, pass_line}, {-1, -1}}});
        }
    }

    // castling: king and rook on their home squares, empty path, king not in/through check
    // (the destination square is verified after the move is made)
    int king_line = on_move ? 7 : 0;
    char king = on_move ? 'K' : 'k';
    char rook = on_move ? 'R' : 'r';
    if (getField(4, king_line) == king)
    {
        if ((castles & (0b10 << (on_move ? 2 : 0))) && getField(7, king_line) == rook &&
            getField(5, king_line) == '\0' && getField(6, king_line) == '\0' &&
            !isCheck({4, king_line}) && !isCheck({5, king_line}))
        {
            moves.push_back({{{4, king_line}, {6, king_line}}, {{7, king_line}, {5, king_line}}});
        }

        if ((castles & (0b01 << (on_move ? 2 : 0))) && getField(0, king_line) == rook &&
            getField(1, king_line) == '\0' && getField(2, king_line) == '\0' && getField(3, king_line) == '\0' &&
            !isCheck({4, king_line}) && !isCheck({3, king_line}))
        {
            moves.push_back({{{4, king_line}, {2, king_line}}, {{0, king_line}, {3, king_line}}});
        }
    }

    return moves;
}

std::vector<Move> Board::allMoves(){
    std::vector<Nmove> nmoves = Board::normalMoves();
    std::vector<Smove> smoves = Board::specialMoves();

    std::vector<Move> moves;
    moves.reserve(1 + nmoves.size() + smoves.size());
    moves.insert(moves.end(), nmoves.begin(), nmoves.end());
    moves.insert(moves.end(), smoves.begin(), smoves.end());

    return moves;
}

bool Board::isCheck(Coords from)
{
    if (from.x == -1)
    {
        from = getKingOnMove();
    }
    return bChecking(from) || rChecking(from) || nChecking(from) || pChecking(from) || qChecking(from) || kChecking(from);
}

bool Board::hasLegalMove()
{
    for (const auto &move : allMoves())
    {
        if (movePiece(move))
        {
            undoMove();
            return true;
        }
    }
    return false;
}

bool Board::isMate()
{
    return isCheck() && !hasLegalMove();
}

bool Board::isStaleMate()
{
    return !isCheck() && !hasLegalMove();
}

// castling right lost when a piece moves from or to this square
static unsigned int cornerMask(Coords c)
{
    if (c.x == 7 && c.y == 7) return 0b1000;
    if (c.x == 0 && c.y == 7) return 0b0100;
    if (c.x == 7 && c.y == 0) return 0b0010;
    if (c.x == 0 && c.y == 0) return 0b0001;
    return 0;
}

UndoNmove Board::applyNmove(const Nmove &move)
{
    const auto &[from, to] = move;
    UndoNmove undo = {from, getField(from.x, from.y), to, getField(to.x, to.y), castles, enpass};

    char piece = undo.from_field;
    setField(from.x, from.y, '\0');
    setField(to.x, to.y, piece);

    if (tolower(piece) == 'k')
        castles &= on_move ? 0b0011 : 0b1100;
    // rook moved away from, or was captured on, its home square
    castles &= ~(cornerMask(from) | cornerMask(to));

    if (tolower(piece) == 'p' && to.y >= 0 && abs(to.y - from.y) == 2)
        enpass = {from.x, (from.y + to.y) / 2};
    else
        enpass = {-1, -1};

    return undo;
}

void Board::restore(const UndoNmove &undo)
{
    castles = undo.castles;
    enpass = undo.enpass;
    setField(undo.to.x, undo.to.y, undo.to_field);
    setField(undo.from.x, undo.from.y, undo.from_field);
}

bool Board::nmovePiece(const Nmove *move)
{
    UndoNmove undo = applyNmove(*move);
    if (isCheck())
    {
        restore(undo);
        return false;
    }
    undo_stack.push_back(undo);
    on_move = !on_move;
    return true;
}

bool Board::smovePiece(const Smove *smove)
{
    // both halves are applied before the legality check, so e.g. en passant
    // capturing the checking pawn, or a promotion blocking a check, is legal
    UndoNmove first = applyNmove(smove->first);
    UndoNmove second = applyNmove(smove->second);
    if (isCheck())
    {
        restore(second);
        restore(first);
        return false;
    }
    undo_stack.push_back(UndoSmove{first, second});
    on_move = !on_move;
    return true;
}

bool Board::movePiece(const Move &move)
{
    if (auto *nm = std::get_if<Nmove>(&move))
    {
        return nmovePiece(nm);
    }
    else if (auto *sm = std::get_if<Smove>(&move))
    {
        return smovePiece(sm);
    }
    return false;
}

bool Board::undoMove()
{
    if (undo_stack.empty())
        return false;

    if (auto *nmn = std::get_if<UndoNmove>(&undo_stack.back()))
    {
        restore(*nmn);
    }
    else if (auto *smn = std::get_if<UndoSmove>(&undo_stack.back()))
    {
        restore(smn->second);
        restore(smn->first);
    }
    undo_stack.pop_back();
    on_move = !on_move;
    return true;
}

static int pieceValue(char piece)
{
    switch (tolower(piece))
    {
    case 'p': return 1;
    case 'n': return 3;
    case 'b': return 3;
    case 'r': return 5;
    case 'q': return 9;
    default: return 0;
    }
}

int Board::getScore()
{
    if (!hasLegalMove())
    {
        if (isCheck())
            return on_move ? -1000 : 1000;
        return 0;
    }
    int score = 0;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            char piece = getField(i, j);
            if (piece == '\0')
                continue;
            score += isupper(piece) ? pieceValue(piece) : -pieceValue(piece);
        }
    }
    return score;
}

int Board::eval()
{
    int eval = getScore();
    if (!on_move) return -eval;
    return eval;
}
