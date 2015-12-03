/*
    Cortex - Self-learning Chess Engine
    @filename uci.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief Includes everything needed to support the UCI
           (Universal Chess Interface) protocol.

    ******************** VERSION CONTROL ********************
    * 02/12/2015 File created.
    * 02/12/2015 0.1.0 Initial version.
*/

#include "debug.h"

#include <iostream>

#include "uci.h"
#include "hash_table.h"

#define FEN_START "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

// Function Prototypes

void uci_loop();
void parse_uci_position(const std::string& cmd, Board& board);
void parse_uci_go(const std::string& cmd, SearchInfo& search_info,
    Board& board);

// Functions

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
        std::getline(std::cin, cmd);
        if(cmd == "\n") continue;

        if(cmd.compare(0, 2, "go"))
        {
            parse_uci_go(cmd, search_info, board);
        }
        else if(cmd.compare(0, 8, "position"))
        {
            parse_uci_position(cmd, board);
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

    @return void.
*/

void parse_uci_position(const std::string& cmd, Board& board)
{

}

/**
    @brief Parses the UCI 'go' command and starts a search.

    @param cmd is the string that was received from the GUI.
    @param search_info is the search information structure.
    @param board is the board to perform the search on.

    @return void.
*/

void parse_uci_go(const std::string& cmd, SearchInfo& search_info,
    Board& board)
{

}