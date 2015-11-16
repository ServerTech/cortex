/*
    Cortex - Self-learning Chess Engine
    @filename index_proc.h
    @author Shreyas Vinod
    @version 1.0.0

    @brief A set of functions to process a bitboard and return integer indices.

    ******************** VERSION CONTROL ********************
    * 08/11/2015 File created.
    * 08/11/2015 1.0.0 Initial version.
*/

#ifndef INDEX_PROC_H
#define INDEX_PROC_H

#include <stdint.h> // uint64_t.

/**
    @brief A set of functions to process a bitboard and return integer indices.
*/

namespace IndexProc
{
    int get_lerf_index(uint64_t bit_conv); // Get index in LERF.
    int get_be_index(uint64_t bit_conv); // Get index in Big-Endian.
}

#endif // INDEX_PROC_H