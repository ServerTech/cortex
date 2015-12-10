/*
    Cortex - Self-learning Chess Engine
    @filename perft.cpp
    @author Shreyas Vinod
    @version 0.1.2

    @brief Performs basic perft testing on the move generator.

    ******************** VERSION CONTROL ********************
    * 22/11/2015 File created.
    * 23/11/2015 0.1.0 Initial version.
    * 07/12/2015 0.1.1 Added perft for just captures.
    * 10/12/2015 0.1.2 Added check for zobrist hashes.
*/

/**
    @file
    @filename perft.cpp
    @author Shreyas Vinod

    @brief Performs basic perft testing on the move generator.
*/

#include "defs.h"

#include <iostream>

#include "perft.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "hash.h"

// Prototypes

void perft(Board& board, uint64& leaf_nodes, unsigned int depth);
void perftc(Board& board, uint64& leaf_nodes, unsigned int depth);
uint64 perform_perft(Board& board, unsigned int depth);
uint64 perform_perft_verbose(Board& board, unsigned int depth);
uint64 perform_perftc_verbose(Board& board, unsigned int depth);

// Function definitions

/**
    @brief Recursive function that performs a basic perft test on the
           move generator to count the number of leaf nodes.

    @param board is the board to perform the test on.
    @param leaf_nodes is the integer which is incremented when a leaf node
           is found.
    @param depth is the depth to which to search to.

    @return void.
*/

void perft(Board& board, uint64& leaf_nodes, unsigned int depth)
{
    if(depth == 0)
    {
        leaf_nodes++;
        return;
    }

    MoveList ml = gen_moves(board);

    unsigned int movegen_count = ml.list.size();

    for(unsigned int i = 0; i < movegen_count; i++)
    {
        if(!make_move(board, ml.list.at(i).move)) continue;

        if(board.hash_key != gen_hash(board))
            std::cout << "HASH ERROR!" << std::endl;

        perft(board, leaf_nodes, depth - 1);
        undo_move(board);
    }

    return;
}

/**
    @brief Recursive function that performs a basic perft test on the
           move generator to count the number of leaf nodes of just captures.

    @param board is the board to perform the test on.
    @param leaf_nodes is the integer which is incremented when a leaf node
           is found.
    @param depth is the depth to which to search to.

    @return void.
*/

void perftc(Board& board, uint64& leaf_nodes, unsigned int depth)
{
    if(depth == 0)
    {
        leaf_nodes++;
        return;
    }

    MoveList ml = gen_captures(board);

    unsigned int movegen_count = ml.list.size();

    for(unsigned int i = 0; i < movegen_count; i++)
    {
        if(!make_move(board, ml.list.at(i).move)) continue;

        if(board.hash_key != gen_hash(board))
            std::cout << "HASH ERROR!" << std::endl;

        perft(board, leaf_nodes, depth - 1);
        undo_move(board);
    }

    return;
}

/**
    @brief Given a board and depth value, performs a basic perft test
           on the move generator and returns the number of leaf nodes
           found.

    @param board is the board to perform the test on.
    @param depth is the depth to which to search to.

    @return uint64 value corresponding to the number of leaf nodes visited.
*/

uint64 perform_perft(Board& board, unsigned int depth)
{
    assert(depth != 0);

    uint64 leaf_nodes = 0;

    perft(board, leaf_nodes, depth);

    return leaf_nodes;
}

/**
    @brief Given a board and depth value, performs a basic perft test
           on the move generator and returns the number of leaf nodes
           found. This function prints out what it's doing.

    @param board is the board to perform the test on.
    @param depth is the depth to which to search to.

    @return uint64 value corresponding to the number of leaf nodes visited.
*/

uint64 perform_perft_verbose(Board& board, unsigned int depth)
{
    assert(depth != 0);

    uint64 leaf_nodes = 0;

    MoveList ml = gen_moves(board);

    unsigned int movegen_count = ml.list.size(), move, num_moves = 0;

    std::cout << "Performing perft to depth " << depth << ":" << std::endl <<
        std::endl;

    for(unsigned int i = 0; i < movegen_count; i++)
    {
        move = ml.list.at(i).move;
        if(!make_move(board, move)) continue;

        if(board.hash_key != gen_hash(board))
            std::cout << "HASH ERROR!" << std::endl;

        num_moves++;
        uint64 cum_nodes = leaf_nodes;
        perft(board, leaf_nodes, depth - 1);
        undo_move(board);

        std::cout << "Move " << num_moves << ": " << COORD_MOVE(move) <<
            " > " << leaf_nodes - cum_nodes << std::endl;
    }

    std::cout << std::endl << "Total leaf nodes visited: " << leaf_nodes <<
        std::endl;

    return leaf_nodes;
}

/**
    @brief Given a board and depth value, performs a basic perft test
           on the move generator and returns the number of leaf nodes
           found of just captures. This function prints out what it's doing.

    @param board is the board to perform the test on.
    @param depth is the depth to which to search to.

    @return uint64 value corresponding to the number of leaf nodes visited of
            just captures.
*/

uint64 perform_perftc_verbose(Board& board, unsigned int depth)
{
    assert(depth != 0);

    uint64 leaf_nodes = 0;

    MoveList ml = gen_captures(board);

    unsigned int movegen_count = ml.list.size(), move, num_moves = 0;

    std::cout << "Performing capture perft to depth " << depth << ":" <<
        std::endl << std::endl;

    for(unsigned int i = 0; i < movegen_count; i++)
    {
        move = ml.list.at(i).move;
        if(!make_move(board, move)) continue;

        if(board.hash_key != gen_hash(board))
            std::cout << "HASH ERROR!" << std::endl;

        num_moves++;
        uint64 cum_nodes = leaf_nodes;
        perftc(board, leaf_nodes, depth - 1);
        undo_move(board);

        std::cout << "Move " << num_moves << ": " << COORD_MOVE(move) <<
            " > " << leaf_nodes - cum_nodes << std::endl;
    }

    std::cout << std::endl << "Total leaf nodes visited: " << leaf_nodes <<
        std::endl;

    return leaf_nodes;
}