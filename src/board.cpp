/*
    Cortex - Self-learning Chess Engine
    @filename board.cpp
    @author Shreyas Vinod
    @version 0.4.5

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
*/

#include "debug.h"

#include <sstream> // std::stringstream
#include <ctype.h> // isalpha() and isdigit()

#include "board.h"
#include "move.h" // COORD()
#include "movegen.h" // is_sq_attacked()
#include "hash.h" // gen_hash() and hash helper functions
#include "lookup_tables.h" // Lookup tables

// Prototypes

void reset_board(Board& board);
bool parse_fen(Board& board, const std::string fen);
unsigned int determine_type(const Board& board, uint64 bit_chk);
char conv_char(const Board& board, unsigned int index);
std::string pretty_board(const Board& board);
inline void spawn_piece(Board& board, unsigned int piece_type,
    unsigned int index);
inline void obliterate_piece(Board& board, unsigned int piece_type,
    unsigned int index);
inline void move_piece_tu(Board& board, unsigned int dep_cell,
    unsigned int dst_cell);
inline void move_piece_tk(Board& board, unsigned int piece_type,
    unsigned int dep_cell, unsigned int dst_cell);
inline void move_piece_cap(Board& board, unsigned int piece_type,
    unsigned int cap_type, unsigned int dep_cell, unsigned int dst_cell);
bool make_move(Board& board, unsigned int move);
void undo_move(Board& board);
void make_null_move(Board& board);
void undo_null_move(Board& board);
unsigned int parse_move(Board& board, std::string str_move);
inline bool move_exists(Board& board, unsigned int move);
unsigned int probe_pv_line(Board& board, unsigned int depth);

// Functions

/**
    @brief Resets the given board structure.

    @param board is the board to reset.

    @return void.

    @warning 'side' defaults to white.
    @warning 'castling_perm' defaults to zero.
*/

void reset_board(Board& board)
{
    board.side = WHITE; // Defaults to white.

    board.ply = 0;
    board.his_ply = 0;

    board.castle_perm = 0; // Defaults to zero.

    board.en_pas_sq = NO_SQ; // Set to 'NO_SQ' (64).

    board.fifty = 0;

    board.hash_key = 0ULL;

    board.history.clear(); // Clear history vector.

    for(int i = 0; i < 14; i++) board.chessboard[i] = 0ULL;
}

/**
    @brief Parses a FEN string and initialises the board.

    Pares a FEN (Forsyth–Edwards Notation) string and initialises
    the given board structure.

    @param board is the board to initialise with the FEN string.
    @param fen is the FEN string.

    @return bool value representing whether initialisation was
            successful.

    @warning Do NOT have more than king for each side. Although this is not
             checked, the consequence of having multiple kings is undefined for
             the engine in its entirety. It just won't work.
    @warning Do NOT place pawns on ranks one or eight. This is an
             impossible occurrence, and the engine will fail to promote them.
             In fact, they might vanish.
    @warning Assumes ASCII.
    @warning This will reset 'board'.
    @warning Please check if the function returns 1, else the initialisation
             failed, and you shouldn't hit your head on the wall like I did,
             trying to figure out what went wrong.
*/

bool parse_fen(Board& board, const std::string fen)
{
    int file = FILE_A, rank = RANK_8, piece, count, i = 0;
    char c;

    reset_board(board); // Reset the board to fill new values.

    // Pieces

    while((rank >= RANK_1) && (c = fen[i]))
    {
        if(isalpha(c))
        {
            switch(c)
            {
                case 'P': piece = wP; break;
                case 'p': piece = bP; break;
                case 'R': piece = wR; break;
                case 'r': piece = bR; break;
                case 'N': piece = wN; break;
                case 'n': piece = bN; break;
                case 'B': piece = wB; break;
                case 'b': piece = bB; break;
                case 'Q': piece = wQ; break;
                case 'q': piece = bQ; break;
                case 'K': piece = wK; break;
                case 'k': piece = bK; break;
                default: return 0; // Parse error.
            }

            board.chessboard[piece] |= B_FILE[file] & B_RANK[rank];
            file++;
        }
        else if(isdigit(c))
        {
            count = c - '0'; // Get the integer value by subtracting ASCII '0'.
            for(int j = 0; j < count; j++) file++;
        }
        else if(c == '/' || c == ' ')
        {
            rank--;
            file = FILE_A;
        }
        else return 0; // Parse error.

        i++;
    }

    // Side

    if((c = fen[i]) == 'w') board.side = WHITE;
    else if(c == 'b') board.side = BLACK;
    else return 0; // Parse error.

    i++;
    if(fen[i] != ' ') return 0; // Parse error.
    i++;

    // Castling permissions

    count = 0; // Used to make sure the loop isn't crazy, due to incorrect FEN.

    if(fen[i] != '-') // Castling permissions exist.
    {
        while((c = fen[i]) != ' ')
        {
            if(count == 4) return 0; // Parse error.

            switch(c)
            {
                case 'K': board.castle_perm |= WKCA; break;
                case 'Q': board.castle_perm |= WQCA; break;
                case 'k': board.castle_perm |= BKCA; break;
                case 'q': board.castle_perm |= BQCA; break;
                default: return 0; // Parse error.
            }

            count++;
            i++;
        }
    }
    else i++;

    i++;

    // En passant square

    if((c = fen[i]) != '-') // En passant square exists.
    {
        if(isalpha(c)) file = c - 'a' + 1; // Integer value for file.
        else return 0; // Parse error.

        i++;

        if(isdigit(c = fen[i])) rank = c - '1' + 1; // Integer value for rank.
        else return 0;

        assert(file >= FILE_A && file <= FILE_H);
        assert(rank >= RANK_1 && rank <= RANK_8);

        board.en_pas_sq = GET_INDEX(file, rank);
    }

    i += 2;

    // Fifty-move rule counter and move counter ignored for now.

    board.hash_key = gen_hash(board); // Generate zobrist hash.

    update_secondary(board); // Update 'all white' and 'all black' boards.

    return 1;
}

/**
    @brief Determines the type of pieces occupying a cell.

    @param board is the board on which to check on.
    @param bit_chk is a uint64_t value with exactly one bit set.

    @return int corresponding to piece type in standard convention if the
            cell is indeed occupied, EMPTY (14) otherwise.

    @warning Exactly one bit must be set in 'bit_chk'.
*/

unsigned int determine_type(const Board& board, uint64 bit_chk)
{
    // Exactly one bit must be set.

    assert((bit_chk != 0ULL) && ((bit_chk & (bit_chk - 1)) == 0ULL));

    if(bit_chk & board.chessboard[ALL_WHITE])
    {
        for(int i = wP; i <= wK; i++)
            if(bit_chk & board.chessboard[i]) return i;
    }
    else if(bit_chk & board.chessboard[ALL_BLACK])
    {
        for(int i = bP; i <= bK; i++)
            if(bit_chk & board.chessboard[i]) return i;
    }

    return EMPTY;
}

/**
    @brief Converts a given piece into a character.

    Converts a given piece on a cell into a character for use in
    pretty(). Returns a '.' if empty.

    @param board is the board to check for the piece on.
    @param index is a value indexing which cell to convert in LERF mapping.

    @return char corresponding to piece type by standard convention.

    @warning 'index' must be between (or equal to) 0 and 63.
    @warning 'index' must be represented in LERF layout.
*/

char conv_char(const Board& board, unsigned int index)
{
    assert(index < 64);

    int type = determine_type(board, GET_BB(index));

    if(type == EMPTY) return '.';
    else if(type >= wP && type <= wK)
    {
        switch(type)
        {
            case wP: return 'P';
            case wR: return 'R';
            case wN: return 'N';
            case wB: return 'B';
            case wQ: return 'Q';
            case wK: return 'K';
            default: return '.';
        }
    }
    else
    {
        switch(type)
        {
            case bP: return 'p';
            case bR: return 'r';
            case bN: return 'n';
            case bB: return 'b';
            case bQ: return 'q';
            case bK: return 'k';
            default: return '.';
        }
    }
}

/**
    @brief Converts the chess board into a 'pretty' string.

    Converts the chess board into a string of a 'pretty' bitboard suitable
    for standard output. Useful for debugging or command line operation
    of the engine.

    @param board is the board to convert into a 'pretty' string.

    @return string which is a 'pretty' version of the chess board and comes
            pre-packed with newline characters. Suitable for printing out of
            the box.
*/

std::string pretty_board(const Board& board)
{
    std::stringstream pretty_str;
    pretty_str << "8    ";
    int cur_rank = 8, index;

    // Board

    for(int i = 0; i < 64; i++)
    {
        index = (56 - (8 * (i / 8))) + (i % 8); // LERF translation.

        if(((i % 8) == 0) && (i != 0)) // Check if to move to the next rank.
        {
            cur_rank--;
            pretty_str << "\n\n" << cur_rank << "    ";
        }

        pretty_str << conv_char(board, index);
        pretty_str << "   ";
    }

    pretty_str << "\n\n\n     a   b   c   d   e   f   g   h\n\n";

    // Side

    pretty_str << "Side to play: ";

    if(board.side == WHITE) pretty_str << "White\n";
    else pretty_str << "Black\n";

    // Ply

    pretty_str << "Ply: " << board.ply << "\n";
    pretty_str << "History Ply: " << board.his_ply << "\n";

    // Castling permissions

    pretty_str << "Castling permissions: ";

    if(board.castle_perm & WKCA) pretty_str << "K";
    if(board.castle_perm & WQCA) pretty_str << "Q";
    if(board.castle_perm & BKCA) pretty_str << "k";
    if(board.castle_perm & BQCA) pretty_str << "q";
    if(board.castle_perm == 0) pretty_str << "None";

    pretty_str << "\n";

    // En passant square

    pretty_str << "En passant square: ";

    if(board.en_pas_sq == NO_SQ) pretty_str << "None\n";
    else pretty_str << COORD(board.en_pas_sq) << "\n";

    // Fifty-move rule counter

    pretty_str << "Fifty-move rule counter: " << board.fifty << "\n";

    // Zobrist hash

    pretty_str << "Zobrist hash: " << board.hash_key;

    return pretty_str.str();
}

/**
    @brief Adds a piece on a given cell on the given board.

    @param board is the move to add the piece on.
    @param piece_type is type of piece to add on the cell.
    @param index is the integer index representing the cell on which the piece
           should exist.

    @return void.

    @warning 'piece_type' must be between (or equal to) wP (0) and bK (11).
    @warning 'index' must be between (or equal to) 0 and 63.
*/

inline void spawn_piece(Board& board, unsigned int piece_type,
    unsigned int index)
{
    assert(piece_type < 12);
    assert(index < 64);

    uint64 cell_bb = GET_BB(index);

    HASH_PIECE(board, piece_type, index); // Hash piece in.

    board.chessboard[piece_type] |= cell_bb;

    if(piece_type <= wK) // Added piece is white.
        board.chessboard[ALL_WHITE] |= cell_bb;
    else // Removed piece is black.
        board.chessboard[ALL_BLACK] |= cell_bb;
}

/**
    @brief Clears a piece on a given cell on the given board.

    @param board is the move to clear the piece on.
    @param piece_type is the type of piece being cleared.
    @param index is the integer index representing the cell on which the piece
           exists.

    @return void.

    @warning A piece must exist on the cell indexed by 'index', but this is NOT
             checked.
    @warning 'piece_type' must be between (or equal to) wP (0) and bK (11).
    @warning 'index' must be between (or equal to) 0 and 63.
*/

inline void obliterate_piece(Board& board, unsigned int piece_type,
    unsigned int index)
{
    assert(piece_type < 12);
    assert(index < 64);

    uint64 cell_bb = GET_BB(index);

    HASH_PIECE(board, piece_type, index); // Hash piece out.

    board.chessboard[piece_type] ^= cell_bb;

    if(piece_type <= wK) // Removed piece is white.
        board.chessboard[ALL_WHITE] ^= cell_bb;
    else // Removed piece is black.
        board.chessboard[ALL_BLACK] ^= cell_bb;
}

/**
    @brief Moves a piece from one cell to another. This function should be used
           if the type of piece being moved is unknown.

    @param board is the move to move the piece on.
    @param dep_cell is the integer index representing the departure cell.
    @param dst_cell is the integer index representing the destination cell.

    @warning A piece must exist on the cell indexed by 'dep_cell', but this is
             NOT checked.
    @warning This function only moves a piece. It will not check whether a
             piece already exists on the destination cell. That is, it will
             not make capture moves. Use move_piece_cap() for capture moves.
    @warning 'dep_cell' must be between (or equal to) 0 and 63.
    @warning 'dst_cell' must be between (or equal to) 0 and 63.
*/

inline void move_piece_tu(Board& board, unsigned int dep_cell,
    unsigned int dst_cell)
{
    assert(dep_cell < 64);
    assert(dst_cell < 64);

    unsigned int piece_type = determine_type(board, GET_BB(dep_cell));

    obliterate_piece(board, piece_type, dep_cell);
    spawn_piece(board, piece_type, dst_cell);
}

/**
    @brief Moves a piece from one cell to another. This function should be used
           if the type of piece being moved is known.

    @param board is the move to move the piece on.
    @param piece_type is the type of piece being moved.
    @param dep_cell is the integer index representing the departure cell.
    @param dst_cell is the integer index representing the destination cell.

    @warning A piece must exist on the cell indexed by 'dep_cell', but this is
             NOT checked.
    @warning This function only moves a piece. It will not check whether a
             piece already exists on the destination cell. That is, it will
             not make capture moves.
    @warning 'piece_type' must be between (or equal to) wP (0) and bK (11).
    @warning 'dep_cell' must be between (or equal to) 0 and 63.
    @warning 'dst_cell' must be between (or equal to) 0 and 63.
*/

inline void move_piece_tk(Board& board, unsigned int piece_type,
    unsigned int dep_cell, unsigned int dst_cell)
{
    assert(piece_type < 12);
    assert(dep_cell < 64);
    assert(dst_cell < 64);

    obliterate_piece(board, piece_type, dep_cell);
    spawn_piece(board, piece_type, dst_cell);
}

/**
    @brief Make the given move on the given board.

    @param board is the board to make the move on.
    @param move is the integer representation of the move to be made in
           standard convention.

    @return bool value representing whether move was actually made, that is,
            if the given move was legal.
*/

bool make_move(Board& board, unsigned int move)
{
    assert(move != NO_MOVE);

    unsigned int dep = DEP_CELL(move);
    unsigned int dst = DST_CELL(move);
    unsigned int dep_type = determine_type(board, GET_BB(dep));
    unsigned int cap_type = CAPTURED(move);
    unsigned int prom_type = PROMOTED(move);
    uint64 king_bb; // Used to check move legality.

    assert(cap_type != wK && cap_type != bK);

    bool side = board.side;

    UndoMove undo_ms(move, board.castle_perm, board.en_pas_sq, board.fifty,
        board.hash_key); // Create the undo move structure.

    board.history.push_back(undo_ms); // Push undo structure into history.

    // Clear en passant square

    if(board.en_pas_sq != NO_SQ) HASH_EP(board); // Hash en passant square out.
    board.en_pas_sq = NO_SQ; // Set en passant square to 'NO_SQ' (65).

    // Increment counters

    board.ply++;
    board.his_ply++;
    board.fifty++;

    // Handle special cases

    if((dep_type == wP) || (dep_type == bP)) // Pawn move
    {
        board.fifty = 0; // Reset fifty-move rule counter.

        if(IS_PSTR(move)) // Pawn start
        {
            // Update en passant square.

            if(side == WHITE) board.en_pas_sq = dst - 8;
            else board.en_pas_sq = dst + 8;

            HASH_EP(board); // Hash en passant square in.
        }

        if(IS_ENPAS_CAP(move)) // En passant capture
        {
            assert((cap_type == wP) || (cap_type == bP));

            if(side == WHITE) obliterate_piece(board, bP, dst - 8);
            else obliterate_piece(board, wP, dst + 8);
        }
    }
    else if(IS_CAS(move)) // Move rook if castling
    {
        HASH_CA(board); // Hash castling permissions out.

        switch(dst)
        {
            case g1:
            {
                board.castle_perm &= 3;
                move_piece_tk(board, wR, h1, f1);
                break;
            }
            case c1:
            {
                board.castle_perm &= 3;
                move_piece_tk(board, wR, a1, d1);
                break;
            }
            case g8:
            {
                board.castle_perm &= 12;
                move_piece_tk(board, bR, h8, f8);
                break;
            }
            case c8:
            {
                board.castle_perm &= 12;
                move_piece_tk(board, bR, a8, d8);
                break;
            }
            default: assert(false); break; // Something's wrong with castling.
        }

        HASH_CA(board); // Hash castling permissions in.
    }

    // Update castling permissions

    HASH_CA(board); // Hash castling permissions out.

    if(board.castle_perm)
    {
        switch(dep_type)
        {
            case wR: // White
            {
                if(dep == h1) board.castle_perm &= 7; // King-side
                else if(dep == a1) board.castle_perm &= 11; // Queen-side
            }
            case bR: // Black
            {
                if(dep == h8) board.castle_perm &= 13; // King-side
                else if(dep == a8) board.castle_perm &= 14; // Queen-side
            }
            case wK: // White
            {
                if(dep == e1) board.castle_perm &= 3;
            }
            case bK: // Black
            {
                if(dep == e8) board.castle_perm &= 12;
            }
            default: break; // Do nothing.
        }

        switch(cap_type)
        {
            case wR: // White
            {
                if(dst == h1) board.castle_perm &= 7; // King-side
                else if(dst == a1) board.castle_perm &= 11; // Queen-side
            }
            case bR: // Black
            {
                if(dst == h8) board.castle_perm &= 13; // King-side
                else if(dst == a8) board.castle_perm &= 14; // Queen-side
            }
            default: break; // Do nothing.
        }
    }

    HASH_CA(board); // Hash castling permissions in.

    // Update fifty-move rule counter and clear captured piece, if any.

    if((cap_type != EMPTY) && !IS_ENPAS_CAP(move))
    {
        assert(((side == WHITE && cap_type > 5) ||
            (side == BLACK && cap_type < 6)) && cap_type < 12);

        obliterate_piece(board, cap_type, dst);
        board.fifty = 0;
    }

    assert(determine_type(board, GET_BB(dep)) < 12);
    move_piece_tu(board, dep, dst); // Move the piece.

    // Update as necessary if the move is a promotion.

    if(prom_type != EMPTY)
    {
        assert((prom_type < 12) && (prom_type != wP) && (prom_type != bP));

        if(side == WHITE) // Clear the pawn.
            obliterate_piece(board, wP, dst);
        else
            obliterate_piece(board, bP, dst);

        spawn_piece(board, prom_type, dst); // Add the promoted piece.
    }

    board.side = !board.side; // Swap sides.
    HASH_SIDE(board); // Hash the side (swap).

    if(side == WHITE) king_bb = board.chessboard[wK];
    else king_bb = board.chessboard[bK];

    assert((king_bb != 0ULL) && ((king_bb & (king_bb - 1)) == 0ULL));

    if(is_sq_attacked(POP_BIT(king_bb), side, board)) // Check move legality.
    {
        undo_move(board);

        return 0; // The move was illegal and hence not made.
    }

    assert(board.his_ply == board.history.size());

    return 1; // The move was legal and has been correctly made.
}

/**
    @brief Undo the previous move.

    This function unmakes the previous move that was made on the board.

    @param board is the board to undo the move on.

    @return void.
*/

void undo_move(Board& board)
{
    assert(board.history.size() > 0);

    UndoMove ms = board.history.back();

    unsigned int move = ms.move;
    unsigned int dep = DEP_CELL(ms.move);
    unsigned int dst = DST_CELL(ms.move);
    unsigned int cap_type = CAPTURED(ms.move);
    unsigned int prom_type = PROMOTED(ms.move);

    bool side = !board.side;

    // Decrement ply counters

    board.ply--;
    board.his_ply--;

    if(board.en_pas_sq != NO_SQ) HASH_EP(board); // Hash out en passant square.

    HASH_CA(board); // Hash out castling permissions.

    board.castle_perm = ms.castle_perm;
    board.en_pas_sq = ms.en_pas_sq;
    board.fifty = ms.fifty;

    if(board.en_pas_sq != NO_SQ) HASH_EP(board); // Hash in en passant square.

    HASH_CA(board); // Hash in castling permissions.

    board.side = !board.side; // Swap sides.
    HASH_SIDE(board); // Hash the side (swap).

    // Handle special cases

    if(IS_ENPAS_CAP(move)) // En passant capture
    {
        assert((cap_type == wP) || (cap_type == bP));

        if(side == WHITE) spawn_piece(board, bP, dst - 8);
        else spawn_piece(board, wP, dst + 8);
    }
    else if(IS_CAS(move)) // Move rook if castling
    {
        switch(dst)
        {
            case g1: move_piece_tk(board, wR, f1, h1); break;
            case c1: move_piece_tk(board, wR, d1, a1); break;
            case g8: move_piece_tk(board, bR, f8, h8); break;
            case c8: move_piece_tk(board, bR, d8, a8); break;
            default: assert(false); break; // Something's wrong with castling.
        }
    }

    assert(determine_type(board, GET_BB(dst)) < 12);
    move_piece_tu(board, dst, dep); // Move the piece back.

    // Put the captured piece back where it was.

    if((cap_type != EMPTY) && !IS_ENPAS_CAP(move))
    {
        assert(((side == WHITE && cap_type > 5) ||
            (side == BLACK && cap_type < 6)) && cap_type < 12);

        spawn_piece(board, cap_type, dst);
    }

    // Demote the pawn!

    if(prom_type != EMPTY)
    {
        assert((prom_type < 12) && (prom_type != wP) && (prom_type != bP));

        obliterate_piece(board, prom_type, dep); // Clear the promote piece.

        if(side == WHITE) // Add a pawn on the departure cell.
            spawn_piece(board, wP, dep);
        else
            spawn_piece(board, bP, dep);
    }

    board.history.pop_back(); // Pop the last move out.

    assert(board.his_ply == board.history.size());
}

/**
    @brief Make a null move on the given board.

    @param board is the board to make the move on.

    @return void.
*/

void make_null_move(Board& board)
{
    UndoMove undo_ms(NO_MOVE, board.castle_perm, board.en_pas_sq, board.fifty,
        board.hash_key); // Create the undo move structure.

    board.history.push_back(undo_ms); // Push undo structure into history.

    // Clear en passant square

    if(board.en_pas_sq != NO_SQ) HASH_EP(board); // Hash en passant square out.
    board.en_pas_sq = NO_SQ; // Set en passant square to 'NO_SQ' (65).

    // Increment counters

    board.ply++;
    board.his_ply++;

    board.side = !board.side; // Swap sides.
    HASH_SIDE(board); // Hash the side (swap).

    assert(board.his_ply == board.history.size());
}

/**
    @brief Undo the previous move, which was made using the make_null_move()
           function.

    This function unmakes the previous move that was made on the board, which
    should have been a null move.

    @param board is the board to undo the move on.

    @return void.

    @warning The previous move must have been a null move. Otherwise, bad things
             will happen.
*/

void undo_null_move(Board& board)
{
    assert(board.history.size() > 0);

    UndoMove ms = board.history.back();

    // Decrement ply counters

    board.ply--;
    board.his_ply--;

    if(board.en_pas_sq != NO_SQ) HASH_EP(board); // Hash out en passant square.

    HASH_CA(board); // Hash out castling permissions.

    board.castle_perm = ms.castle_perm;
    board.en_pas_sq = ms.en_pas_sq;
    board.fifty = ms.fifty;

    if(board.en_pas_sq != NO_SQ) HASH_EP(board); // Hash in en passant square.

    board.side = !board.side; // Swap sides.
    HASH_SIDE(board); // Hash the side (swap).

    board.history.pop_back(); // Pop the last move out.

    assert(board.his_ply == board.history.size());
}

/**
    @brief Converts a string representation of a move in pure algebraic
           notation into the standard convention for representing moves
           in the engine.

    @param board is the board the move is being made on.
    @param str_move is the string representation of the move in pure
           algebraic notation.

    @return unsigned int value representing the move in standard convention.

    @warning Returns 'NO_MOVE' (0) on failure to parse, or if the move does
             not exist or is not legal.
*/

unsigned int parse_move(Board& board, std::string str_move)
{
    if(str_move.length() < 4 || str_move.length() > 5)
        return 0; // Parse error.

    unsigned int dep_cell = 0, dst_cell = 0; // Indices of cells.
    unsigned int prom_type = EMPTY; // Type of promoted piece, if any.
    unsigned int move = NO_MOVE; // The move itself.

    unsigned int list_size, list_move; // Temporary variables.
    char c; // Temporary character.

    MoveList ml = gen_moves(board);

    c = str_move[0]; // Departure file

    if(c >= 'a' && c <= 'h') dep_cell += c - 'a';
    else return 0; // Parse error.

    c = str_move[1]; // Departure rank

    if(c >= '1' && c <= '8') dep_cell += (c - '1') * 8;
    else return 0; // Parse error.

    c = str_move[2]; // Destination file

    if(c >= 'a' && c <= 'h') dst_cell += c - 'a';
    else return 0; // Parse error.

    c = str_move[3]; // Destination rank

    if(c >= '1' && c <= '8') dst_cell += (c - '1') * 8;

    c = str_move[4]; // Type of promoted piece

    if(c)
    {
        switch(c)
        {
            case 'q':
            {
                if(board.side == WHITE) prom_type = wQ;
                else prom_type = bQ;
                break;
            }
            case 'n':
            {
                if(board.side == WHITE) prom_type = wN;
                else prom_type = bN;
                break;
            }
            case 'r':
            {
                if(board.side == WHITE) prom_type = wR;
                else prom_type = bR;
                break;
            }
            case 'b':
            {
                if(board.side == WHITE) prom_type = wB;
                else prom_type = bB;
                break;
            }
            default: return 0; // Parse error.
        }
    }

    list_size = ml.list.size();

    for(unsigned int i = 0; i < list_size; i++) // Compare with every move.
    {
        list_move = ml.list.at(i).move;

        if(DEP_CELL(list_move) == dep_cell && DST_CELL(list_move) == dst_cell)
        {
            if(IS_PROM(list_move))
            {
                if(PROMOTED(list_move) == prom_type)
                {
                    move = list_move;
                    break;
                }
            }
            else
            {
                move = list_move;
                break;
            }
        }
    }

    if(move) // Check if legal.
    {
        if(make_move(board, move))
        {
            undo_move(board);
            return move;
        }
    }

    return NO_MOVE;
}

/**
    @brief Checks if the given move actually exists on the board.

    @param board is the board to check on.
    @param move is the integer representation of the move to check in
           standard convention.

    @return bool denoting whether the move exists on the board represented
            by 'board'.
*/

inline bool move_exists(Board& board, unsigned int move)
{
    unsigned int list_move, list_size;

    MoveList ml = gen_moves(board);

    list_size = ml.list.size();

    for(unsigned int i = 0; i < list_size; i++) // Compare with every move.
    {
        list_move = ml.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        undo_move(board);
        if(move == list_move) return 1;
    }

    return 0;
}

/**
    @brief Retrieves a PV line from the table.

    @param board is the board on which to probe and fill the PV array on.
    @param depth is the depth to which to probe the PV line to.

    @return unsigned int value representing the depth to which the PV line
            was found (or in other words, the number of moves found).
*/

unsigned int probe_pv_line(Board& board, unsigned int depth)
{
    assert(board.ply == 0);
    assert(depth < MAX_DEPTH);

    unsigned int move = probe_pv_table(board.t_table, board.hash_key);
    unsigned int count = 0;

    // Probe the table.

    while(move != NO_MOVE && count < depth)
    {
        if(move_exists(board, move))
        {
            make_move(board, move);
            board.pv_array[count] = move;
            count++;
        }
        else break;

        move = probe_pv_table(board.t_table, board.hash_key);
    }

    // Reset the board to the original position.

    while(board.ply > 0) undo_move(board);

    return count;
}