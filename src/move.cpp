/*
    Cortex - Self-learning Chess Engine
    @filename move.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Keeps key structures for handling moves, especially during move
           generation.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 15/11/2015 0.1.0 Initial version.
*/

#include "move.h"

// Globals

const unsigned int MFLAGEP = 0x10000; // En passant flag mask.
const unsigned int MFLAGPS = 0x200000; // Pawn start flag mask.
const unsigned int MFLAGCA = 0x400000; // Castling flag mask.