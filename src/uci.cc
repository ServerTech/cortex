/*
    Cortex - Self-learning Chess Engine
    @filename uci.cc
    @author Shreyas Vinod
    @version 1.0.0

    @brief Includes everything needed to support the UCI
           (Universal Chess Interface) protocol.

    ******************** VERSION CONTROL ********************
    * 02/12/2015 File created.
    * 02/12/2015 0.1.0 Initial version.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename uci.cc
    @author Shreyas Vinod

    @brief Includes everything needed to support the UCI
           (Universal Chess Interface) protocol.
*/

#include "defs.h"

#include <iostream>
#include <string> // std::string
#include <sstream> // std::stringstream

#include "uci.h"
#include "board.h"
#include "search.h"
#include "hash_table.h"
#include "chronos.h"

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// Prototypes

void uci_loop();
bool parse_uci_position(const std::string& cmd, Board& board);
void parse_uci_go(const std::string& cmd, SearchInfo& search_info,
    Board& board);

// Function definitions

/**
    @brief UCI infinite loop to listen for commands from UCI protocol
           enabled graphical interfaces.

    @return void.
*/

void uci_loop()
{
    std::string cmd; // Stores any commands from the GUI.

    std::cout << "id name Cortex" << std::endl;
    std::cout << "id author Shreyas Vinod, Anna Grygierzec" << std::endl;
    std::cout << "uciok" << std::endl;

    Board board;
    init_table(board.t_table, 268435456); // Initialise hash table to 256 MB.

    SearchInfo search_info;

    while(1)
    {
        std::cin.clear();
        std::cin.sync();
        std::getline(std::cin, cmd);

        if(cmd == "\n") continue;

        if(cmd.compare(0, 2, "go") == 0)
        {
            parse_uci_go(cmd, search_info, board);
        }
        else if(cmd.compare(0, 8, "position") == 0)
        {
            if(!parse_uci_position(cmd, board)) return; // Fatal error.
        }
        else if(cmd == "isready")
        {
            std::cout << "readyok" << std::endl;
            continue;
        }
        else if(cmd == "quit") break;

        if(search_info.quit) break;
    }

    free_table(board.t_table);
}

/**
    @brief Parses the UCI 'position' command and sets up the board as
           instructed by the GUI.

    @param cmd is the string that was received from the GUI.
    @param board is the board to set up with the command.

    @return bool denoting whether parsing was successful.

    @warning Will return on error.
*/

bool parse_uci_position(const std::string& cmd, Board& board)
{
    unsigned int i = 9;

    if(cmd.compare(i, 8, "startpos") == 0) // Set start position.
    {
        i += 9;
        unsigned int j = 0;
        parse_fen(board, FEN_START, j);
    }
    else if(cmd.compare(i, 3, "fen") == 0) // Initialise with FEN string.
    {
        i += 4;
        parse_fen(board, cmd, i);
    }
    else return 0; // Parse error.

    if(cmd.find("moves") != std::string::npos) // Check for move list.
    {
        i = cmd.find("moves") + 6;

        unsigned int move;
        std::string move_buf;
        assert(cmd[i]);
        std::stringstream moves(cmd.substr(i));

        while(moves >> move_buf)
        {
            move = parse_move(board, move_buf);

            if(move == NO_MOVE) return 0; // Parse error.
            if(!make_move(board, move)) return 0; // Parse error.
        }
    }

    std::cout << std::endl << pretty_board(board) << std::endl << std::endl;

    return 1;
}

/**
    @brief Parses the UCI 'go' command and starts a search.

    @param cmd is the string that was received from the GUI.
    @param search_info is the search information structure.
    @param board is the board to perform the search on.

    @return void.

    @warning Will mess up if incorrect (or not enough) commands are
             given to the engine.
    @warning Calling the function with no commands will result in an
             infinite search to 'MAX_DEPTH' (64).
*/

void parse_uci_go(const std::string& cmd, SearchInfo& search_info,
    Board& board)
{
    int depth = -1, moves_to_go = 30;
    int time_val = -1, move_time = -1, inc = 0;

    search_info.time_set = 0;
    search_info.stopped = 0;
    search_info.quit = 0;

    // Find every option in the string.

    if(cmd.find("movetime") != std::string::npos)
    {
        move_time = std::stoi(cmd.substr(cmd.find("movetime") + 9));
    }

    if(cmd.find("depth") != std::string::npos)
    {
        depth = std::stoi(cmd.substr(cmd.find("depth") + 6));
    }

    if(cmd.find("movestogo") != std::string::npos)
    {
        moves_to_go = std::stoi(cmd.substr(cmd.find("movestogo") + 10));
    }

    if((cmd.find("wtime") != std::string::npos) && board.side == WHITE)
    {
        time_val = std::stoi(cmd.substr(cmd.find("wtime") + 6));
    }

    if((cmd.find("btime") != std::string::npos) && board.side == BLACK)
    {
        time_val = std::stoi(cmd.substr(cmd.find("btime") + 6));
    }

    if((cmd.find("winc") != std::string::npos) && board.side == WHITE)
    {
        inc = std::stoi(cmd.substr(cmd.find("winc") + 5));
    }

    if((cmd.find("binc") != std::string::npos) && board.side == BLACK)
    {
        inc = std::stoi(cmd.substr(cmd.find("binc") + 5));
    }

    // Set up the search information structure.

    if(move_time != -1)
    {
        time_val = move_time;
        moves_to_go = 1;
    }

    if(depth == -1) search_info.depth = MAX_DEPTH;
    else search_info.depth = depth;

    if(time_val != -1)
    {
        search_info.time_set = 1;
        time_val /= moves_to_go;
        time_val -= 50;
        search_info.move_time = time_val + inc;
    }

    std::cout << "move_time " << search_info.move_time << " depth " <<
        search_info.depth << " time_set ";

    if(search_info.time_set) std::cout << "true" << std::endl;
    else std::cout << "false" << std::endl;

    search_info.start_time = get_cur_time();

    search(board, search_info); // Search!

    std::cout << std::endl;
}