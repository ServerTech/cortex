/*
    Cortex - Self-learning Chess Engine
    @filename perft.h
    @author Shreyas Vinod
    @version 0.1.1

    @brief Performs basic perft testing on the move generator.

    ******************** VERSION CONTROL ********************
    * 22/11/2015 File created.
    * 23/11/2015 0.1.0 Initial version.
    * 07/12/2015 0.1.1 Added perft for just captures.
*/

/**
    @file
    @filename perft.h
    @author Shreyas Vinod

    @brief Performs basic perft testing on the move generator.
*/

#ifndef PERFT_H
#define PERFT_H

#include "defs.h"

#include "board.h"

// External function declarations

// Perform perft without any printing.

extern uint64 perform_perft(Board& board, unsigned int depth);

// A verbose variant of perform_perft(), which prints out divide perft results.

extern uint64 perform_perft_verbose(Board& board, unsigned int depth);

// A variant of perform_perft_verbose(), generating just capture moves.

extern uint64 perform_perftc_verbose(Board& board, unsigned int depth);

#endif // PERFT_H