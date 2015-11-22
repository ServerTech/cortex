/*
    Cortex - Self-learning Chess Engine
    @filename movegen.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Generates moves given a board position.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 22/11/2015 0.1.0 Initial version.
*/

#include <sstream> // std::stringstream

#include "movegen.h"
#include "lookup_tables.h"

// Prototypes

std::string pretty_move_list(const std::vector<Move>& list);
inline void push_quiet_move(std::vector<Move>& list, unsigned int move,
    const Board& board);
inline void push_capture_move(std::vector<Move>& list, unsigned int move,
    const Board& board);
inline void push_castling_move(std::vector<Move>& list, unsigned int move);
void gen_rook_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_knight_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_bishop_moves(uint64 u64_1, bool gen_side, MoveList& ml,
    const Board& board);
void gen_pawn_moves(bool gen_side, MoveList& ml, const Board& board);
void gen_king_moves(bool gen_side, MoveList& ml, const Board& board);
bool is_sq_attacked(unsigned int index, bool chk_side, const Board& board);
MoveList gen_moves(const Board& board);

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

    @param list is the move list vector.
    @param move is an integer value representing a move.
    @param board is the board the move is being made on.

    @return void.
*/

inline void push_quiet_move(std::vector<Move>& list, unsigned int move,
    const Board& board)
{
    Move move_push(move, 0);
    list.push_back(move_push);
}

/**
    @brief Pushes a capture move to the move list vector.

    @param list is the move list vector.
    @param move is an integer value representing the move.
    @param board is the board the move is being made on.

    @return void.
*/

inline void push_capture_move(std::vector<Move>& list, unsigned int move,
    const Board& board)
{
    Move move_push(move, 0);
    list.push_back(move_push);
}

/**
    @brief Pushes a castling move to the move list vector.

    @param list is the move list vector.
    @param move is an integer value representing the move.

    @return void.
*/

inline void push_castling_move(std::vector<Move>& list, unsigned int move)
{
    Move move_push(move, 0);
    list.push_back(move_push);
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
            push_quiet_move(ml.list,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            ml.attacked |= u64_3;
            push_capture_move(ml.list, GET_MOVE(uint_1, POP_BIT(u64_2),
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
                ml.attacked |= u64_3;
                push_capture_move(ml.list, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml.list,
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
            push_quiet_move(ml.list,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            ml.attacked |= u64_3;
            push_capture_move(ml.list, GET_MOVE(uint_1, POP_BIT(u64_2),
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
                ml.attacked |= u64_3;
                push_capture_move(ml.list, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml.list,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }
    }
}

/**
    @brief Generates and pushes all pseudo-legal knight moves into the move
           list vector for the given board state.

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
    uint64 u64_2; // Temporary variable.
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
            u64_2 = GET_BB(uint_3);
            ml.attacked |= u64_2;
            push_capture_move(ml.list, GET_MOVE(uint_1, uint_3,
                determine_type(board, u64_2), EMPTY, 0), board);
        }

        u64_2 = KNIGHT_LT[uint_1] & FREE;

        uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml.list, GET_MOVE(uint_1, POP_BIT(u64_2),
                EMPTY, EMPTY, 0), board);
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
            push_quiet_move(ml.list,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            ml.attacked |= u64_3;
            push_capture_move(ml.list, GET_MOVE(uint_1, POP_BIT(u64_2),
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
            push_quiet_move(ml.list,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
        }

        // Pop the capture move last.

        if(u64_3 && ((gen_side == WHITE && (u64_2 & black_bb)) ||
            (gen_side == BLACK && (u64_2 & white_bb))))
        {
            u64_3 = u64_2;
            ml.attacked |= u64_3;
            push_capture_move(ml.list, GET_MOVE(uint_1, POP_BIT(u64_2),
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
                ml.attacked |= u64_3;
                push_capture_move(ml.list, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml.list,
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
                ml.attacked |= u64_3;
                push_capture_move(ml.list, GET_MOVE(uint_1, uint_3,
                    determine_type(board, u64_3), EMPTY, 0), board);
            }
        }
        else uint_2 = CNT_BITS(u64_2);

        for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
        {
            push_quiet_move(ml.list,
                GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, 0), board);
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
            u64_3 = 0ULL;
            u64_3 |= (u64_2 << 8) & FREE;

            if(u64_3)
            {
                u64_4 = u64_3;
                uint_2 = POP_BIT(u64_4);

                if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, wB, 0), board);
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, wR, 0), board);
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, wN, 0), board);
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, wQ, 0), board);
                }
                else
                {
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, EMPTY, 0),
                        board);
                }

                // Two cells upward

                u64_2 = (u64_2 << 16) & B_RANK[4] & FREE;

                if(u64_2 != 0ULL) push_quiet_move(ml.list,
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
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                ml.attacked |= u64_3;

                if(uint_3 == board.en_pas_sq)
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, bP, EMPTY, MFLAGEP),
                        board);
                }
                else if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wB, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wR, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wN, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wQ, 0), board);
                }
                else
                {
                    push_capture_move(ml.list,
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
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                ml.attacked |= u64_3;

                if(uint_3 == board.en_pas_sq)
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, bP, EMPTY, MFLAGEP),
                        board);
                }
                else if(u64_3 & B_RANK[8]) // Check if the pawn reached rank 8.
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wB, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wR, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wN, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, wQ, 0), board);
                }
                else
                {
                    push_capture_move(ml.list,
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
            u64_3 = 0ULL;
            u64_3 |= (u64_2 >> 8) & FREE;

            if(u64_3 != 0ULL)
            {
                u64_4 = u64_3;
                uint_2 = POP_BIT(u64_4);

                if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, bB, 0), board);
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, bR, 0), board);
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, bN, 0), board);
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, bQ, 0), board);
                }
                else
                {
                    push_quiet_move(ml.list,
                        GET_MOVE(uint_1, uint_2, EMPTY, EMPTY, 0),
                        board);
                }

                // Two cells downward

                u64_2 = (u64_2 >> 16) & B_RANK[5] & FREE;

                if(u64_2 != 0ULL) push_quiet_move(ml.list,
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
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                ml.attacked |= u64_3;

                if(uint_3 == board.en_pas_sq)
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, wP, EMPTY, MFLAGEP),
                        board);
                }
                else if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bB, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bR, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bN, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bQ, 0), board);
                }
                else
                {
                    push_capture_move(ml.list,
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
                uint_2 = determine_type(board, u64_3);
                u64_4 = u64_3;
                uint_3 = POP_BIT(u64_4);

                ml.attacked |= u64_3;

                if(uint_3 == board.en_pas_sq)
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, wP, EMPTY, MFLAGEP),
                        board);
                }
                else if(u64_3 & B_RANK[1]) // Check if the pawn reached rank 1.
                {
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bB, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bR, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bN, 0), board);
                    push_capture_move(ml.list,
                        GET_MOVE(uint_1, uint_3, uint_2, bQ, 0), board);
                }
                else
                {
                    push_capture_move(ml.list,
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
    uint64 u64_1; // Temporary variable.
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
        u64_1 = GET_BB(uint_3);
        ml.attacked |= u64_1;
        push_capture_move(ml.list, GET_MOVE(uint_1, uint_3,
            determine_type(board, u64_1), EMPTY, 0), board);
    }

    u64_1 = KING_LT[uint_1] & FREE;

    uint_2 = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < uint_2; i++) // Push quiet moves.
    {
        push_quiet_move(ml.list, GET_MOVE(uint_1, POP_BIT(u64_1),
            EMPTY, EMPTY, 0), board);
    }

    // Castling

    if(board.castle_perm)
    {
        if(gen_side == WHITE)
        {
            not_in_check = !is_sq_attacked(e1, WHITE, board);

            if(board.castle_perm & WKCA) // White king-side castling
            {
                if(not_in_check && !is_sq_attacked(f1, WHITE, board))
                {
                    push_castling_move(ml.list, GET_MOVE(e1, g1, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }

            if(board.castle_perm & WQCA) // White queen-side castling
            {
                if(not_in_check && !is_sq_attacked(d1, WHITE, board))
                {
                    push_castling_move(ml.list, GET_MOVE(e1, d1, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }
        }
        else
        {
            not_in_check = !is_sq_attacked(e1, BLACK, board);

            if(board.castle_perm & BKCA) // Black king-side castling
            {
                if(not_in_check && !is_sq_attacked(f8, BLACK, board))
                {
                    push_castling_move(ml.list, GET_MOVE(e8, g8, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }

            if(board.castle_perm & BQCA) // Black queen-side castling
            {
                if(not_in_check && !is_sq_attacked(d8, BLACK, board))
                {
                    push_castling_move(ml.list, GET_MOVE(e8, c8, EMPTY, EMPTY,
                        MFLAGCA));
                }
            }
        }
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

    if(gen_side == WHITE) // Check for black pawns
    {
        if((u64_1 << 7 | u64_1 << 9) & board.chessboard[bP])
            return 1;
    }
    else // Check for white pawns
    {
        if((u64_1 >> 7 | u64_1 >> 9) & board.chessboard[wP])
            return 1;
    }

    // Check knights

    if(gen_side == WHITE) u64_2 = KNIGHT_LT[index] & black_bb;
    else u64_2 = KNIGHT_LT[index] & white_bb;

    uint_1 = CNT_BITS(u64_2);

    for(unsigned int i = 0; i < uint_1; i++)
    {
        uint_2 = determine_type(board, GET_BB(POP_BIT(u64_2)));

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
    }

    return 0;
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
    MoveList ml; // Move list structure.
    ml.list.reserve(50); // Reserve enough memory for fifty moves.

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