#include <iostream>

#include "defs.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "hash.h"

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FEN_1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
#define FEN_2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN_3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"
#define PAWN_TEST_WHITE "4kq2/2P1P3/8/3pPb1r/p2n2P1/2P4p/P1P2P1P/4K3 w - d6 0 1"
#define PAWN_TEST_BLACK "4Kq2/2p1p3/8/3PpB1R/P2N2p1/2p4P/p1p4p/4k3 b - d6 0 1"

int main()
{
    init_hash();

    Board board;

    if(!parse_fen(board, PAWN_TEST_WHITE)) std::cout << "Parse error." << std::endl;
    else std::cout << pretty_board(board) << std::endl;

    std::cout << std::endl;

    MoveList ml = gen_moves(board);
    std::cout << pretty_move_list(ml.list) << std::endl;
}