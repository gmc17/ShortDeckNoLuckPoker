#pragma once
#include <iostream>
#include <array>
#include <map>
#include <algorithm>
#include <immintrin.h>
#include <set>

#include "constants.h"
#include "game_state.h"
#include "info_set.h"
#include "best_response.h"
#include "thread_pool.h"
#include "tree.h"
#include "helpers.h"

Tree cfr_plus(
    int iterations, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
    std::array<uint8_t, 5> board_cards, float pot_size);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> cfr_plus_traverse_tree(
    Tree::Node* node,
    bool traversing_player, 
    float weight,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range,
    ThreadPool& pool);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> terminal_node_utility(
    const Tree::Node* node, 
    bool traversing_player, 
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_reach_probabilities(
    const Tree::Node* node, 
    int action, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> node_update_chance_reach_probabilities(
    const Tree::Node* node, 
    int num, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);