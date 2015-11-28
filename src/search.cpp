/*
    Cortex - Self-learning Chess Engine
    @filename search.cpp
    @author Shreyas Vinod
    @version 0.1.0

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 28/11/2015 0.1.0 Initial version.
*/

#include <iostream> // std::cout
#include <algorithm> // std::sort()

#include "search.h"
#include "move.h" // COORD_MOVE()
#include "movegen.h"
#include "evaluate.h"
#include "hash_table.h"

// Globals

const int INFINITY_C = 50000;

// Function Prototypes

inline bool is_repetition(const Board& board);
inline void clear_for_search(Board& board, SearchInfo& search_info);
int alpha_beta(int alpha, int beta, unsigned int depth, Board& board,
    SearchInfo& search_info, bool do_null);
void search(Board& board, SearchInfo& search_info);

// Functions

/**
    @brief Checks if the given position is a repetition.

    @param board is the board to check.

    @return bool denoting whether the current position is a repetition.

    @warning This functions only checks if the position is a twofold
             repetition.
*/

inline bool is_repetition(const Board& board)
{
    int bound = board.his_ply - 1;

    for(int i = board.his_ply - board.ply; i < bound; i++)
    {
        if(board.history.at(i).hash_key == board.hash_key) return 1;
    }

    return 0;
}

/**
    @brief Clears various parameters in the board and search information
           structure for search.

    @param board is the board the search is going to be made on.
    @param search_info is the search information structure to clear.

    @return void.
*/

inline void clear_for_search(Board& board, SearchInfo& search_info)
{
    // Clear the history heuristic array.

    for(unsigned int i = 0; i < 12; i++)
    {
        for(unsigned int j = 0; j < 64; j++)
            board.search_history[i][j] = 0;
    }

    // Clear the killer heuristic array.

    for(unsigned int i = 0; i < 2; i++)
    {
        for(unsigned int j = 0; j < MAX_DEPTH; j++)
            board.search_killers[i][j] = 0;
    }

    clear_pv_table(board.pv_table); // Reset the PV table.

    board.ply = 0; // Reset the ply to zero.

    search_info.start_time = 0;
    search_info.stop_time = 0;
    search_info.nodes = 0;
    search_info.fh = 0;
    search_info.fhf = 0;
}

/**
    @brief The heart of the engine; an implementation of the Alpha-Beta
           algorithm.

    @param alpha refers to the current value of alpha.
    @param beta refers to the current value of beta.
    @param depth is the depth to search to.
    @param board is the board to search on.
    @param search_info is the search information structure.
    @param do_null denotes whether to use the null-move heuristic.

    @return int value denoting the score of the best move for this state.
*/

int alpha_beta(int alpha, int beta, unsigned int depth, Board& board,
    SearchInfo& search_info, bool do_null)
{
    if(depth == 0)
    {
        // *** Replace with quiescence search! ***
        search_info.nodes++;
        return static_eval(board);
    }

    search_info.nodes++;

    // *** Check for possible interrupts! ***

    if((is_repetition(board) || board.fifty >= 100) && board.ply) return 0;

    if(board.ply >= MAX_DEPTH) // Maximum depth.
    {
        return static_eval(board);
    }

    unsigned int best_move = NO_MOVE;
    int score = -INFINITY_C;

    int old_alpha = alpha;
    unsigned int legal = 0; // Number of legal moves found.

    unsigned int list_move, list_size;

    MoveList ml = gen_moves(board);

    std::sort(ml.list.begin(), ml.list.end(),
        [](const Move& lhs, const Move& rhs){ return lhs.score > rhs.score; });

    list_size = ml.list.size();

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = ml.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        legal++;

        score = -alpha_beta(-beta, -alpha, depth - 1, board,
            search_info, 1);

        undo_move(board);

        if(score > alpha)
        {
            if(score >= beta)
            {
                if(legal == 1) search_info.fhf++;
                search_info.fh++;
                return beta;
            }
            alpha = score;
            best_move = list_move;
        }
    }

    uint64 u64_1;

    if(board.side == WHITE) u64_1 = board.chessboard[wK];
    else u64_1 = board.chessboard[bK];

    assert((u64_1 != 0ULL) && ((u64_1 & (u64_1 - 1)) == 0ULL));

    if(legal == 0)
    {
        if(is_sq_attacked(POP_BIT(u64_1), board.side, board)) // Checkmate
        {
            return -49000 + board.ply;
        }
        else return 0; // Stalemate
    }

    if(alpha != old_alpha)
        store_pv_move(board.pv_table, board.hash_key, best_move);

    return alpha;
}

/**
    @brief Implements a layer of iterative deepening on top of Alpha-Beta.

    @param board is the board to perform the search on.
    @param search_info is the search information structure.

    @return void.
*/

void search(Board& board, SearchInfo& search_info)
{
    unsigned int best_move = NO_MOVE;
    int best_score = -INFINITY_C;

    unsigned int pv_moves; // Number of PV moves found.

    clear_for_search(board, search_info); // Get prepped for search.

    for(unsigned int current_depth = 1; current_depth <= search_info.depth;
        current_depth++) // Iterative deepening!
    {
        best_score = alpha_beta(-INFINITY_C, INFINITY_C, current_depth,
            board, search_info, 1); // Call Alpha-Beta and get the best score.

        // *** Check if out of time! ***

        pv_moves = probe_pv_line(board, current_depth); // Probe for PV line.
        best_move = board.pv_array[0];

        // Output some key information to standard output.

        std::cout << "Depth: " << current_depth << "; score: " << best_score <<
            "; move: " << COORD_MOVE(best_move) << "; nodes: " <<
            search_info.nodes;

        std::cout << "; PV:";

        for(unsigned int i = 0; i < pv_moves; i++)
        {
            std::cout << " " << COORD_MOVE(board.pv_array[i]);
        }

        std::cout << "; ordering: " <<
            ((search_info.fhf / search_info.fh) * 100) << "%" << std::endl;
    }
}