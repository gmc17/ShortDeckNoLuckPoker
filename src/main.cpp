#include <iostream>

#include "game_state.h"
#include "constants.h"
#include "info_set.h"
#include "best_response.h"
#include "cfr.h"
#include "tree.h"
#include "tree_cfr.h"

int main(int argc, char* argv[]) {
    // if (argc > 1) {
    //     std::string arg = argv[1];
    //     if (arg == "train") {
    //         int iterations = 1000000;  // Default value
    //         if (argc > 2) {
    //             iterations = std::stoi(argv[2]);
    //         }
    //         std::cout << "Training for " << iterations << " iterations..." << std::endl;
    //         as_mccfr(iterations);
    //         std::cout << "Training complete." << std::endl;
    //     } else if (arg == "play") {
    //         play_computer();
    //     } else if (arg == "generate-ars") {
    //         std::cout << "Generating ARS table. This may take a while..." << std::endl;
    //         generate_ars_tables();
    //         std::cout << "ARS table generation complete. File 'ars_table.dat' created." << std::endl;
    //     } else if (arg == "exploit") {
    //         int hands = 1000;  // Default value
    //         if (argc > 2) {
    //             hands = std::stoi(argv[2]);
    //         }
    //         std::cout << "Calculating approximate exploitability over " << hands << " hands..." << std::endl;
    //         calculate_exploitability(hands);
    //     } else {
    //         std::cout << "Invalid argument. Use 'train', 'play', 'generate-ars', or 'exploit'." << std::endl;
    //     }
    // } else {
    //     std::cout << "Usage: ./shortdeck [train <iterations>|play|generate-ars|exploit <hands>]" << std::endl;
    // }
    // return 0;

    // GameState gs;

    //SB cards:   7♠K♥
	//BB cards:   7♥9♥
	//Flop cards: T♥K♦7♣
	//Turn card:  8♥
	//River card: T♦

	// gs.suita |= (0b1<<1); // sb7s
	// gs.suitb |= (0b1<<7); // sbKh

	// gs.suitb |= (0b1<<10); // bb7h
	// gs.suitb |= (0b1<<12); // bb9h

	// gs.suitb |= (0b1<<22); // Th
	// gs.suitc |= (0b1<<25); // Kd
	// gs.suitd |= (0b1<<19); // 7c
	// gs.turn = 0b010010; // 8h
	// gs.rivr = 0b100100; // Td

	// gs.pflp_history = 0b111001;
	// gs.flop_history = 0b1110111;
	// gs.pot_size = 200;

	// gs.turn_seen = true;
	// gs.rivr_seen = true;
	// gs.flop_seen = true;

    // std::cout << gs.to_string() << "\n";

    // std::cout << "sb best hand: " << gs.best_hand(0);
    // std::cout << "\nbb best hand: " << gs.best_hand(1) << "\n";

    // std::cout << "sb utility: " << gs.utility(0);
    // std::cout << "\nbb utility: " << gs.utility(1) << "\n";

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

    // std::vector<float> flop = calculate_flop_bucket_boundaries();
    // std::vector<float> turn = calculate_turn_bucket_boundaries();
    // std::vector<float> rivr = calculate_rivr_bucket_boundaries();
	
	// std::cout << "Flop: {";
    // for (int i=0; i<FLOP_BUCKETS; i++) std::cout << flop[i] << "f, ";
    // std::cout << "}\nTurn: {";
    // for (int i=0; i<TURN_BUCKETS; i++) std::cout << turn[i] << "f, ";
    // std::cout << "}\nRivr: {";
	// for (int i=0; i<RIVR_BUCKETS; i++) std::cout << rivr[i] << "f, ";
	// std::cout << "}";



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

	

	// for (int i=0; i<5; i++) {
	// 	std::array<std::array<float, 36>, 36> range;
	//     for (int r=0; r<36; r++) {
	//     	for (int c=0; c<36; c++) {
	//     		range[r][c] = (r==c) ? 0.0f : 1.0f / (36.0f * 35.0f);
	//     	}	
	//     }

	//     std::array<int, 4> turn_cards = {0, 17, 7, 24};
	//     int pot_size = 0;

	//     GameState gs = random_state_from_ranges(range, range, turn_cards, pot_size);
	//     std::cout << gs.to_string() << "\n";
	// }

	std::array<uint8_t, 5> board_cards = {1, 16, 21, 25, 0};	
	float pot_size = 60.0f;
    int iterations = 50;

	GameState gs;
	gs.fp1 = board_cards[0];
	gs.fp2 = board_cards[1];
	gs.fp3 = board_cards[2];
	gs.trn = board_cards[3];
	gs.pfp_history = 0b111001;
	gs.flp_history = 0b1001;
	gs.pot_size = pot_size;
	gs.flp_seen = true;
	gs.trn_seen = true;

	Tree t = Tree(gs);

	std::array<std::array<float, NUM_CARDS>, NUM_CARDS> range;
	for (int r=0; r<NUM_CARDS; r++) range[r].fill(0.0f);

	for (int c1=1; c1<=NUM_CARDS; c1++) {
    	for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
			if (gs.has_card(c1) || gs.has_card(c2)) continue; 
    		range[c1 - 1][c2 - 1] = 1.0f / (32.0f * 31.0f / 2.0f);
    	}	
    }

    // generate_rank_table(gs);

    std::cout << "Number of action nodes: " << count_nodes(gs) << "\n";

    // print_opponent_reach_probabilities(range);

	//as_mccfr(iterations, range, range, board_cards, pot_size);
	// generate_rank_table();
	// test_rank_table(gs);
	//cfr_plus_parallel(iterations, range, range, board_cards, pot_size);
	Tree tree = tree_cfr_plus_parallel(iterations, range, range, board_cards, pot_size);
	tree_calculate_exploitability_fast(tree, range, range, board_cards, pot_size);


    std::cout << "threads: " << std::thread::hardware_concurrency() << "\n";

	// play_computer();

	// as_mccfr(1000);

	for (int k=0; k<7; k++) {
		GameState gs;

		gs.fp1 = board_cards[0];
	    gs.fp2 = board_cards[1];
	    gs.fp3 = board_cards[2];
	    gs.trn = board_cards[3];
	   	gs.pfp_history = 0b111001;
	    gs.flp_history = 0b1001;
	    gs.pot_size = pot_size;
	    gs.flp_seen = true;
	    gs.trn_seen = true;

	    gs.apply_index(gs.action_to_index(2)); // 0.5x pot

	    // gs.print_range_turn(k);
	}



	//calculate_exploitability(range, range, board_cards, pot_size);
	// calculate_exploitability_fast(range, range, board_cards, pot_size);


    return 0;
}
