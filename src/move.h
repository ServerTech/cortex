/*
    Cortex - Self-learning Chess Engine
    @filename move.h
    @author Shreyas Vinod
    @version 0.1.0

    @brief Keeps key structures for handling moves, especially during move
           generation.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 15/11/2015 0.1.0 Initial version.
*/

/**
    @file
    @filename move.h
    @author Shreyas Vinod

    @brief Keeps key structures for handling moves, especially during move
           generation.
*/

#ifndef MOVE_H
#define MOVE_H

#include "defs.h"

#include <string> // std::string and std::to_string()HO

// Globals

extern const unsigned int MFLAGEP; // En passant flag mask.
extern const unsigned int MFLAGPS; // Pawn start flag mask.
extern const unsigned int MFLAGCA; // Castling flag mask.

// Structures

/**
    @struct Move

    @brief Holds a representation of a move.

    The Move structure holds all the information required to describe a move
    in a memory-efficient manner. It also includes a score variable to help
    with move ordering for optimal search.

    @var Move::move
         An integer representation of a move. A detailed description of the
         representation can be found in the code comments.
    @var Move::score
         The score value assigned by the move generation for move ordering.
*/

/*
    Move Representation

    Moves are represented in the structure called 'Move' using two
    32-bit integers. The 'info' integer holds information about the
    move itself in the following representation for each of its
    bits: 

    0000 0000 0000 0000 0000 0000 00xx xxxx -> From cell; mask: 0x3f
    0000 0000 0000 0000 0000 xxxx xx00 0000 -> To cell; mask: >> 6 0x3f
    0000 0000 0000 0000 xxxx 0000 0000 0000 -> Captured piece; mask: >> 12 0xf
    0000 0000 0000 000x 0000 0000 0000 0000 -> En passant flag; mask: 0x10000
    0000 0000 000x xxx0 0000 0000 0000 0000 -> Promoted piece; mask: >> 17 0xf
    0000 0000 00x0 0000 0000 0000 0000 0000 -> Pawn start flag; mask: 0x200000
    0000 0000 0x00 0000 0000 0000 0000 0000 -> Castle flag; mask: 0x400000
    xxxx xxxx x000 0000 0000 0000 0000 0000 -> Unused.

    The en passant flag is a flag to denote that the move is an en passant
    capture move.

    Captured piece stores the type of piece captured, if any.

    Promoted piece stores the type of piece a pawn is being promoted to, if so.

    The pawn start flag denotes whether the move involves a pawn moving forward
    two squares.

    The castle flag denotes whether the move is a castling move.

    If there is piece to store, the value will equal 'EMPTY' (14).

    The 'score' integer holds a score value, for sorting during search.
*/

struct Move
{
    unsigned int move; // Move represented in standard convention.
    unsigned int score; // Score assigned to the move for ordering.

    Move(unsigned int m, unsigned int s)
    :move(m), score(s)
    {}
};

// Helper functions to dissect moves

/**
    @brief Returns the departure cell index in LERF layout of the given move.

    @param move is the integer value representing the move.

    @return unsigned int value representing the index of the departure cell
            for the move in LERF layout.
*/

inline unsigned int DEP_CELL(unsigned int move)
{
    return move & 0x3f;
}

/**
    @brief Returns the destination cell index in LERF layout of the given move.

    @param move is the integer value representing the move.

    @return unsigned int value representing the index of the destination cell
            for the move in LERF layout.
*/

inline unsigned int DST_CELL(unsigned int move)
{
    return (move >> 6) & 0x3f;
}

/**
    @brief Returns the type of piece that is captured, if any, in the given
           move in standard convention.

    @param move is the integer value representing the move.

    @return unsigned int value representing the type of piece that is
            captured in the move represented by 'move' in standard convention.
            If there is no capture, returns 'EMPTY' (14).
*/

inline unsigned int CAPTURED(unsigned int move)
{
    return (move >> 12) & 0xf;
}

/**
    @brief Returns the type of piece that a pawn was promoted to, if a
           promotion occurred, in the given move in standard convention.

    @param move is the integer value representing the move.

    @return unsigned int value representing the type of piece that a
            pawn was promoted to, if a promotion occurred, in the move
            represented by 'move' in standard convention. If a promotion
            did not occur, returns 'EMPTY' (14).
*/

inline unsigned int PROMOTED(unsigned int move)
{
    return (move >> 17) & 0xf;
}

/**
    @brief Boolean check on whether the given move is a capture.

    @param move is the integer value representing the move.

    @return bool representing whether 'move' is a capture.
*/

inline bool IS_CAP(unsigned int move)
{
    if(CAPTURED(move) == EMPTY) return 0; else return 1;
}

/**
    @brief Boolean check on whether the given move in an en passant capture.

    @param move is the integer value representing the move.

    @return bool representing whether 'move' is an en passant capture.
*/

inline bool IS_ENPAS_CAP(unsigned int move)
{
    return move & 0x10000;
}

/**
    @brief Boolean check on whether the given move is a promotion.

    @param move is the integer value representing the move.

    @return bool representing whether 'move' is a promotion.
*/

inline bool IS_PROM(unsigned int move)
{
    if(PROMOTED(move) == EMPTY) return 0; else return 1;
}

/**
    @brief Boolean check on whether the given move is a pawn start, that is,
           the pawn in question moved forward two squares.

    @param move is the integer value representing the move.

    @return bool representing whether 'move' is a pawn start.
*/

inline bool IS_PSTR(unsigned int move)
{
    return move & 0x200000;
}

/**
    @brief Boolean check on whether the given move is a castling move.

    @param move is the integer value representing the move.

    @return bool representing whether 'move' is a castling move.
*/

inline bool IS_CAS(unsigned int move)
{
    return move & 0x400000;
}

/**
    @brief Given parameters, returns a valid integer representation of the
           move.

    @param dep is the departure cell index.
    @param dst is the destination cell index.
    @param cap_piece is the type of piece captured in standard convention.
    @param prom_piece is the type of piece promoted to in standard convention.
    @param flag is any of the three move flags (MFLAGEP; MFLAGPS; MFLAGCA).

    @return unsigned int value representing the move defined by the given
            parameters.

    @warning 'cap_piece' should be EMPTY (14) if none.
    @warning 'prom_piece' should be EMPTY (14) if none.
    @warning 'flag' should be zero if none.
*/

inline unsigned int GET_MOVE(unsigned int dep, unsigned int dst,
    unsigned int cap_piece, unsigned prom_piece, unsigned int flag)
{
    return dep | (dst << 6) | (cap_piece << 12) | (prom_piece << 17) | flag;
}

/**
    @brief Given an index in LERF layout, returns a string in
           pure algebraic notation (coordinate notation).

    @param index is an integer value denoting the index to find
           the algebraic notation equivalent for, in LERF layout.

    @return string conversion of the index to pure algebraic notation.

    @warning 'index' must be beween (or equal to) 0 and 63.
    @warning 'index' must be represented in LERF layout.
*/

inline std::string COORD(unsigned int index)
{
    assert(index < 64);

    return char('a' + (index % 8)) + std::to_string((index / 8) + 1);
}

/**
    @brief Given a move, returns a string representing the move in
           pure algebraic notation (coordinate notation).

    @param move is the integer value representing the move.

    @return string representation of the move in pure algebraic notation.

    @warning Promoted piece is an 'e' if there was an error parsing the
             move.
*/

inline std::string COORD_MOVE(unsigned int move)
{
    if(move == NO_MOVE) return "NO_MOVE";

    std::string alg_coord;
    int promoted = PROMOTED(move);

    alg_coord += COORD(DEP_CELL(move));
    alg_coord += COORD(DST_CELL(move));

    if(promoted != 14)
    {
        switch(PROMOTED(move))
        {
            case wQ:
            case bQ: alg_coord += 'q'; break;
            case wR:
            case bR: alg_coord += 'r'; break;
            case wN:
            case bN: alg_coord += 'n'; break;
            case wB:
            case bB: alg_coord += 'b'; break;
            default: alg_coord += 'e'; // Denotes an error.
        }
    }

    return alg_coord;
}

#endif // MOVE_H