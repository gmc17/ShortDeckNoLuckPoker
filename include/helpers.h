#pragma once
#include <array>
#include <thread>
#include <immintrin.h>
#include <iomanip>

#include "constants.h"
#include "game_state.h"
#include "tree.h"

inline int get_cpu_cores() {
    int cores = std::thread::hardware_concurrency();
    return (cores > 0) ? cores : 1;
}

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
    const std::vector<int>& actions);