/*
    Cortex - Self-learning Chess Engine
    @filename move.h
    @author Shreyas Vinod
    @version 2.0.0

    @brief A simple data structure to store a move.

    Limitations:
    Considered creating an inherited class for capture moves, but rejected
    the idea due to std::vector<> technicalities and overhead associated
    with storing vectors of objects of base and inherited objects together.
    Because millions of objects of this class are bound to be created,
    overhead is kept to a minimum by reducing error detection. Checking
    for move legality is not a responsibility of the class.

    ******************** VERSION CONTROL ********************
    * 27/07/2015 File created.
    * 28/07/2015 1.0.0 Initial version.
    * 01/07/2015 1.1.0 Added information storage for captures.
    * 03/08/2015 1.2.0 Complete rewrite.
        * Fixed messy code and removed silly exceptions.
    * 06/11/2015 1.2.1 Replaced 'short' with 'int'. Integers
        are more natural to memory.
    * 11/11/2015 2.0.0 Complete rewrite.
        * Fixed messy code.
        * 'assert()' is now well and truly used, as it should be.
        * Exceptions have been entirely removed.
        * Now uses 'uint64_t' from stdint.h instead of 'U64'.
*/

#ifndef MOVE_H
#define MOVE_H

#include <stdint.h> // uint64_t.

/**
    @brief A simple data structure to store a move.

    Limitations:
    Considered creating an inherited class for capture moves, but rejected
    the idea due to std::vector<> technicalities and overhead associated
    with storing vectors of objects of base and inherited objects together.
    Because millions of objects of this class are bound to be created,
    overhead is kept to a minimum by reducing error detection. Checking
    for move legality is not a responsibility of the class.
*/

class Move
{

public:

    Move(int dep_cell, int dest_cell); // Construct with indices.
    Move(int dep_cell, int dest_cell, int what_piece_cap); // Capture move.
    Move(const Move& move_obj); // Construct by copy.
    bool is_capture() const; // Returns capture flag.
    int what_piece() const; // Returns type of captured piece.
    int get_dep_cell() const; // Returns departure cell index.
    int get_dest_cell() const; // Returns destination cell index.
    uint64_t get_dep_bb() const; // Returns departure cell bitboard.
    uint64_t get_dest_bb() const; // Returns destination cell bitboard.

private:

    const int dep, dest; // Departure and destination cells.
    const bool cap_flag; // This flag is set if the move is a capture.
    const int cap_piece; // Type of captured piece in standard convention.
};

#endif // MOVE_H