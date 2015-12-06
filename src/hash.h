/*
    Cortex - Self-learning Chess Engine
    @filename hash.h
    @author Shreyas Vinod
    @version 0.1.1

    @brief Handles zobrist hashing to generate hashes for game states.

    Uses zobrist hashing to generate 64-bit hash keys for a given state of
    the board.

    ******************** VERSION CONTROL ********************
    * 14/11/2015 File created.
    * 14/11/2015 0.1.0 Initial version.
    * 22/11/2015 0.1.1 Added helper functions.
        * Added HASH_PIECE(Board&, unsigned int, unsigned int),
            HASH_SIDE(Board&), HASH_CA(Board&) and HASH_EP(Board&).
*/

/**
    @file
    @filename hash.h
    @author Shreyas Vinod

    @brief Handles zobrist hashing to generate hashes for game states.

    Uses zobrist hashing to generate 64-bit hash keys for a given state of
    the board.
*/

#ifndef HASH_H
#define HASH_H

#include "defs.h"

#include "board.h"

// Globals

extern uint64 PIECE_KEYS[13][64]; // 64 keys for each piece; 64 for en passant.
extern uint64 SIDE_KEY; // Hashed in if side to play is white.
extern uint64 CASTLE_KEYS[16]; // 16 keys for castling permissions.

// Helper functions for hashing

/**
    @brief Hash a piece in or out of the hash key on the given board.

    @param board is the board to change the hash on.
    @param piece_type is the integer representation of the piece to hash in
           standard convention.
    @param index is the integer index of the cell on the board the piece is on
           in LERF layout.

    @return void.

    @warning This function does not keep track of whether the piece is being
             hashed in or out.
    @warning 'piece_type' must be between (or equal to) wP (0) and bK (11).
    @warning 'index' must be between (or equal to) 0 and 63.
*/

inline void HASH_PIECE(Board& board, unsigned int piece_type,
    unsigned int index)
{
    assert(piece_type < 12);
    assert(index < 64);

    board.hash_key ^= PIECE_KEYS[piece_type][index];
}

/**
    @brief Hash the current side to play in or out of the hash key on the given
           board.

    @param board is the board to change the hash on.

    @return void.

    @warning This function does not keep track of whether the side is being
             hashed in or out.
*/

inline void HASH_SIDE(Board& board)
{
    board.hash_key ^= SIDE_KEY;
}

/**
    @brief Hash castling permissions in or out of the hash key on the given
           board.

    @param board is the board to change the hash on.

    @return void.

    @warning This function does not keep track of whether the permissions are
             being hashed in or out.
*/

inline void HASH_CA(Board& board)
{
    board.hash_key ^= CASTLE_KEYS[board.castle_perm];
}

/**
    @brief Hash the en passant square in or out of the hash key on the given
           board.

    @param board is the board to change the hash on.

    @return void.

    @warning This function does not keep track of whether the side is being
             hashed in or out.
*/

inline void HASH_EP(Board& board)
{
    if(board.en_pas_sq != NO_SQ)
    {
        board.hash_key ^= PIECE_KEYS[12][board.en_pas_sq];
    }
}

// External function declarations

extern void init_hash(); // Initialise keys.
extern uint64 gen_hash(const Board& board); // Generate hash from board.

#endif // HASH_H