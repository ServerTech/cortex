/*
    Cortex - Self-learning Chess Engine
    @filename lookup_tables.h
    @author Shreyas Vinod
    @version 0.1.2

    @brief A collection of pre-calculated lookup tables for move generation.
           Based on Little-Endian Rank-File mapping (LERF).

    ******************** VERSION CONTROL ********************
    * 09/07/2015 File created.
    * 27/07/2015 0.1.0 Initial version.
    * 15/11/2015 0.1.2 Names are now more generic.
*/

#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include "defs.h" // uint64

// Globals

extern const uint64 KING_LT[64]; // King lookup.
extern const uint64 KNIGHT_LT[64]; // Knight lookup.
extern const uint64 LINE_LT[64]; // Lines.
extern const uint64 DIAG_LT[64]; // Diagonals.
extern const uint64 LINE_DIAG_LT[64]; // Lines and diagonals.
extern const uint64 LINE_N_LT[64]; // North line.
extern const uint64 LINE_S_LT[64]; // South line.
extern const uint64 LINE_E_LT[64]; // East line.
extern const uint64 LINE_W_LT[64]; // West line.
extern const uint64 DIAG_NE_LT[64]; // Northeast diagonal.
extern const uint64 DIAG_NW_LT[64]; // Northwest diagonal.
extern const uint64 DIAG_SE_LT[64]; // Southeast diagonal.
extern const uint64 DIAG_SW_LT[64]; // Southwest diagonal.

#endif // LOOKUP_TABLES_H