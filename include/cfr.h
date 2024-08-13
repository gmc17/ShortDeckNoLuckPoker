#pragma once
#include <iostream>
#include <array>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <immintrin.h>

#include "constants.h"
#include "game_state.h"
#include "info_set.h"
#include "best_response.h"
#include "thread_pool.h"

extern std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> regret_sum;
extern std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> strategy_sum;

void load_cfr_data(const std::string& filename,
                   std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& strategy_sum);

void save_cfr_data(const std::string& filename,
                   const std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   const std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& strategy_sum);

std::array<float, 7> get_strategy(const InfoSet& info_set);
std::array<float, 7> get_average_strategy(const InfoSet& info_set);

int sample_action(const std::array<float, 7>& strategy);

void as_mccfr(int iterations, std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                              std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                              std::array<uint8_t, 5> board_cards, float pot_size);

void as_mccfr_worker(int start, int end, std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                         std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                         std::array<uint8_t, 5> board_cards, float pot_size);

float as_traverse_tree(GameState& gs, bool active_player, float q);

void cfr_plus(int iterations, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                              const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                              std::array<uint8_t, 5> board_cards, float pot_size);

void cfr_plus_worker(int start, int end, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                         std::array<uint8_t, 5> board_cards, float pot_size, int thread_id);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> dcfr_traverse_tree_fast(GameState& state, bool traversing_player, int iteration,
                                                                                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
                                                                                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> cfr_plus_traverse_tree_fast(GameState& state, bool traversing_player, float weight,
                                                                                                 const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
                                                                                                 const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                                 const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> lcfr_traverse_tree_fast(GameState& state, bool traversing_player, float weight,
                                                                                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
                                                                                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_reach_probabilities_cfr_plus(GameState state, int action, 
                                                                                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_chance_reach_probabilities_cfr_plus(const GameState& state, int num, 
                                                                                                                const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> expected_utility_fast_cfr_plus(const GameState& state, bool traversing_player, 
                                                                                                    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> reach_probabilities,
                                                                                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range);



void cfr_plus_parallel(int iterations, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                              const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                              std::array<uint8_t, 5> board_cards, float pot_size);

void cfr_plus_worker_parallel(int start, int end, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                         std::array<uint8_t, 5> board_cards, float pot_size);

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> cfr_plus_traverse_tree_fast_parallel(
    GameState& state, bool traversing_player, float weight,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range,
    ThreadPool& pool);

inline int get_cpu_cores() {
    int cores = std::thread::hardware_concurrency();
    return (cores > 0) ? cores : 1;  // Return at least 1 if detection fails
}

void add_2d_arrays_simd(std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
                        const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b);

void multiply_2d_arrays_simd(std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
                             const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b);

void fast_initialize_array(std::array<float, NUM_CARDS>& array);