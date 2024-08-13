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
#include "cfr.h"

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