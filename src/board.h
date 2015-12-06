/*
    Cortex - Self-learning Chess Engine
    @filename board.h
    @author Shreyas Vinod
    @version 0.4.8

    @brief Handles the board representation for the engine.

    Extensive board handling with a bitboard representation. Can be initialised
    with a FEN (Forsyth–Edwards Notation) string. Keeps track of moves,
    en passant squares, castling permissions, move history, and more. Based on
    Little-Endian Rank-File mapping (LERF).

    ******************** VERSION CONTROL ********************
    * 06/07/2015 File created.
    * 28/07/2015 0.1.0 Initial version.
    * 28/07/2015 0.1.1 Added exception handling.
    * 28/07/2015 0.1.2 Move history enhancements.
        * Class now keeps full move history in a vector and allows undo()
          to the beginning.
    * 02/08/2015 0.2.0 Bug fixes and speed overhaul.
        * Fixed significant bug in Bitboard::move(const Move&). Now useful,
          as it supports captures.
        * Speed overhaul owing to the fact that the collection of bitboards
          is now an array, reducing switch-case ugliness and branch prediction
          uncertainties.
        * Added Bitboard::determine_type(U64).
        * Added Bitboard::get_char_array().
        * Added Bitboard::is_castling_legal(int).
        * Added Bitboard::current_ply().
    * 07/11/2015 0.3.0 Complete rewrite.
        * Fixed messy code.
        * std::assert() is now well and truly used, as it should be.
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
    * 08/11/2015 0.3.1 Restructuring.
        * Moved Bitboard::get_lerf_index(uint64_t) to index_proc.cpp.
        * Moved Bitboard::get_be_index(uint64_t) to index_proc.cpp.
    * 11/11/2015 0.3.2 C++ standard conformity changes.
        * Removed Bitboard::determine_type(int) (it's ambiguous).
        * Removed Bitboard::conv_char(uint64_t) (it's ambiguous).
    * 14/11/2015 0.4.0 Complete rewrite.
        * Can no longer be constructed with a 64 element char array.
        * Now uses structures.
        * Supports initialisation with a FEN string.
        * Better support for disabling std::assert() with #define NDEBUG.
        * Now uses typedef unsigned long long instead of 'uint64_t'.
    * 23/11/2015 0.4.1 Added the ability to make and unmake (undo) moves.
    * 25/11/2015 0.4.2 Added parse_move(Board&, std::string).
    * 28/11/2015 0.4.3 Added PV capabilities and history/killer heuristics.
    * 02/12/2015 0.4.4 Added extra functions for null move pruning.
        * Added make_null_move(Board&).
        * Added undo_null_move(Board&).
    * 02/12/2015 0.4.5 Added transposition table.
    * 04/12/2015 0.4.6 Added FEN parsing for fifty move and ply counters.
    * 06/12/2015 0.4.7 Added board_flipv(Board&).
    * 06/12/2015 0.4.8 pretty_board(Board&) now prints evaluation score.
*/

/**
    @file
    @filename board.h
    @author Shreyas Vinod

    @brief Handles the board representation for the engine.

    Extensive board handling with a bitboard representation. Can be initialised
    with a FEN (Forsyth–Edwards Notation) string. Keeps track of moves,
    en passant squares, castling permissions, move history, and more. Based on
    Little-Endian Rank-File mapping (LERF).
*/

#ifndef BOARD_H
#define BOARD_H

#include "defs.h"

#include <string> // std::string
#include <vector> // std::vector

#include "hash_table.h"

// Structures

/**
    @struct UndoMove

    @brief Structure to help undo a move.

    This structure holds enough information to be able to correctly undo a
    move that is made.

    @var UndoMove::move
         An integer representation of the move that was made.
    @var UndoMove::castle_perm
         Represents the castling permissions before the move was made.
    @var UndoMove::en_pas_sq
         Represents the LERF index of the en passant cell, before the move was
         made.
    @var UndoMove::fifty
         Counter value to keep track of the fifty-move rule, before the move
         was made.
    @var UndoMove::hash_key
         The 64-bit zobrist hash of the board position before the move was
         made. Used to keep track of threefold repetition.

    @warning 'en_pas_sq' or the en passant square should be set to value
             'NO_SQ' (64) when there is no en passant square.
    @warning 'castle_perm', 'en_pas_sq', 'fifty' and 'hash_key' refer to
             values BEFORE the move represented by 'move' was actually made.
*/

struct UndoMove
{
    unsigned int move; // The move that was made.
    unsigned int castle_perm; // Castle permissions BEFORE the move.
    unsigned int en_pas_sq; // En passant square, BEFORE the move.
    unsigned int fifty; // Fifty-move rule counter, BEFORE the move.
    uint64 hash_key; // Hash key of the board BEFORE the move was made.

    UndoMove()
    :move(0), castle_perm(15), en_pas_sq(NO_SQ), fifty(0), hash_key(0)
    {}

    UndoMove(unsigned int m, unsigned int cp, unsigned int enpsq,
        unsigned int f, uint64 hk)
    :move(m), castle_perm(cp), en_pas_sq(enpsq), fifty(f), hash_key(hk)
    {}
};

/**
    @struct Board

    @brief Board structure.

    This structure holds the entire game state at any given point in time.
    Refer to the code comments for detailed descriptions.

    @var Board::side
         The side whose turn it is to play. Set to true if white.
    @var Board::ply
         The number of half-moves in the current search.
    @var Board::his_ply
         The number of half-moves in the history of the game.
    @var Board::castle_perm
         Stores castling permissions for both sides efficiently.
    @var Board::en_pas_sq
         Stores the en passant square, if any, or 'NO_SQ' (64) otherwise.
    @var Board::fifty
         A counter that helps keep track of the fifty-move rule.
    @var Board::hash_key
         The zobrist 64-bit hash key for the current game state. Used to keep
         track of threefold repetition and in the transposition table.
    @var Board::history
         A vector of 'UndoMove' structures to help with reverting to a previous
         state, if necessary, which is mostly used in search to unmake moves.
    @var Board::chessboard
         A 14 element array of 64-bit unsigned integers, each storing the state
         of the board in bitboard representation, indexed in standard
         convention.
    @var Board::t_table
         The transposition hash table.
    @var Board::pv_array
         Stores the current best PV line obtained from the transposition table.
    @var Board::search_history
         An array used for the history heuristic in move ordering.
    @var Board::search_killers
         An array used for the killer heuristic in move ordering.

    @warning Do NOT have more than king for each side. Although this is not
             checked, the consequence of having multiple kings is undefined for
             the engine in its entirety. It just won't work.
    @warning Do NOT place pawns on ranks one or eight. This is an
             impossible occurrence, and the engine will fail to promote them.
             In fact, they might vanish.
    @warning White must start at the bottom of the board, or move generation
             for pawns will not function correctly.
    @warning 'en_pas_sq' or the en passant square should be set to value
             'NO_SQ' (64) when there is no en passant square.
    @warning 'ply' is for use by search only, while 'his_ply' stores the
             current ply value of the actual game.
    @warning Since the killer heuristic array is indexed by depth, it assumes
             the maximum search depth to be 'MAX_DEPTH'.
*/

/*
    Standard Indexing Convention

    Unless otherwise mentioned, the standard indexing convention is:

    0: white pawns; 1: white rooks; 2: white knights; 3: white bishops;
    4: white queen(s); 5: white king; 6: black pawns; 7: black rooks;
    8: black knights; 9: black bishops; 10: black queen(s); 11: black king;
    12: all white pieces; 13: all black pieces; 14: empty.

    Castle Permissions

    'castle_perm' is a 32-bit unsigned integer, out of which only the least
    significant four bits are used in the following manner:

    000x -> Black queen-side castling; mask: 0x1
    00x0 -> Black king-side castling; mask: 0x2
    0x00 -> White queen-side castling; mask: 0x4
    x000 -> White king-side castling; mask: 0x8
*/

struct Board
{
    bool side; // Side to play next; true for white.

    unsigned int ply; // Number of half-moves in the current search.
    unsigned int his_ply; // Number of half-moves in the history of the game.

    unsigned int castle_perm; // Castle permissions.

    unsigned int en_pas_sq; // En passant square, if any, 'NO_SQ' otherwise.

    unsigned int fifty; // Counter to keep track of the fifty-move rule.

    uint64 hash_key; // 64-bit zobrist hash key for the board position.

    std::vector<UndoMove> history; // Move history for undo purposes.

    uint64 chessboard[14]; // Board representation.

    TranspositionTable t_table; // Principal Variation (PV) hash table.
    unsigned int pv_array[MAX_DEPTH]; // PV line array.

    unsigned int search_history[12][64]; // Array for history heuristics.
    unsigned int search_killers[2][MAX_DEPTH]; // Array for killer heuristics.

    Board()
    :side(WHITE), ply(0), his_ply(0), castle_perm(15), en_pas_sq(NO_SQ),
        fifty(0), hash_key(0ULL), history(), t_table()
    {
        history.reserve(256);

        for(unsigned int i = 0; i < 14; i++) chessboard[i] = 0ULL;

        for(unsigned int i = 0; i < 12; i++)
        {
            for(unsigned int j = 0; j < 64; j++)
                search_history[i][j] = 0;
        }

        for(unsigned int i = 0; i < 2; i++)
        {
            for(unsigned int j = 0; j < MAX_DEPTH; j++)
                search_killers[i][j] = 0;
        }

        for(unsigned int i = 0; i < MAX_DEPTH; i++)
            pv_array[i] = 0;
    }

    Board(bool s, unsigned int p, unsigned int hp, unsigned int cp,
        unsigned int enpsq, unsigned int f, uint64 hk)
    :side(s), ply(p), his_ply(hp), castle_perm(cp), en_pas_sq(enpsq),
        fifty(f), hash_key(hk), history(), t_table()
    {
        history.reserve(256);

        for(unsigned int i = 0; i < 14; i++) chessboard[i] = 0ULL;

        for(unsigned int i = 0; i < 12; i++)
        {
            for(unsigned int j = 0; j < 64; j++)
                search_history[i][j] = 0;
        }

        for(unsigned int i = 0; i < 2; i++)
        {
            for(unsigned int j = 0; j < MAX_DEPTH; j++)
                search_killers[i][j] = 0;
        }

        for(unsigned int i = 0; i < MAX_DEPTH; i++)
            pv_array[i] = 0;
    }
};

// Helper functions

/**
    @brief Updates the 'all white' and 'all black' bitboards.

    @param board is the board on which to update the bitboards.

    @return void.
*/

inline void update_secondary(Board& board)
{
    board.chessboard[ALL_WHITE] =
        board.chessboard[wP] | board.chessboard[wR] | board.chessboard[wN] |
        board.chessboard[wB] | board.chessboard[wQ] | board.chessboard[wK];

    board.chessboard[ALL_BLACK] =
        board.chessboard[bP] | board.chessboard[bR] | board.chessboard[bN] |
        board.chessboard[bB] | board.chessboard[bQ] | board.chessboard[bK];
}

// External function declarations

extern void reset_board(Board& board); // Resets the board.

// Parse FEN

extern bool parse_fen(Board& board, const std::string fen, unsigned int& i);

// Determine type of piece.

extern unsigned int determine_type(const Board& board, uint64 bit_chk);

// Convert piece at cell indexed by 'index' to a character.

extern char conv_char(const Board& board, unsigned int index);

// Returns a 'pretty' version of the board for standard output.

extern std::string pretty_board(Board& board);

extern bool make_move(Board& board, unsigned int move); // Make a move.
extern void undo_move(Board& board); // Unmake (undo) the previous move.
extern void make_null_move(Board& board); // Make a null move.
extern void undo_null_move(Board& board); // Unmake (undo) the null move.

// Parse a move in pure algebraic notation.

extern unsigned int parse_move(Board& board, std::string str_move);

// Probe and fill the PV line array.

extern unsigned int probe_pv_line(Board& board, unsigned int depth);

// Flip board vertically for evaluation purposes.

extern void board_flipv(Board& board);

#endif // BOARD_H