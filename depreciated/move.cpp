/*
    Cortex - Self-learning Chess Engine
    @filename move.cpp
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

#include <assert.h> // assert().
#include "move.h"

/**
    @brief Constructs an object representing a non-capture move.

    @param dep_cell is an integer denoting the departure cell index.
    @param dest_cell is an integer denoting the destination cell index.

    @warning 'dep_cell' and 'dest_cell' cannot be equal.
    @warning 'dep_cell' must be between (or equal to) 0 and 63.
    @warning 'dest_cell' must be between (or equal to) 0 and 63.
*/

Move::Move(int dep_cell, int dest_cell)
:dep(dep_cell), dest(dest_cell), cap_flag(0), cap_piece(-1)
{
    assert(!(dep_cell == dest_cell)); // Cannot be equal.
    assert(dep_cell >= 0 && dep_cell < 64); // Should be a valid index.
    assert(dest_cell >= 0 && dest_cell < 64); // Should be a valid index.
}

/**
    @brief Constructs an object representing a capture move.

    This is an overloaded function that constructs an object that represents
    a capture move, instead of a non-capture move.

    @param dep_cell is an integer denoting the departure cell index.
    @param dest_cell is an integer denoting the destination cell index.
    @param what_piece is an integer denoting the type of piece captured
           in standard convention.

    @warning 'dep_cell' and 'dest_cell' cannot be equal.
    @warning 'dep_cell' must be between (or equal to) 0 and 63.
    @warning 'dest_cell' must be between (or equal to) 0 and 63.
    @warning 'what_piece_cap' must be a valid integer in standard convention.
    @warning This function does not check whether the captured piece belongs
             to the opposite side, or legality checks of any kind.
*/

Move::Move(int dep_cell, int dest_cell, int what_piece)
:dep(dep_cell), dest(dest_cell), cap_flag(1), cap_piece(what_piece)
{
    assert(!(dep_cell == dest_cell)); // Cannot be equal.
    assert(dep_cell >= 0 && dep_cell < 64); // Should be a valid index.
    assert(dest_cell >= 0 && dest_cell < 64); // Should be a valid index.
    assert(what_piece >= 2 && what_piece <= 13); // Should be valid.
}

/**
    @brief Copy constructor.

    This is an overloaded function that copies an already existing object.

    @param move_obj is the Move object to copy from.
*/

Move::Move(const Move& move_obj)
:dep(move_obj.dep), dest(move_obj.dest),
cap_flag(move_obj.cap_flag), cap_piece(move_obj.cap_piece)
{}

/**
    @brief Checks whether the move stored in the object is a capture.

    @return bool representing whether the move stored in the object is a
            capture.
*/

bool Move::is_capture() const
{
    return Move::cap_flag;
}

/**
    @brief Returns the type of piece captured in standard convention.

    @return int value denoting type of captured piece in standard convention.
*/

int Move::what_piece() const
{
    return cap_piece;
}

/**
    @brief Returns the departure cell index.

    @return int value denoting the departure cell index.
*/

int Move::get_dep_cell() const
{
    return Move::dep;
}

/**
    @brief Returns the destination cell index.

    @return int value denoting the destination cell index.
*/

int Move::get_dest_cell() const
{
    return Move::dest;
}

/**
    @brief Returns the departure cell index, converted to a bitboard.

    @return uint64_t value with one bit set, denoting the departure cell.
*/

uint64_t Move::get_dep_bb() const
{
    return 1ULL << Move::dep;
}

/**
    @brief Returns the destination cell index, converted to a bitboard.

    @return uint64_t value with one bit set, denoting the destination cell.
*/

uint64_t Move::get_dest_bb() const
{
    return 1ULL << Move::dest;
}