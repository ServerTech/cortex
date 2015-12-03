/*
    Cortex - Self-learning Chess Engine
    @filename hash_table.h
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

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "debug.h"

#include <new> // ::operator new

#include "defs.h"

// Enumerations

enum { TFALPHA = 1, TFBETA, TFEXACT }; // Flags

// Structures

/**
    @brief Holds a bunch of information about previous searches, to be inserted
           into the transposition table for future use.

    @var hash_key is the zobrist hash of the board.
    @var move is the move made on the board.
    @var score is the evaluation of the board for the given move.
    @var depth is the depth this particular game state was evaluated to.
    @var flag represents one of three flags: TFALPHA; TFBETA; TFEXACT.
*/

struct TableEntry
{
    uint64 hash_key; // Zobrist hash of the board.
    unsigned int move; // Move made.
    int score; // Evaluation of the board after the move is made.
    unsigned int depth; // Depth to which the board was previously searched.
    unsigned int flag; // Flag set.

    TableEntry()
    :hash_key(0ULL), move(NO_MOVE), score(0), depth(0), flag(0)
    {}

    TableEntry(uint64 hk, unsigned int m, int s, unsigned int d,
        unsigned int f)
    :hash_key(hk), move(m), score(s), depth(d), flag(f)
    {}
};

/**
    @brief Stores a bunch of table entries for the transposition table.

    @var t_entry is the t_entry array, which is dynamically allocated.
    @var num_entries is the number of entries in the array.

    @warning Memory must be initialised.
    @warning num_entries musn't be changed after initialisation. If it is,
             the memory must be reinitialised.
*/

struct TranspositionTable
{
    TableEntry* t_entry;
    unsigned int num_entries;

    TranspositionTable()
    :t_entry(nullptr), num_entries(0)
    {}
};

// External function definitions

// Initialise hash table.

extern void init_table(TranspositionTable& t_table, unsigned int t_size);

extern void free_table(TranspositionTable& t_table); // Free table memory.
extern void clear_table(TranspositionTable& t_table); // Clear out the table.

// Store a hash entry.

extern void store_entry(TranspositionTable& t_table, unsigned int ply,
    uint64 hash_key, unsigned int move, int score, unsigned int depth,
    unsigned int flag);

// Retrieve a hash entry.

extern bool probe_table(TranspositionTable& t_table, unsigned int ply,
    uint64 hash_key, unsigned int depth, unsigned int& pv_move, int& score,
    int alpha, int beta);

// Retrieve a PV move from the hash table.

extern unsigned int probe_pv_table(TranspositionTable& t_table, uint64 hash_key);

#endif // HASH_TABLE_H