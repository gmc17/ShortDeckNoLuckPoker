#pragma once
#include <iostream>
#include <array>

#include "game_state.h"
#include "ars_table.h"
#include "cfr.h"
#include "constants.h"
#include "info_set.h"

float calculate_exploitability(int iterations);
float lbr_traverse_tree(GameState gs, bool active_player, float q);
float win_probability_rollout(GameState gs, bool exploitative_player, const std::array<std::array<float, 36>, 36>& opponent_range, int samples);