#pragma once
#include <iostream>
#include <array>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <atomic>
#include <thread>

#include "game_state.h"
#include "info_set.h"
#include "constants.h"

extern std::mutex regret_sum_mutex;
extern std::mutex strategy_sum_mutex;
extern std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> regret_sum;
extern std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> strategy_sum;

std::ostream& operator<<(std::ostream& os, const std::array<float, 7>& arr);

void load_cfr_data(const std::string& filename,
                   std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& strategy_sum);

void save_cfr_data(const std::string& filename,
                   const std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   const std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& strategy_sum);

std::array<float, 7> get_strategy(const InfoSet& info_set);
std::array<float, 7> get_average_strategy(const InfoSet& info_set);
int sample_action(std::array<float, 7> strategy);
void print_nonzero_strategy(int n, std::string filename);

void as_mccfr(int iterations);
void as_mccfr_worker(int start, int end);
float as_traverse_tree(GameState gs, bool active_player, float q);
void sample_games(int iterations);




