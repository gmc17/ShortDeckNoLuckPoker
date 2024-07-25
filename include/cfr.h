#pragma once
#include "game_state.h"
#include <iostream>
#include <array>

int sample_action(std::array<float, 3> strategy);
std::array<float, 3> get_strategy(const GameState& info_set);
std::array<float, 3> get_average_strategy(const GameState& info_set);
float traverse_tree(GameState gs, bool active_player, float p0, float p1);
float mccfr(int iterations);
void print_nonzero_strategy(int n);

// Define the output operator for std::array<float, 3>
std::ostream& operator<<(std::ostream& os, const std::array<float, 3>& arr);
