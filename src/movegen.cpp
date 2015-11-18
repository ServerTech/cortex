/*
    Cortex - Self-learning Chess Engine
    @filename movegen.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief Generates moves given a board position.

    ******************** VERSION CONTROL ********************
    * 15/11/2015 File created.
    * 15/11/2015 0.1.0 Initial version.
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

    int s = list.size();

    for(int i = 0; i < s; i++)
    {
        pretty_str << "Move " << i + 1 << ": " << COORD_MOVE(list.at(i).move);
        pretty_str << "    Score: " << list.at(i).score;
        pretty_str << "    Captured: " << CAPTURED(list.at(i).move);
        pretty_str << "    Promoted: " << PROMOTED(list.at(i).move) << "\n";
    }

    pretty_str << "\nTotal moves: " << s;

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
    Move move_to_add(move, 0);
    list.push_back(move_to_add);
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
    Move move_to_add(move, 0);
    list.push_back(move_to_add);
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
    MoveList ml;
    ml.list.reserve(50); // Reserve enough memory for fifty moves.

    const uint64 white_bb = board.chessboard[ALL_WHITE]; // White bitboard.
    const uint64 black_bb = board.chessboard[ALL_BLACK]; // Black bitboard.

    const uint64 OCC = white_bb | black_bb; // Occupied bitboard.

    const uint64 FREE = ~OCC; // Free bitboard.

    uint64 moves; // Temporary bitboard to store moves as they are generated.
    unsigned int uint_1, uint_2, uint_3; // Temporary variables.
    uint64 u64_1, u64_2, u64_3, u64_4; // Temporary variables.
    unsigned int bit_cnt; // Number of bits in 'moves'; temporary variable.

    // Pawns

    // White Pawns

    if(board.side == WHITE)
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

            if(u64_3 != 0ULL)
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

                u64_2 = (u64_2 << 16) & B_RANK[2] & FREE;

                if(u64_2 != 0ULL) push_quiet_move(ml.list,
                    GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, MFLAGPS),
                    board);
            }

            // Captures

            u64_2 = GET_BB(uint_1);

            // Capture left

            u64_3 = (u64_2 << 7) & B_RANK[GET_RANK(uint_1 + 8)] &
               (black_bb | GET_BB(board.en_pas_sq));

            if(u64_3 != 0)
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

            u64_3 = (u64_2 << 9) & B_RANK[GET_RANK(uint_1 + 8)] &
                (black_bb | GET_BB(board.en_pas_sq));

            if(u64_3 != 0)
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

                u64_2 = (u64_2 >> 16) & B_RANK[6] & FREE;

                if(u64_2 != 0ULL) push_quiet_move(ml.list,
                    GET_MOVE(uint_1, POP_BIT(u64_2), EMPTY, EMPTY, MFLAGPS),
                    board);
            }

            // Captures

            u64_2 = GET_BB(uint_1);

            // Capture left

            u64_3 = (u64_2 >> 7) & B_RANK[GET_RANK(uint_1 - 8)] &
               (white_bb | GET_BB(board.en_pas_sq));

            if(u64_3 != 0)
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

            u64_3 = (u64_2 >> 9) & B_RANK[GET_RANK(uint_1 - 8)] &
                (white_bb | GET_BB(board.en_pas_sq));

            if(u64_3 != 0)
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

    return ml;
}