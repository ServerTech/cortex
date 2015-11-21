#include <iostream>
#include <ctime>

#include "defs.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "hash.h"

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define PAWN_TEST_WHITE "4kq2/2P1P3/8/3pPb1r/p2n2P1/2P4p/P1P2P1P/4K3 w - d6 0 1"
#define PAWN_TEST_BLACK "4Kq2/2p1p3/8/3PpB1R/P2N2p1/2p4P/p1p4p/4k3 b - d6 0 1"
#define BISHOP_TEST "2B5/5B2/6r1/8/2n5/3B4/4P3/8 w - - 0 1"
#define ROOK_TEST "8/3b4/5R2/8/3R1n2/8/3P4/8 w - - 0 1"
#define TEST_FEN_1 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" // 48 nodes
#define TEST_FEN_2 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" // 14 nodes (+2 illegal)

int main()
{
    init_hash();

    Board board;

    if(!parse_fen(board, TEST_FEN_1)) std::cout << "Parse error." << std::endl;
    else std::cout << pretty_board(board) << std::endl;

    std::cout << std::endl;

    MoveList ml = gen_moves(board);
    std::cout << pretty_move_list(ml.list) << std::endl << std::endl;

    std::cout << "Running that a million times:" << std::endl;

    std::clock_t begin = std::clock();

    for(unsigned int i = 0; i < 1000000; i++)
        gen_moves(board);

    std::clock_t end = std::clock();

    std::cout << "It took: " << double(end - begin) / CLOCKS_PER_SEC <<
        " cs." << std::endl;
}