/*
    Cortex - Self-learning Chess Engine
    @filename search.cc
    @author Shreyas Vinod
    @version 1.0.0

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.

    ******************** VERSION CONTROL ********************
    * 25/11/2015 File created.
    * 29/11/2015 0.1.0 Initial version.
    * 02/12/2015 0.1.1 Added time handling.
    * 02/12/2015 0.1.2 Added null move pruning.
    * 06/12/2015 0.1.3 Added ponder move output during search.
    * 06/12/2015 0.1.4 Added in-check extensions.
    * 21/12/2015 0.1.5 Added aspiration windows.
    * 10/04/2016 0.1.6 Removed aspiration windows (buggy).
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename search.cc
    @author Shreyas Vinod

    @brief The heart of the alpha-beta algorithm that makes computer
           chess possible.

    Includes structures and functions to effectively search and deduce the
    best possible move for a given position using the alpha-beta algorithm.
*/

#include "defs.h"

#include <iostream> // std::cout
#include <algorithm> // std::sort()

#include "search.h"
#include "board.h"
#include "move.h" // IS_CAP() and COORD_MOVE()
#include "movegen.h"
#include "evaluate.h"
#include "hash_table.h"
#include "chronos.h" // Time and get_time_diff()
#include "misc.h"

// Prototypes

inline void check_up(SearchInfo& search_info);
inline bool is_repetition(const Board& board);
inline void clear_for_search(Board& board, SearchInfo& search_info);
int quiescence(int alpha, int beta, Board& board, SearchInfo& search_info);
int alpha_beta(int alpha, int beta, unsigned int depth, Board& board,
    SearchInfo& search_info, bool do_null);
void search(Board& board, SearchInfo& search_info);

// Function definitions

/**
    @brief Performs a check on whether the time for search has been
           exhausted.

    @param search_info is the search information structure.

    return void.
*/

inline void check_up(SearchInfo& search_info)
{
    if(search_info.time_set &&
        get_time_diff(search_info.start_time) >= search_info.move_time)
    {
        search_info.stopped = 1;
    }

    read_input(search_info);
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

    for(int i = board.his_ply - board.fifty; i < bound; i++)
        if(board.history.at(i).hash_key == board.hash_key) return 1;

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
    if((search_info.nodes & 8191) == 0) check_up(search_info);

    search_info.nodes++;

    if((is_repetition(board) || board.fifty >= 100) && board.ply) return 0;

    if(board.ply >= MAX_DEPTH - 1) // Maximum depth.
    {
        return static_eval(board);
    }

    int score = static_eval(board);

    if(score >= beta) return beta;

    if(score > alpha) alpha = score;

    score = -INFINITY_C;

    unsigned int legal = 0; // Number of legal moves found.

    unsigned int list_move, list_size;

    MoveList ml = gen_captures(board);

    list_size = ml.list.size();

    // Sort the move list based on scores.

    std::sort(ml.list.begin(), ml.list.end(),
        [](const Move& lhs, const Move& rhs){ return lhs.score > rhs.score; });

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = ml.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        legal++;

        score = -quiescence(-beta, -alpha, board, search_info);

        undo_move(board);

        if(score > alpha) // Alpha cutoff.
        {
            if(score >= beta) // Beta cutoff.
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

    if((search_info.nodes & 8191) == 0) check_up(search_info);

    search_info.nodes++;

    // Check if the board is a repetition.

    if((is_repetition(board) || board.fifty >= 100) && board.ply) return 0;

    // Check if we've reached the maximum depth.

    if(board.ply >= MAX_DEPTH - 1) // Maximum depth.
    {
        return static_eval(board);
    }

    uint64 king_bb; // Used to check move legality.

    if(board.side == WHITE) king_bb = board.chessboard[wK];
    else king_bb = board.chessboard[bK];

    assert((king_bb != 0ULL) && ((king_bb & (king_bb - 1)) == 0ULL));

    bool in_check = is_sq_attacked(POP_BIT(king_bb), board.side, board);

    if(in_check) depth++; // In-check search extension.

    int score = -INFINITY_C;
    unsigned int pv_move = NO_MOVE;

    // Check if an entry exists in the transposition table.

    if(probe_table(board.t_table, board.ply, board.hash_key, depth, pv_move,
        score, alpha, beta))
    {
        return score;
    }

    // Null move pruning (zugzwang positions still possible)

    if(do_null && !in_check && depth >= 4 && board.ply &&
        (board.chessboard[wQ] | board.chessboard[wR] |
        board.chessboard[bQ] | board.chessboard[bR]))
    {
        make_null_move(board);
        score = -alpha_beta(-beta, -beta + 1, depth - 4, board,
            search_info, 0);
        undo_null_move(board);

        if(search_info.stopped) return 0;

        if(score >= beta && score < IS_MATE && score > -IS_MATE) return beta;
    }

    // Alpha-Beta!

    unsigned int best_move = NO_MOVE;
    score = -INFINITY_C;

    int old_alpha = alpha;
    unsigned int legal = 0; // Number of legal moves found.

    unsigned int list_move, list_size;

    MoveList ml = gen_moves(board);

    list_size = ml.list.size();

    // If a PV line was found, find the PV move and increase its score.

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

    // Sort the move list based on scores.

    std::sort(ml.list.begin(), ml.list.end(),
        [](const Move& lhs, const Move& rhs){ return lhs.score > rhs.score; });

    // Loop over every move.

    for(unsigned int i = 0; i < list_size; i++)
    {
        list_move = ml.list.at(i).move;

        if(!make_move(board, list_move)) continue;
        legal++;

        score = -alpha_beta(-beta, -alpha, depth - 1, board,
            search_info, 1);

        undo_move(board);

        if(search_info.stopped == 1) return 0;

        if(score > alpha) // Alpha cutoff.
        {
            if(score >= beta) // Beta cutoff.
            {
                if(legal == 1) search_info.fhf++;
                search_info.fh++;

                // Killer heuristic.

                if(!IS_CAP(list_move))
                {
                    board.search_killers[1][board.ply] =
                        board.search_killers[0][board.ply];

                    board.search_killers[0][board.ply] = list_move;
                }

                /********** BUGGY CODE **********/
                /*
                store_entry(board.t_table, board.ply, board.hash_key, list_move,
                    beta, depth, TFBETA);
                */
                /********************************/

                return beta;
            }

            alpha = score;
            best_move = list_move;

            // History heuristic.

            if(!IS_CAP(best_move))
            {
                assert((GET_BB(DEP_CELL(best_move)) != 0ULL) &&
                    ((GET_BB(DEP_CELL(best_move)) &
                    (GET_BB(DEP_CELL(best_move)) - 1)) == 0ULL));

                board.search_history[determine_type(board,
                    GET_BB(DEP_CELL(best_move)))]
                    [DST_CELL(best_move)] += depth;

                board.search_history[determine_type(board,
                    GET_BB(DEP_CELL(best_move)))]
                    [DST_CELL(best_move)] += depth;
            }
        }
    }

    // If there were no legal moves, it must be checkmate/stalemate.

    if(legal == 0)
    {
        if(in_check) return -INFINITY_C + board.ply; // Checkmate
        else return 0; // Stalemate
    }

    // Check if we improved alpha.

    assert(alpha >= old_alpha);

    if(alpha != old_alpha)
    {
        store_entry(board.t_table, board.ply, board.hash_key, best_move,
            alpha, depth, TFEXACT);
    }

    /********** BUGGY CODE **********/
    /*
    else
    {
        store_entry(board.t_table, board.ply, board.hash_key, best_move,
            alpha, depth, TFALPHA);
    }
    */
    /********************************/

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
    unsigned int best_move = NO_MOVE, ponder_move = NO_MOVE;
    int best_score = -INFINITY_C;

    unsigned int pv_moves; // Number of PV moves found.

    clear_for_search(board, search_info); // Get prepped for search.

    for(unsigned int current_depth = 1; current_depth <= search_info.depth;
        current_depth++) // Iterative deepening!
    {
        best_score = alpha_beta(-INFINITY_C, INFINITY_C, current_depth,
            board, search_info, 1); // Call Alpha-Beta and get the best score.

        if(search_info.stopped) break; // Break out if search was interrupted.

        // Get the PV line.

        pv_moves = probe_pv_line(board, current_depth); // Probe for PV line.
        best_move = board.pv_array[0];
        if(pv_moves > 1) ponder_move = board.pv_array[1];
        else ponder_move = NO_MOVE;

        // Output some key information to standard output (in UCI format).

        std::cout << "info score cp " << best_score << " depth " <<
            current_depth << " nodes " << search_info.nodes << " time " <<
            get_time_diff(search_info.start_time);

        std::cout << " pv";

        for(unsigned int i = 0; i < pv_moves; i++)
        {
            std::cout << " " << COORD_MOVE(board.pv_array[i]);
        }

        std::cout << std::endl;

#ifdef VERBOSE
        std::cout << "ordering " <<
            ((search_info.fhf / search_info.fh) * 100) << "%" << std::endl;
#endif // VERBOSE
    }

    if(ponder_move != NO_MOVE)
    {
        std::cout << "bestmove " << COORD_MOVE(best_move) << " ponder " <<
            COORD_MOVE(ponder_move) << std::endl;
    }
    else
    {
        std::cout << "bestmove " << COORD_MOVE(best_move) << std::endl;
    }
}
