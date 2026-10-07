// Minimal dependency-free test suite. Build and run with `make test`.
#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../board.h"
#include "../engine.h"

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                                 \
    do                                                                              \
    {                                                                               \
        checks++;                                                                   \
        if (!(cond))                                                                \
        {                                                                           \
            failures++;                                                             \
            std::cout << "    FAIL " << __FILE__ << ":" << __LINE__ << ": " #cond "\n"; \
        }                                                                           \
    } while (0)

#define CHECK_EQ(actual, expected)                                                  \
    do                                                                              \
    {                                                                               \
        checks++;                                                                   \
        auto a_ = (actual);                                                         \
        auto e_ = (expected);                                                       \
        if (!(a_ == e_))                                                            \
        {                                                                           \
            failures++;                                                             \
            std::cout << "    FAIL " << __FILE__ << ":" << __LINE__ << ": " #actual \
                      << " = " << a_ << ", expected " << e_ << "\n";                \
        }                                                                           \
    } while (0)

// ---------------------------------------------------------------- helpers

static unsigned long long perft(Board &bd, int depth)
{
    if (depth == 0)
        return 1;
    unsigned long long nodes = 0;
    for (const auto &move : bd.allMoves())
    {
        if (!bd.movePiece(move))
            continue;
        nodes += perft(bd, depth - 1);
        bd.undoMove();
    }
    return nodes;
}

static std::vector<std::string> legalMoves(Board &bd)
{
    std::vector<std::string> out;
    for (const auto &move : bd.allMoves())
    {
        if (!bd.movePiece(move))
            continue;
        out.push_back(Board::descMove(move));
        bd.undoMove();
    }
    std::sort(out.begin(), out.end());
    return out;
}

static size_t countMoves(const std::string &fen)
{
    Board bd(fen);
    return legalMoves(bd).size();
}

static bool hasMove(const std::string &fen, const std::string &desc)
{
    Board bd(fen);
    auto moves = legalMoves(bd);
    return std::find(moves.begin(), moves.end(), desc) != moves.end();
}

static bool play(Board &bd, const std::string &desc)
{
    for (const auto &move : bd.allMoves())
    {
        if (Board::descMove(move) == desc)
            return bd.movePiece(move);
    }
    return false;
}

static std::string show(Board bd)
{
    std::ostringstream os;
    os << bd;
    return os.str();
}

static bool throwsOnFen(const std::string &fen)
{
    try
    {
        Board bd(fen);
    }
    catch (const std::runtime_error &)
    {
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- perft

struct PerftCase
{
    const char *name;
    const char *fen;
    std::vector<unsigned long long> expected; // nodes for depth 1, 2, ...
};

static void testPerft()
{
    const std::vector<PerftCase> cases = {
        {"startpos", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20, 400, 8902, 197281}},
        {"kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48, 2039, 97862}},
        {"position 3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14, 191, 2812, 43238}},
        {"position 4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6, 264, 9467}},
        {"position 4 mirrored", "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", {6, 264, 9467}},
        {"position 5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44, 1486, 62379}},
        {"position 6", "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46, 2079, 89890}},
    };

    for (const auto &c : cases)
    {
        std::cout << "  perft " << c.name << "\n";
        Board bd(c.fen);
        std::string before = show(bd);
        for (size_t d = 0; d < c.expected.size(); d++)
        {
            CHECK_EQ(perft(bd, (int)d + 1), c.expected[d]);
        }
        // make/undo must restore the position exactly
        CHECK(show(bd) == before);
    }
}

// ---------------------------------------------------------------- regressions

static void testPawns()
{
    // a pawn must not jump over a piece on its way to the double push
    CHECK(!hasMove("4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1", "E2 -> E4"));
    CHECK(!hasMove("4k3/8/8/8/8/4n3/4P3/4K3 w - - 0 1", "E2 -> E3"));
    CHECK(hasMove("4k3/8/8/8/8/8/4P3/4K3 w - - 0 1", "E2 -> E4"));

    // promotion: only onto an empty square, never onto an own or enemy piece
    CHECK_EQ(countMoves("r6k/P7/8/8/8/8/8/7K w - - 0 1"), 3u);
    CHECK_EQ(countMoves("N6k/P7/8/8/8/8/8/7K w - - 0 1"), 5u);
    CHECK_EQ(countMoves("7k/P7/8/8/8/8/8/7K w - - 0 1"), 3u + 4u);

    // promotion by capture: 4 pieces x (push + capture)
    CHECK_EQ(countMoves("n6k/1P6/8/8/8/8/8/7K w - - 0 1"), 3u + 4u + 4u);
    CHECK(hasMove("n6k/1P6/8/8/8/8/8/7K w - - 0 1", "B7 -> A8 =queen"));
    CHECK(hasMove("n6k/1P6/8/8/8/8/8/7K w - - 0 1", "B7 -> B8 =knight"));

    // the promoted piece appears and undo brings the pawn (and the victim) back
    Board bd("n6k/1P6/8/8/8/8/8/7K w - - 0 1");
    std::string before = show(bd);
    CHECK(play(bd, "B7 -> A8 =queen"));
    {
        Board expected("Q6k/8/8/8/8/8/8/7K b - - 0 1");
        CHECK(show(bd) == show(expected));
    }
    CHECK(bd.undoMove());
    CHECK(show(bd) == before);

    // black promotes downwards
    CHECK_EQ(countMoves("7k/8/8/8/8/8/p7/4K3 b - - 0 1"), 3u + 4u);
}

static void testEnPassant()
{
    // no phantom queen from the virtual a-file square (white to move, a-pawn just double-pushed)
    CHECK_EQ(countMoves("4k3/8/8/p7/8/8/8/4K3 w - a6 0 1"), 5u);

    // en passant needs a pawn of the side to move, not any piece
    CHECK_EQ(countMoves("4k3/8/8/pN6/8/8/8/4K3 w - a6 0 1"), 5u + 6u); // king + knight moves only
    CHECK(!hasMove("4k3/8/8/pN6/8/8/8/4K3 w - a6 0 1", "B5 -> A6 enpass"));

    // en passant square from FEN is read with the correct rank
    {
        const std::string fen = "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 1";
        CHECK(hasMove(fen, "E5 -> D6 enpass"));
        Board bd(fen);
        CHECK(play(bd, "E5 -> D6 enpass"));
        Board expected("4k3/8/3P4/8/8/8/8/4K3 b - - 0 1");
        CHECK(show(bd) == show(expected));
        CHECK(bd.undoMove());
        CHECK(show(bd) == show(Board(fen)));
    }

    // black en passant
    CHECK(hasMove("4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1", "D4 -> E3 enpass"));

    // en passant created by a double push, then used
    {
        Board bd("4k3/3p4/8/4P3/8/8/8/4K3 b - - 0 1");
        CHECK(play(bd, "D7 -> D5"));
        CHECK(show(bd).find("enpass: D6") != std::string::npos);
        CHECK(play(bd, "E5 -> D6 enpass"));
        Board expected("4k3/8/3P4/8/8/8/8/4K3 b - - 0 1");
        CHECK(show(bd) == show(expected)); // black pawn is gone
    }

    // en passant may remove the pawn that gives check
    CHECK(hasMove("8/8/8/2k5/3Pp3/8/8/4K3 b - d3 0 1", "E4 -> D3 enpass"));
    CHECK_EQ(countMoves("8/8/8/2k5/3Pp3/8/8/4K3 b - d3 0 1"), 9u);

    // ... and must not be allowed if it leaves the king in check (pinned pawn)
    CHECK(!hasMove("8/8/8/KPp4r/8/8/8/7k w - c6 0 1", "B5 -> C6 enpass"));
}

static void testCastling()
{
    const std::string both = "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1";
    CHECK(hasMove(both, "ks castle"));
    CHECK(hasMove(both, "qs castle"));

    // castling requires the rook (and the king) on the home square
    CHECK(!hasMove("4k3/8/8/8/8/8/8/4K3 w K - 0 1", "ks castle"));
    CHECK(!hasMove("4k3/8/8/8/8/8/8/R2K3R w KQ - 0 1", "ks castle"));

    // queenside castling is legal when only b1 is attacked
    CHECK(hasMove("1r2k3/8/8/8/8/8/8/R3K3 w Q - 0 1", "qs castle"));
    // ... but not when the king passes an attacked square, starts in check or lands in check
    CHECK(!hasMove("3rk3/8/8/8/8/8/8/R3K3 w Q - 0 1", "qs castle")); // d1 attacked
    CHECK(!hasMove("4r1k1/8/8/8/8/8/8/R3K2R w KQ - 0 1", "ks castle")); // in check
    CHECK(!hasMove("5rk1/8/8/8/8/8/8/R3K2R w KQ - 0 1", "ks castle")); // f1 attacked
    CHECK(!hasMove("6rk/8/8/8/8/8/8/R3K2R w KQ - 0 1", "ks castle")); // g1 attacked
    // blocked path
    CHECK(!hasMove("r3k2r/8/8/8/8/8/8/RN2K2R w KQkq - 0 1", "qs castle"));

    // castling really moves both pieces, and undo restores them
    {
        Board bd(both);
        std::string before = show(bd);
        CHECK(play(bd, "ks castle"));
        Board expected("r3k2r/8/8/8/8/8/8/R4RK1 b kq - 0 1");
        CHECK(show(bd) == show(expected));
        CHECK(bd.undoMove());
        CHECK(show(bd) == before);
    }
    // black castles too
    CHECK(hasMove("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "ks castle"));
    CHECK(hasMove("r3k2r/8/8/8/8/8/8/4K3 b kq - 0 1", "qs castle"));

    // a captured rook takes the castling right with it
    {
        Board bd("4k3/8/8/8/8/8/6b1/4K2R b K - 0 1");
        CHECK(play(bd, "G2 -> H1"));
        CHECK(show(bd).find("castles: -") != std::string::npos);
        CHECK(!hasMove("4k3/8/8/8/8/8/8/4K2b w K - 0 1", "ks castle")); // no rook on h1
    }

    // moving a different rook off the h-file must not cost the kingside right
    {
        Board bd("4k3/8/8/7R/8/8/8/R3K2R w KQ - 0 1");
        CHECK(play(bd, "H5 -> H6"));
        CHECK(show(bd).find("castles: KQ") != std::string::npos);
    }
    // moving the home rook does
    {
        Board bd("4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        CHECK(play(bd, "H1 -> H2"));
        CHECK(show(bd).find("castles: Q") != std::string::npos);
    }
}

static void testMateAndStalemate()
{
    // back-rank mate
    {
        Board bd("R5k1/5ppp/8/8/8/8/8/4K3 b - - 0 1");
        CHECK(bd.isMate());
        CHECK(!bd.isStaleMate());
        CHECK_EQ(bd.getScore(), 1000);
        CHECK_EQ(bd.eval(), -1000);
    }
    // stalemate
    {
        Board bd("7k/5Q2/6K1/8/8/8/8/8 b - - 0 1");
        CHECK(!bd.isMate());
        CHECK(bd.isStaleMate());
        CHECK_EQ(bd.getScore(), 0);
    }
    // the only defence is a promotion that blocks the check: not mate
    {
        const std::string fen = "r6K/6P1/8/8/4b3/8/8/k7 w - - 0 1";
        Board bd(fen);
        CHECK(!bd.isMate());
        CHECK(!bd.isStaleMate());
        CHECK_EQ(countMoves(fen), 4u);
    }
    // the only legal moves are promotions: not stalemate
    {
        const std::string fen = "6r1/P7/8/8/2k5/8/1r6/7K w - - 0 1";
        Board bd(fen);
        CHECK(!bd.isCheck());
        CHECK(!bd.isStaleMate());
        CHECK_EQ(countMoves(fen), 4u);
    }
    // isMate / isStaleMate must leave the position untouched
    {
        Board bd("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
        std::string before = show(bd);
        bd.isMate();
        bd.isStaleMate();
        CHECK(show(bd) == before);
    }
}

static void testKings()
{
    // kings may not stand next to each other
    CHECK(!hasMove("8/8/8/3k4/8/3K4/8/8 w - - 0 1", "D3 -> D4"));
    CHECK_EQ(countMoves("8/8/8/8/8/8/k7/2K5 w - - 0 1"), 3u); // c2, d1, d2
}

static void testFen()
{
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq"));          // too few fields
    CHECK(throwsOnFen("   ")); // an empty string alone means "start position", whitespace does not
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP w KQkq - 0 1"));              // 7 ranks
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/8 w KQkq - 0 1"));   // 9 ranks
    CHECK(throwsOnFen("rnbqkbnrr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));    // 9 squares
    CHECK(throwsOnFen("9/8/8/8/8/8/8/8 w - - 0 1"));
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQXBNR w KQkq - 0 1"));     // bad piece
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR x KQkq - 0 1"));     // bad side
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkz - 0 1"));     // bad castling
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e9 0 1"));    // bad en passant
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e4 0 1"));    // bad en passant rank
    CHECK(throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQQBNR w KQkq - 0 1"));     // no white king
    CHECK(throwsOnFen("P3k3/8/8/8/8/8/8/4K3 w - - 0 1"));                               // pawn on last rank
    CHECK(!throwsOnFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));

    // default board is the start position
    Board bd;
    CHECK_EQ(legalMoves(bd).size(), 20u);
}

static void testEngine()
{
    // depth 0, mate and stalemate at the root must not crash
    for (const char *flags : {"", "noprint"})
    {
        std::vector<std::string> f;
        if (std::string(flags) != "")
            f.push_back(flags);
        for (const char *fen : {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                                "R5k1/5ppp/8/8/8/8/8/4K3 b - - 0 1",
                                "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"})
        {
            std::ostringstream out;
            std::streambuf *old = std::cout.rdbuf(out.rdbuf());
            Engine engine(fen, f);
            engine.findBestVariant(0);
            engine.findBestVariant(2);
            std::cout.rdbuf(old);
            CHECK(!out.str().empty());
        }
    }

    // negative depth is rejected instead of recursing forever
    {
        Engine engine("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        bool threw = false;
        try
        {
            engine.findBestVariant(-1);
        }
        catch (const std::invalid_argument &)
        {
            threw = true;
        }
        CHECK(threw);
    }

    // finds mate in one, for white and for black
    struct MateCase
    {
        const char *fen;
        const char *move;
        const char *score;
    };
    for (const MateCase &c : {MateCase{"6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1", "A1 -> A8", "1000"},
                              MateCase{"r3k3/8/8/8/8/8/5PPP/6K1 b - - 0 1", "A8 -> A1", "-1000"}})
    {
        std::ostringstream out;
        std::streambuf *old = std::cout.rdbuf(out.rdbuf());
        Engine engine(c.fen, {"noprint", "noclock"});
        engine.findBestVariant(2);
        std::cout.rdbuf(old);
        CHECK(out.str().find(std::string("first move: ") + c.move) != std::string::npos);
        CHECK(out.str().find(std::string("best variant: ") + c.score) != std::string::npos);
    }

    // wins a free queen
    {
        std::ostringstream out;
        std::streambuf *old = std::cout.rdbuf(out.rdbuf());
        Engine engine("4k3/8/8/3q4/8/8/8/3RK3 w - - 0 1", {"noprint", "noclock"});
        engine.findBestVariant(1);
        std::cout.rdbuf(old);
        CHECK(out.str().find("first move: D1 -> D5") != std::string::npos);
    }
}

// ---------------------------------------------------------------- runner

int main()
{
    struct Suite
    {
        const char *name;
        std::function<void()> fn;
    };
    const std::vector<Suite> suites = {
        {"pawns / promotion", testPawns},
        {"en passant", testEnPassant},
        {"castling", testCastling},
        {"mate and stalemate", testMateAndStalemate},
        {"kings", testKings},
        {"FEN parsing", testFen},
        {"engine", testEngine},
        {"perft", testPerft},
    };

    for (const auto &suite : suites)
    {
        int before = failures;
        std::cout << "[" << suite.name << "]\n";
        suite.fn();
        std::cout << (failures == before ? "  ok" : "  FAILED") << "\n";
    }

    std::cout << "\n" << checks - failures << "/" << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
