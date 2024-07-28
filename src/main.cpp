#include <iostream>
#include <array>
#include <unordered_map>

#include "game_state.h"
#include "ars_table.h"
#include "cfr.h"
#include "constants.h"

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


    // const int NUM_SAMPLES = 5000;
    // float total = 0.0f;
    // for (int i=0; i<NUM_SAMPLES; i++) {
    // 	GameState random = generate_random_initial_state();
	//     random.apply_chance_action(32);
	//     random.apply_chance_action(31);
	//     random.apply_chance_action(30);
	//     random.apply_action(0);
	//     random.apply_action(0);
	//     random.apply_chance_action(29);
	//     random.apply_action(0);
	//     random.apply_action(0);
	//     random.apply_chance_action(28);
	//     total += random.rivr_hand_strength();


	//     //std::cout << random.to_string();
	//     //std::cout << random.rivr_hand_strength() << "\n\n";
    // }

    extern ARSTable ars_table;

    for (int i=0; i<10; i++) {
    	GameState random = generate_random_initial_state();
    	// random.apply_action(1);
    	// random.apply_chance_action(32);
    	// random.apply_chance_action(31);
    	// random.apply_chance_action(30);

	    std::cout << "Board:\n" << random.to_string() << "\nInformation set:\n";
	    
	    InfoSet is = random.to_information_set();

	    std::cout << is.to_string() << "\n";
    }

    std::cout << "1\n";

    std::cout << ars_table(0, 3987, 71) << "\n";

    
	// for (int r=0; r<50; r++) {
	//     for (int p1=0; p1<36; p1++) {
	//     	for (int p2=p1+1; p2<36; p2++) {
	//     		std::array<uint16_t, 4> suits = {0, 0, 0, 0};

	//     		suits[p1/9] |= ((0b1)<<(p1%9));
	//     		suits[p2/9] |= ((0b1)<<(p2%9));

	//     		GameState gs(suits[0],
	// 		    			 suits[1],
	// 						 suits[2],
	// 						 suits[3],
	// 						 0, 0, 0, 0, 0, 0, 0);

	// 		    gs.apply_chance_action(32);
	// 		    gs.apply_chance_action(31);
	// 		    gs.apply_chance_action(30);

	// 		    int p_id = pocket_id(p1,p2);
	// 		    int rank = gs.best_hand(0)/100;

	// 	    	std::cout << gs.to_string();
    // 			std::cout << "(" << rank << ", " << CARD_NAMES[p1%9] << SUIT_NAMES[p1/9] << CARD_NAMES[p2%9] << SUIT_NAMES[p2/9] << ", p_id: " << p_id << "): " << ars_table(0, rank, p_id) << "\n\n";
	// 		}
	//     } 
	// }

    //mccfr(100000);

    as_mccfr(100000);

    // print_nonzero_strategy(100, "as_latest_checkpoint.dat");

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



	/**
	 * AA
	 * 
	 * 
	 * 
	 */ 
