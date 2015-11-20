/*
    Cortex - Self-learning Chess Engine
    @filename movegen.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief Generates moves given a board position.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 15/11/2015 0.1.0 Initial version.
*/

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <string> // std::string
#include <vector> // std::vector

#include "defs.h"
#include "board.h" // Board structure.
#include "move.h" // Move structure.

// Structures

/**
    @brief A simple structure to store generated move lists.

    This is a simple structure that stores a vector of moves along with
    a bitboard that represents all the cells on the board which are
    under attack. This helps with checking whether the king is in check.

    @var list is a vector of Move objects, storing the list of moves.
    @var attacked is a bitboard representation of the pieces on the board
         that are currently under attack (by the side the moves are being
         generated for).
*/

struct MoveList
{
    std::vector<Move> list; // Vector of moves.
    uint64 attacked; // Bitboard representation of all pieces under attack.

    MoveList()
    :list(), attacked(0ULL)
    {};
};

// External function definitions

// Returns a 'pretty' version of the move list for standard output.

extern std::string pretty_move_list(const std::vector<Move>& list);

extern MoveList gen_moves(const Board& board); // Generate all moves.

#endif // MOVEGEN_H