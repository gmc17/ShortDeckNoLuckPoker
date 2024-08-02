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
float win_probability_rollout(GameState gs, bool active_player, std::array<float, 81> opponent_range);