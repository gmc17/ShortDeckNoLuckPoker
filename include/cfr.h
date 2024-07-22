#pragma once
#include "game_state.h"
#include <iostream>
#include <array>

std::array<float, 3> get_strategy(const GameState& info_set);

// Function to get the strategy for a given information set
std::array<float, 3> get_strategy(const GameState& info_set);

// Define the output operator for std::array<float, 3>
std::ostream& operator<<(std::ostream& os, const std::array<float, 3>& arr);
