/*
    Cortex - Self-learning Chess Engine
    @filename evaluate.h
    @author Anna Grygierzec
    @version 1.0.0

    @brief Static evaluation function that returns an objective score
           of the game state.

    ******************** VERSION CONTROL ********************
    * 28/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 06/12/2015 0.1.1 Added checks for isolated and passed pawns.
    * 06/12/2015 0.1.2 Added checks of open/half-open files.
    * 22/12/2015 0.1.3 Added backward pawns, king on and near open file,
                       pawn shield, rook and bishop bonus for lost pawns.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename evaluate.h
    @author Anna Grygierzec

    @brief Static evaluation function that returns an objective score
           of the game state.
*/

#ifndef EVALUATE_H
#define EVALUATE_H

#include "defs.h"

#include "board.h"

// External function declarations

extern void init_evalmasks(); // Initialise all bitmasks.
extern int static_eval(Board& board); // Static evaluation.

#endif // EVALUATE_H