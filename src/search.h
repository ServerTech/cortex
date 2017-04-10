/*
    Cortex - Self-learning Chess Engine
    @filename search.h
    @author Shreyas Vinod
    @version 1.0.0

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 02/12/2015 0.1.1 Added time handling.
    * 02/12/2015 0.1.2 Added null move pruning.
    * 06/12/2015 0.1.3 Added ponder move output during search.
    * 06/12/2015 0.1.4 Added in-check extensions.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename search.h
    @author Shreyas Vinod

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.
*/

#ifndef SEARCH_H
#define SEARCH_H

#include "defs.h"

#include "board.h"
#include "chronos.h" // Time and get_time_diff()

// Structures

/**
    @struct SearchInfo

    @brief Holds a bunch of information that's helpful while searching.

    @var SearchInfo::start_time
         The time the search began.
    @var SearchInfo::move_time
         The maximum amount of time the search should take in milliseconds.
    @var SearchInfo::depth
         The total depth to search to.
    @var SearchInfo::moves_to_go
         The number of moves to go, for time control.
    @var SearchInfo::nodes
         The number of nodes searched so far.
    @var SearchInfo::depth_set
         Denotes whether a maximum depth has been set.
    @var SearchInfo::time_set
         Denotes whether maximum time has been set.
    @var SearchInfo::stopped
         Denotes whether an interrupt was acknowledged, where the search should
         be interrupted.
    @var SearchInfo::quit
         Denotes whether to quit the program.
    @var SearchInfo::fh
         Stands for 'fail-high', used for move ordering statistics.
    @var SearchInfo::fhf
         Stands for 'fail-high-first', used for move ordering statistics.
*/

struct SearchInfo
{
    Time start_time;
    uint64 move_time;

    unsigned int depth;
    unsigned int moves_to_go;

    uint64 nodes;

    bool depth_set;
    bool time_set;
    bool stopped;
    bool quit;

    double fh;
    double fhf;

    SearchInfo()
    :start_time(), move_time(0), depth(1), moves_to_go(0), nodes(0),
        depth_set(0), time_set(0), stopped(0), quit(0), fh(0), fhf(0)
    {}
};

// External function declarations

// Iterative deepening implementation.

extern void search(Board& board, SearchInfo& search_info);

#endif // SEARCH_H