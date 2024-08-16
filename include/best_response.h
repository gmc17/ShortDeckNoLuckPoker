#pragma once
#include <iostream>
#include <utility>
#include <algorithm>
#include <array>
#include <limits>
#include <unordered_map>

#include "game_state.h"
#include "ars_table.h"
#include "constants.h"
#include "info_set.h"
#include "helpers.h"
#include "tree.h"
#include "cfr.h"

float calculate_exploitability(
    const Tree& tree,
    float pot_size,
    const std::array<uint8_t, 5>& board_cards,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& op_reach_probabilities, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& ip_reach_probabilities);

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> best_response_traverse_tree(
    Tree::Node* node,
    bool exploitative_player, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& exploitative_player_range);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> best_response_terminal_node_utility(
    const Tree::Node* node, 
    bool traversing_player, 
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& exploitative_player_range);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_reach_probabilities_using_average_strategy(
    const Tree::Node* node, 
    int action, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);