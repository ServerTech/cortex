/*
    Cortex - Self-learning Chess Engine
    @filename index_proc.cpp
    @author Shreyas Vinod
    @version 1.0.0

    @brief A set of functions to process a bitboard and return integer indices.

    ******************** VERSION CONTROL ********************
    * 08/11/2015 File created.
    * 08/11/2015 1.0.0 Initial version.
*/

#include <assert.h> // assert().
#include "index_proc.h"
#include "defs.h"

/**
    @brief Calculates the LERF mapping equivalent index.

    @param bit_conv is a uint64_t value with exactly one bit set.

    @return int corresponding to the index of the bit set in 'bit_conv'
            in LERF mapping.

    @warning Exactly one bit must be set in 'bit_conv'.
    @warning Returns -1 on error.
*/

int IndexProc::get_lerf_index(uint64_t bit_conv)
{
    // Exactly one bit must be set.

    assert((bit_conv != 0ULL) && ((bit_conv & (bit_conv - 1)) == 0ULL));

    // This might seem absurd, but I think that this is the fastest way!
    // Actually, there's a faster way, but it takes 450 lines to do it.
    // This is *probably* fast enough.

    if(bit_conv & R_UP_HALF) // Ranks 5, 6, 7 and 8.
    {
        if(bit_conv & R_UP_MID) // Ranks 5 and 6 (more likely).
        {
            if(bit_conv & B_RANK[5]) // Rank 5.
            {
                if(bit_conv & F_L_HALF) // Left half of rank 5.
                    for(int i = 35;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 5.
                    for(int i = 36;; i++) if(bit_conv & (1ULL << i)) return i;
            }
            else // Rank 6.
            {
                if(bit_conv & F_L_HALF) // Left half of rank 6.
                    for(int i = 43;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 6.
                    for(int i = 44;; i++) if(bit_conv & (1ULL << i)) return i;
            }
        }
        else // Ranks 7 and 8.
        {
            if(bit_conv & B_RANK[7]) // Rank 7 (more likely).
            {
                if(bit_conv & F_L_HALF) // Left half of rank 7.
                    for(int i = 51;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 7.
                    for(int i = 52;; i++) if(bit_conv & (1ULL << i)) return i;
            }
            else // Rank 8.
            {
                if(bit_conv & F_L_HALF) // Left half of rank 8.
                    for(int i = 59;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 8.
                    for(int i = 60;; i++) if(bit_conv & (1ULL << i)) return i;
            }
        }
    }
    else // Ranks 1, 2, 3 and 4.
    {
        if(bit_conv & R_LOW_MID) // Ranks 3 and 4 (more likely).
        {
            if(bit_conv & B_RANK[4]) // Rank 4.
            {
                if(bit_conv & F_L_HALF) // Left half of rank 4.
                    for(int i = 27;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 4.
                    for(int i = 28;; i++) if(bit_conv & (1ULL << i)) return i;
            }
            else // Rank 3.
            {
                if(bit_conv & F_L_HALF) // Left half of rank 3.
                    for(int i = 19;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 3.
                    for(int i = 20;; i++) if(bit_conv & (1ULL << i)) return i;
            }
        }
        else // Ranks 1 and 2.
        {
            if(bit_conv & B_RANK[2]) // Rank 2 (more likely).
            {
                if(bit_conv & F_L_HALF) // Left half of rank 2.
                    for(int i = 11;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 2.
                    for(int i = 12;; i++) if(bit_conv & (1ULL << i)) return i;
            }
            else // Rank 1.
            {
                if(bit_conv & F_L_HALF) // Left half of rank 1.
                    for(int i = 3;; i--) if(bit_conv & (1ULL << i)) return i;
                else // Right half of rank 1.
                    for(int i = 4;; i++) if(bit_conv & (1ULL << i)) return i;
            }
        }
    }

    return -1;
}

/**
    @brief Calculates the Big-Endian mapping equivalent index.

    @param bit_conv is a uint64_t value with exactly one bit set.

    @return int corresponding to the index of the bit set in 'bit_conv'
            in regular Big-Endian mapping.

    @warning Exactly one bit must be set in 'bit_conv'.
    @warning If 55 is returned, it could be an error, resulting from
             IndexProc::get_lerf_index(uint64_t) returning -1.
*/

int IndexProc::get_be_index(uint64_t bit_conv)
{
    // Exactly one bit must be set.

    assert((bit_conv != 0ULL) && ((bit_conv & (bit_conv - 1)) == 0ULL));

    int i = IndexProc::get_lerf_index(bit_conv);

    return (i + 56) - (16 * (i / 8));
}