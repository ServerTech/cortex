/*
    Cortex - Self-learning Chess Engine
    @filename bitboard.cpp
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

#include <assert.h> // assert().
#include "bitboard.h"
#include "defs.h"

/**
    @brief Constructs an object given the game state.

    Capable of constructing the class at any game state given every parameter.
    Format: K: King; Q: Queen; B: Bishop; N: Knight; R: Rook; P: Pawn;
    Space: Empty; non-capitalised letters for enemy pieces.

    @param is_w denotes whether the engine's side is white.
    @param c_ply is the current ply value of the game.
    @param mk_cr denotes whether King side castling is legal for the engine.
    @param mq_cr denotes whether Queen side castling is legal for the engine.
    @param ek_cr denotes whether King side castling is legal for the enemy.
    @param eq_cr denotes whether Queen side castling is legal for the enemy.
    @param vis_board[64] is a 64 element array for initialisation.

    @warning Do NOT have more than king for each side. Although this is not
             checked, the consequence of having multiple kings is undefined for
             the engine in its entirety.
    @warning Do NOT place pawns on ranks one or eight. This is an
             impossible occurrence, and the engine will fail to promote them.
             In fact, they might vanish.
    @warning 'c_ply' must be zero or positive.
*/

Bitboard::Bitboard(bool is_w, unsigned int c_ply, bool mk_cr, bool mq_cr,
    bool ek_cr, bool eq_cr, const char vis_board[64])
:is_white(is_w), ply(c_ply), me_k_castling(mk_cr), me_q_castling(mq_cr),
ey_k_castling(ek_cr), ey_q_castling(eq_cr), move_history()
{
    Bitboard::chessboard[16] = ~(0ULL); // Sentinel bitboard.

    // Initialise with a 64 element char array.

    int temp; // Holds the LERF conversion index.

    for(int i = 0; i < 16; i++) Bitboard::chessboard[i] = 0ULL;

    for(int i = 0; i < 64; i++)
    {
        temp = (56 - (8 * (i / 8))) + (i % 8); // LERF translation.

        switch(vis_board[i])
        {
            case ' ': continue;
            case 'P':
                if(Bitboard::is_white)
                {
                    Bitboard::chessboard[MP] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EP] |= 1ULL << temp; break;
                }
            case 'p':
                if(!Bitboard::is_white)
                {
                    Bitboard::chessboard[MP] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EP] |= 1ULL << temp; break;
                }
            case 'R':
                if(Bitboard::is_white)
                {
                    Bitboard::chessboard[MR] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[ER] |= 1ULL << temp; break;
                }
            case 'r':
                if(!Bitboard::is_white)
                {
                    Bitboard::chessboard[MR] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[ER] |= 1ULL << temp; break;
                }
            case 'N':
                if(Bitboard::is_white)
                {
                    Bitboard::chessboard[MN] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EN] |= 1ULL << temp; break;
                }
            case 'n':
                if(!Bitboard::is_white)
                {
                    Bitboard::chessboard[MN] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EN] |= 1ULL << temp; break;
                }
            case 'B':
                if(Bitboard::is_white)
                {
                    Bitboard::chessboard[MB] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EB] |= 1ULL << temp; break;
                }
            case 'b':
                if(!Bitboard::is_white)
                {
                    Bitboard::chessboard[MB] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EB] |= 1ULL << temp; break;
                }
            case 'Q':
                if(Bitboard::is_white)
                {
                    Bitboard::chessboard[MQ] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EQ] |= 1ULL << temp; break;
                }
            case 'q':
                if(!Bitboard::is_white)
                {
                    Bitboard::chessboard[MQ] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EQ] |= 1ULL << temp; break;
                }
            case 'K':
                if(Bitboard::is_white)
                {
                    Bitboard::chessboard[MK] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EK] |= 1ULL << temp; break;
                }
            case 'k':
                if(!Bitboard::is_white)
                {
                    Bitboard::chessboard[MK] |= 1ULL << temp; break;
                }
                else
                {
                    Bitboard::chessboard[EK] |= 1ULL << temp; break;
                }
            default: assert(false); // Invalid character.
        }
    }

    Bitboard::update();
}

/**
    @brief Copy constructor.

    This is an overloaded function that copies an already existing object.

    @param board_obj is the Bitboard object to copy from.
*/

Bitboard::Bitboard(const Bitboard& board_obj)
:is_white(board_obj.is_white), ply(board_obj.ply),
me_k_castling(board_obj.me_k_castling),
me_q_castling(board_obj.me_q_castling),
ey_k_castling(board_obj.ey_k_castling),
ey_q_castling(board_obj.ey_q_castling),
move_history(board_obj.move_history)
{
    for(int i = 0; i < 17; i++) // Copy bitboards.
        Bitboard::chessboard[i] = board_obj.chessboard[i];
}

/**
    @brief Accessor function to access bitboard values for every type.

    @param sel is a value corresponding to the bitboard type, from defs.h.

    @return uint64_t value of the bitboard at index specified by 'sel'.

    @warning 'sel' must be within bounds defined in defs.h.
*/

uint64_t Bitboard::get_val(int sel) const
{
    assert(sel >= ME && sel <= FREE); // 'sel' must be within bounds.

    return Bitboard::chessboard[sel];
}

/**
    @brief Mutator function to explicitly change bitboard values.

    @param sel is a value corresponding to the bitboard type, from defs.h.
    @param val is the value to replace the value at 'sel' with.

    @return void.

    @warning 'sel' must be within bounds defined in defs.h.
    @warning Do not try to set ME, EY, OCC, FREE or NONE. The first four are
             automatically updated and the fifth is a sentinel value. This
             restriction prevents object corruption due to user error.
    @warning Do NOT have more than king for each side. Although this is not
             checked, the consequence of having multiple kings is undefined for
             the engine in its entirety.
    @warning Do NOT place pawns on ranks zero or eight. This is an
             impossible occurrence, and the engine will fail to promote them.
             In fact, they might vanish.
*/

void Bitboard::set_val(int sel, uint64_t val)
{
    assert(sel >= MP && sel <= EK); // 'sel' must be within bounds.

    Bitboard::chessboard[sel] = val;
}

/**
    @brief Used to know which side the engine is playing for.

    @return bool which is true if the engine is playing for white.
*/

bool Bitboard::get_side() const
{
    return Bitboard::is_white;
}

/**
    @brief Determines the type of pieces occupying a cell.

    @param bit_chk is a uint64_t value with exactly one bit set.

    @return int corresponding to piece type in standard convention if the
            cell is indeed occupied, sentinel value NONE (16) otherwise.

    @warning Exactly one bit must be set in 'bit_chk'.
*/

int Bitboard::determine_type(uint64_t bit_chk) const
{
    // Exactly one bit must be set.

    assert((bit_chk != 0ULL) && ((bit_chk & (bit_chk - 1)) == 0ULL));

    if(bit_chk & Bitboard::chessboard[ME])
    {
        for(int i = MP;; i++) if(bit_chk & Bitboard::chessboard[i]) return i;
    }
    else if(bit_chk & Bitboard::chessboard[EY])
    {
        for(int i = EP;; i++) if(bit_chk & Bitboard::chessboard[i]) return i;
    }

    return NONE;
}

/**
    @brief Checks whether a particular type of castling is currently legal.

    @param index is an index in standard indexing convention; (MK; MQ; EK; EQ).

    @return bool denoting whether the type of castling selected by 'index'
            is legal.

    @warning 'index' has only four legal values (MK; MQ; EK; EQ).
*/

bool Bitboard::is_castling_legal(int index) const
{
    switch(index)
    {
        case MK: if(me_k_castling) return 1; else return 0;
        case MQ: if(me_q_castling) return 1; else return 0;
        case EK: if(ey_k_castling) return 1; else return 0;
        case EQ: if(ey_q_castling) return 1; else return 0;
        default: assert(false); // Invalid index.
    }
}

/**
    @brief Gets the value of the current ply the game is at.

    @return unsigned int value corresponding to the current ply.
*/

unsigned int Bitboard::get_ply() const
{
    return Bitboard::ply;
}

/**
    @brief Converts a given piece into a character.

    Converts a given piece on a cell into a character for use in
    Bitboard::pretty(). Can also be used to create a char array of the
    board if required.

    @param index is a value indexing which cell to convert in LERF mapping.

    @return char corresponding to piece type by standard convention.

    @warning 'index' must be between (or equal to) 0 and 63.
*/

char Bitboard::conv_char(int index) const
{
    assert(index >= 0 && index < 64);

    int type = Bitboard::determine_type(1ULL << index);

    if(type == NONE) return ' ';
    else if(type >= MP && type <= MK) // Engine's piece.
    {
        switch(type)
        {
            case MP: if(Bitboard::is_white) return 'P'; else return 'p';
            case MR: if(Bitboard::is_white) return 'R'; else return 'r';
            case MN: if(Bitboard::is_white) return 'N'; else return 'n';
            case MB: if(Bitboard::is_white) return 'B'; else return 'b';
            case MQ: if(Bitboard::is_white) return 'Q'; else return 'q';
            case MK: if(Bitboard::is_white) return 'K'; else return 'k';
            default: return ' ';
        }
    }
    else // Enemy's piece.
    {
        switch(type)
        {
            case EP: if(Bitboard::is_white) return 'p'; else return 'P';
            case ER: if(Bitboard::is_white) return 'r'; else return 'R';
            case EN: if(Bitboard::is_white) return 'n'; else return 'N';
            case EB: if(Bitboard::is_white) return 'b'; else return 'B';
            case EQ: if(Bitboard::is_white) return 'q'; else return 'Q';
            case EK: if(Bitboard::is_white) return 'k'; else return 'K';
            default: return ' ';
        }
    }
}

/**
    @brief Converts the chess board into a 'pretty' string.

    Converts the chess board into a string of a 'pretty' bitboard suitable
    for standard output. Useful for debugging or command line operation
    of the engine.

    @return string which is a 'pretty' version of the chess board and comes
            pre-packed with newline characters. Suitable for printing out of
            the box.
*/

std::string Bitboard::pretty() const
{
    std::string pretty_str = "8    ";
    int cur_rank = 8, temp = 0;

    for(int i = 0; i < 64; i++)
    {
        temp = (56 - (8 * (i / 8))) + (i % 8); // LERF translation.

        if(((i % 8) == 0) && (i != 0)) // Check if to move to the next rank.
        {
            cur_rank--;
            pretty_str += "\n\n" + std::to_string(cur_rank) + "    ";
        }

        pretty_str += Bitboard::conv_char(temp);
        pretty_str += "   ";
    }

    pretty_str += "\n\n\n     a   b   c   d   e   f   g   h";

    return pretty_str;
}

/**
    @brief Makes a move.

    This function makes a move specified by the Move object argument. Further,
    it will update castling rights automatically based on the move made.

    @param pros_move is a Move object, denoting what move to make.

    @return void.

    @warning This function does NOT check the legality of the move. It will
             only check if the departure and destination cells differ, and
             whether a piece exists on the departure cell. If the move is a
             capture, it checks if the destination cell is occupied by an
             enemy piece. If these conditions are met, the move is made, no
             questions asked.
    @warning An std::assert() is called to check the above mentioned conditions
             and therefore this function should not be called with invalid
             arguments.
*/

void Bitboard::move(const Move& pros_move)
{
    uint64_t dep_bb = pros_move.get_dep_bb(); // Departure cell.
    uint64_t dest_bb = pros_move.get_dest_bb(); // Destination cell.
    int dep_type = Bitboard::determine_type(dep_bb); // Departure piece type.
    bool is_capture = pros_move.is_capture();
    int cap_type = pros_move.what_piece();

    // Move integrity checks.

    assert(dep_bb & Bitboard::chessboard[OCC]); // Departure cell occupancy.

    if(is_capture)
    {
        // Destination cell cannot be empty if the move is a capture.

        assert(dest_bb & Bitboard::chessboard[OCC]);
        assert((cap_type >= MP) && (cap_type <= EK));

        // Cannot capture friendly piece.

        if(dep_type <= MK) assert(cap_type > MK);
        else if(dep_type >= EP) assert(cap_type < EP);
    }

    // Move integrity checks complete. If everything looks okay, make the move.

    Bitboard::chessboard[dep_type] ^= dep_bb;
    Bitboard::chessboard[dep_type] |= dest_bb;
    if(is_capture) Bitboard::chessboard[cap_type] ^= dest_bb;

    // The move has been made.

    // Update castling rights.

    if(me_k_castling || me_q_castling)
    {
        if(dep_type == MK) // Check engine's king movement.
        {
            Bitboard::me_k_castling = 0;
            Bitboard::me_q_castling = 0;
        }
        else if()
    }

    if(ey_k_castling || ey_q_castling)
    {
        if(dep_type == EK) // Check enemy's king movement.
        {
            Bitboard::ey_k_castling = 0;
            Bitboard::ey_q_castling = 0;
        }
        else if()
    }

    Bitboard::update();
    Bitboard::ply++;
    Bitboard::move_history.push_back(pros_move);
}

/**
    @brief Undo the previous move.

    This function makes an inverse of the previous move, and decrements
    the value of ply.

    @return void.

    @warning If there is no previous move, the function will simply return.
*/

void Bitboard::undo()
{
    if(Bitboard::ply > 0)
    {
        Move temp(Bitboard::move_history.back());
        Move move_undo(temp.get_dest_cell(), temp.get_dep_cell());

        Bitboard::move(move_undo); // Undo the previous move.

        if(temp.is_capture()) // Previous move was a capture.
        {
            // Restore the piece.

            Bitboard::chessboard[temp.what_piece()] |= temp.get_dest_bb();
        }

        ply -= 2; // Decrement ply by two; Bitboard::move() incremented by 1.
        Bitboard::move_history.pop_back(); // Pop what Bitboard::move() added.
        Bitboard::move_history.pop_back(); // Pop the previous move.
    }
}

/**
    @brief Updates all dependent bitboards.

    @return void.
*/

void Bitboard::update()
{
    Bitboard::chessboard[ME] =
        Bitboard::chessboard[MK] | Bitboard::chessboard[MQ] |
        Bitboard::chessboard[MB] | Bitboard::chessboard[MN] |
        Bitboard::chessboard[MR] | Bitboard::chessboard[MP];

    Bitboard::chessboard[EY] =
        Bitboard::chessboard[EK] | Bitboard::chessboard[EQ] |
        Bitboard::chessboard[EB] | Bitboard::chessboard[EN] |
        Bitboard::chessboard[ER] | Bitboard::chessboard[EP];

    Bitboard::chessboard[OCC] =
        Bitboard::chessboard[ME] | Bitboard::chessboard[EY];

    Bitboard::chessboard[FREE] = ~Bitboard::chessboard[OCC];
}