#pragma once
#include <array>
#include <thread>
#include <immintrin.h>
#include <iomanip>
#include <random>
#include <regex>
#include <iostream>

#include "game_state.h"
#include "tree.h"

void add_2d_arrays_simd(
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b);

void multiply_2d_arrays_simd(
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b);

void fast_initialize_array(
    std::array<float, NUM_CARDS>& array);

void print_reach_probabilities(
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities);

void print_range(
    const Tree& tree,
    GameState initial_state,
    const std::vector<int>& actions);

GameState generate_random_initial_state();

GameState initial_state(
    float pot_size,
    const std::array<uint8_t, 5>& board_cards);

GameState get_state(
    GameState initial_state,
    const std::vector<int>& history);

inline int get_cpu_cores() {
    int cores = std::thread::hardware_concurrency();
    return (cores > 0) ? cores : 1;
}

inline std::mt19937& get_random_generator() {
    static thread_local std::mt19937 gen(std::random_device{}());
    return gen;
}
inline std::uniform_real_distribution<float>& get_uniform_distribution() {
    static thread_local std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    return dis;
}
inline std::uniform_int_distribution<> get_card_distribution() {
    static thread_local std::uniform_int_distribution<> dis(1, 36);
    return dis;
}
constexpr std::array<int, 36 * 36> generate_pocket_id_table() {
    std::array<int, 36 * 36> table{};
    for (int p1 = 0; p1 < 36; ++p1) {
        for (int p2 = 0; p2 < 36; ++p2) {
            bool suited = ((p1/9)==(p2/9));
            table[p1 * 36 + p2] = (suited) ? std::max((p1%9)*9 + p2%9, (p2%9)*9 + p1%9)
                                           : std::min((p1%9)*9 + p2%9, (p2%9)*9 + p1%9);
        }
    }
    return table;
}
inline constexpr auto POCKET_ID_TABLE = generate_pocket_id_table();
inline constexpr int pocket_id(int p1, int p2) noexcept {
    return POCKET_ID_TABLE[(p1-1) * 36 + (p2-1)];
}
constexpr auto create_ones_array() {
    std::array<std::array<float, 36>, 36> res = {};
    for (auto& row : res) {
        for (auto& elem : row) {
            elem = 1.0f;
        }
    }
    return res;
}