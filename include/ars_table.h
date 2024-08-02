#pragma once
#include <vector>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <array>
#include <thread>
#include <mutex>
#include <random>

#include "game_state.h"
#include "constants.h"

class ARSTable {
private:
    std::vector<float> data;

public:
    ARSTable();
    float& operator()(int round, int rank, int pocket_pair_id);
    const float& operator()(int round, int rank, int pocket_pair_id) const;
    void save_to_file(const std::string& filename) const;
    void load_from_file(const std::string& filename);
};

void ars_worker_exhaustive_flop(int start, int thread_id, ARSTable& ars_table, ARSTable& cum_sum_table, ARSTable& count_table);
void ars_worker_exhaustive_turn(int start, int thread_id, ARSTable& ars_table, ARSTable& cum_sum_table, ARSTable& count_table);
void ars_worker_exhaustive_river(int start, int thread_id, ARSTable& ars_table, ARSTable& cum_sum_table, ARSTable& count_table);
void generate_ars_tables();

std::vector<float> calculate_flop_bucket_boundaries();
std::vector<float> calculate_turn_bucket_boundaries();
std::vector<float> calculate_rivr_bucket_boundaries();
uint8_t ars_to_bucket_flop(float ars);
uint8_t ars_to_bucket_turn(float ars);
uint8_t ars_to_bucket_rivr(float ars);

extern ARSTable ars_table;