/*
    Cortex - Self-learning Chess Engine
    @filename movegen.h
    @author Shreyas Vinod
    @version 0.1.2

    @brief Generates moves given a board position.

    Includes structures and functions that help generate and store
    pseudo-legal (and legal) moves for given game states.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 24/11/2015 0.1.0 Initial version.
    * 29/11/2015 0.1.1 Added functions to generate just captures.
    * 05/12/2015 0.1.2 Added functions to generate legal moves and captures.
*/

/**
    @file
    @filename movegen.h
    @author Shreyas Vinod

    @brief Generates moves given a board position.

    Includes structures and functions that help generate and store
    pseudo-legal (and legal) moves for given game states.
*/

#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "defs.h"

#include <string> // std::string
#include <vector> // std::vector

#include "board.h" // Board structure.
#include "move.h" // Move structure.

// Structures

/**
    @struct MoveList

    @brief A simple structure to store generated move lists.

    This is a simple structure that stores a vector of moves along with
    a bitboard that represents all the cells on the board which are
    under attack. This helps with checking whether the king is in check.

    @var MoveList::list
         A vector of Move objects, storing the list of moves.
    @var MoveList::attacked
         A bitboard representation of the pieces on the board that are
         currently under attack (by the side the moves are being generated
         for).
*/

struct MoveList
{
    std::vector<Move> list; // Vector of moves.
    uint64 attacked; // Bitboard representation of all pieces under attack.

    MoveList()
    :list(), attacked(0ULL)
    {
        list.reserve(64); // Reserve enough memory for 64 moves.
    };
};

// External function declarations

extern void init_mvv_lva(); // Initialise MVV-LVA scores table.

// Returns a 'pretty' version of the move list for standard output.

extern std::string pretty_move_list(const std::vector<Move>& list);

// Generate moves for rooks/line moves for queens.

extern void gen_rook_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
extern void gen_rook_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);

// Generate moves for knights.

extern void gen_knight_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
extern void gen_knight_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);

// Generate moves for bishops/diagonal moves for queens.

extern void gen_bishop_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
extern void gen_bishop_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);

// Generate moves for pawns.

extern void gen_pawn_moves(bool gen_side, MoveList& ml, const Board& board);
extern void gen_pawn_cap_moves(bool gen_side, MoveList& ml,
    const Board& board);

// Generate moves for kings.

extern void gen_king_moves(bool gen_side, MoveList& ml, const Board& board);
extern void gen_king_cap_moves(bool gen_side, MoveList& ml,
    const Board& board);

// Check if a cell is under attack.

extern bool is_sq_attacked(unsigned int index, bool gen_side,
    const Board& board);

extern MoveList gen_moves(const Board& board); // Generate all moves.
extern MoveList gen_captures(const Board& board); // Generate all captures.
extern MoveList gen_legal_moves(Board& board); // Generate legal moves.

// Generate legal captures.

extern MoveList gen_legal_captures(Board& board);

#endif // MOVEGEN_H