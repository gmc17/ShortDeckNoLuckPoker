#pragma once
#include <vector>
#include <sstream>
#include <regex>
#include <limits>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include "helpers.h"
#include "thread_pool.h"
#include "game_state.h"
#include "tree.h"
#include "best_response.h"
#include "cfr_parameters.h"
#include "cfr.h"

void user_interface();

std::array<uint8_t, 5> get_board_cards();

float get_pot_size();

std::string bytes_to_readable_string(unsigned long long bytes);

bool prompt_tree_building(const GameState& initial_state);

CFRParameters get_cfr_parameters();

std::pair<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> get_ranges();
std::array<std::array<float, NUM_CARDS>, NUM_CARDS> convert_9x9_to_36x36(const std::array<std::array<float, NUM_RANKS>, NUM_RANKS>& range_9x9);
void read_range_file(const std::string& filename, std::array<std::array<float, NUM_RANKS>, NUM_RANKS>& range_9x9);

void explore_tree(const Tree& tree, const GameState& root_state);
void display_current_state(const std::vector<GameState>& states, const std::vector<int>& actions);
void print_centered(const std::string& text, int width);
void print_separator(int width, char sep = '-');
uint8_t parse_card_input(const std::string& input, const GameState& current_state);
void get_bet_raise_sizes();
float get_starting_stack_size();