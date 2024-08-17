#pragma once
#include <vector>
#include <sstream>
#include <sstream>
#include <regex>
#include <limits>

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