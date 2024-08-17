#include <iostream>

#include "game_state.h"
#include "constants.h"
#include "info_set.h"
#include "best_response.h"
#include "tree.h"
#include "cfr.h"
#include "user_interface.h"

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


	user_interface();

	// std::array<uint8_t, 5> board_cards = {1, 16, 21, 0, 0};	
	// float pot_size = 80.0f;
    // int iterations = 50;

	// GameState state = initial_state(pot_size, board_cards);

	// std::array<std::array<float, NUM_CARDS>, NUM_CARDS> range;
	// for (int r=0; r<NUM_CARDS; r++) range[r].fill(0.0f);

	// for (int c1=1; c1<=NUM_CARDS; c1++) {
    // 	for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
	// 		if (!(state.has_card(c1) || state.has_card(c2))) { 
    // 			range[c1 - 1][c2 - 1] = 1.0f / (32.0f * 33.0f / 2.0f);
    // 		}
    // 	}	
    // }

    // std::cout << "Number of action nodes: " << count_nodes(state) << "\n";
	// Tree tree = cfr_plus(iterations, pot_size, board_cards, range, range);
    // std::cout << "threads: " << std::thread::hardware_concurrency() << "\n";

    // std::vector<int> actions = {1};

    // print_range(tree, state, actions);


    return 0;
}
