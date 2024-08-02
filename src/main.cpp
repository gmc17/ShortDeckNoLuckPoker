#include <iostream>

#include "game_state.h"
#include "ars_table.h"
#include "cfr.h"
#include "constants.h"
#include "ars_table.h"
#include "info_set.h"
#include "lbr.h"

int main() {

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

    

    // for (int i=0; i<10; i++) {
    // 	GameState random = generate_random_initial_state();
    // 	// random.apply_action(1);
    // 	// random.apply_chance_action(32);
    // 	// random.apply_chance_action(31);
    // 	// random.apply_chance_action(30);

	//     std::cout << "Board:\n" << random.to_string() << "\nInformation set:\n";
	    
	//     InfoSet is = random.to_information_set();

	//     std::cout << is.to_string() << "\n";
    // }

    // std::cout << "1\n";

    // std::cout << ars_table(0, 3987, 71) << "\n";
    // generate_ARS_tables();

    // ars_table.load_from_file("ars_table.dat");

    // std::array<int, TURN_BUCKETS> buckets;

    // for (int i=0; i<TURN_BUCKETS; i++) buckets[i]=0;

	// for (int r=0; r<20; r++) {
	//     for (int p1=0; p1<36; p1++) {
	//     	for (int p2=p1+1; p2<36; p2++) {
	//     		std::array<uint16_t, 4> suits = {0, 0, 0, 0};

	//     		suits[p1/9] |= ((0b1)<<(p1%9));
	//     		suits[p2/9] |= ((0b1)<<(p2%9));

	//     		GameState gs;

	//     		gs.suita = suits[0];
	// 			gs.suitb = suits[1];
	// 			gs.suitc = suits[2];
	// 			gs.suitd = suits[3];

	// 		    gs.apply_chance_action(32);
	// 		    gs.apply_chance_action(31);
	// 		    gs.apply_chance_action(30);
	// 		    gs.apply_index(gs.action_to_index(1)); // check
	// 		    gs.apply_index(gs.action_to_index(1)); // check
	// 		    gs.apply_chance_action(29);
	// 		    gs.apply_index(gs.action_to_index(1)); // check
	// 		    gs.apply_index(gs.action_to_index(1)); // check
	// 		    gs.apply_chance_action(28);

	// 		    int p_id = pocket_id(p1,p2);
	// 		    int rank = gs.best_hand(0)/100;

	// 		    // if (ars_table(2, rank, p_id)<=0.05) {
	// 		    // 	std::cout << gs.to_string() << "\n\n";
	// 		    // }

	// 		    buckets[ars_to_bucket_rivr(ars_table(2, rank, p_id))]++;
	// 		}
	//     } 
	// }

	// for (int i=0; i<TURN_BUCKETS; i++) std::cout << "River in bucket " << i << ": " << buckets[i] << "\n";

	// as_mccfr(30000000);

	// sample_games(5);

	// GameState gs;
	// gs.print_range(6);
	// gs.apply_index(gs.action_to_index(6)); // call preflop
	// gs.print_range(7);
	// gs.apply_index(gs.action_to_index(7)); // check
    // gs.apply_chance_action(32);
    // gs.apply_chance_action(31);
    // gs.apply_chance_action(30);
    // gs.print_range(2); // range of bet pot on flop

	// for (int i=1; i<8; i++) {
	// 	GameState gs;
	//     gs.print_range(i);
	// }

    // play_computer();

	std::array<std::array<float, 36>, 36> opponent_range;
    for (int r=0; r<1296; r++) {
    	opponent_range[r/36][r%36] = 1.0f / (36.0f * 35.0f);
    }


	for (int i=0; i<5; i++) {
		GameState gs = generate_random_initial_state();
		gs.apply_index(gs.action_to_index(7)); // call
		gs.apply_index(gs.action_to_index(1)); // check
		gs.apply_chance_action(32);
		gs.apply_chance_action(31);
		gs.apply_chance_action(30);
		std::cout << gs.to_string() << "\n" << win_probability_rollout(gs, 1, opponent_range, 1000) << "\n\n";
	}

    return 0;
}
