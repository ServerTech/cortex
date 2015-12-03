/*
    Cortex - Self-learning Chess Engine
    @filename evaluate.cpp
    @author Anna Grygierzec
    @version 0.1.0

    @brief Static evaluation function that returns an objective score
           of the game state.

    ******************** VERSION CONTROL ********************
    * 28/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
*/

#include "evaluate.h"

// Globals

const int ROOK_ST[64] = {
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
25  ,   25  ,   25  ,   25  ,   25  ,   25  ,   25  ,   25  ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0
};

const int KNIGHT_ST[64] = {
0   ,  -10  ,   0   ,   0   ,   0   ,   0   ,  -10  ,   0   ,
0   ,   0   ,   0   ,   5   ,   5   ,   0   ,   0   ,   0   ,
0   ,   0   ,   10  ,   10  ,   10  ,   10  ,   0   ,   0   ,
0   ,   0   ,   10  ,   20  ,   20  ,   10  ,   5   ,   0   ,
5   ,   10  ,   15  ,   20  ,   20  ,   15  ,   10  ,   5   ,
5   ,   10  ,   10  ,   20  ,   20  ,   10  ,   10  ,   5   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0
};

const int BISHOP_ST[64] = {
0   ,   0   ,  -10  ,   0   ,   0   ,  -10  ,   0   ,   0   ,
0   ,   0   ,   0   ,   10  ,   10  ,   0   ,   0   ,   0   ,
0   ,   0   ,   10  ,   15  ,   15  ,   10  ,   0   ,   0   ,
0   ,   10  ,   15  ,   20  ,   20  ,   15  ,   10  ,   0   ,
0   ,   10  ,   15  ,   20  ,   20  ,   15  ,   10  ,   0   ,
0   ,   0   ,   10  ,   15  ,   15  ,   10  ,   0   ,   0   ,
0   ,   0   ,   0   ,   10  ,   10  ,   0   ,   0   ,   0   ,
0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0
};

const int PAWN_ST[64] = {
0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
10  ,   10  ,   0   ,  -10  ,  -10  ,   0   ,   10  ,   10  ,
5   ,   0   ,   0   ,   5   ,   5   ,   0   ,   0   ,   5   ,
0   ,   0   ,   10  ,   20  ,   20  ,   10  ,   0   ,   0   ,
5   ,   5   ,   5   ,   10  ,   10  ,   5   ,   5   ,   5   ,
10  ,   10  ,   10  ,   20  ,   20  ,   10  ,   10  ,   10  ,
20  ,   20  ,   20  ,   30  ,   30  ,   20  ,   20  ,   20  ,
0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0
};

const int MIRROR[64] = {
56  ,   57  ,   58  ,   59  ,   60  ,   61  ,   62  ,   63  ,
48  ,   49  ,   50  ,   51  ,   52  ,   53  ,   54  ,   55  ,
40  ,   41  ,   42  ,   43  ,   44  ,   45  ,   46  ,   47  ,
32  ,   33  ,   34  ,   35  ,   36  ,   37  ,   38  ,   39  ,
24  ,   25  ,   26  ,   27  ,   28  ,   29  ,   30  ,   31  ,
16  ,   17  ,   18  ,   19  ,   20  ,   21  ,   22  ,   23  ,
8   ,   9   ,   10  ,   11  ,   12  ,   13  ,   14  ,   15  ,
0   ,   1   ,   2   ,   3   ,   4   ,   5   ,   6   ,   7
};

// Function prototypes

int static_eval(Board& board);

// Functions

/**
    @brief Performs a static evaluation of the given board state and deduces
           an objective integer score for it.

    @param board is the game state to evaluate.

    @return integer value representing the score, in NegaMax fashion.

    @warning The score is calculated and returned in NegaMax fashion, that is,
             relative to each side. A higher score is always better, for the
             side given by the 'side' variable in 'board'.
    @warning Assumes exactly one king exists.
*/

int static_eval(Board& board)
{
    int score = 0;

    unsigned int uint_1; // Temporary variables.
    uint64 u64_1; // Temporary variables.
/*
    // White's material

    score += CNT_BITS(board.chessboard[wQ]) * 900; // White queens
    score += CNT_BITS(board.chessboard[wR]) * 500; // White rooks
    score += CNT_BITS(board.chessboard[wK]) * 315; // White knights
    score += CNT_BITS(board.chessboard[wB]) * 300; // White bishops
    score += CNT_BITS(board.chessboard[wP]) * 100; // White pawns

    // Black's material

    score -= CNT_BITS(board.chessboard[bQ]) * 900; // Black queens
    score -= CNT_BITS(board.chessboard[bR]) * 500; // Black rooks
    score -= CNT_BITS(board.chessboard[bK]) * 315; // Black knights
    score -= CNT_BITS(board.chessboard[bB]) * 300; // Black bishops
    score -= CNT_BITS(board.chessboard[bP]) * 100; // Black pawns
*/
    // Piece-square tables

    // White

    // White Queens

    score += CNT_BITS(board.chessboard[wQ]) * 900;

    // White rooks

    u64_1 = board.chessboard[wR];
    uint_1 = CNT_BITS(u64_1);
    score += uint_1 * 500; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score += ROOK_ST[POP_BIT(u64_1)];
    }

    // White knights

    u64_1 = board.chessboard[wN];
    uint_1 = CNT_BITS(u64_1);
    score += uint_1 * 315; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score += KNIGHT_ST[POP_BIT(u64_1)];
    }

    // White bishops

    u64_1 = board.chessboard[wB];
    uint_1 = CNT_BITS(u64_1);
    score += uint_1 * 300; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score += BISHOP_ST[POP_BIT(u64_1)];
    }

    // White pawns

    u64_1 = board.chessboard[wP];
    uint_1 = CNT_BITS(u64_1);
    score += uint_1 * 100; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score += PAWN_ST[POP_BIT(u64_1)];
    }

    // Black

    // Black Queens

    score -= CNT_BITS(board.chessboard[bQ]) * 900;

    // Black rooks

    u64_1 = board.chessboard[bR];
    uint_1 = CNT_BITS(u64_1);
    score -= uint_1 * 500; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score -= ROOK_ST[MIRROR[POP_BIT(u64_1)]];
    }

    // Black knights

    u64_1 = board.chessboard[bN];
    uint_1 = CNT_BITS(u64_1);
    score -= uint_1 * 315; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score -= KNIGHT_ST[MIRROR[POP_BIT(u64_1)]];
    }

    // Black bishops

    u64_1 = board.chessboard[bB];
    uint_1 = CNT_BITS(u64_1);
    score -= uint_1 * 300; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score -= BISHOP_ST[MIRROR[POP_BIT(u64_1)]];
    }

    // Black pawns

    u64_1 = board.chessboard[bP];
    uint_1 = CNT_BITS(u64_1);
    score -= uint_1 * 100; // Material score

    for(unsigned int i = 0; i < uint_1; i++)
    {
        score -= PAWN_ST[MIRROR[POP_BIT(u64_1)]];
    }

    if(board.side == WHITE) return score;
    else return -score;
}