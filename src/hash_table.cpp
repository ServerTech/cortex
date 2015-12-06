/*
    Cortex - Self-learning Chess Engine
    @filename hash_table.cpp
    @author Shreyas Vinod
    @version 0.1.1

    @brief Handles hash tables for efficient move searching.

    Includes functions that help create and manage dynamic hash
    tables for improving move search efficiency.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 28/11/2015 0.1.0 Initial version.
    * 03/12/2015 0.1.1 Updated to a full transposition table.
*/

/**
    @file
    @filename hash_table.cpp
    @author Shreyas Vinod

    @brief Handles hash tables for efficient move searching.

    Includes functions that help create and manage dynamic hash
    tables for improving move search efficiency.
*/

#include "defs.h"

#include <new> // ::operator new
#include <assert.h> // std::assert()

#include "hash_table.h"
#include "movegen.h"

// Prototypes

void init_table(TranspositionTable& t_table, unsigned int t_size);
void free_table(TranspositionTable& t_table);
void clear_table(TranspositionTable& t_table);
void store_entry(TranspositionTable& t_table, unsigned int ply,
    uint64 hash_key, unsigned int move, int score, unsigned int depth,
    unsigned int flag);
bool probe_table(TranspositionTable& t_table, unsigned int ply,
    uint64 hash_key, unsigned int depth, unsigned int& pv_move, int& score,
    int alpha, int beta);
unsigned int probe_pv_table(TranspositionTable& t_table, uint64 hash_key);

// Function definitions

/**
    @brief Initialises memory for a transposition table. Everything is zeroed.

    @param t_table is the hash table to initialise.
    @param t_size is the size in bytes of the hash table to be initialised.

    @return void.
*/

void init_table(TranspositionTable& t_table, unsigned int t_size)
{
    t_table.num_entries = t_size / sizeof(TableEntry);
    if(t_table.t_entry) delete t_table.t_entry;
    t_table.t_entry = new TableEntry[t_table.num_entries];
}

/**
    @brief Frees table memory.

    @param t_table is the hash table to free.

    @return void.
*/

void free_table(TranspositionTable& t_table)
{
    if(t_table.t_entry) delete t_table.t_entry;
}

/**
    @brief Clears the given table by zeroing everything out.

    @param t_table is the hash table to clear.
*/

void clear_table(TranspositionTable& t_table)
{
    for(unsigned int i = 0; i < t_table.num_entries; i++)
    {
        t_table.t_entry[i].hash_key = 0ULL;
        t_table.t_entry[i].move = NO_MOVE;
        t_table.t_entry[i].score = 0;
        t_table.t_entry[i].depth = 0;
        t_table.t_entry[i].flag = 0;
    }
}

/**
    @brief Store a hash entry.

    @param t_table is the hash table to store into.
    @param ply the current ply in search.
    @param hash_key is the zobrist hash of the board.
    @param move is the move to store into the table.
    @param score is the evaluation of the board after the move is made.
    @param depth is the depth to which the evaluation holds.
    @param flag represents the set flag.

    @return void.
*/

void store_entry(TranspositionTable& t_table, unsigned int ply,
    uint64 hash_key, unsigned int move, int score, unsigned int depth,
    unsigned int flag)
{
    unsigned int index = hash_key % t_table.num_entries;

    assert(index < t_table.num_entries);

    if(score > IS_MATE) score += ply;
    else if(score < -IS_MATE) score -= ply;

    t_table.t_entry[index].hash_key = hash_key;
    t_table.t_entry[index].move = move;
    t_table.t_entry[index].score = score;
    t_table.t_entry[index].depth = depth;
    t_table.t_entry[index].flag = flag;
}

/**
    @brief Retrieve a hash entry if it exists in the table.

    @param t_table is the hash table to probe.
    @param ply the current ply in search.
    @param hash_key is the zobrist hash of the board to index the table with.
    @param depth is the current search depth.
    @param pv_move is a reference to the PV move variable in search.
    @param score is a reference to the score variable in search.
    @param alpha is the current value of alpha.
    @param beta is the current value of beta.

    @return bool denoting whether a hash hit occurred, that is, an entry with
            depth greater than or equal to the current search depth was found.

    @warning At least one flag must exist in the hash entry.
*/

bool probe_table(TranspositionTable& t_table, unsigned int ply,
    uint64 hash_key, unsigned int depth, unsigned int& pv_move, int& score,
    int alpha, int beta)
{
    unsigned int index = hash_key % t_table.num_entries;

    assert(index < t_table.num_entries);

    if(t_table.t_entry[index].hash_key == hash_key)
    {
        pv_move = t_table.t_entry[index].move;

        if(t_table.t_entry[index].depth >= depth)
        {
            score = t_table.t_entry[index].score;

            if(score > IS_MATE) score -= ply;
            else if(score < -IS_MATE) score += ply;

            switch(t_table.t_entry[index].flag)
            {
                case TFALPHA:
                {
                    if(score <= alpha)
                    {
                        score = alpha;
                        return 1;
                    }
                }
                case TFBETA:
                {
                    if(score >= beta)
                    {
                        score = beta;
                        return 1;
                    }
                }
                case TFEXACT:
                {
                    return 1;
                }
                default: assert(false); // At least one flag must be set.
            }
        }
    }

    return 0;
}

/**
    @brief Retrieve a PV move if it exists in the table.

    @param t_table is the hash table to probe.
    @param hash_key is the zobrist hash of the board to index the table with.

    @return unsigned int value presenting the move.

    @warning Returns 'NO_MOVE' (0) if no move was found or an index collision
             had previously occurred.
*/

unsigned int probe_pv_table(TranspositionTable& t_table, uint64 hash_key)
{
    unsigned int index = hash_key % t_table.num_entries;

    assert(index < t_table.num_entries);

    if(t_table.t_entry[index].hash_key == hash_key)
        return t_table.t_entry[index].move;

    return NO_MOVE;
}