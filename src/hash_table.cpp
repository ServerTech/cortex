/*
    Cortex - Self-learning Chess Engine
    @filename hash_table.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Handles hash tables for efficient move searching.

    Includes functions that help create and manage dynamic hash
    tables for improving move search efficiency.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 28/11/2015 0.1.0 Initial version.
*/

#include "debug.h"

#include <assert.h> // std::assert()

#include "hash_table.h"
#include "movegen.h"

// Function Prototypes

void init_pv_table(PVTable& pv_table, unsigned int pv_size);
void store_pv_move(PVTable& pv_table, uint64 hash_key, unsigned int move);
unsigned int probe_pv_table(PVTable& pv_table, uint64 hash_key);
void clear_pv_table(PVTable& pv_table);

// Functions

/**
    @brief Initialises memory for a PV table. Everything is zeroed out.

    @param pv_table is the hash table to initialise.
    @param pv_size is the size in bytes of the PV hash table to be initialised.

    @return void.
*/

void init_pv_table(PVTable& pv_table, unsigned int pv_size)
{
    pv_table.num_entries = pv_size / sizeof(PVEntry);
    if(pv_table.pv_entry) delete pv_table.pv_entry;
    pv_table.pv_entry = new PVEntry[pv_table.num_entries];
}

/**
    @brief Clears the given PV table by zeroing everything out.

    @param pv_table is the hash table to clear.
*/

void clear_pv_table(PVTable& pv_table)
{
    for(unsigned int i = 0; i < pv_table.num_entries; i++)
    {
        pv_table.pv_entry[i].hash_key = 0ULL;
        pv_table.pv_entry[i].move = 0;
    }
}

/**
    @brief Store a PV move into the PV table.

    @param pv_table is the hash table to store the move into.
    @param hash_key is the zobrist hash of the board the move belongs to.
    @param move is the move to store into the table.

    @return void.
*/

void store_pv_move(PVTable& pv_table, uint64 hash_key, unsigned int move)
{
    unsigned int index = hash_key % pv_table.num_entries;

    assert(index < pv_table.num_entries);

    pv_table.pv_entry[index].hash_key = hash_key;
    pv_table.pv_entry[index].move = move;
}

/**
    @brief Retrieve a PV move if it exists in the table.

    @param pv_table is the hash table to probe.
    @param hash_key is the zobrist hash of the board the move belongs to.

    @return unsigned int value presenting the move.

    @warning Returns 'NO_MOVE' (0) if no move was found or an index collision
             had previously occurred.
*/

unsigned int probe_pv_table(PVTable& pv_table, uint64 hash_key)
{
    unsigned int index = hash_key % pv_table.num_entries;

    assert(index < pv_table.num_entries);

    if(pv_table.pv_entry[index].hash_key == hash_key)
        return pv_table.pv_entry[index].move;

    return NO_MOVE;
}