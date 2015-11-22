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

#include <random> // std::mt19937_64()

#include "hash.h"
#include "defs.h"

// Prototypes

void init_hash();
uint64 gen_hash(const Board& board);

// Globals

uint64 PIECE_KEYS[13][64]; // 64 keys for each piece; 64 for en passant.
uint64 SIDE_KEY; // Hashed in if side to play is white.
uint64 CASTLE_KEYS[16]; // 16 keys for castling permissions.

// Functions

/**
    @brief Initialises hash keys declared in hash.h with 64-bit random numbers.

    Initialises 'PIECE_KEYS[13][64]' in hash.h with 64-bit random numbers
    generated using the Mersenne Twister Engine for use in zobrist hashing.

    @return void.

    @warning std::mt19937_64() or the 64-bit equivalent of the Mersenne Twister
             Engine requires the C++11 standard.
    @warning Do not call twice; your hashes will become useless, won't they?
*/

void init_hash()
{
    // Mersenne Twister Engine, seeded with the answer to the ultimate question
    // of life, the universe and everything.

    std::mt19937_64 gen_rand(42);

    for(int i = 0; i < 13; i++)
    {
        for(int j = 0; j < 64; j++)
        {
            PIECE_KEYS[i][j] = gen_rand(); // Piece and en passant hashes.
        }
    }

    SIDE_KEY = gen_rand(); // Hash to determine side.

    for(int i = 0; i < 16; i++)
    {
        CASTLE_KEYS[i] = gen_rand();
    }
}

/**
    @brief Generates a 64-bit zobrist hash of the given board state.

    @param board is the board structure to generate a hash for.

    @return uint64 zobrist hash key for the board state defined by
            'board'.

    @warning 'init_hash()' must've been previously called once to
             initialise keys.
*/

uint64 gen_hash(const Board& board)
{
    int num_bits, index;
    uint64 hash_key = 0ULL, temp;

    // Pieces

    for(int i = wP; i <= bK; i++)
    {
        temp = board.chessboard[i];
        num_bits = CNT_BITS(temp);

        for(int j = 0; j < num_bits; j++)
        {
            index = POP_BIT(temp);
            assert(index < 64);
            hash_key ^= PIECE_KEYS[i][index];
        }
    }

    if(board.side == WHITE) hash_key ^= SIDE_KEY; // Side

    // En passant

    if(board.en_pas_sq != NO_SQ)
    {
        assert(board.en_pas_sq < 64);
        hash_key ^= PIECE_KEYS[12][board.en_pas_sq];
    }

    // Castling Permissions

    assert(board.castle_perm < 16);
    hash_key ^= CASTLE_KEYS[board.castle_perm];

    return hash_key;
}