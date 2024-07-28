#pragma once
#include <iostream>
#include <array>
#include <map>
#include <algorithm>
#include <random>
#include <array>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <atomic>
#include <thread>

#include "game_state.h"
#include "info_set.h"

std::array<float, 3> get_strategy(const InfoSet& info_set);
std::array<float, 3> get_average_strategy(const InfoSet& info_set);
int sample_action(std::array<float, 3> strategy);
void print_nonzero_strategy(int n, std::string filename);

void sample_games(int iterations);
float calculate_exploitability(int iterations);

float as_mccfr(int iterations);
void as_mccfr_worker(int start, int end);
float as_traverse_tree(GameState gs, bool active_player, float q);

std::ostream& operator<<(std::ostream& os, const std::array<float, 3>& arr);

void load_cfr_data(const std::string& filename,
                   std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& strategy_sum);

void save_cfr_data(const std::string& filename,
                   const std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   const std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& strategy_sum);
