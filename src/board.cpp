/*
    Cortex - Self-learning Chess Engine
    @filename board.cpp
    @author Shreyas Vinod
    @version 0.4.0

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
*/

#include <sstream> // std::stringstream
#include <ctype.h> // isalpha() and isdigit()

#include "board.h"
#include "move.h" // COORD()
#include "hash.h" // gen_hash()
#include "lookup_tables.h" // Lookup tables

// Prototypes

void reset_board(Board& board);
bool parse_fen(Board& board, const std::string fen);
unsigned int determine_type(const Board& board, uint64 bit_chk);
char conv_char(const Board& board, unsigned int index);
std::string pretty_board(const Board& board);

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

    i++;
    if(fen[i] != ' ') return 0; // Parse error.
    i++;

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