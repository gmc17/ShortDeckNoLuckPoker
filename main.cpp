//g++ -O3 main.cpp -o name && ./name
#include <iostream>
#include "game_state.h"

int main() {
	std::cout << "Working." << "\n";

	uint32_t suita = 0b00000000101000100000000000000110;
    uint32_t suitb = 0b00000000000000000000000000000000;
    uint32_t suitc = 0b00000000010000001000000000000000;
    uint32_t suitd = 0b00000000000000000010000000000000;
    uint8_t  turn  = 						 0b00000000;
	uint8_t  rivr  = 						 0b00000000;
    uint8_t  flop_history = 			     0b00110010;
    uint8_t  turn_history = 				 0b00110010;
    uint8_t  rivr_history = 			     0b00001010;
    bool is_information_set = 0;
    bool player = 0;

    GameState gs(suita,
    			 suitb,
				 suitc,
				 suitd,	  
				 turn,
				 rivr,
				 flop_history,
				 turn_history,
				 rivr_history,
				 is_information_set,
				 player);

    std::cout << gs.to_string();

    std::cout << "SB best hand value: " << gs.best_hand(0) << "\n";
    std::cout << "BB best hand value: " << gs.best_hand(1) << "\n";

    std::cout << "Utility to SB: " << gs.utility(0) << "\n";
    std::cout << "Utility to BB: " << gs.utility(1) << "\n";
}