/*
    Cortex - Self-learning Chess Engine
    @filename search.cpp
    @author Shreyas Vinod
    @version 0.1.2

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 02/12/2015 0.1.1 Added time handling.
    * 02/12/2015 0.1.2 Added null move pruning.
*/

#include <iostream> // std::cout
#include <algorithm> // std::sort()

#include "search.h"
#include "move.h" // IS_CAP() and COORD_MOVE()
#include "movegen.h"
#include "evaluate.h"
#include "hash_table.h"

// Function Prototypes

inline void check_up(SearchInfo& search_info);
inline bool is_repetition(const Board& board);
inline void clear_for_search(Board& board, SearchInfo& search_info);
int quiescence(int alpha, int beta, Board& board, SearchInfo& search_info);
int alpha_beta(int alpha, int beta, unsigned int depth, Board& board,
    SearchInfo& search_info, bool do_null);
void search(Board& board, SearchInfo& search_info);

// Functions

/**
    @brief Performs a check on whether the time for search has been
           exhausted.

    @param search_info is the search information structure.

    return void.
*/

inline void check_up(SearchInfo& search_info)
{
    if(search_info.time_set &&
        get_time_diff(search_info.start_time) >= search_info.max_time)
    {
        search_info.stopped = 1;
    }
}

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

    board.ply = 0; // Reset the ply to zero.

    search_info.nodes = 0;
    search_info.fh = 0;
    search_info.fhf = 0;
}

/**
    @brief Performs a quiescence search to try to find a quiet position, in
           order to get rid of the horizon effect.

    @param alpha refers to the value of alpha.
    @param beta refers to the value of beta.
    @param board refers to the board the search is being made on.
    @param search_info is the search information structure.

    @return int value denoting the value of the best move for this state.
*/

int quiescence(int alpha, int beta, Board& board, SearchInfo& search_info)
{
    if((search_info.nodes & 4095) == 0) check_up(search_info);

    search_info.nodes++;

    if((is_repetition(board) || board.fifty >= 100) && board.ply) return 0;

    if(board.ply >= MAX_DEPTH) // Maximum depth.
    {
        return static_eval(board);
    }

    int score = static_eval(board);

    if(score >= beta) return beta;

    if(score > alpha) alpha = score;

    score = -INFINITY_C;

    unsigned int legal = 0; // Number of legal moves found.

    unsigned int list_move, list_size;

    MoveList ml = gen_capture_moves(board);

    list_size = ml.list.size();

    std::sort(ml.list.begin(), ml.list.end(),
        [](const Move& lhs, const Move& rhs){ return lhs.score > rhs.score; });

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = ml.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        legal++;

        score = -quiescence(-beta, -alpha, board, search_info);

        undo_move(board);

        if(score > alpha) // Alpha cutoff
        {
            if(score >= beta) // Beta cutoff
            {
                if(legal == 1) search_info.fhf++;
                search_info.fh++;

                return beta;
            }

            alpha = score;
        }
    }

    return alpha;
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
    if(depth == 0) return quiescence(alpha, beta, board, search_info);

    if((search_info.nodes & 4095) == 0) check_up(search_info);

    search_info.nodes++;

    if((is_repetition(board) || board.fifty >= 100) && board.ply) return 0;

    if(board.ply >= MAX_DEPTH - 1) // Maximum depth.
    {
        return static_eval(board);
    }

    uint64 king_bb; // Used to check move legality.

    if(board.side == WHITE) king_bb = board.chessboard[wK];
    else king_bb = board.chessboard[bK];

    assert((king_bb != 0ULL) && ((king_bb & (king_bb - 1)) == 0ULL));

    bool in_check = is_sq_attacked(POP_BIT(king_bb), board.side, board);

    int score = -INFINITY_C;
    unsigned int pv_move = NO_MOVE;

    if(probe_table(board.t_table, board.ply, board.hash_key, depth, pv_move,
        score, alpha, beta))
    {
        return score;
    }

    // Null move pruning

    uint64 big_pieces =
        board.chessboard[wQ] | board.chessboard[wR] | board.chessboard[wB] |
        board.chessboard[bQ] | board.chessboard[bR] | board.chessboard[bB];

    if(do_null && !in_check && depth >= 4 && board.ply && big_pieces)
    {
        make_null_move(board);
        score = -alpha_beta(-beta, -beta + 1, depth - 4, board, search_info, 0);
        undo_null_move(board);

        if(search_info.stopped) return 0;

        if(score >= beta) return beta;
    }

    // Alpha-Beta!

    unsigned int best_move = NO_MOVE;
    int best_score = -INFINITY_C;
    score = -INFINITY_C;

    int old_alpha = alpha;
    unsigned int legal = 0; // Number of legal moves found.

    unsigned int list_move, list_size;

    MoveList ml = gen_moves(board);

    list_size = ml.list.size();

    if(pv_move != NO_MOVE)
    {
        for(unsigned int i = 0; i < list_size; i++)
        {
            list_move = ml.list.at(i).move;

            if(pv_move == ml.list.at(i).move)
            {
                ml.list.at(i).score = 200000;
                break;
            }
        }
    }

    std::sort(ml.list.begin(), ml.list.end(),
        [](const Move& lhs, const Move& rhs){ return lhs.score > rhs.score; });

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = ml.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        legal++;

        score = -alpha_beta(-beta, -alpha, depth - 1, board,
            search_info, 1);

        undo_move(board);

        if(search_info.stopped == 1) return 0;

        if(score > best_score)
        {
            best_score = score;
            best_move = list_move;

            if(score > alpha) // Alpha cutoff
            {
                if(score >= beta) // Beta cutoff
                {
                    if(legal == 1) search_info.fhf++;
                    search_info.fh++;

                    if(!IS_CAP(best_move))
                    {
                        board.search_killers[1][board.ply] =
                            board.search_killers[0][board.ply];

                        board.search_killers[0][board.ply] = best_move;
                    }

                    store_entry(board.t_table, board.ply, board.hash_key, best_move,
                        beta, depth, TFBETA);

                    return beta;
                }

                alpha = score;

                if(!IS_CAP(best_move))
                {
                    assert((GET_BB(DEP_CELL(best_move)) != 0ULL) &&
                        ((GET_BB(DEP_CELL(best_move)) &
                        (GET_BB(DEP_CELL(best_move)) - 1)) == 0ULL));

                    assert((GET_BB(DST_CELL(best_move)) != 0ULL) &&
                        ((GET_BB(DST_CELL(best_move)) &
                        (GET_BB(DST_CELL(best_move)) - 1)) == 0ULL));

                    board.search_history[determine_type(board,
                        GET_BB(DEP_CELL(best_move)))][determine_type(board,
                        GET_BB(DST_CELL(best_move)))] += depth;
                }
            }
        }
    }

    if(legal == 0)
    {
        if(in_check) return -INFINITY_C + board.ply; // Checkmate
        else return 0; // Stalemate
    }

    if(alpha != old_alpha)
    {
        store_entry(board.t_table, board.ply, board.hash_key, best_move,
            best_score, depth, TFEXACT);
    }
    else
    {
        store_entry(board.t_table, board.ply, board.hash_key, best_move,
            alpha, depth, TFALPHA);
    }

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

    unsigned int to_depth;

    if(search_info.infinite) to_depth = MAX_DEPTH - 1;
    else to_depth = search_info.depth;

    for(unsigned int current_depth = 1; current_depth <= to_depth;
        current_depth++) // Iterative deepening!
    {
        best_score = alpha_beta(-INFINITY_C, INFINITY_C, current_depth,
            board, search_info, 1); // Call Alpha-Beta and get the best score.

        if(search_info.stopped) break;

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