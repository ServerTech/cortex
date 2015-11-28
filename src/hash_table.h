/*
    Cortex - Self-learning Chess Engine
    @filename hash_table.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief Handles hash tables for efficient move searching.

    Includes functions that help create and manage dynamic hash
    tables for improving move search efficiency.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 28/11/2015 0.1.0 Initial version.
*/

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "debug.h"

#include <new> // ::operator new

#include "defs.h"

// Structures

/**
    @brief Holds information about a move and its corresponding board's hash
           key in order to store Principal Variation (PV) lines in a PV table.

    @var move is the move made on the board.
    @var hash_key is the zobrist hash of the board.
*/

struct PVEntry
{
    unsigned int move; // Move made.
    uint64 hash_key; // Zobrist hash of the board.

    PVEntry()
    :move(0), hash_key(0ULL)
    {}

    PVEntry(unsigned int m, uint64 hk)
    :move(m), hash_key(hk)
    {}
};

/**
    @brief Stores a bunch of PV entries.

    @var pv_entry is the pv_entry array, dynamically allocated.
    @var num_entries is the number of entries in the array.

    @warning Memory must be initialised.
    @warning num_entries musn't be changed after initialisation. If it is,
             the memory must be reinitialised.
*/

struct PVTable
{
    PVEntry* pv_entry;
    unsigned int num_entries;

    PVTable()
    :pv_entry(nullptr), num_entries(0)
    {}
};

// External function definitions

// Initialise PV hash table.

extern void init_pv_table(PVTable& pv_table, unsigned int pv_size);

// Store move into the PV hash table.

extern void store_pv_move(PVTable& pv_table, uint64 hash_key, unsigned int move);

// Retrieve move from the PV hash table.

extern unsigned int probe_pv_table(PVTable& pv_table, uint64 hash_key);

extern void clear_pv_table(PVTable& pv_table); // Clear out PV table.

#endif // HASH_TABLE_H