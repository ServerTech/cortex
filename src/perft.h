/*
    Cortex - Self-learning Chess Engine
    @filename perft.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief Performs basic perft testing on the move generator.

    ******************** VERSION CONTROL ********************
    * 22/11/2015 File created.
    * 23/11/2015 0.1.0 Initial version.
*/

#ifndef PERFT_H
#define PERFT_H

#include "debug.h"

#include "board.h"
#include "defs.h"

// External function definitions

// Perform perft without any printing.

uint64 perform_perft(Board& board, unsigned int depth);

// A verbose variant of perform_perft(), which prints out divide perft results.

uint64 perform_perft_verbose(Board& board, unsigned int depth);

#endif // PERFT_H