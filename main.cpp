//g++ -O3 main.cpp -o name && ./name
#include <iostream>
#include "game_state.h"

int main() {
	std::cout << "Working." << "\n";

	uint32_t suita = 0b00000000111000000000110011000000;
    uint32_t suitb = 0b00000000000000000000000000000000;
    uint32_t suitc = 0b00000000000000000000000000000000;
    uint32_t suitd = 0b00000000000000000000000000000000;
    uint8_t  turn  = 						 0b00000000;
	uint8_t  rivr  = 						 0b00000000;
    uint8_t  flop_history = 			     0b00110010;
    uint8_t  turn_history = 				 0b00110010;
    uint8_t  rivr_history = 			     0b11010010;
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

    std::hash<GameState> hasher;

    std::cout << hasher(gs) << "\n";

    gs.suita++;

    std::cout << hasher(gs) << "\n";

    std::cout << gs.pot_size() << "\n";

    std::cout << gs.best_hand(0) << "\n";
    std::cout << gs.best_hand(1);
}