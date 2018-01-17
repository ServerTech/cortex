/*
    Cortex - Self-learning Chess Engine
    @filename bitboard.h
    @author Shreyas Vinod
    @version 3.0.2

    @brief The Bitboard class handles bitboards extensively.

    Extensive bitboard class with a full-board representation. Can be
    initialised with a char array. Also capable of making and unmaking
    moves. Based on Little-Endian Rank-File mapping (LERF).

    ******************** VERSION CONTROL ********************
    * 06/07/2015 File created.
    * 28/07/2015 1.0.0 Initial version.
    * 28/07/2015 1.1.0 Added exception handling.
    * 28/07/2015 1.2.0 Move history enhancements.
        * Class now keeps full move history in a vector and allows undo()
          to the beginning.
    * 02/08/2015 2.0.0 Bug fixes and speed overhaul.
        * Fixed significant bug in Bitboard::move(const Move&). Now useful,
          as it supports captures.
        * Speed overhaul owing to the fact that the collection of bitboards
          is now an array, reducing switch-case ugliness and branch prediction
          uncertainties.
        * Added Bitboard::determine_type(U64).
        * Added Bitboard::get_char_array().
        * Added Bitboard::is_castling_legal(int).
        * Added Bitboard::current_ply().
    * 07/11/2015 3.0.0 Complete rewrite.
        * Fixed messy code.
        * 'assert()' is now well and truly used, as it should be.
        * Exceptions have been removed entirely.
        * Removed ability to select layout. LERF is now the standard.
        * As a result, removed Bitboard::translate_index(int).
        * Two stage construction is bad; removed Bitboard::init(const char*).
        * Renamed Bitboard::current_ply() to Bitboard::get_ply().
        * Now uses 'uint64_t' from stdint.h instead of 'U64'.
        * Sides are now referred to as 'me' and 'enemy'.
        * Added Bitboard::get_lerf_index(uint64_t).
        * Added Bitboard::get_be_index(uint64_t).
        * Added Bitboard::determine_type(int).
        * Added Bitboard::conv_char(uint64_t).
        * Better documentation!
    * 08/11/2015 3.0.1 Restructuring.
        * Moved Bitboard::get_lerf_index(uint64_t) to index_proc.cpp.
        * Moved Bitboard::get_be_index(uint64_t) to index_proc.cpp.
    * 11/11/2015 3.0.2 C++ standard conformity changes.
        * Removed Bitboard::determine_type(int) (it's ambiguous).
        * Removed Bitboard::conv_char(uint64_t) (it's ambiguous).
*/

#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdint.h> // uint64_t.
#include <string>
#include <vector>
#include "move.h"

/**
    @brief The Bitboard class handles bitboards extensively.

    Extensive bitboard class with a full-board representation. Can be
    initialised with a char array. Also capable of making and unmaking
    moves. Based on Little-Endian Rank-File mapping (LERF).

    Unless otherwise mentioned, the standard indexing convention is:
    0: Me; 1: Enemy; 2: My Pawns; 3: My Rooks;
    4: My Knights; 5: My Bishops; 6: My Queens;
    7: My King; 8: Enemy Pawns; 9: Enemy Rooks;
    10: Enemy Knights; 11: Enemy Bishops; 12: Enemy Queens;
    13: Enemy King; 14: Occupied; 15: Free; 16: None.
*/

class Bitboard
{

public:

    Bitboard(bool is_w, unsigned int c_ply, bool mk_cr, bool mq_cr,
        bool ek_cr, bool eq_cr, const char vis_board[64]); // Constructor.
    Bitboard(const Bitboard& board_obj); // Construct by copy.
    uint64_t get_val(int sel) const; // Accessor.
    void set_val(int sel, uint64_t val); // Mutator.
    bool get_side() const; // True for white, false for black.
    int determine_type(uint64_t bit_chk) const; // Determine piece type.
    bool is_castling_legal(int index) const; // Checks if castling is legal.
    unsigned int get_ply() const; // Gets the current ply value.
    char conv_char(int index) const; // Character conversion of a cell.
    std::string pretty() const; // A 'pretty' bitboard for output.
    void move(const Move& pros_move); // Make a move.
    void undo(); // Unmake the previous move.

private:

    const bool is_white; // The engine's side; false for black.
    unsigned int ply; // Stores the current ply where the game is at.
    bool me_k_castling, me_q_castling; // Castling rights for the engine.
    bool ey_k_castling, ey_q_castling; // Casting rights for the enemy.
    std::vector<Move> move_history; // Complete move history.

    uint64_t chessboard[17]; // Board, with the last value being a sentinel.

    void update(); // Updates all dependent bitboards.
};

#endif // BITBOARD_H