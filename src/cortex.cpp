#include <iostream>
#include <ctime>

#include "defs.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "hash.h"
#include "perft.h"

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TEST_FEN_1 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
#define TEST_FEN_2 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
#define TEST_FEN_3 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 1"
#define TEST_FEN_4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define TEST_FEN_5 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"
#define TEST_FEN_6 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 0 1"
#define TEST_FEN_7 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"

int main()
{
    init_hash();

    Board board;

    if(!parse_fen(board, TEST_FEN_1)) std::cout << "Parse error." << std::endl;
    else std::cout << pretty_board(board) << std::endl << std::endl;

    std::clock_t begin = std::clock();

    perform_perft_verbose(board, 6);

    std::clock_t end = std::clock();

    std::cout << "It took: " << double(end - begin) / CLOCKS_PER_SEC << " cs." << std::endl;
}
