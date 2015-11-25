/*
    Cortex - Self-learning Chess Engine
    @filename perft.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Performs basic perft testing on the move generator.

    ******************** VERSION CONTROL ********************
    * 22/11/2015 File created.
    * 23/11/2015 0.1.0 Initial version.
*/

#include "debug.h"

#include <iostream>

#include "perft.h"
#include "movegen.h"
#include "move.h"

// Prototypes

uint64 perform_perft(Board& board, unsigned int depth);
void perft(Board& board, uint64& leaf_nodes, unsigned int depth);
uint64 perform_perft_verbose(Board& board, unsigned int depth);

// Functions

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
    uint64 leaf_nodes = 0;

    perft(board, leaf_nodes, depth);

    return leaf_nodes;
}

/**
    @brief Recursive function that performs a basic perft test on the
           move generator to count the number of leaf nodes. Not meant
           to be called directly.

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
        perft(board, leaf_nodes, depth - 1);
        undo_move(board);
    }

    return;
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
    uint64 leaf_nodes = 0;

    MoveList ml = gen_moves(board);

    unsigned int movegen_count = ml.list.size(), move, num_moves = 0;

    std::cout << "Performing perft to depth " << depth << ":" << std::endl <<
        std::endl;

    for(unsigned int i = 0; i < movegen_count; i++)
    {
        move = ml.list.at(i).move;
        if(!make_move(board, move)) continue;
        num_moves++;
        uint64 cum_nodes = leaf_nodes;
        perft(board, leaf_nodes, depth - 1);
        undo_move(board);

        std::cout << "Move " << num_moves << ": " << COORD_MOVE(move) <<
            " > " << leaf_nodes - cum_nodes << std::endl;
    }

    std::cout << std::endl << "Total nodes visited: " << leaf_nodes <<
        std::endl;

    return leaf_nodes;
}