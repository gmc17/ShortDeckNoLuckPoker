#pragma once
#include <iostream>
#include <array>

#include "game_state.h"
#include "ars_table.h"
#include "cfr.h"
#include "constants.h"
#include "info_set.h"

float calculate_exploitability(int hands);
float lbr_traverse_tree(GameState gs, bool active_player, std::array<std::array<float, 36>, 36> range);
float win_probability_rollout(GameState gs, bool exploitative_player, const std::array<std::array<float, 36>, 36>& opponent_range, int samples);
float calculate_fold_probability(GameState gs, const std::array<std::array<float, 36>, 36>& range);
int local_best_response(GameState gs, bool exploitative_player, std::array<std::array<float, 36>, 36> opponent_range);
std::array<std::array<float, 36>, 36> update_range(GameState gs, int opponent_action, bool exploitative_player, bool exclusive, std::array<std::array<float, 36>, 36> range);
void print_opponent_range(const std::array<std::array<float, 36>, 36> range);