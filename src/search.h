/*
    Cortex - Self-learning Chess Engine
    @filename search.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 28/11/2015 0.1.0 Initial version.
*/

#ifndef SEARCH_H
#define SEARCH_H

#include "debug.h"

#include "board.h"
#include "defs.h"

// Structures

/**
    @brief Holds a bunch of information that's helpful while searching.

    @var start_time is the time the search began.
    @var stop_time is when the search should end.
    @var depth is the total depth to search to.
    @var depth_set is the maximum depth to search to.
    @var time_set is the maximum time to search for.
    @var moves_to_go is the number of moves to go, for time control.
    @var nodes is the number of nodes searched so far.
    @var infinite denotes whether the search should go on infinitely, until
         it is interrupted.
    @var stopped denotes whether an interrupt was acknowledged, where the search
         should be interrupted.
    @var quit denotes whether to quit the program.
    @var fh stands for 'fail-high', used for move ordering.
    @var fhf stands for 'fail-high-first', used for move ordering.
*/

struct SearchInfo
{
    unsigned int start_time;
    unsigned int stop_time;
    unsigned int depth;
    unsigned int depth_set;
    unsigned int time_set;
    unsigned int moves_to_go;

    uint64 nodes;

    bool infinite;
    bool stopped;
    bool quit;

    double fh;
    double fhf;

    SearchInfo()
    :start_time(0), stop_time(0), depth(1), depth_set(1), time_set(0),
        moves_to_go(0), nodes(0), infinite(0), stopped(0), quit(0),
        fh(0), fhf(0)
    {}
};

// External function definitions

// Iterative deepening implementation.

extern void search(Board& board, SearchInfo& search_info);

#endif // SEARCH_H