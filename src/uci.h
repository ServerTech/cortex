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

#ifndef UCI_H
#define UCI_H

#include "debug.h"

#include <string> // std::string

#include "board.h"
#include "search.h"
#include "defs.h"

// External function definitions

extern void uci_loop(); // UCI infinite loop.

// Parses the UCI 'position' command.

extern void parse_uci_position(const std::string& cmd, Board& board);

// Parses the UCI 'go' command.

extern void parse_uci_go(const std::string& cmd, SearchInfo& search_info,
    Board& board);

#endif // UCI_H