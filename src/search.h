/*
    Cortex - Self-learning Chess Engine
    @filename search.h
    @author Shreyas Vinod
    @version 0.1.2

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 02/12/2015 0.1.1 Added time handling.
    * 02/12/2015 0.1.2 Added null move pruning.
*/

#ifndef SEARCH_H
#define SEARCH_H

#include "debug.h"

#include "board.h"
#include "chronos.h" // Time and get_time_diff()
#include "defs.h"

// Structures

/**
    @brief Holds a bunch of information that's helpful while searching.

    @var start_time is the time the search began.
    @var move_time is the maximum amount of time the search should take in
         milliseconds.
    @var depth is the total depth to search to.
    @var moves_to_go is the number of moves to go, for time control.
    @var nodes is the number of nodes searched so far.
    @var depth_set denotes whether a maximum depth has been set.
    @var time_set denotes whether maximum time has been set.
    @var stopped denotes whether an interrupt was acknowledged, where the search
         should be interrupted.
    @var quit denotes whether to quit the program.
    @var fh stands for 'fail-high', used for move ordering statistics.
    @var fhf stands for 'fail-high-first', used for move ordering statistics.
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

// External function definitions

// Iterative deepening implementation.

extern void search(Board& board, SearchInfo& search_info);

#endif // SEARCH_H