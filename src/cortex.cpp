#include <iostream>
#include <ctime>

#include "defs.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "hash.h"

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_FEN_1 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1" // 48 nodes
#define TEST_FEN_2 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1" // 14 nodes (+2 illegal)
#define TEST_FEN_3 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 1" // 44 nodes
#define ATK_TEST "4k3/8/8/8/8/8/8/4K3 w - - 0 1"

int main()
{
    init_hash();

    Board board;

    if(!parse_fen(board, TEST_FEN_3)) std::cout << "Parse error." << std::endl;
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