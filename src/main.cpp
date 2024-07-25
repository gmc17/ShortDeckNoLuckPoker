//g++ -O3 main.cpp -o name && ./name
#include <iostream>
#include "game_state.h"
#include <array>
#include "cfr.h"
#include <unordered_map>

int main() {
	uint32_t suita = 0b00000000000000000000000000000110;
    uint32_t suitb = 0b00000000000000000000000000000000;
    uint32_t suitc = 0b00000000000000001001000000000000;
    uint32_t suitd = 0b00000000000000000000000000000000;
    uint8_t  turn  = 						 0b00000000;
	uint8_t  rivr  = 						 0b00000000;
    uint8_t  flop_history = 			     0b00000000;
    uint8_t  turn_history = 				 0b00000000;
    uint8_t  rivr_history = 			     0b00000000;
    bool call_preflop = 1;
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
				 call_preflop,
				 player);

    // GameState random = generate_random_initial_state();

    // random.call_preflop = true;
    // random.apply_chance_action();
    // random.apply_chance_action();
    // random.apply_chance_action();

    // random.flop_history = 0b1011;
    // random.apply_chance_action();

    //std::cout << random.to_string();

    //mccfr(100000);

    print_nonzero_strategy(1000);

    // std::unordered_map<GameState, std::array<float, 3>, GameStateHash> strategy_map;

	// std::array<float, 3> strategy = {0.5f, 0.3f, 0.2f};

	// // Insert into the unordered_map
	// strategy_map[gs] = strategy;

	// // Retrieve from the unordered_map
	// auto it = strategy_map.find(gs);
	// if (it != strategy_map.end()) {
	//     std::array<float, 3> retrieved_strategy = it->second;
	//     std::cout << "hash: " << it->first.to_string() << "\n";
	// }
}