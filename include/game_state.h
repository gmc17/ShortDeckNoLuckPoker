#pragma once
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <stack>
#include <array>
#include <stdexcept>
#include <initializer_list>
#include <iostream>
#include <bitset>
#include <random>
#include <cstdint>
#include <unordered_map>

#include "constants.h"
#include "info_set.h"

extern int rank_table[NUM_CARDS][NUM_CARDS][NUM_CARDS][NUM_CARDS];
extern std::vector<std::vector<std::unordered_map<int, std::vector<std::tuple<int, int>>>>> strength_map_table;
extern std::vector<std::vector<std::vector<int>>> sorted_strengths_table;

class GameState {

public:
	/**
	 * To be filled in
	 * @param
	*/

	GameState();

	// Utility methods
	std::string to_string(bool verbose = false) const;
	bool operator==(const GameState& other) const;

	// Helpers
	inline bool has_card(uint8_t card) const {
		if ((card == fp1) ||
        (card == fp2) ||
        (card == fp3) ||
        (card == trn) ||
        (card == rvr) ||
        (card == op1) ||
        (card == op2) ||
        (card == ip1) ||
        (card == ip2))
        	return true;
    	return false;
	}
	int num_in_deck() const;

	// Game logic
	bool is_chance() const;
	bool is_fold() const;
	int best_hand(bool p) const;
	inline int best_hand_fast() const {
		return rank_table[op1 - 1][op2 - 1][trn - 1][rvr - 1];
	}
	float showdown(bool p) const;
	float utility(bool p) const;
	float utility_with_precomputed_hand_ranks(int p_hand_rank, int o_hand_rank) const;

	int index_to_action(int index) const;
	std::string action_to_string(int action) const;
	std::string histories_to_string() const;
	int action_to_index(int action) const;
	int num_actions() const;
	void apply_index(int index);
	void deal_card(uint8_t card);
	void undo(bool prev_player, float prev_pot);

	// CFR helpers
	InfoSet to_information_set() const;
	float rivr_hand_strength();
	int p_id(bool p) const;
	
	bool player;
	uint8_t op1;
	uint8_t op2;
	uint8_t ip1;
	uint8_t ip2;
	uint8_t fp1;
	uint8_t fp2;
	uint8_t fp3;
	uint8_t trn;
	uint8_t rvr;
	uint32_t pfp_history;
	uint32_t flp_history;
	uint32_t trn_history;
	uint32_t rvr_history;
	bool flp_seen;
	bool trn_seen;
	bool rvr_seen;
	bool is_terminal;
	float pot_size;
	std::stack<float> bets;
};

void play_computer();
int ith_action(uint32_t history, int i);
std::array<int, 2> pocket_id_to_row_col(int id);
inline int suit(uint8_t card) {
	return (card-1)/9;
}
inline int rank(uint8_t card) {
    return (card-1)%9;
}
void generate_terminal_node_evaluation_tables(GameState state);
void generate_rank_table(GameState state);
void test_rank_table(GameState state);