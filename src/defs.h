/*
    Cortex - Self-learning Chess Engine
    @B_FILEname defs.h
    @author Shreyas Vinod
    @version 0.1.2

    @brief Holds definitions for code readability and speed improvements.

    ******************** VERSION CONTROL ********************
    * 27/07/2015 B_FILE created.
    * 27/07/2015 0.1.0 Initial version.
    * 08/11/2015 0.1.1 Added more values.
        * Added B_RANK[9] and B_FILE[9].
    * 13/11/2015 0.1.2 Added bit manipulation functions.
*/

#ifndef DEFS_H
#define DEFS_H

#include <string> // std::string
#include <assert.h> // std::assert()

typedef unsigned long long uint64;

// Globals

enum { FALSE, TRUE };

enum { BLACK, WHITE };

enum { WKCA = 8, WQCA = 4, BKCA = 2, BQCA = 1 };

enum { wP, wR, wN, wB, wQ, wK, bP, bR, bN, bB, bQ, bK, ALL_WHITE, ALL_BLACK,
    EMPTY };

enum { NONE, FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };

enum { RANK_1 = 1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

enum
{
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8, NO_SQ
};

const uint64 B_FILE[9] = {
    0x0000000000000000ULL, 0x0101010101010101ULL, 0x0202020202020202ULL,
    0x0404040404040404ULL, 0x0808080808080808ULL, 0x1010101010101010ULL,
    0x2020202020202020ULL, 0x4040404040404040ULL, 0x8080808080808080ULL
};

const uint64 B_RANK[9] = {
    0x0000000000000000ULL, 0x00000000000000ffULL, 0x000000000000ff00ULL,
    0x0000000000ff0000ULL, 0x00000000ff000000ULL, 0x000000ff00000000ULL,
    0x0000ff0000000000ULL, 0x00ff000000000000ULL, 0xff00000000000000ULL
};

// Bit manipulation helper functions

/**
    @brief Generates a bitboard out of the given index.

    This function currently uses bitshifting. It may be later changed to
    utilise lookup tables, if they are found to be faster.
    Refer: http://stackoverflow.com/questions/33703233/

    @param index is the integer index of the bit to set.

    @return uint64 board with one bit indexed by 'index' set.

    @warning 'index' must be between (or equal to) 0 and 63.
*/

inline uint64 GET_BB(unsigned int index)
{
    assert(index < 64);

    return 1ULL << index;
}

/**
    @brief Sets the bit on the given index on the given bitboard.

    @param bb is the bitboard to set the bit on.
    @param index is the integer index denoting which bit to set.

    @return void.

    @warning 'index' must be between (or equal to) 0 and 63.
*/

inline void SET_BIT(uint64& bb, unsigned int index)
{
    assert(index < 64);

    bb |= GET_BB(index);
}

/**
    @brief Clears the bit on the given index on the given bitboard.

    @param bb is the bitboard to clear the bit on.
    @param index is the integer index denoting which bit to clear.

    @return void.

    @warning 'index' must be between (or equal to) 0 and 63.
*/

inline void CLR_BIT(uint64& bb, unsigned int index)
{
    assert(index < 64);

    bb &= ~(GET_BB(index));
}

/**
    @brief Returns the number of set bits in the given integer.

    Uses __builtin_popcountll() to count the number of bits.

    @param bb is the integer (bitboard) in which to count the number of bits.

    @return unsigned int value denoting the number of set bits in 'bb'.
*/

inline unsigned int CNT_BITS(const uint64& bb)
{
    if(bb != 0ULL) return __builtin_popcountll(bb);
    else return 0;
}

/**
    @brief Pops the least significant bit in the given integer and returns its
           index.

    Uses __builtin_ffsll() to get the index of the least significant bit.

    @param bb is the non-zero integer (bitboard) to pop the bit from.

    @return unsigned int value denoting the index where the bit existed.

    @warning 'bb' must be a non-zero integer. Otherwise, its value will be
             corrupted and the returned index will overflow, being equal
             to the maximum unsigned 32-bit integer.
*/

inline unsigned int POP_BIT(uint64& bb)
{
    assert(bb != 0ULL);

    unsigned int index = __builtin_ffsll(bb) - 1;
    bb ^= GET_BB(index);
    return index;
}

/**
    @brief Calculates which file the given cell index is on.

    @param index is the integer index denoting which cell to check.

    @return unsigned int value denoting which file (1 to 8) the cell
            indexed by 'index' is on.
*/

inline unsigned int GET_FILE(unsigned int index)
{
    return (index % 8) + 1;
}

/**
    @brief Calculates which rank the given cell index is on.

    @param index is the integer index denoting which cell to check.

    @return unsigned int value denoting which rank (1 to 8) the cell
            indexed by 'index' is on.
*/

inline unsigned int GET_RANK(unsigned int index)
{
    return (index / 8) + 1;
}

/**
    @brief Given a file and rank, returns the index of the cell.

    Given a file and rank, this function finds a cell on the board
    which is common to both (the intersection) and then returns its
    index in LERF layout.

    Uses __builtin_ffsll() to get the index of the least significant bit.

    @param file is a value between (or equal to) 1 and 8, or
           FILE_A to FILE_H.
    @param rank is a value between (or equal to) 1 and 8, or
           RANK_1 to RANK_8.

    @return unsigned int value denoting the index of the intersection
            cell of 'file' and 'rank'. The index is in LERF layout.

    @warning 'file' and 'rank' must be between or equal to 1 and 8, or
             bad things will happen as arrays are indexed out of bounds.
    @warning The returned index value is in LERF layout.
*/

inline unsigned int GET_INDEX(unsigned int file, unsigned int rank)
{
    assert(file >= FILE_A && file <= FILE_H);
    assert(rank >= RANK_1 && rank <= RANK_8);

    return __builtin_ffsll(B_FILE[file] & B_RANK[rank]) - 1;
}

#endif // DEFS_H