/*
    Cortex - Self-learning Chess Engine
    @filename movegen.cpp
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
    @filename movegen.cpp
    @author Shreyas Vinod

    @brief Generates moves given a board position.

    Includes structures and functions that help generate and store
    pseudo-legal (and legal) moves for given game states.
*/

#include "defs.h"

#include <string> // std::string
#include <vector> // std::vector
#include <sstream> // std::stringstream

#include "movegen.h"
#include "board.h" // Board structure.
#include "move.h" // Move structure.
#include "lookup_tables.h"

// Globals

const int VICTIM_SCORE[12] = {100, 400, 300, 200, 500, 600,
    100, 400, 300, 200, 500, 600};
unsigned int MVV_LVA_ST[12][12]; // MVV-LVA scores lookup table.

// Prototypes

void init_mvv_lva();
std::string pretty_move_list(const std::vector<Move>& list);
inline void push_quiet_move(MoveList& ml, unsigned int move,
    const Board& board);
inline void push_capture_move(MoveList& ml, unsigned int move,
    const Board& board);
inline void push_enp_capture_move(MoveList& ml, unsigned int move);
inline void push_castling_move(MoveList& ml, unsigned int move);
void gen_rook_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_rook_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_knight_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_knight_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_bishop_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_bishop_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_pawn_moves(bool gen_side, MoveList& ml, const Board& board);
void gen_pawn_cap_moves(bool gen_side, MoveList& ml, const Board& board);
void gen_king_moves(bool gen_side, MoveList& ml, const Board& board);
void gen_king_cap_moves(bool gen_side, MoveList& ml, const Board& board);
bool is_sq_attacked(unsigned int index, bool gen_side, const Board& board);
MoveList gen_moves(const Board& board);
MoveList gen_captures(const Board& board);
MoveList gen_legal_moves(Board& board);
MoveList gen_legal_captures(Board& board);

// Function definitions

/**
    @brief Initialises the MVV-LVA scores lookup table.

    @return void.
*/

void init_mvv_lva()
{
    for(unsigned int attacker = wP; attacker <= bK; attacker++)
    {
        for(unsigned int victim = wP; victim <= bK; victim++)
        {
            MVV_LVA_ST[victim][attacker] = VICTIM_SCORE[victim] + 6 -
                (VICTIM_SCORE[attacker] / 100);
        }
    }
}

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

    int s = list.size(), cap = 0, prom = 0, prom_cap = 0;

    for(int i = 0; i < s; i++)
    {
        pretty_str << "Move " << i + 1 << ": " << COORD_MOVE(list.at(i).move);
        pretty_str << "    Score: " << list.at(i).score;
        pretty_str << "    Captured: ";

        if(IS_CAP(list.at(i).move))
        {
            cap++;
            pretty_str << CAPTURED(list.at(i).move);
        }
        else pretty_str << "-";

        pretty_str << "    Promoted: ";

        if(IS_PROM(list.at(i).move))
        {
            prom++;
            if(IS_CAP(list.at(i).move)) prom_cap++;
            pretty_str << PROMOTED(list.at(i).move);
        }
        else pretty_str << "-";

        pretty_str << "    Flag:";

        if(IS_PSTR(list.at(i).move)) pretty_str << " PS\n";
        else if(IS_ENPAS_CAP(list.at(i).move)) pretty_str << " EPCAP\n";
        else if(IS_CAS(list.at(i).move)) pretty_str << " CA\n";
        else pretty_str << " -\n";
    }

    pretty_str << "\nTotal moves: " << s;
    pretty_str << "    Total captures: " << cap - (3 * (prom_cap / 4));
    pretty_str << "    Total promotions: " << prom / 4;

    return pretty_str.str();
}

/**
    @brief Pushes a quiet move to the move list vector.

    @param list is the move list structure.
    @param move is an integer value representing a move.
    @param board is the board the move is being made on.

    @return void.
*/

inline void push_quiet_move(MoveList& ml, unsigned int move,
    const Board& board)
{
    if(board.search_killers[0][board.ply] == move)
    {
        Move move_push(move, 90000);
        ml.list.push_back(move_push);
    }
    else if(board.search_killers[1][board.ply] == move)
    {
        Move move_push(move, 80000);
        ml.list.push_back(move_push);
    }
    else
    {
        assert((GET_BB(DEP_CELL(move)) != 0ULL) &&
            ((GET_BB(DEP_CELL(move)) &
            (GET_BB(DEP_CELL(move)) - 1)) == 0ULL));

        Move move_push(move, board.search_history[determine_type(board,
            GET_BB(DEP_CELL(move)))][DST_CELL(move)]);
        ml.list.push_back(move_push);
    }
}

/**
    @brief Pushes a capture move to the move list vector.

    @param list is the move list structure.
    @param move is an integer value representing the move.
    @param board is the board the move is being made on.

    @return void.
*/

inline void push_capture_move(MoveList& ml, unsigned int move,
    const Board& board)
{
    unsigned int cap_type = CAPTURED(move);

    if(cap_type == wK || cap_type == bK) return;
    else
    {
        ml.attacked |= GET_BB(DST_CELL(move));

        assert((GET_BB(DEP_CELL(move)) != 0ULL) &&
            ((GET_BB(DEP_CELL(move)) & (GET_BB(DEP_CELL(move)) - 1)) == 0ULL));

        Move move_push(move, MVV_LVA_ST[cap_type][determine_type(board,
            GET_BB(DEP_CELL(move)))] + 100000);
        ml.list.push_back(move_push);
    }
}

/**
    @brief Pushes an en passant capture move to the move list vector.

    @param list is the move list structure.
    @param move is an integer value representing the move.
    @param board is the board the move is being made on.

    @return void.
*/

inline void push_enp_capture_move(MoveList& ml, unsigned int move)
{
    ml.attacked |= GET_BB(DST_CELL(move));
    Move move_push(move, 100105);
    ml.list.push_back(move_push);
}

/**
    @brief Pushes a castling move to the move list vector.

    @param list is the move list structure.
    @param move is an integer value representing the move.

    @return void.
*/

inline void push_castling_move(MoveList& ml, unsigned int move)
{
    Move move_push(move, 50000);
    ml.list.push_back(move_push);
}

/**
    @brief Generates and pushes all pseudo-legal rook moves into the move
           list vector for the given board state.

    This function generates all pseudo-legal moves for a given bitboard,
    considering all set bits as rooks. This is also useful for generating
    line moves for queens.

    @param u64_1 is the bitboard representing all pieces which are to be
           considered as rooks during generation.
    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_rook_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_2, u64_3; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // North

        u64_2 = LINE_N_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 8) | (u64_2 << 16) | (u64_2 << 24) |
            (u64_2 << 32) | (u64_2 << 40) | (u64_2 << 48);
        u64_2 &= LINE_N_LT[uint_1];
        u64_2 ^= LINE_N_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // South

        u64_2 = LINE_S_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 8) | (u64_2 >> 16) | (u64_2 >> 24) |
            (u64_2 >> 32) | (u64_2 >> 40) | (u64_2 >> 48);
        u64_2 &= LINE_S_LT[uint_1];
        u64_2 ^= LINE_S_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // East

        u64_2 = LINE_E_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 1) | (u64_2 << 2) | (u64_2 << 3) |
            (u64_2 << 4) | (u64_2 << 5) | (u64_2 << 6);
        u64_2 &= LINE_E_LT[uint_1];
        u64_2 ^= LINE_E_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // West

        u64_2 = LINE_W_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 1) | (u64_2 >> 2) | (u64_2 >> 3) |
            (u64_2 >> 4) | (u64_2 >> 5) | (u64_2 >> 6);
        u64_2 &= LINE_W_LT[uint_1];
        u64_2 ^= LINE_W_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal rook capture moves into the
           move list vector for the given board state.

    This function generates all pseudo-legal captures moves for a given.
    bitboard, considering all set bits as rooks. This is also useful for
    generating line capture moves for queens.

    @param u64_1 is the bitboard representing all pieces which are to be
           considered as rooks during generation.
    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_rook_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_2, u64_3; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // North

        u64_2 = LINE_N_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 8) | (u64_2 << 16) | (u64_2 << 24) |
            (u64_2 << 32) | (u64_2 << 40) | (u64_2 << 48);
        u64_2 &= LINE_N_LT[uint_1];
        u64_2 ^= LINE_N_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++)
        {
            POP_BIT(u64_2);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // South

        u64_2 = LINE_S_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 8) | (u64_2 >> 16) | (u64_2 >> 24) |
            (u64_2 >> 32) | (u64_2 >> 40) | (u64_2 >> 48);
        u64_2 &= LINE_S_LT[uint_1];
        u64_2 ^= LINE_S_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }

        // East

        u64_2 = LINE_E_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 1) | (u64_2 << 2) | (u64_2 << 3) |
            (u64_2 << 4) | (u64_2 << 5) | (u64_2 << 6);
        u64_2 &= LINE_E_LT[uint_1];
        u64_2 ^= LINE_E_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++)
        {
            POP_BIT(u64_2);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // West

        u64_2 = LINE_W_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 1) | (u64_2 >> 2) | (u64_2 >> 3) |
            (u64_2 >> 4) | (u64_2 >> 5) | (u64_2 >> 6);
        u64_2 &= LINE_W_LT[uint_1];
        u64_2 ^= LINE_W_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal knight moves into the move
           list vector for the given board state.

    This function generates all pseudo-legal moves for a given bitboard,
    considering all set bits as knights.

    @param u64_1 is the bitboard representing all knights.
    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_knight_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 FREE = ~white_bb & ~black_bb; // Free bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_2, u64_3; // Temporary variable.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // Captures

        if(gen_side == WHITE) u64_2 = KNIGHT_LT[uint_1] & black_bb;
        else u64_2 = KNIGHT_LT[uint_1] & white_bb;

        uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push capture moves.
        {
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        u64_2 = KNIGHT_LT[uint_1] & FREE;

        uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                EMPTY, EMPTY, 0), board);
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal knight capture moves into the
           move list vector for the given board state.

    This function generates all pseudo-legal capture moves for a given
    bitboard, considering all set bits as knights.

    @param u64_1 is the bitboard representing all knights.
    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_knight_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_2, u64_3; // Temporary variable.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // Captures

        if(gen_side == WHITE) u64_2 = KNIGHT_LT[uint_1] & black_bb;
        else u64_2 = KNIGHT_LT[uint_1] & white_bb;

        uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push capture moves.
        {
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                determine_type(board, u64_3), EMPTY, 0), board);
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal bishop moves into the move
           list vector for the given board state.

    This function generates all pseudo-legal moves for a given bitboard,
    considering all set bits as bishops. This is also useful for generating
    diagonal moves for queens.

    @param u64_1 is the bitboard representing all pieces which are to be
           considered as bishops during generation.
    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_bishop_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_2, u64_3; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // Northeast

        u64_2 = DIAG_NE_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 9) | (u64_2 << 18) | (u64_2 << 27) |
            (u64_2 << 36) | (u64_2 << 45) | (u64_2 << 54);
        u64_2 &= DIAG_NE_LT[uint_1];
        u64_2 ^= DIAG_NE_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // Northwest

        u64_2 = DIAG_NW_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 7) | (u64_2 << 14) | (u64_2 << 21) |
            (u64_2 << 28) | (u64_2 << 35) | (u64_2 << 42);
        u64_2 &= DIAG_NW_LT[uint_1];
        u64_2 ^= DIAG_NW_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // Southeast

        u64_2 = DIAG_SE_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 7) | (u64_2 >> 14) | (u64_2 >> 21) |
            (u64_2 >> 28) | (u64_2 >> 35) | (u64_2 >> 42);
        u64_2 &= DIAG_SE_LT[uint_1];
        u64_2 ^= DIAG_SE_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Southwest

        u64_2 = DIAG_SW_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 9) | (u64_2 >> 18) | (u64_2 >> 27) |
            (u64_2 >> 36) | (u64_2 >> 45) | (u64_2 >> 54);
        u64_2 &= DIAG_SW_LT[uint_1];
        u64_2 ^= DIAG_SW_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal bishop capture moves into the
           move list vector for the given board state.

    This function generates all pseudo-legal capture moves for a given
    bitboard, considering all set bits as bishops. This is also useful for
    generating diagonal capture moves for queens.

    @param u64_1 is the bitboard representing all pieces which are to be
           considered as bishops during generation.
    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_bishop_cap_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_2, u64_3; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // Northeast

        u64_2 = DIAG_NE_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 9) | (u64_2 << 18) | (u64_2 << 27) |
            (u64_2 << 36) | (u64_2 << 45) | (u64_2 << 54);
        u64_2 &= DIAG_NE_LT[uint_1];
        u64_2 ^= DIAG_NE_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++)
        {
            POP_BIT(u64_2);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // Northwest

        u64_2 = DIAG_NW_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 << 7) | (u64_2 << 14) | (u64_2 << 21) |
            (u64_2 << 28) | (u64_2 << 35) | (u64_2 << 42);
        u64_2 &= DIAG_NW_LT[uint_1];
        u64_2 ^= DIAG_NW_LT[uint_1];

        if(u64_3)
        {
            uint_2 = CNT_BITS(u64_2);
            uint_2--;
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++)
        {
            POP_BIT(u64_2);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            push_capture_move(ml, GET_MOVE(uint_1, POP_BIT(u64_2),
                determine_type(board, u64_3), EMPTY, 0), board);
        }

        // Southeast

        u64_2 = DIAG_SE_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 7) | (u64_2 >> 14) | (u64_2 >> 21) |
            (u64_2 >> 28) | (u64_2 >> 35) | (u64_2 >> 42);
        u64_2 &= DIAG_SE_LT[uint_1];
        u64_2 ^= DIAG_SE_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }

        // Southwest

        u64_2 = DIAG_SW_LT[uint_1] & OCC;
        u64_3 = u64_2;

        u64_2 = (u64_2 >> 9) | (u64_2 >> 18) | (u64_2 >> 27) |
            (u64_2 >> 36) | (u64_2 >> 45) | (u64_2 >> 54);
        u64_2 &= DIAG_SW_LT[uint_1];
        u64_2 ^= DIAG_SW_LT[uint_1];

        if(u64_3) // Pop the capture move first.
        {
            uint_3 = POP_BIT(u64_2);
            u64_3 = GET_BB(uint_3);

            if((gen_side == WHITE && (u64_3 & black_bb)) ||
                (gen_side == BLACK && (u64_3 & white_bb)))
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                push_capture_move(ml, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal pawn moves into the move
           list vector for the given board state.

    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.

    @warning Pawns shouldn't be present on the promotion ranks (1 and 8).
*/

void gen_pawn_moves(bool gen_side, MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.
    const uint64 FREE = ~OCC; // Free bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2, u64_3, u64_4; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    if(gen_side == WHITE) // White Pawns
    {
        u64_1 = board.chessboard[wP];
        bit_cnt = CNT_BITS(u64_1);

        for(unsigned int i = 0; i < bit_cnt; i++)
        {
            // One cell upward

            uint_1 = POP_BIT(u64_1);
            u64_2 = GET_BB(uint_1);
            u64_3 = (u64_2 << 8) & FREE;

            if(u64_3)
            {
                u64_4 = u64_3;
                uint_2 = POP_BIT(u64_4);

                if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, wB, 0), board);
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, wR, 0), board);
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, wN, 0), board);
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, wQ, 0), board);
                }
                else
                {
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, EMPTY, 0),
                        board);
                }

                // Two cells upward

                u64_2 = (u64_2 << 16) & B_RANK[4] & FREE;

                if(u64_2 != 0ULL) push_quiet_move(ml,
                    GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, MFLAGPS),
                    board);
            }

            // Captures

            u64_2 = GET_BB(uint_1);

            // Capture left

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 << 7) & B_RANK[GET_RANK(uint_1 + 8)] &
                    (black_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 << 7) & B_RANK[GET_RANK(uint_1 + 8)] &
                    black_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, bP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }

            // Capture right

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 << 9) & B_RANK[GET_RANK(uint_1 + 8)] &
                    (black_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 << 9) & B_RANK[GET_RANK(uint_1 + 8)] &
                    black_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, bP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }
        }
    }
    else // Black Pawns
    {
        u64_1 = board.chessboard[bP];
        bit_cnt = CNT_BITS(u64_1);

        for(unsigned int i = 0; i < bit_cnt; i++)
        {
            // One cell downward

            uint_1 = POP_BIT(u64_1);
            u64_2 = GET_BB(uint_1);
            u64_3 = (u64_2 >> 8) & FREE;

            if(u64_3 != 0ULL)
            {
                u64_4 = u64_3;
                uint_2 = POP_BIT(u64_4);

                if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, bB, 0), board);
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, bR, 0), board);
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, bN, 0), board);
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, bQ, 0), board);
                }
                else
                {
                    push_quiet_move(ml,
                        GET_MOVE(uint_1, uint_2, EMPTY, EMPTY, 0),
                        board);
                }

                // Two cells downward

                u64_2 = (u64_2 >> 16) & B_RANK[5] & FREE;

                if(u64_2 != 0ULL) push_quiet_move(ml,
                    GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, MFLAGPS),
                    board);
            }

            // Captures

            u64_2 = GET_BB(uint_1);

            // Capture left

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 >> 7) & B_RANK[GET_RANK(uint_1 - 8)] &
                    (white_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 >> 7) & B_RANK[GET_RANK(uint_1 - 8)] &
                    white_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, wP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }

            // Capture right

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 >> 9) & B_RANK[GET_RANK(uint_1 - 8)] &
                    (white_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 >> 9) & B_RANK[GET_RANK(uint_1 - 8)] &
                    white_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, wP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal pawn capture moves into the
           move list vector for the given board state.

    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.

    @warning Pawns shouldn't be present on the promotion ranks (1 and 8).
*/

void gen_pawn_cap_moves(bool gen_side, MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2, u64_3, u64_4; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    if(gen_side == WHITE) // White Pawns
    {
        u64_1 = board.chessboard[wP];
        bit_cnt = CNT_BITS(u64_1);

        for(unsigned int i = 0; i < bit_cnt; i++)
        {
            // Captures

            uint_1 = POP_BIT(u64_1);
            u64_2 = GET_BB(uint_1);

            // Capture left

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 << 7) & B_RANK[GET_RANK(uint_1 + 8)] &
                    (black_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 << 7) & B_RANK[GET_RANK(uint_1 + 8)] &
                    black_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, bP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }

            // Capture right

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 << 9) & B_RANK[GET_RANK(uint_1 + 8)] &
                    (black_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 << 9) & B_RANK[GET_RANK(uint_1 + 8)] &
                    black_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, bP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, wQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }
        }
    }
    else // Black Pawns
    {
        u64_1 = board.chessboard[bP];
        bit_cnt = CNT_BITS(u64_1);

        for(unsigned int i = 0; i < bit_cnt; i++)
        {
            // Captures

            uint_1 = POP_BIT(u64_1);
            u64_2 = GET_BB(uint_1);

            // Capture left

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 >> 7) & B_RANK[GET_RANK(uint_1 - 8)] &
                    (white_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 >> 7) & B_RANK[GET_RANK(uint_1 - 8)] &
                    white_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, wP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }

            // Capture right

            if(board.en_pas_sq != NO_SQ)
            {
                u64_3 = (u64_2 >> 9) & B_RANK[GET_RANK(uint_1 - 8)] &
                    (white_bb | GET_BB(board.en_pas_sq));
            }
            else
            {
                u64_3 = (u64_2 >> 9) & B_RANK[GET_RANK(uint_1 - 8)] &
                    white_bb;
            }

            if(u64_3)
            {
                assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                if(uint_3 == board.en_pas_sq)
                {
                    push_enp_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, wP, EMPTY, MFLAGEP));
                }
                else if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bB, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bR, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bN, 0), board);
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, bQ, 0), board);
                }
                else
                {
                    push_capture_move(ml,
                        GET_MOVE(uint_1, uint_3, uint_2, EMPTY, 0), board);
                }
            }
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal king moves into the move
           list vector for the given board state.

    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.

    @warning There must be exactly ONE king (zero is also invalid).
*/

void gen_king_moves(bool gen_side, MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 FREE = ~white_bb & ~black_bb; // Free bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2; // Temporary variable.
    bool not_in_check; // Temporary variable.

    // Generation

    if(gen_side == WHITE) u64_1 = board.chessboard[wK];
    else u64_1 = board.chessboard[bK];

    assert((u64_1 != 0ULL) && ((u64_1 & (u64_1 - 1)) == 0ULL));

    uint_1 = POP_BIT(u64_1);

    // Captures

    if(gen_side == WHITE) u64_1 = KING_LT[uint_1] & black_bb;
    else u64_1 = KING_LT[uint_1] & white_bb;

    uint_2 = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < uint_2; i++) // Push capture moves.
    {
        uint_3 = POP_BIT(u64_1);
        u64_2 = GET_BB(uint_3);
        assert((u64_2 != 0ULL) && ((u64_2 & (u64_2 - 1)) == 0ULL));
        push_capture_move(ml, GET_MOVE(uint_1, uint_3,
            determine_type(board, u64_2), EMPTY, 0), board);
    }

    u64_1 = KING_LT[uint_1] & FREE;

    uint_2 = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
    {
        push_quiet_move(ml, GET_MOVE(uint_1, POP_BIT(u64_1),
            EMPTY, EMPTY, 0), board);
    }

    // Castling

    if(board.castle_perm &&
        ((gen_side == WHITE && uint_1 == e1) ||
        (gen_side == BLACK && uint_1 == e8)))
    {
        if(gen_side == WHITE)
        {
            not_in_check = !is_sq_attacked(e1, WHITE, board);

            if(board.castle_perm & WKCA) // White king-side castling
            {
                if(not_in_check && (determine_type(board, GET_BB(f1)) == EMPTY) &&
                    (determine_type(board, GET_BB(g1)) == EMPTY) &&
                    !is_sq_attacked(f1, WHITE, board))
                {
                    push_castling_move(ml, GET_MOVE(e1, g1, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }

            if(board.castle_perm & WQCA) // White queen-side castling
            {
                if(not_in_check && (determine_type(board, GET_BB(d1)) == EMPTY) &&
                    (determine_type(board, GET_BB(c1)) == EMPTY) &&
                    (determine_type(board, GET_BB(b1)) == EMPTY) &&
                    !is_sq_attacked(d1, WHITE, board))
                {
                    push_castling_move(ml, GET_MOVE(e1, c1, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }
        }
        else
        {
            not_in_check = !is_sq_attacked(e8, BLACK, board);

            if(board.castle_perm & BKCA) // Black king-side castling
            {
                if(not_in_check && (determine_type(board, GET_BB(f8)) == EMPTY) &&
                    (determine_type(board, GET_BB(g8)) == EMPTY) &&
                    !is_sq_attacked(f8, BLACK, board))
                {
                    push_castling_move(ml, GET_MOVE(e8, g8, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }

            if(board.castle_perm & BQCA) // Black queen-side castling
            {
                if(not_in_check && (determine_type(board, GET_BB(d8)) == EMPTY) &&
                    (determine_type(board, GET_BB(c8)) == EMPTY) &&
                    (determine_type(board, GET_BB(b8)) == EMPTY) &&
                    !is_sq_attacked(d8, BLACK, board))
                {
                    push_castling_move(ml, GET_MOVE(e8, c8, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal king capture moves into the
           move list vector for the given board state.

    @param gen_side is the side to generate moves for.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.

    @warning There must be exactly ONE king (zero is also invalid).
*/

void gen_king_cap_moves(bool gen_side, MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2; // Temporary variable.

    // Generation

    if(gen_side == WHITE) u64_1 = board.chessboard[wK];
    else u64_1 = board.chessboard[bK];

    assert((u64_1 != 0ULL) && ((u64_1 & (u64_1 - 1)) == 0ULL));

    uint_1 = POP_BIT(u64_1);

    // Captures

    if(gen_side == WHITE) u64_1 = KING_LT[uint_1] & black_bb;
    else u64_1 = KING_LT[uint_1] & white_bb;

    uint_2 = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < uint_2; i++) // Push capture moves.
    {
        uint_3 = POP_BIT(u64_1);
        u64_2 = GET_BB(uint_3);
        assert((u64_2 != 0ULL) && ((u64_2 & (u64_2 - 1)) == 0ULL));
        push_capture_move(ml, GET_MOVE(uint_1, uint_3,
            determine_type(board, u64_2), EMPTY, 0), board);
    }
}

/**
    @brief Determines whether the given cell index is under attack.

    This function efficiently generates captures moves and such, to check
    the given cell is currently under attack for the player denoted by
    'gen_side'. It works by imagining a rook and bishop on the cell, belonging
    to 'gen_side', and finding if this rook or bishop attacks any pieces.
    If the attacked pieces happen to be queens, rooks or bishops of the
    opposite side, the cell is under attack. There are also checks to check
    for pawns and knights. A lot effort was put into making this function fast.

    @param index is the integer index of the cell to check in LERF layout.
    @param gen_side is the side to be considered when checking whether the cell
           indexed by 'index' is attacked. It represents the defender.
    @param board is the board to check on.

    @return bool denoting whether the cell indexed by 'index' is under attack
            by the opposite side (opposite to 'gen_side').

    @warning 'index' must be between (or equal to) 0 and 63.
    @warning 'index' must be in LERF layout.
*/

bool is_sq_attacked(unsigned int index, bool gen_side, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.

    unsigned int uint_1, uint_2; // Temporary variables.
    uint64 u64_1, u64_2, u64_3; // Temporary variables.

    u64_1 = GET_BB(index);

    // Check for pawns

    if(gen_side == WHITE && (u64_1 & B_RANK[8]) == 0ULL) // Black pawns
    {
        if(((u64_1 << 7 | u64_1 << 9) & B_RANK[GET_RANK(index + 8)])
            & board.chessboard[bP])
            return 1;
    }
    else if((u64_1 & B_RANK[1]) == 0ULL) // White pawns
    {
        if(((u64_1 >> 7 | u64_1 >> 9) & B_RANK[GET_RANK(index - 8)])
            & board.chessboard[wP])
            return 1;
    }

    // Check knights

    if(gen_side == WHITE) u64_2 = KNIGHT_LT[index] & black_bb;
    else u64_2 = KNIGHT_LT[index] & white_bb;

    uint_1 = CNT_BITS(u64_2);

    for(unsigned int i = 0; i < uint_1; i++)
    {
        u64_3 = GET_BB(POP_BIT(u64_2));

        assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
        uint_2 = determine_type(board, u64_3);

        if(gen_side == WHITE) // Check if black knight
        {
            if(uint_2 == bN) return 1;
        }
        else // Check if white knight
        {
            if(uint_2 == wN) return 1;
        }
    }

    // Check lines

    // North

    u64_2 = LINE_N_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 << 8) | (u64_2 << 16) | (u64_2 << 24) |
        (u64_2 << 32) | (u64_2 << 40) | (u64_2 << 48);
    u64_2 &= LINE_N_LT[index];
    u64_2 ^= LINE_N_LT[index];

    if(u64_3)
    {
        uint_2 = CNT_BITS(u64_2);
        uint_2--;
    }
    else uint_2 = CNT_BITS(u64_2);

    for(unsigned int i = 0; i < uint_2; i++) POP_BIT(u64_2);

    if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
        (gen_side == BLACK && (u64_2 & white_bb))))
    {
        assert((u64_2 != 0ULL) && ((u64_2 & (u64_2 - 1)) == 0ULL));
        uint_2 = determine_type(board, u64_2);

        if(gen_side == WHITE) // Check if black rook/queen
        {
            if(uint_2 == bR || uint_2 == bQ) return 1;
        }
        else // Check if white rook/queen
        {
            if(uint_2 == wR || uint_2 == wQ) return 1;
        }
    }

    // South

    u64_2 = LINE_S_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 >> 8) | (u64_2 >> 16) | (u64_2 >> 24) |
        (u64_2 >> 32) | (u64_2 >> 40) | (u64_2 >> 48);
    u64_2 &= LINE_S_LT[index];
    u64_2 ^= LINE_S_LT[index];

    if(u64_3)
    {
        u64_3 = GET_BB(POP_BIT(u64_2));

        if((gen_side == WHITE && (u64_3 & black_bb)) ||
            (gen_side == BLACK && (u64_3 & white_bb)))
        {
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            uint_2 = determine_type(board, u64_3);

            if(gen_side == WHITE) // Check if black rook/queen
            {
                if(uint_2 == bR || uint_2 == bQ) return 1;
            }
            else // Check if white rook/queen
            {
                if(uint_2 == wR || uint_2 == wQ) return 1;
            }
        }
    }

    // East

    u64_2 = LINE_E_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 << 1) | (u64_2 << 2) | (u64_2 << 3) |
        (u64_2 << 4) | (u64_2 << 5) | (u64_2 << 6);
    u64_2 &= LINE_E_LT[index];
    u64_2 ^= LINE_E_LT[index];

    if(u64_3)
    {
        uint_2 = CNT_BITS(u64_2);
        uint_2--;
    }
    else uint_2 = CNT_BITS(u64_2);

    for(unsigned int i = 0; i < uint_2; i++) POP_BIT(u64_2);

    if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
        (gen_side == BLACK && (u64_2 & white_bb))))
    {
        assert((u64_2 != 0ULL) && ((u64_2 & (u64_2 - 1)) == 0ULL));
        uint_2 = determine_type(board, u64_2);

        if(gen_side == WHITE) // Check if black rook/queen
        {
            if(uint_2 == bR || uint_2 == bQ) return 1;
        }
        else // Check if white rook/queen
        {
            if(uint_2 == wR || uint_2 == wQ) return 1;
        }
    }

    // West

    u64_2 = LINE_W_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 >> 1) | (u64_2 >> 2) | (u64_2 >> 3) |
        (u64_2 >> 4) | (u64_2 >> 5) | (u64_2 >> 6);
    u64_2 &= LINE_W_LT[index];
    u64_2 ^= LINE_W_LT[index];

    if(u64_3)
    {
        u64_3 = GET_BB(POP_BIT(u64_2));

        if((gen_side == WHITE && (u64_3 & black_bb)) ||
            (gen_side == BLACK && (u64_3 & white_bb)))
        {
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            uint_2 = determine_type(board, u64_3);

            if(gen_side == WHITE) // Check if black rook/queen
            {
                if(uint_2 == bR || uint_2 == bQ) return 1;
            }
            else // Check if white rook/queen
            {
                if(uint_2 == wR || uint_2 == wQ) return 1;
            }
        }
    }

    // Check diagonals

    // Northeast

    u64_2 = DIAG_NE_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 << 9) | (u64_2 << 18) | (u64_2 << 27) |
        (u64_2 << 36) | (u64_2 << 45) | (u64_2 << 54);
    u64_2 &= DIAG_NE_LT[index];
    u64_2 ^= DIAG_NE_LT[index];

    if(u64_3)
    {
        uint_2 = CNT_BITS(u64_2);
        uint_2--;
    }
    else uint_2 = CNT_BITS(u64_2);

    for(unsigned int i = 0; i < uint_2; i++) POP_BIT(u64_2);

    if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
        (gen_side == BLACK && (u64_2 & white_bb))))
    {
        assert((u64_2 != 0ULL) && ((u64_2 & (u64_2 - 1)) == 0ULL));
        uint_2 = determine_type(board, u64_2);

        if(gen_side == WHITE) // Check if black bishop/queen
        {
            if(uint_2 == bB || uint_2 == bQ) return 1;
        }
        else // Check if white bishop/queen
        {
            if(uint_2 == wB || uint_2 == wQ) return 1;
        }
    }

    // Northwest

    u64_2 = DIAG_NW_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 << 7) | (u64_2 << 14) | (u64_2 << 21) |
        (u64_2 << 28) | (u64_2 << 35) | (u64_2 << 42);
    u64_2 &= DIAG_NW_LT[index];
    u64_2 ^= DIAG_NW_LT[index];

    if(u64_3)
    {
        uint_2 = CNT_BITS(u64_2);
        uint_2--;
    }
    else uint_2 = CNT_BITS(u64_2);

    for(unsigned int i = 0; i < uint_2; i++) POP_BIT(u64_2);

    if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
        (gen_side == BLACK && (u64_2 & white_bb))))
    {
        assert((u64_2 != 0ULL) && ((u64_2 & (u64_2 - 1)) == 0ULL));
        uint_2 = determine_type(board, u64_2);

        if(gen_side == WHITE) // Check if black bishop/queen
        {
            if(uint_2 == bB || uint_2 == bQ) return 1;
        }
        else // Check if white bishop/queen
        {
            if(uint_2 == wB || uint_2 == wQ) return 1;
        }
    }

    // Southeast

    u64_2 = DIAG_SE_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 >> 7) | (u64_2 >> 14) | (u64_2 >> 21) |
        (u64_2 >> 28) | (u64_2 >> 35) | (u64_2 >> 42);
    u64_2 &= DIAG_SE_LT[index];
    u64_2 ^= DIAG_SE_LT[index];

    if(u64_3)
    {
        u64_3 = GET_BB(POP_BIT(u64_2));

        if((gen_side == WHITE && (u64_3 & black_bb)) ||
            (gen_side == BLACK && (u64_3 & white_bb)))
        {
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            uint_2 = determine_type(board, u64_3);

            if(gen_side == WHITE) // Check if black bishop/queen
            {
                if(uint_2 == bB || uint_2 == bQ) return 1;
            }
            else // Check if white bishop/queen
            {
                if(uint_2 == wB || uint_2 == wQ) return 1;
            }
        }
    }

    // Southwest

    u64_2 = DIAG_SW_LT[index] & OCC;
    u64_3 = u64_2;

    u64_2 = (u64_2 >> 9) | (u64_2 >> 18) | (u64_2 >> 27) |
        (u64_2 >> 36) | (u64_2 >> 45) | (u64_2 >> 54);
    u64_2 &= DIAG_SW_LT[index];
    u64_2 ^= DIAG_SW_LT[index];

    if(u64_3)
    {
        u64_3 = GET_BB(POP_BIT(u64_2));

        if((gen_side == WHITE && (u64_3 & black_bb)) ||
            (gen_side == BLACK && (u64_3 & white_bb)))
        {
            assert((u64_3 != 0ULL) && ((u64_3 & (u64_3 - 1)) == 0ULL));
            uint_2 = determine_type(board, u64_3);

            if(gen_side == WHITE) // Check if black bishop/queen
            {
                if(uint_2 == bB || uint_2 == bQ) return 1;
            }
            else // Check if white bishop/queen
            {
                if(uint_2 == wB || uint_2 == wQ) return 1;
            }
        }
    }

    // Check neighbouring cells for kings

    if(gen_side == WHITE) // Check for black king
    {
        if(KING_LT[index] & board.chessboard[bK]) return 1;
    }
    else // Check for white king
    {
        if(KING_LT[index] & board.chessboard[wK]) return 1;
    }

    return 0;
}

/**
    @brief Generates and returns a move list vector of all the possible
           pseudo-legal moves for the given board state.

    @param board is the board to generate all pseudo-legal moves for.

    @return MoveList representing a collection of all pseudo-legal
            moves for the given board state.
*/

MoveList gen_moves(const Board& board)
{
    MoveList ml; // Move list structure.

    // Queens

    // Line moves

    if(board.side == WHITE) // White queens
        gen_rook_moves(board.chessboard[wQ], WHITE, ml, board);
    else // Black queens
        gen_rook_moves(board.chessboard[bQ], BLACK, ml, board);

    // Diagonal moves

    if(board.side == WHITE) // White queens
        gen_bishop_moves(board.chessboard[wQ], WHITE, ml, board);
    else // Black queens
        gen_bishop_moves(board.chessboard[bQ], BLACK, ml, board);

    // Rooks

    if(board.side == WHITE) // White rooks
        gen_rook_moves(board.chessboard[wR], WHITE, ml, board);
    else // Black rooks
        gen_rook_moves(board.chessboard[bR], BLACK, ml, board);

    // Knights

    if(board.side == WHITE) // White knights
        gen_knight_moves(board.chessboard[wN], WHITE, ml, board);
    else // Black knights
        gen_knight_moves(board.chessboard[bN], BLACK, ml, board);

    // Bishops

    if(board.side == WHITE) // White bishops
        gen_bishop_moves(board.chessboard[wB], WHITE, ml, board);
    else // Black bishops
        gen_bishop_moves(board.chessboard[bB], BLACK, ml, board);

    // Pawns

    gen_pawn_moves(board.side, ml, board);

    // King

    gen_king_moves(board.side, ml, board);

    return ml;
}

/**
    @brief Generates and returns a move list vector of all the possible
           pseudo-legal capture moves for the given board state.

    @param board is the board to generate all pseudo-legal capture moves for.

    @return MoveList representing a collection of all pseudo-legal
            capture moves for the given board state.
*/

MoveList gen_captures(const Board& board)
{
    MoveList ml; // Move list structure.

    // Queens

    // Line moves

    if(board.side == WHITE) // White queens
        gen_rook_cap_moves(board.chessboard[wQ], WHITE, ml, board);
    else // Black queens
        gen_rook_cap_moves(board.chessboard[bQ], BLACK, ml, board);

    // Diagonal moves

    if(board.side == WHITE) // White queens
        gen_bishop_cap_moves(board.chessboard[wQ], WHITE, ml, board);
    else // Black queens
        gen_bishop_cap_moves(board.chessboard[bQ], BLACK, ml, board);

    // Rooks

    if(board.side == WHITE) // White rooks
        gen_rook_cap_moves(board.chessboard[wR], WHITE, ml, board);
    else // Black rooks
        gen_rook_cap_moves(board.chessboard[bR], BLACK, ml, board);

    // Knights

    if(board.side == WHITE) // White knights
        gen_knight_cap_moves(board.chessboard[wN], WHITE, ml, board);
    else // Black knights
        gen_knight_cap_moves(board.chessboard[bN], BLACK, ml, board);

    // Bishops

    if(board.side == WHITE) // White bishops
        gen_bishop_cap_moves(board.chessboard[wB], WHITE, ml, board);
    else // Black bishops
        gen_bishop_cap_moves(board.chessboard[bB], BLACK, ml, board);

    // Pawns

    gen_pawn_cap_moves(board.side, ml, board);

    // King

    gen_king_cap_moves(board.side, ml, board);

    return ml;
}

/**
    @brief Generates and returns a move list vector of all the possible
           legal moves for the given board state.

    @param board is the board to generate all legal moves for.

    @return MoveList representing a collection of all legal moves for
            the given board state.

    @warning The 'attacked' bitboard inside the bitboard is not calculated.
*/

MoveList gen_legal_moves(Board& board)
{
    unsigned int list_move, list_size;

    MoveList ml;
    MoveList pseudo_moves = gen_moves(board);

    list_size = pseudo_moves.list.size();

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = pseudo_moves.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        undo_move(board);
        ml.list.push_back(pseudo_moves.list.at(i));
    }

    return ml;
}

/**
    @brief Generates and returns a move list vector of all the possible
           legal capture moves for the given board state.

    @param board is the board to generate all legal capture moves for.

    @return MoveList representing a collection of all legal capture
            moves for the given board state.

    @warning The 'attacked' bitboard inside the bitboard is not calculated.
*/

MoveList gen_legal_captures(Board& board)
{
    unsigned int list_move, list_size;

    MoveList ml;
    MoveList pseudo_moves = gen_captures(board);

    list_size = pseudo_moves.list.size();

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = pseudo_moves.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        undo_move(board);
        ml.list.push_back(pseudo_moves.list.at(i));
    }

    return ml;
}