/*
    Cortex - Self-learning Chess Engine
    @filename main.cpp
    @author Shreyas Vinod, Anna Grygierzec
    @version 0.1.0

    @brief Has the main function.

    ******************** VERSION CONTROL ********************
    * xx/06/2015 File created.
    * 02/12/2015 0.1.0 Added this bit.
*/

#include <iostream>
#include <string>

#include "defs.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "search.h"
#include "hash.h"
#include "hash_table.h"
#include "chronos.h"
#include "uci.h"
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
#define WAC1 "2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PPB4P/R4RK1 w - -"
#define WAC2 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"

int main()
{
    std::cin.setf(std::ios_base::unitbuf); // Set stdin as unbuffered.
    std::cout.setf(std::ios_base::unitbuf); // Set stdout as unbuffered.

    std::string usr_cmd;

    init_hash();
    init_mvv_lva();

    std::cout << "Hi, I'm Cortex." << std::endl;
    std::cout << "What mode would you like to enter? ";
    std::cin >> usr_cmd;
    std::cin.ignore();

    if(usr_cmd == "uci")
    {
        uci_loop(); // Enter UCI loop.
        return 0;
    }
    else if(usr_cmd != "cmd")
    {
        std::cout << "Huh? Defaulting to cmd now.";
    }

    std::cout << std::endl;

    Board board;
    init_table(board.t_table, 268435456); // Initialise hash table to 256 MB.

    if(!parse_fen(board, FEN_START)) std::cout << "Parse error." << std::endl;
    else std::cout << pretty_board(board) << std::endl << std::endl;

    unsigned int argument, move;

    while(1)
    {
        std::cout << "What can I do for you? ";
        std::cin >> usr_cmd;
        std::cout << std::endl;

        if(usr_cmd == "quit") break;
        else if(usr_cmd == "undo")
        {
            if(board.history.size() > 0)
            {
                undo_move(board);
                board.ply = 0;
                std::cout << pretty_board(board) << std::endl << std::endl;
            }
            else std::cout << "ERROR: No move to undo." << std::endl <<
                std::endl;
        }
        else if(usr_cmd == "searchd")
        {
            std::cin >> argument;

            SearchInfo search_info;
            search_info.depth = argument;

            search(board, search_info);
            std::cout << std::endl;
        }
        else if(usr_cmd == "searcht")
        {
            std::cin >> argument;

            SearchInfo search_info;
            search_info.time_set = 1;
            search_info.infinite = 1;
            search_info.max_time = argument * 1000;

            search_info.start_time = get_cur_time();

            search(board, search_info);
            std::cout << std::endl;
        }
        else if(usr_cmd == "pseudo")
        {
            MoveList ml = gen_moves(board);
            std::cout << pretty_move_list(ml.list) << std::endl << std::endl;
        }
        else if(usr_cmd == "perft")
        {
            std::cin >> argument;

            Time begin = get_cur_time();

            perform_perft_verbose(board, argument);

            std::cout << "It took: " << get_time_diff(begin) <<
                " s." << std::endl << std::endl;
        }
        else if(usr_cmd == "cleartable")
        {
            clear_table(board.t_table);
            std::cout << "Cleared transposition table successfully." <<
                std::endl << std::endl;
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
            std::cout << "ERROR: What'd you mess up, huh? " <<
                "Be glad I'm not sentient, yet." << std::endl << std::endl;
        }
    }

    free_table(board.t_table);

    return 0;
}