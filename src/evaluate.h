/*
    Cortex - Self-learning Chess Engine
    @filename evaluate.h
    @author Anna Grygierzec
    @version 0.1.0

    @brief Static evaluation function that returns an objective score
           of the game state.

    ******************** VERSION CONTROL ********************
    * 28/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
*/

#ifndef EVALUATE_H
#define EVALUATE_H

#include "debug.h"

#include "board.h"
#include "defs.h"

// External function definitions

extern int static_eval(Board& board);

#endif // EVALUATE_H