/*
    Cortex - Self-learning Chess Engine
    @filename evaluate.cc
    @author Anna Grygierzec
    @version 1.0.0

    @brief Static evaluation function that returns an objective score
           of the game state.

    ******************** VERSION CONTROL ********************
    * 28/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 06/12/2015 0.1.1 Added checks for isolated and passed pawns.
    * 06/12/2015 0.1.2 Added checks of open/half-open files.
    * 22/12/2015 0.1.3 Added backward pawns, king on and near open file,
                       pawn shield, rook and bishop bonus for lost pawns.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename evaluate.cc
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

const int S_QUEEN = 900;
const int S_ROOK = 500;
const int S_KNIGHT = 300;
const int S_BISHOP = 335;
const int S_PAWN = 100;

const int S_QUEEN_END = 900;
const int S_ROOK_END = 500;
const int S_KNIGHT_END = 250;
const int S_BISHOP_END = 300;
const int S_PAWN_END = 100;

// Global values

// int S_MOBILITY = 10;
// const int S_KING_IN_CHECK = -200;
const int S_ENDGAME = 1500;

// King

const int S_KING_OPENFILE = -20;

// Queens

const int S_QUEEN_OPENFILE = 5;
const int S_QUEEN_HALFOPENFILE = 3;

// Rooks

const int S_ROOK_OPENFILE = 10;
const int S_ROOK_HALFOPENFILE = 5;
const int S_ROOK_PAWNBONUS = 7;

// Bishops

const int S_BISHOP_PAIR = 25;
const int S_BISHOP_PAWNBONUS = 4;

// Pawns

const int S_PAWN_ISOLATED = -10;
const int S_PAWN_DOUBLED = -15;
const int S_PAWN_PASSED[9] = { 0, 0, 5, 10, 20, 35, 60, 100, 0 };
const int S_PAWN_SHIELD = 10;
const int S_PAWN_BACKWARD = -15;

uint64 PAWN_ISO_MASK[64]; // Isolated pawn mask.
uint64 PAWN_WPAS_MASK[64]; // White passed pawn mask.
uint64 PAWN_BPAS_MASK[64]; // Black passed pawn mask.
uint64 PAWN_NEXT_MASK[64]; // Fields near a pawn mask.
uint64 PAWN_WKS = 0xe000ULL;
uint64 PAWN_WQS = 0x700ULL;
uint64 PAWN_BKS = 0x7000000000000ULL;
uint64 PAWN_BQS = 0xe0000000000000ULL;
uint64 KING_WCK = 0xe0ULL;
uint64 KING_WCQ = 0x7ULL;
uint64 KING_BCK = 0x700000000000000ULL;
uint64 KING_BCQ = 0xe000000000000000ULL;

// Piece-square tables

const int KING_ST[64] = {
 5  ,   10  ,   8   ,  -5   ,   0   ,   5   ,   10  ,   5   ,
-15 ,  -15  ,  -15  ,  -15  ,  -15  ,  -15  ,  -15  ,  -15  ,
-30 ,  -30  ,  -30  ,  -30  ,  -30  ,  -30  ,  -30  ,  -30  ,
-70 ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,
-70 ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,
-70 ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,
-70 ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,
-70 ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70  ,  -70
};

const int KING_ST_END[64] = {
-50   ,  -20  ,  -10  ,  -10  ,  -10  ,  -10  ,  -20  ,  -50  ,
-20   ,   0   ,   0   ,   10  ,   10  ,   0   ,   0   ,  -20  ,
-10   ,   0   ,   10  ,   15  ,   15  ,   10  ,   0   ,  -10  ,
-10   ,   10  ,   15  ,   30  ,   30  ,   15  ,   10  ,  -10  ,
-10   ,   10  ,   15  ,   30  ,   30  ,   15  ,   10  ,  -10  ,
-10   ,   0   ,   10  ,   15  ,   15  ,   10  ,   0   ,  -10  ,
-20   ,   0   ,   0   ,   10  ,   10  ,   0   ,   0   ,  -20  ,
-50   ,  -20  ,  -10  ,  -10  ,  -10  ,  -10  ,  -20  ,  -50
};

const int QUEEN_ST[64] = {
0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
0   ,   0   ,   0   ,   3   ,   3   ,   0   ,   0   ,   0   ,
0   ,   2   ,   5   ,   8   ,   8   ,   5   ,   2   ,   0   ,
0   ,   5   ,   8   ,   10  ,   10  ,   8   ,   5   ,   0   ,
0   ,   5   ,   8   ,   10  ,   10  ,   8   ,   5   ,   0   ,
0   ,   2   ,   5   ,   8   ,   8   ,   5   ,   2   ,   0   ,
0   ,   0   ,   0   ,   3   ,   3   ,   0   ,   0   ,   0   ,
0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0
};

const int ROOK_ST[64] = {
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0   ,
0   ,   3   ,   5   ,   10  ,   10  ,   5   ,   3   ,   0   ,
0   ,   3   ,   5   ,   10  ,   10  ,   5   ,   3   ,   0   ,
0   ,   3   ,   5   ,   10  ,   10  ,   5   ,   3   ,   0   ,
0   ,   3   ,   5   ,   10  ,   10  ,   5   ,   3   ,   0   ,
15  ,   15  ,   15  ,   15  ,   15  ,   15  ,   15  ,   15  ,
0   ,   0   ,   5   ,   10  ,   10  ,   5   ,   0   ,   0
};

const int KNIGHT_ST[64] = {
0   ,  -10  ,   0   ,   0   ,   0   ,   0   ,  -10  ,   0   ,
0   ,   0   ,   0   ,   5   ,   5   ,   0   ,   0   ,   0   ,
5   ,   0   ,   10  ,   10  ,   10  ,   10  ,   0   ,   5   ,
0   ,   5   ,   10  ,   20  ,   20  ,   10  ,   5   ,   0   ,
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
10  ,   5   ,   0   ,  -10  ,  -10  ,   0   ,   5   ,   10  ,
5   ,   20  ,   0   ,   10  ,   10  ,   0   ,   20  ,   5   ,
0   ,   0   ,   5   ,   20  ,   20  ,   5   ,   0   ,   0   ,
5   ,   5   ,   5   ,   10  ,   10  ,   5   ,   5   ,   5   ,
10  ,   10  ,   10  ,   25  ,   25  ,   10  ,   10  ,   10  ,
15  ,   15  ,   15  ,   30  ,   30  ,   15  ,   15  ,   15  ,
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

        PAWN_NEXT_MASK[i] =
            (PAWN_WPAS_MASK[i] | PAWN_BPAS_MASK[i]) ^
            (B_FILE[GET_FILE(i)] | PAWN_ISO_MASK[i]) ^ GET_BB(i);
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
    int bishop_score, bishop_score_end, rook_score, rook_score_end;
    unsigned int count, index, file, rank; // Temporary variables.
    uint64 piece_bb; // Temporary variable.
    bool isolated;

    unsigned int white_mat = 0, black_mat = 0;
    unsigned int wq = 0, wr = 0, wn = 0, wb = 0, wp = 0,
        bq = 0, br = 0, bn = 0, bb = 0, bp = 0;

    // White

    /************************* WHITE QUEENS *************************/

    piece_bb = board.chessboard[wQ];
    wq = CNT_BITS(piece_bb);
    white_mat += wq * S_QUEEN;

    /************************* WHITE ROOKS *************************/

    piece_bb = board.chessboard[wR];
    wr = CNT_BITS(piece_bb);
    white_mat += wr * S_ROOK;

    /************************* WHITE KNIGHTS *************************/

    piece_bb = board.chessboard[wN];
    wn = CNT_BITS(piece_bb);
    white_mat += wn * S_KNIGHT;

    /************************* WHITE BISHOPS *************************/

    piece_bb = board.chessboard[wB];
    wb = CNT_BITS(piece_bb);
    white_mat += wb * S_BISHOP;

    /************************* WHITE PAWNS *************************/

    piece_bb = board.chessboard[wP];
    wp = CNT_BITS(piece_bb);
    white_mat += wp * S_PAWN;

    // Black

    /************************* BLACK QUEENS *************************/

    piece_bb = board.chessboard[bQ];
    bq = CNT_BITS(piece_bb);
    black_mat += bq * S_QUEEN;

    /************************* BLACK ROOKS *************************/

    piece_bb = board.chessboard[bR];
    br = CNT_BITS(piece_bb);
    black_mat += br * S_ROOK;

    /************************* BLACK KNIGHTS *************************/

    piece_bb = board.chessboard[bN];
    bn = CNT_BITS(piece_bb);
    black_mat += bn * S_KNIGHT;

    /************************* BLACK BISHOPS *************************/

    piece_bb = board.chessboard[bB];
    bb = CNT_BITS(piece_bb);
    black_mat += bb * S_BISHOP;

    /************************* BLACK PAWNS *************************/

    piece_bb = board.chessboard[bP];
    bp = CNT_BITS(piece_bb);
    black_mat += bp * S_PAWN;

    // Draw by insufficient material detection.
    // Note: Does not detect draw when two or more bishops on the same colour
    //       are involved. KNN vs. K is considered a draw, since a forced
    //       checkmate is not possible.

    count = 0;
    piece_bb = board.chessboard[wP];

    for(unsigned int i = 0; i < wp; i++) // Loop over white pawns.
    {
        index = POP_BIT(piece_bb);

        if((GET_BB(index + 8) & board.chessboard[bP]) &&
            (PAWN_NEXT_MASK[index + 8] & board.chessboard[bP]) == 0ULL)
        {
            count++;
        }
    }

    piece_bb = board.chessboard[bP];

    for(unsigned int i = 0; i < bp; i++) // Loop over black pawns.
    {
        index = POP_BIT(piece_bb);

        if((GET_BB(index - 8) & board.chessboard[wP]) &&
            (PAWN_NEXT_MASK[index - 8] & board.chessboard[wP]) == 0ULL)
        {
            count++;
        }
    }

    if(wp + bp == 0 || wp + bp == count) // No pawns remaining/movable.
    {
        if(wq + wr + wn + wb + bq + br + bn + bb == 0) return 0;
        else if((wq + wr + wn + bq + br + bn + bb == 0) && wb <= 1) return 0;
        else if((wq + wr + wn + wb + bq + br + bn == 0) && bb <= 1) return 0;
        else if((wq + wr + wb + bq + br + bn + bb == 0) && wn <= 2) return 0;
        else if((wq + wr + wn + wb + bq + br + bb == 0) && bn <= 2) return 0;
    }

    // Adjust scores as pawns as lost.

    bishop_score = S_BISHOP + (16 - wp - bp) * S_BISHOP_PAWNBONUS;
    bishop_score_end = S_BISHOP_END + (16 - wp - bp) * S_BISHOP_PAWNBONUS;
    rook_score = S_ROOK + (16 - wp - bp) * S_ROOK_PAWNBONUS;
    rook_score_end = S_ROOK_END + (16 - wp - bp) * S_ROOK_PAWNBONUS;

    /************************* WHITE EVALUATION *************************/

    if(white_mat > S_ENDGAME) // Regular evaluation.
    {
        /************************* KING *************************/

        piece_bb = board.chessboard[wK];

        index = POP_BIT(piece_bb);
        file = GET_FILE(index);

        if(file == 1) 
        {
            if((pawns_bb & B_FILE[1]) == 0ULL) score += S_KING_OPENFILE;
            if((pawns_bb & B_FILE[2]) == 0ULL) score += S_KING_OPENFILE;
        }
        else if(file == 8)
        {
            if((pawns_bb & B_FILE[7]) == 0ULL) score += S_KING_OPENFILE;
            if((pawns_bb & B_FILE[8]) == 0ULL) score += S_KING_OPENFILE;
        }
        else 
        {
            if((pawns_bb & B_FILE[file - 1]) == 0ULL) score += S_KING_OPENFILE;
            if((pawns_bb & B_FILE[file]) == 0ULL) score += S_KING_OPENFILE;
            if((pawns_bb & B_FILE[file + 1]) == 0ULL) score += S_KING_OPENFILE;
        }

        score += KING_ST[index];

        // if(is_sq_attacked(index, WHITE, board)) score += S_KING_IN_CHECK;

        /************************* QUEENS *************************/

        piece_bb = board.chessboard[wQ];
        count = wq;
        score += count * S_QUEEN; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score += S_QUEEN_OPENFILE;
            else if((board.chessboard[wP] & file) == 0) // Half-open file
                score += S_QUEEN_HALFOPENFILE;

            score += QUEEN_ST[index]; // Piece-square table
        }

        /************************* ROOKS *************************/

        piece_bb = board.chessboard[wR];
        count = wr;
        score += count * rook_score; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score += S_ROOK_OPENFILE;
            else if((board.chessboard[wP] & file) == 0) // Half-open file
                score += S_ROOK_HALFOPENFILE;

            score += ROOK_ST[index]; // Piece-square table
        }

        /************************* KNIGHTS *************************/

        piece_bb = board.chessboard[wN];
        count = wn;
        score += count * S_KNIGHT; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score += KNIGHT_ST[POP_BIT(piece_bb)]; // Piece-square table
        }

        /************************* BISHOPS *************************/

        piece_bb = board.chessboard[wB];
        count = wb;
        score += count * bishop_score; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score += BISHOP_ST[POP_BIT(piece_bb)]; // Piece-square table
        }

        if(count >= 2) score += S_BISHOP_PAIR;

        /************************* PAWNS *************************/

        piece_bb = board.chessboard[wP];
        count = wp;
        score += count * S_PAWN; // Material score


        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);
            rank = GET_RANK(index);

            // Isolated

            isolated = ((board.chessboard[wP] & PAWN_ISO_MASK[index]) == 0ULL);

            if(isolated) score += S_PAWN_ISOLATED;

            uint64 pawn_on_file = (board.chessboard[wP] & B_FILE[file]) ^
                GET_BB(index);

            // Doubled

            if(pawn_on_file) score += S_PAWN_DOUBLED;

            // Passed or backward

            if((board.chessboard[bP] & PAWN_WPAS_MASK[index]) == 0ULL)
                score += S_PAWN_PASSED[rank];
            else if(board.chessboard[bP] & PAWN_ISO_MASK[index] & PAWN_WPAS_MASK[index])
            {
                if(isolated)
                    score += S_PAWN_BACKWARD;
                else if((index > 15) && (index < 40) &&
                    ((board.chessboard[wP] & PAWN_ISO_MASK[index] &
                    PAWN_BPAS_MASK[index + 8]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index + 16] & board.chessboard[bP]))
                {
                    score += S_PAWN_BACKWARD;
                }
                else if((index < 16) &&
                    ((PAWN_NEXT_MASK[index] & board.chessboard[wP]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index + 16] & board.chessboard[bP]) &&
                    (PAWN_NEXT_MASK[index + 24] & board.chessboard[bP]))
                {
                    score += S_PAWN_BACKWARD;
                }
            }

            score += PAWN_ST[index]; // Piece-square table

             // Pawn shield

            if((board.chessboard[wK] & KING_WCK) && (board.chessboard[wP] & PAWN_WKS))
                score += S_PAWN_SHIELD;

            if((board.chessboard[wK] & KING_WCQ) && (board.chessboard[wP] & PAWN_WQS))
                score += S_PAWN_SHIELD;
            
        }
    }
    else // Endgame evaluation.
    {
        /************************* KING *************************/

        piece_bb = board.chessboard[wK];

        index = POP_BIT(piece_bb);

        score += KING_ST_END[index];

        // if(is_sq_attacked(index, WHITE, board)) score += S_KING_IN_CHECK;

        /************************* QUEENS *************************/

        piece_bb = board.chessboard[wQ];
        count = wq;
        score += count * S_QUEEN_END; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score += S_QUEEN_OPENFILE;
            else if((board.chessboard[wP] & file) == 0) // Half-open file
                score += S_QUEEN_HALFOPENFILE;

            score += QUEEN_ST[index]; // Piece-square table
        }

        /************************* ROOKS *************************/

        piece_bb = board.chessboard[wR];
        count = wr;
        score += count * rook_score_end; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score += S_ROOK_OPENFILE;
            else if((board.chessboard[wP] & file) == 0) // Half-open file
                score += S_ROOK_HALFOPENFILE;

            score += ROOK_ST[index]; // Piece-square table
        }

        /************************* KNIGHTS *************************/

        piece_bb = board.chessboard[wN];
        count = wn;
        score += count * S_KNIGHT_END; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score += KNIGHT_ST[POP_BIT(piece_bb)]; // Piece-square table
        }

        /************************* BISHOPS *************************/

        piece_bb = board.chessboard[wB];
        count = wb;
        score += count * bishop_score_end; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score += BISHOP_ST[POP_BIT(piece_bb)]; // Piece-square table
        }

        if(count >= 2) score += S_BISHOP_PAIR;

        /************************* PAWNS *************************/

        piece_bb = board.chessboard[wP];
        count = wp;
        score += count * S_PAWN_END; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);
            rank = GET_RANK(index);

            // Isolated

            isolated = ((board.chessboard[wP] & PAWN_ISO_MASK[index]) == 0ULL);

            if(isolated) score += S_PAWN_ISOLATED;

            uint64 pawn_on_file = (board.chessboard[wP] & B_FILE[file]) ^
                GET_BB(index);

            // Doubled

            if(pawn_on_file) score += S_PAWN_DOUBLED;

            // Passed or backward

            if((board.chessboard[bP] & PAWN_WPAS_MASK[index]) == 0ULL)
                score += S_PAWN_PASSED[rank];
            else if(board.chessboard[bP] & PAWN_ISO_MASK[index] & PAWN_WPAS_MASK[index])
            {
                if(isolated)
                    score += S_PAWN_BACKWARD;
                else if((index > 15) && (index < 40) &&
                    ((board.chessboard[wP] & PAWN_ISO_MASK[index] &
                    PAWN_BPAS_MASK[index + 8]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index + 16] & board.chessboard[bP]))
                {
                    score += S_PAWN_BACKWARD;
                }
                else if((index < 16) &&
                    ((PAWN_NEXT_MASK[index] & board.chessboard[wP]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index + 16] & board.chessboard[bP]) &&
                    (PAWN_NEXT_MASK[index + 24] & board.chessboard[bP]))
                {
                    score += S_PAWN_BACKWARD;
                }
            }

            score += PAWN_ST[index]; // Piece-square table
        }
    }

    /************************* BLACK EVALUATION *************************/

    if(black_mat > S_ENDGAME) // Regular evaluation.
    {
        /************************* KING *************************/

        piece_bb = board.chessboard[bK];

        index = POP_BIT(piece_bb);
        file = GET_FILE(index);

        if(file == 1) 
        {
            if((pawns_bb & B_FILE[1]) == 0ULL) score -= S_KING_OPENFILE;
            if((pawns_bb & B_FILE[2]) == 0ULL) score -= S_KING_OPENFILE;
        }
        else if(file == 8)
        {
            if((pawns_bb & B_FILE[7]) == 0ULL) score -= S_KING_OPENFILE;
            if((pawns_bb & B_FILE[8]) == 0ULL) score -= S_KING_OPENFILE;
        }
        else 
        {
            if((pawns_bb & B_FILE[file - 1]) == 0ULL) score -= S_KING_OPENFILE;
            if((pawns_bb & B_FILE[file]) == 0ULL) score -= S_KING_OPENFILE;
            if((pawns_bb & B_FILE[file + 1]) == 0ULL) score -= S_KING_OPENFILE;
        }

        score -= KING_ST[FLIPV[index]];

        // if(is_sq_attacked(index, BLACK, board)) score -= S_KING_IN_CHECK;

        /************************* QUEENS *************************/

        piece_bb = board.chessboard[bQ];
        count = bq;
        score -= count * S_QUEEN; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score -= S_QUEEN_OPENFILE;
            else if((board.chessboard[bP] & file) == 0) // Half-open file
                score -= S_QUEEN_HALFOPENFILE;

            score -= QUEEN_ST[FLIPV[index]]; // Piece-square table
        }

        /************************* ROOKS *************************/

        piece_bb = board.chessboard[bR];
        count = br;
        score -= count * rook_score; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score -= S_ROOK_OPENFILE;
            else if((board.chessboard[bP] & file) == 0) // Half-open file
                score -= S_ROOK_HALFOPENFILE;

            score -= ROOK_ST[FLIPV[index]]; // Piece-square table
        }

        /************************* KNIGHTS *************************/

        piece_bb = board.chessboard[bN];
        count = bn;
        score -= count * S_KNIGHT; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score -= KNIGHT_ST[FLIPV[POP_BIT(piece_bb)]]; // Piece-square table
        }

        /************************* BISHOPS *************************/

        piece_bb = board.chessboard[bB];
        count = bb;
        score -= count * bishop_score; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score -= BISHOP_ST[FLIPV[POP_BIT(piece_bb)]]; // Piece-square table
        }

        if(count >= 2) score -= S_BISHOP_PAIR;

        /************************* PAWNS *************************/

        piece_bb = board.chessboard[bP];
        count = bp;
        score -= count * S_PAWN; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);
            rank = GET_RANK(index);

            // Isolated

            isolated = ((board.chessboard[bP] & PAWN_ISO_MASK[index]) == 0ULL);

            if(isolated) score -= S_PAWN_ISOLATED;

            uint64 pawn_on_file = (board.chessboard[bP] & B_FILE[file]) ^
                GET_BB(index);

            // Doubled

            if(pawn_on_file) score -= S_PAWN_DOUBLED;

            // Passed or backward

            if((board.chessboard[wP] & PAWN_BPAS_MASK[index]) == 0ULL)
                score -= S_PAWN_PASSED[9 - rank];
            else if(board.chessboard[wP] & PAWN_ISO_MASK[index] & PAWN_BPAS_MASK[index])
            {
                if(isolated)
                    score -= S_PAWN_BACKWARD;
                else if((index > 15) && (index < 48) &&
                    ((board.chessboard[bP] & PAWN_ISO_MASK[index] &
                    PAWN_WPAS_MASK[index - 8]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index - 16] & board.chessboard[wP]))
                {
                    score -= S_PAWN_BACKWARD;
                }
                else if((index > 47) &&
                    ((PAWN_NEXT_MASK[index] & board.chessboard[bP]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index - 16] & board.chessboard[wP]) &&
                    (PAWN_NEXT_MASK[index - 24] & board.chessboard[wP]))
                {
                    score -= S_PAWN_BACKWARD;
                }
            }

            score -= PAWN_ST[FLIPV[index]]; // Piece-square table

            // Pawn shield

            if((board.chessboard[bK] & KING_BCK) && (board.chessboard[bP] & PAWN_BKS))
                score -= S_PAWN_SHIELD;

            if((board.chessboard[bK] & KING_BCQ) && (board.chessboard[bP] & PAWN_BQS))
                score -= S_PAWN_SHIELD;
        }
    }
    else // Endgame evaluation.
    {
        /************************* KING *************************/

        piece_bb = board.chessboard[bK];

        index = POP_BIT(piece_bb);

        score -= KING_ST_END[FLIPV[index]];

        // if(is_sq_attacked(index, BLACK, board)) score -= S_KING_IN_CHECK;

        /************************* QUEENS *************************/

        piece_bb = board.chessboard[bQ];
        count = bq;
        score -= count * S_QUEEN_END; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score -= S_QUEEN_OPENFILE;
            else if((board.chessboard[bP] & file) == 0) // Half-open file
                score -= S_QUEEN_HALFOPENFILE;

            score -= QUEEN_ST[FLIPV[index]]; // Piece-square table
        }

        /************************* ROOKS *************************/

        piece_bb = board.chessboard[bR];
        count = br;
        score -= count * rook_score_end; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);

            if((pawns_bb & file) == 0) // Open file
                score -= S_ROOK_OPENFILE;
            else if((board.chessboard[bP] & file) == 0) // Half-open file
                score -= S_ROOK_HALFOPENFILE;

            score -= ROOK_ST[FLIPV[index]]; // Piece-square table
        }

        /************************* KNIGHTS *************************/

        piece_bb = board.chessboard[bN];
        count = bn;
        score -= count * S_KNIGHT_END; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score -= KNIGHT_ST[FLIPV[POP_BIT(piece_bb)]]; // Piece-square table
        }

        /************************* BISHOPS *************************/

        piece_bb = board.chessboard[bB];
        count = bb;
        score -= count * bishop_score_end; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            score -= BISHOP_ST[FLIPV[POP_BIT(piece_bb)]]; // Piece-square table
        }

        if(count >= 2) score -= S_BISHOP_PAIR;

        /************************* PAWNS *************************/

        piece_bb = board.chessboard[bP];
        count = bp;
        score -= count * S_PAWN_END; // Material score

        for(unsigned int i = 0; i < count; i++)
        {
            index = POP_BIT(piece_bb);
            file = GET_FILE(index);
            rank = GET_RANK(index);

            // Isolated

            isolated = ((board.chessboard[bP] & PAWN_ISO_MASK[index]) == 0ULL);

            if(isolated) score -= S_PAWN_ISOLATED;

            uint64 pawn_on_file = (board.chessboard[bP] & B_FILE[file]) ^
                GET_BB(index);

            // Doubled

            if(pawn_on_file) score -= S_PAWN_DOUBLED;

            // Passed or backward

            if((board.chessboard[wP] & PAWN_BPAS_MASK[index]) == 0ULL)
                score -= S_PAWN_PASSED[9 - rank];
            else if(board.chessboard[wP] & PAWN_ISO_MASK[index] & PAWN_BPAS_MASK[index])
            {
                if(isolated)
                    score -= S_PAWN_BACKWARD;
                else if((index > 15) && (index < 48) &&
                    ((board.chessboard[bP] & PAWN_ISO_MASK[index] &
                    PAWN_WPAS_MASK[index - 8]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index - 16] & board.chessboard[wP]))
                {
                    score -= S_PAWN_BACKWARD;
                }
                else if((index > 47) &&
                    ((PAWN_NEXT_MASK[index] & board.chessboard[bP]) == 0ULL) &&
                    (PAWN_NEXT_MASK[index - 16] & board.chessboard[wP]) &&
                    (PAWN_NEXT_MASK[index - 24] & board.chessboard[wP]))
                {
                    score -= S_PAWN_BACKWARD;
                }
            }

            score -= PAWN_ST[FLIPV[index]]; // Piece-square table
        }
    }

    // Return relative score.

    if(board.side == WHITE) return score;
    else return -score;
}
