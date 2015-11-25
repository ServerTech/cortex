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
#define TEST_FEN_8 "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 1"
#define TEST_FEN_9 "rnb1k1nr/pppp1ppp/8/1q2p3/4P1b1/5P2/PPPPQ1PP/RNB1KBNR w - - 0 1"
#define TEST_FEN_10 "r1B1k2r/p3p1P1/5n1p/2n5/1p6/7b/Q4PPP/RNB1KBNR w - - 0 1"

int main()
{
    init_hash();

    Board board;

    if(!parse_fen(board, TEST_FEN_10)) std::cout << "Parse error." << std::endl;
    else std::cout << pretty_board(board) << std::endl << std::endl;

    std::string usr_cmd;
    unsigned int depth, move;

    while(1)
    {
        std::cout << "What can I do for you? ";
        std::cin >> usr_cmd;
        std::cout << std::endl;

        if(usr_cmd == "quit") break;
        else if(usr_cmd == "undo")
        {
            undo_move(board);
            std::cout << pretty_board(board) << std::endl << std::endl;
        }
        else if(usr_cmd == "perft")
        {
            std::cin >> depth;

            MoveList ml = gen_moves(board);

            std::clock_t begin = std::clock();

            perform_perft_verbose(board, depth);

            std::clock_t end = std::clock();

            std::cout << "It took: " << double(end - begin) / CLOCKS_PER_SEC << " cs." << std::endl << std::endl;
        }
        else if(usr_cmd == "clear")
        {
            std::cout << "\x1B[2J\x1B[H";
            std::cout << pretty_board(board) << std::endl << std::endl;
        }
        else if((move = parse_move(board, usr_cmd)))
        {
            make_move(board, move);
            std::cout << pretty_board(board) << std::endl << std::endl;
        }
        else
        {
            std::cout << "ERROR: What'd you mess up, huh? Be glad I'm not sentient, yet." << std::endl << std::endl;
        }
    }
}

