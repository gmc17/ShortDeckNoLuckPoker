#pragma once
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>
#include <initializer_list>
#include <iostream>
#include <bitset>
#include <random>
#include <cstdint>
#include <sstream>
#include <unordered_map>

#include "info_set.h"

class GameState {

public:
	/**
	* @param 'suita' 32-bit integer representing cards in suit A:
	*                		- Bits 0-8: Player 0's cards
	*                		- Bits 9-17: Player 1's cards
	*                		- Bits 18-26: Community cards (flop)
	*                		- Bits 27-31: Unused
	* 			     		- Note: 'Bit 0' is the rightmost bit
	* @param 'suitb' same
	* @param 'suitc' same
	* @param 'suitd' same
	* @param 'turn' 8-bit integer representing the turn card:
	*              			- Bits 0-3: Card rank
	*               		- Bits 4-5: Suit (00: a, 01: b, 10: c, 11: d)
	*               		- Bits 6-7: Unused
	* @param 'rivr' 8-bit integer representing the river card (same structure as turn)
	* @param 'flop_history' 8-bit integer representing betting history on the flop:
	*                       - Bit 0: Flag for flop appearance (0: not yet, 1: appeared)
	* 					    - Bits 1-2: Player 0's initial action
	* 						         -->  01: check
	* 						         -->  10: bet
	* 								 -->  11: fold (preflop)
	*                       - Bits 3-4: Player 1's response
	* 								 -->  01: check/call
	* 	 						     -->  10: bet/raise
	* 								 -->  11: fold
	* 						- Bits 5-6: Player 0's response to bet/raise
	* 						         -->  01: call
	* 								 -->  10: raise (check raise)
	* 								 -->  11: fold
	*                       - Bit 7: Player 1's response to check raise
	* 								 -->   1: call
	* 								 -->   all 8 1's: fold
	* @param 'turn_history' 8-bit integer representing betting history on the turn:
	*                       - Bit 0: Flag for turn appearance (0: not yet, 1: appeared)
	* 						- Bits 1-2: Player 0's action
	* 								 -->  01: check
	* 								 -->  10: bet
	*                       - Bits 3-4: Player 1's response
	* 								 -->  01: check
	* 								 -->  10: call bet (if bet) or bet (if check)
	*  								 -->  11: fold
	*                       - Bits 5-7: Unused
	* @param 'rivr_history' 8-bit integer representing betting history on the river:
	* 						- Bit 0: Flag for river appearance (0: not yet, 1: appeared)
	*                       - Bits 1-2: Player 0's initial action
	* 						         -->  01: check
	* 						         -->  10: bet
	*                       - Bits 3-4: Player 1's response
	* 								 -->  01: check/call
	* 	 						     -->  10: bet/raise
	*  								 -->  11: fold
	* 						- Bits 5-6: Player 0's response to raise
	* 						         -->  01: call
	*  								 -->  11: fold
	* 						- Bit 7: Player 1's response to check raise
	* 								 -->   1: call
	* 								 -->   all 8 1's: fold
	* @param 'player' Boolean indicating the active player (false for small blind, true for big blind)
	*/

	GameState(uint32_t suita, 
			  uint32_t suitb, 
			  uint32_t suitc, 
              uint32_t suitd, 
              uint8_t  turn,
              uint8_t  rivr,
              uint8_t  flop_history, 
              uint8_t  turn_history, 
              uint8_t  rivr_history,
              bool call_preflop,
              bool player);

	// Utility methods
	std::string to_string() const;
	bool operator==(const GameState& other) const;

	// Game logic
	bool is_terminal() const;
	bool is_chance() const;
	int best_hand(bool p) const;
	float showdown() const;
	float pot_size() const;
	float utility(bool p) const;
	
	int num_actions() const;
	int num_chance_actions() const;
	void apply_action(int action);
	void apply_chance_action(int actions);

	// CFR helpers
	InfoSet to_information_set();
	float rivr_hand_strength();
	int p_id(bool p) const;
	
	uint32_t suita;
	uint32_t suitb;
	uint32_t suitc;
	uint32_t suitd;
	uint8_t  turn;
	uint8_t  rivr;
	uint8_t flop_history;
	uint8_t turn_history;
	uint8_t rivr_history;
	bool call_preflop;
	bool player;
};

size_t hash_gamestate(const GameState& gs);

int pocket_id(int p1, int p2);
GameState generate_random_initial_state();