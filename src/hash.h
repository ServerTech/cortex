/*
    Cortex - Self-learning Chess Engine
    @filename hash.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief Handles zobrist hashing to generate hashes for game states.

    Uses zobrist hashing to generate 64-bit hash keys for a given state of
    the board.

    ******************** VERSION CONTROL ********************
    * 14/11/2015 File created.
    * 14/11/2015 0.1.0 Initial version.
*/

#ifndef HASH_H
#define HASH_H

#include "defs.h"
#include "board.h"

// Globals

extern uint64 PIECE_KEYS[13][64]; // 64 keys for each piece, plus 64 for en passant.
extern uint64 SIDE_KEY; // Hashed in if side to play is white.
extern uint64 CASTLE_KEYS[16]; // 16 keys for castling permissions.

// External function definitions

extern void init_hash(); // Initialise keys.
extern uint64 gen_hash(const Board& board); // Generate hash from board.

#endif // HASH_H