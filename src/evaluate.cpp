/*
    Cortex - Self-learning Chess Engine
    @filename evaluate.cpp
    @author Anna Grygierzec
    @version 0.1.2

    @brief Static evaluation function that returns an objective score
           of the game state.

    ******************** VERSION CONTROL ********************
    * 28/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 06/12/2015 0.1.1 Added checks for isolated and passed pawns.
    * 06/12/2015 0.1.2 Added checks of open/half-open files and mobility bonus.
*/

/**
    @file
    @filename evaluate.cpp
    @author Anna Grygierzec

    @brief Static evaluation function that returns an objective score
           of the game state.
*/

#include "defs.h"

#include "evaluate.h"
#include "board.h"
#include "movegen.h"
#include "hash.h"

// Globals

// Piece values

int S_QUEEN = 900;
int S_ROOK = 500;
int S_KNIGHT = 315;
int S_BISHOP = 300;
int S_PAWN = 100;

// Mobility bonuses

int S_QUEEN_MOBILITY = 5;
int S_ROOK_MOBILITY = 5;
int S_KNIGHT_MOBILITY = 5;
int S_BISHOP_MOBILITY = 5;
int S_PAWN_MOBILITY = 5;

// Queens

int S_QUEEN_OPENFILE = 5;
int S_QUEEN_HALFOPENFILE = 3;

// Rooks

int S_ROOK_OPENFILE = 10;
int S_ROOK_HALFOPENFILE = 5;

// Pawns

const int S_PAWN_ISOLATED = -10;
const int S_PAWN_PASSED[9] = { 0, 0, 5, 10, 20, 35, 60, 100, 0 };

uint64 PAWN_ISO_MASK[64]; // Isolated pawn mask.
uint64 PAWN_WPAS_MASK[64]; // White passed pawn mask.
uint64 PAWN_BPAS_MASK[64]; // Black passed pawn mask.

// Piece-square tables

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

// Prototypes

void init_evalmasks();
int static_eval(Board& board);

// Function definitions

/**
    @brief Initialises all bitmasks required for evaluation.

    @return void.
*/

void init_evalmasks()
{
    for(unsigned int i = 0; i < 64; i++)
    {
        PAWN_ISO_MASK[i] = 0ULL;
        PAWN_WPAS_MASK[i] = 0ULL;
        PAWN_BPAS_MASK[i] = 0ULL;
    }

    for(int i = 0; i < 64; i++)
    {
        int sq = i + 8, file;

        while(sq < 64)
        {
            PAWN_WPAS_MASK[i] |= GET_BB(sq);
            sq += 8;
        }

        sq = i - 8;

        while(sq >= 0)
        {
            PAWN_BPAS_MASK[i] |= GET_BB(sq);
            sq -= 8;
        }

        file = GET_FILE(i);

        if(file > FILE_A)
        {
            PAWN_ISO_MASK[i] |= B_FILE[file - 1];

            sq = i + 7;

            while(sq < 64)
            {
                PAWN_WPAS_MASK[i] |= GET_BB(sq);
                sq += 8;
            }

            sq = i - 9;

            while(sq >= 0)
            {
                PAWN_BPAS_MASK[i] |= GET_BB(sq);
                sq -= 8;
            }
        }

        if(file < FILE_H)
        {
            PAWN_ISO_MASK[i] |= B_FILE[file + 1];

            sq = i + 9;

            while(sq < 64)
            {
                PAWN_WPAS_MASK[i] |= GET_BB(sq);
                sq += 8;
            }

            sq = i - 7;

            while(sq >= 0)
            {
                PAWN_BPAS_MASK[i] |= GET_BB(sq);
                sq -= 8;
            }
        }
    }
}

/**
    @brief Performs a static evaluation of the given board state and deduces
           an objective integer score for it.

    @param board is the game state to evaluate.

    @return integer value representing the score, in NegaMax fashion.

    @warning The score is calculated and returned in NegaMax fashion, that is,
             relative to each side. A higher score is always better, for the
             side given by the 'side' variable in 'board'.
    @warning Assumes exactly one king exists.
    @warning Pawms musn't exist on ranks 1 or 8 (promotion ranks).
*/

int static_eval(Board& board)
{
    int score = 0;

    uint64 pawns_bb = board.chessboard[wP] | board.chessboard[bP];

    unsigned int count, index, file, rank; // Temporary variables.
    uint64 piece_bb; // Temporary variable.

    // White

    /************************* WHITE QUEENS *************************/

    piece_bb = board.chessboard[wQ];
    count = CNT_BITS(piece_bb);
    score += count * S_QUEEN; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        index = POP_BIT(piece_bb);
        file = GET_FILE(index);

        if((pawns_bb & file) == 0) // Open file
            score += S_QUEEN_OPENFILE;
        else if((board.chessboard[wP] & file) == 0) // Half-open file
            score += S_QUEEN_HALFOPENFILE;

        MoveList ml;
        gen_rook_moves(board.chessboard[wQ], WHITE, ml, board);
        gen_bishop_moves(board.chessboard[wQ], WHITE, ml, board);
        score += ml.list.size() * S_QUEEN_MOBILITY; // Mobility bonus
    }

    /************************* WHITE ROOKS *************************/

    piece_bb = board.chessboard[wR];
    count = CNT_BITS(piece_bb);
    score += count * S_ROOK; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        index = POP_BIT(piece_bb);
        file = GET_FILE(index);

        if((pawns_bb & file) == 0) // Open file
            score += S_ROOK_OPENFILE;
        else if((board.chessboard[wP] & file) == 0) // Half-open file
            score += S_ROOK_HALFOPENFILE;

        score += ROOK_ST[index]; // Piece-square table

        MoveList ml;
        gen_rook_moves(board.chessboard[wR], WHITE, ml, board);
        score += ml.list.size() * S_ROOK_MOBILITY; // Mobility bonus
    }

    /************************* WHITE KNIGHTS *************************/

    piece_bb = board.chessboard[wN];
    count = CNT_BITS(piece_bb);
    score += count * S_KNIGHT; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        score += KNIGHT_ST[POP_BIT(piece_bb)]; // Piece-square table

        MoveList ml;
        gen_knight_moves(board.chessboard[wN], WHITE, ml, board);
        score += ml.list.size() * S_KNIGHT_MOBILITY; // Mobility bonus
    }

    /************************* WHITE BISHOPS *************************/

    piece_bb = board.chessboard[wB];
    count = CNT_BITS(piece_bb);
    score += count * S_BISHOP; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        score += BISHOP_ST[POP_BIT(piece_bb)]; // Piece-square table

        MoveList ml;
        gen_bishop_moves(board.chessboard[wB], WHITE, ml, board);
        score += ml.list.size() * S_BISHOP_MOBILITY; // Mobility bonus
    }

    /************************* WHITE PAWNS *************************/

    piece_bb = board.chessboard[wP];
    count = CNT_BITS(piece_bb);
    score += count * S_PAWN; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        index = POP_BIT(piece_bb);
        file = GET_FILE(index);
        rank = GET_RANK(index);

        if(board.chessboard[wP] & PAWN_ISO_MASK[index]) // Isolated pawn
            score += S_PAWN_ISOLATED;

        if(board.chessboard[bP] & PAWN_WPAS_MASK[index]) // Passed pawn
            score += S_PAWN_PASSED[rank];

        score += PAWN_ST[index]; // Piece-square table

        MoveList ml;
        gen_pawn_moves(WHITE, ml, board);
        score += ml.list.size() * S_PAWN_MOBILITY; // Mobility bonus
    }

    // Black

    /************************* BLACK QUEENS *************************/

    piece_bb = board.chessboard[bQ];
    count = CNT_BITS(piece_bb);
    score -= count * S_QUEEN; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        index = POP_BIT(piece_bb);
        file = GET_FILE(index);

        if((pawns_bb & file) == 0) // Open file
            score -= S_QUEEN_OPENFILE;
        else if((board.chessboard[bP] & file) == 0) // Half-open file
            score -= S_QUEEN_HALFOPENFILE;

        MoveList ml;
        gen_rook_moves(board.chessboard[bQ], BLACK, ml, board);
        gen_bishop_moves(board.chessboard[bQ], BLACK, ml, board);
        score -= ml.list.size() * S_QUEEN_MOBILITY; // Mobility bonus
    }

    /************************* BLACK ROOKS *************************/

    piece_bb = board.chessboard[bR];
    count = CNT_BITS(piece_bb);
    score -= count * S_ROOK; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        index = POP_BIT(piece_bb);
        file = GET_FILE(index);

        if((pawns_bb & file) == 0) // Open file
            score -= S_ROOK_OPENFILE;
        else if((board.chessboard[bP] & file) == 0) // Half-open file
            score -= S_ROOK_HALFOPENFILE;

        score -= ROOK_ST[FLIPV[index]]; // Piece-square table

        MoveList ml;
        gen_rook_moves(board.chessboard[bR], BLACK, ml, board);
        score -= ml.list.size() * S_ROOK_MOBILITY; // Mobility bonus
    }

    /************************* BLACK KNIGHTS *************************/

    piece_bb = board.chessboard[bN];
    count = CNT_BITS(piece_bb);
    score -= count * S_KNIGHT; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        score -= KNIGHT_ST[FLIPV[POP_BIT(piece_bb)]]; // Piece-square table

        MoveList ml;
        gen_knight_moves(board.chessboard[bN], BLACK, ml, board);
        score -= ml.list.size() * S_KNIGHT_MOBILITY; // Mobility bonus
    }

    /************************* BLACK BISHOPS *************************/

    piece_bb = board.chessboard[bB];
    count = CNT_BITS(piece_bb);
    score -= count * S_BISHOP; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        score -= BISHOP_ST[FLIPV[POP_BIT(piece_bb)]]; // Piece-square table

        MoveList ml;
        gen_bishop_moves(board.chessboard[bB], BLACK, ml, board);
        score -= ml.list.size() * S_BISHOP_MOBILITY; // Mobility bonus
    }

    /************************* BLACK PAWNS *************************/

    piece_bb = board.chessboard[bP];
    count = CNT_BITS(piece_bb);
    score -= count * S_PAWN; // Material score

    for(unsigned int i = 0; i < count; i++)
    {
        index = POP_BIT(piece_bb);
        file = GET_FILE(index);
        rank = GET_RANK(index);

        if(board.chessboard[bP] & PAWN_ISO_MASK[index]) // Isolated pawn
            score -= S_PAWN_ISOLATED;

        if(board.chessboard[wP] & PAWN_BPAS_MASK[index]) // Passed pawn
            score -= S_PAWN_PASSED[9 - rank];

        score -= PAWN_ST[FLIPV[index]]; // Piece-square table

        MoveList ml;
        gen_pawn_moves(BLACK, ml, board);
        score -= ml.list.size() * S_PAWN_MOBILITY; // Mobility bonus
    }

    if(board.side == WHITE) return score;
    else return -score;
}