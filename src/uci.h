/*
    Cortex - Self-learning Chess Engine
    @filename uci.h
    @author Shreyas Vinod
    @version 1.0.0

    @brief Includes everything needed to support the UCI
           (Universal Chess Interface) protocol.

    ******************** VERSION CONTROL ********************
    * 02/12/2015 File created.
    * 05/12/2015 0.1.0 Initial version.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename uci.h
    @author Shreyas Vinod

    @brief Includes everything needed to support the UCI
           (Universal Chess Interface) protocol.
*/

#ifndef UCI_H
#define UCI_H

#include "defs.h"

#include <string> // std::string

#include "board.h"
#include "search.h"

// External function declarations

extern void uci_loop(); // UCI infinite loop.

// Parses the UCI 'position' command.

extern bool parse_uci_position(const std::string& cmd, Board& board);

// Parses the UCI 'go' command.

extern void parse_uci_go(const std::string& cmd, SearchInfo& search_info,
    Board& board);

#endif // UCI_H