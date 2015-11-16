/*
    Cortex - Self-learning Chess Engine
    @filename movegen.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Generates moves given a board position.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 15/11/2015 0.1.0 Initial version.
*/

#include <sstream> // std::stringstream

#include "movegen.h"
#include "lookup_tables.h"

// Functions

/**
    @brief Converts a move list vector into a 'pretty' string.

    Converts a given move list vector into a 'pretty' string suitable for
    standard output. Useful for debugging or command line operation
    of the engine

    @param list is the move list vector to convert into a 'pretty' string.

    @return string which is a 'pretty' version of the move list and comes
            pre-packed with newline characters. Suitable for printing out of
            the box.
*/

std::string pretty_move_list(const std::vector<Move>& list)
{
    std::stringstream pretty_str;

    int s = list.size();

    for(int i = 0; i < s; i++)
    {
        pretty_str << "Move " << i << ": " << COORD_MOVE(list.at(i).move);
        pretty_str << "    Score: " << list.at(i).score << "\n";
    }

    pretty_str << "\nTotal moves: " << s;

    return pretty_str.str();
}

/**
    @brief Pushes a quiet move to the move list vector.

    @param list is the move list vector.
    @param move is an integer value representing a move.
    @param board is the board the move is being made on.

    @return void.
*/

void push_quiet_move(std::vector<Move>& list, unsigned int move,
    const Board& board)
{
    Move move_to_add(move, 0);
    list.push_back(move_to_add);
}

/**
    @brief Pushes a capture move to the move list vector.

    @param list is the move list vector.
    @param move is an integer value representing the move.
    @param board is the board the move is being made on.

    @return void.
*/

void push_capture_move(std::vector<Move>& list, unsigned int move,
    const Board& board)
{
    Move move_to_add(move, 0);
    list.push_back(move_to_add);
}

/**
    @brief Pushes an en passant capture move to the move list vector.

    @param list is the move list vector.
    @param move is an integer value representing the move.
    @param board is the board the move is being made on.

    @return void.
*/

void push_en_pas_cap_move(std::vector<Move>& list, unsigned int move,
    const Board& board)
{
    Move move_to_add(move, 0);
    list.push_back(move_to_add);
}

/**
    @brief Generates and returns a sorted move list vector of all the possible
           pseudo-legal moves for the given board state.

    @param board is the board to generate all pseudo-legal moves for.

    @return std::vector<Move> representing a sorted collection of all
            pseudo-legal moves for the given board state.
*/

MoveList gen_moves(const Board& board)
{
    MoveList ml;
    ml.list.reserve(50); // Reserve enough memory for fifty moves.

    

    return ml;
}