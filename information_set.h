#pragma once
#include <string>

class GameState {

public:
	/**
	* @param 'suita' 32-bit integer representing cards in suit A:
	*                - Bits 0-8: Player 0's cards
	*                - Bits 9-17: Player 1's cards
	*                - Bits 18-26: Community cards (flop)
	*                - Bits 27-31: Unused
	* 			     - Note: 'Bit 0' is the rightmost bit
	* @param 'suitb' same
	* @param 'suitc' same
	* @param 'suitd' same
	* @param 'turn' 8-bit integer representing the turn card:
	*               - Bits 0-3: Card rank
	*               - Bits 4-5: Suit (00: a, 01: b, 10: c, 11: d)
	*               - Bits 6-7: Unused
	* @param 'rivr' 8-bit integer representing the river card (same structure as turn)
	* @param 'flop_history' 8-bit integer representing betting history on the flop:
	*                       - Bit 0: Flag for flop appearance (0: not yet, 1: appeared)
	* 					    - Bits 1-2: Player 0's initial action
	* 						         -->  01: check
	* 						         -->  10: bet
	*                       - Bits 3-4: Player 1's response
	* 								 -->  01: check/call
	* 	 						     -->  10: bet/raise
	* 								 -->  11: fold
	* 						- Bits 4-5: Player 0's response to raise
	* 						         -->  01: call
	* 								 -->  11: fold
	*                       - Bits 6-7: Unused
	* @param 'turn_history' 8-bit integer representing betting history on the turn:
	*                       - Bit 0: Flag for turn appearance (0: not yet, 1: appeared)
	* 						- Bits 1-2: Player 0's action
	* 								 -->  01: check
	* 								 -->  10: bet
	*                       - Bits 3-4: Player 1's response
	* 								 -->  01: check
	* 								 -->  10: bet
	*  								 -->  11: fold
	*                       - Bits 2-7: Unused
	* @param 'rivr_history' 8-bit integer representing betting history on the river:
	* 						- Bit 0: Flag for river appearance (0: not yet, 1: appeared)
	*                       - Bits 1-2: Player 0's initial action
	* 						         -->  01: check
	* 						         -->  10: bet
	*                       - Bits 3-4: Player 1's response
	* 								 -->  01: check/call
	* 	 						     -->  10: bet/raise
	*  								 -->  11: fold
	* 						- Bits 4-5: Player 0's response to raise
	* 						         -->  01: call
	*  								 -->  11: fold
	*                       - Bits 6-7: Unused
	* @param 'player' Boolean indicating the active player (false for small blind, true for big blind)
	*/

	GameState(const uint32_t suita, 
				   const uint32_t suitb, 
				   const uint32_t suitc, 
                   const uint32_t suitd, 
                   const uint8_t  turn,
                   const uint8_t  rivr,
                   const uint8_t  flop_history, 
                   const uint8_t  turn_history, 
                   const uint8_t  rivr_history,
                   const bool player);

	// Utility methods
	std::string to_string() const;
	bool operator==(const GameState& other) const;

	// CFR methods
	int get_num_actions() const;

	uint32_t suita;
	uint32_t suitb;
	uint32_t suitc;
	uint32_t suitd;
	uint8_t  turn;
	uint8_t  rivr;
	uint16_t flop_history;
	uint16_t turn_history;
	uint16_t rivr_history;
	bool player;
};