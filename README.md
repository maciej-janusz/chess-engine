# Chess Engine ♟️

A simple command-line chess engine written in C++17 from scratch. Give it a position in FEN
notation and a search depth, and it prints the best variant it can find.

## Features

- FEN parsing with validation (bad input is reported, not crashed on)
- Full legal move generation: castling, en passant, promotion (also by capture), check detection
- Checkmate and stalemate detection
- Make/undo move stack
- Negamax search with alpha-beta pruning
- Material-only evaluation (P=1, N=3, B=3, R=5, Q=9)
- Test suite: [perft](https://www.chessprogramming.org/Perft) on well-known positions plus regression tests

## Requirements

- A C++17 compiler (the makefile uses `g++`; override with `make CXX=clang++`)
- `make`

## Build

```bash
git clone https://github.com/maciej-janusz/chess-engine.git
cd chess-engine
make
```

| Command      | What it does                                         |
|--------------|------------------------------------------------------|
| `make`       | builds `./chess_engine`                              |
| `make run`   | builds and runs it                                   |
| `make test`  | builds and runs the test suite (`./chess_tests`)     |
| `make debug` | clean rebuild with `-g -DDEBUG`                      |
| `make clean` | removes build artifacts                              |

## Usage

```bash
./chess_engine [noprint] [noclock]
```

The program asks for the position and the depth on stdin:

```
$ ./chess_engine noprint
Enter FEN: 6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1
Enter depth: 3
best variant: 1000
first move: A1 -> A8
time : 0.000891 s
```

Depth is counted in plies (half-moves). Invalid FEN or depth prints an error and exits with status 1.

### Flags

| Flag      | Effect                                                                 |
|-----------|------------------------------------------------------------------------|
| `noprint` | print only the first move instead of the whole variant with its boards |
| `noclock` | do not print the elapsed time                                          |

Flags are plain words (no `--`) and can be combined.

### Reading the output

- `best variant: N` – evaluation from **White's** point of view, in pawns.
  `1000` means White mates, `-1000` means Black mates, `0` is a draw by stalemate or an equal position.
- Without `noprint`, the variant is printed move by move with the board after each move.
  `score` under a board is the material balance from White's point of view.
- Move notation:

  | Output               | Meaning                                  |
  |----------------------|------------------------------------------|
  | `E2 -> E4`           | normal move or capture                   |
  | `B7 -> A8 =queen`    | promotion (`rook`, `knight`, `bishop`, `queen`) |
  | `E5 -> D6 enpass`    | en passant capture                       |
  | `ks castle`/`qs castle` | king-side / queen-side castling       |

  Squares are written as file letter + rank, e.g. `E2`.

### Example: mate in 3

```
Enter FEN: 2bB4/N2k1N2/3P1P2/4p3/1R3p1P/Q6p/6p1/K3R3 w - - 0 1
Enter depth: 5
best variant: 1000
...
1: A3 -> A4
2: D7 -> E6
3: A4 -> E8
4: E6 -> F5
5: F7 -> H6      (checkmate)
```

## Tests

```bash
make test
```

The suite (`tests/tests.cpp`, no external dependencies) contains:

- **Perft** – node counts of the move generator on the standard test positions
  (start position, "Kiwipete", and positions 3–6 from the
  [Chess Programming Wiki](https://www.chessprogramming.org/Perft_Results)), including a check that
  make/undo restores the board exactly.
- **Regression tests** for pawns and promotion, en passant, castling rights, king safety,
  mate/stalemate detection, FEN validation and the engine front end.

## Project layout

| File            | Contents                                                        |
|-----------------|-----------------------------------------------------------------|
| `board.h/.cpp`  | board representation, FEN, move generation, make/undo, evaluation |
| `engine.h/.cpp` | alpha-beta search and result printing                           |
| `main.cpp`      | command-line interface                                          |
| `tests/`        | test suite                                                      |

### Internals worth knowing

- Board coordinates are `(x, y)` with `x = 0` for file A and `y = 0` for rank **8**, `y = 7` for rank 1.
- A move is either a plain `Nmove` (from → to) or a special `Smove`, a pair of plain moves applied
  together: promotion (pawn move + "virtual" piece placed at `x = -1`), en passant (capture + removal
  of the captured pawn) and castling (king + rook). Legality is checked after the whole `Smove`.

## Limitations

- Evaluation counts material only – no piece-square tables, king safety or pawn structure.
- Mate scores do not depend on the distance to mate, so the engine does not prefer the shortest mate.
- Moves after the first one in the printed variant come from a pruned search and may not be an exact
  principal variation.
- No move ordering, transposition table or time management; deep searches get slow quickly.
- The fifty-move rule, threefold repetition and insufficient material are not detected.
- No UCI interface – the program is a one-shot analysis tool.
