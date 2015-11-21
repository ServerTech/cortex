/*
    Cortex - Self-learning Chess Engine
    @filename movegen.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Generates moves given a board position.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 21/11/2015 0.1.0 Initial version.
*/

#include <sstream> // std::stringstream

#include "movegen.h"
#include "lookup_tables.h"

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

        if(CAPTURED(list.at(i).move) != EMPTY)
        {
            cap++;
            pretty_str << CAPTURED(list.at(i).move);
        }
        else pretty_str << "-";

        pretty_str << "    Promoted: ";

        if(PROMOTED(list.at(i).move) != EMPTY)
        {
            prom++;
            if(CAPTURED(list.at(i).move) != EMPTY) prom_cap++;
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
    @brief Generates and pushes all pseudo-legal rook moves into the move
           list vector for the given board state.

    This function generates all pseudo-legal moves for a given bitboard,
    considering all set bits as rooks. This is also useful for generating
    line moves for queens.

    @param u64_1 is the bitboard representing all pieces which are to be
           considered as rooks during generation.
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_rook_moves(uint64 u64_1, MoveList& ml, const Board& board)
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

        if(u64_3 && ((board.side == WHITE && (u64_2 & black_bb)) ||
            (board.side == BLACK && (u64_2 & white_bb))))
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

            if((board.side == WHITE && (u64_3 & black_bb)) ||
                (board.side == BLACK && (u64_3 & white_bb)))
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

        if(u64_3 && ((board.side == WHITE && (u64_2 & black_bb)) ||
            (board.side == BLACK && (u64_2 & white_bb))))
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

            if((board.side == WHITE && (u64_3 & black_bb)) ||
                (board.side == BLACK && (u64_3 & white_bb)))
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

    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_knight_moves(MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 FREE = ~white_bb & ~black_bb; // Free bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    // Generation

    if(board.side == WHITE) u64_1 = board.chessboard[wN];
    else u64_1 = board.chessboard[bN];

    bit_cnt = CNT_BITS(u64_1);

    for(unsigned int i = 0; i < bit_cnt; i++)
    {
        uint_1 = POP_BIT(u64_1);

        // Captures

        if(board.side == WHITE) u64_2 = KNIGHT_LT[uint_1] & black_bb;
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
    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.
*/

void gen_bishop_moves(uint64 u64_1, MoveList& ml, const Board& board)
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

        if(u64_3 && ((board.side == WHITE && (u64_2 & black_bb)) ||
            (board.side == BLACK && (u64_2 & white_bb))))
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

        if(u64_3 && ((board.side == WHITE && (u64_2 & black_bb)) ||
            (board.side == BLACK && (u64_2 & white_bb))))
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

            if((board.side == WHITE && (u64_3 & black_bb)) ||
                (board.side == BLACK && (u64_3 & white_bb)))
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

            if((board.side == WHITE && (u64_3 & black_bb)) ||
                (board.side == BLACK && (u64_3 & white_bb)))
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

    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.

    @warning Pawns shouldn't be present on the promotion ranks (1 and 8).
*/

void gen_pawn_moves(MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.
    const uint64 FREE = ~OCC; // Free bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2, u64_3, u64_4; // Temporary variables.
    unsigned int bit_cnt; // Number of bits; temporary variable.

    if(board.side == WHITE) // White Pawns
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

    @param ml is the move list structure to which the generated moves are
           to be pushed.
    @param board is the board on which the moves are to be generated.

    @return void.

    @warning There must be only ONE king.
*/

void gen_king_moves(MoveList& ml, const Board& board)
{
    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 FREE = ~white_bb & ~black_bb; // Free bitboard.

    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1; // Temporary variable.

    // Generation

    if(board.side == WHITE) u64_1 = board.chessboard[wK];
    else u64_1 = board.chessboard[bK];

    assert((u64_1 != 0ULL) && ((u64_1 & (u64_1 - 1)) == 0ULL));

    uint_1 = POP_BIT(u64_1);

    // Captures

    if(board.side == WHITE) u64_1 = KING_LT[uint_1] & black_bb;
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
        gen_rook_moves(board.chessboard[wQ], ml, board);
    else // Black queens
        gen_rook_moves(board.chessboard[bQ], ml, board);

    // Diagonal moves

    if(board.side == WHITE) // White queens
        gen_bishop_moves(board.chessboard[wQ], ml, board);
    else // Black queens
        gen_bishop_moves(board.chessboard[bQ], ml, board);

    // Rooks

    if(board.side == WHITE) // White rooks
        gen_rook_moves(board.chessboard[wR], ml, board);
    else // Black rooks
        gen_rook_moves(board.chessboard[bR], ml, board);

    // Knights

    gen_knight_moves(ml, board);

    // Bishops

    if(board.side == WHITE) // White bishops
        gen_bishop_moves(board.chessboard[wB], ml, board);
    else // Black bishops
        gen_bishop_moves(board.chessboard[bB], ml, board);

    // Pawns

    gen_pawn_moves(ml, board);

    // King

    gen_king_moves(ml, board);

    return ml;
}