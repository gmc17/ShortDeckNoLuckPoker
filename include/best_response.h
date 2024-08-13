#pragma once
#include <iostream>
#include <utility>
#include <algorithm>
#include <array>
#include <limits>
#include <unordered_map>

#include "game_state.h"
#include "ars_table.h"
#include "cfr.h"
#include "constants.h"
#include "info_set.h"

float calculate_exploitability(std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                               std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range,
                               const std::array<uint8_t, 5> board_cards, 
                               float pot);

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> update_reach_probabilities(GameState gs, int action, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);
std::array<std::array<float, NUM_CARDS>, NUM_CARDS> update_chance_reach_probabilities(GameState gs, int num, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);

void print_opponent_reach_probabilities(const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);
float br_traverse_tree(GameState gs, bool exploitative_player, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);
float expected_utility(GameState gs, bool exploitative_player, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);



float calculate_exploitability_fast(const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_reach_probabilities, 
                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_reach_probabilities,
                                    const std::array<uint8_t, 5> board_cards, 
                                    float pot);

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> br_traverse_tree_fast(GameState& gs, bool exploitative_player, 
                                                                          const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_reach_probabilities);

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> expected_utility_fast(GameState gs, bool exploitative_player, 
                                                                          const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_reach_probabilities);

inline double clip(double value) {
    return std::max(0.0, std::min(value, 1.0));
}