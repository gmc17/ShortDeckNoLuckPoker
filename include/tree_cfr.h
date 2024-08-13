#pragma once
#include <iostream>
#include <array>
#include <map>
#include <algorithm>

#include "constants.h"
#include "game_state.h"
#include "info_set.h"
#include "best_response.h"
#include "thread_pool.h"
#include "cfr.h"
#include "tree.h"

Tree tree_cfr_plus_parallel(int iterations, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                              const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                              std::array<uint8_t, 5> board_cards, float pot_size);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> tree_cfr_plus_traverse_tree_fast_parallel(
    GameState& state, 
    Tree::Node* node,
    bool traversing_player, 
    float weight,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range,
    ThreadPool& pool);

float tree_calculate_exploitability_fast(const Tree& tree,
                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_reach_probabilities, 
                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_reach_probabilities,
                                         const std::array<uint8_t, 5> board_cards, 
                                         float pot);

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> tree_br_traverse_tree_fast(GameState& gs, 
                                                                               Tree::Node* node,
                                                                               bool exploitative_player, 
                                                                               const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_reach_probabilities);

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> tree_update_reach_probabilities(const GameState& state,
                                                                                    const Tree::Node* node, 
                                                                                    int action, 
                                                                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> tree_update_reach_probabilities_cfr_plus(const GameState& state,
                                                                                                              const Tree::Node* node, 
                                                                                                              int action, 
                                                                                                              const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);