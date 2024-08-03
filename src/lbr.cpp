#include "lbr.h"

float calculate_exploitability(int hands) {
    float total_winnings = 0;
    for (int i=1; i<=hands; i++) {
        for (int exploitative_player=0; exploitative_player<2; exploitative_player++) {
            GameState gs = generate_random_initial_state();
            GameState temp = gs;

            uint32_t mask = (exploitative_player == 0) ? 0b111111111000000000111111111
                                               		   : 0b111111111111111111000000000;
		    gs.suita &= mask;
		    gs.suitb &= mask;
		    gs.suitc &= mask;
		    gs.suitd &= mask;

		    std::array<uint16_t, 4> suit_cards_all = {
		        (uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
		        (uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
		        (uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
		        (uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
    		};

            // initialize uniform range
            std::array<std::array<float, 36>, 36> uniform_range;
		    for (int r=0; r<36; r++) {
		    	for (int c=0; c<36; c++) {
		    		if ((r==c) ||
		    			(suit_cards_all[r/9] & SINGLE_MASKS[8-(r%9)]) || 
                		(suit_cards_all[c/9] & SINGLE_MASKS[8-(c%9)])) {
		    			// impossible hole card combos
		    			uniform_range[r][c] = 0.0f;
		    		}
		    		uniform_range[r][c] = 1.0f / (34.0f * 33.0f);
		    	}
		    }

		    gs = temp;

            total_winnings += lbr_traverse_tree(gs, exploitative_player, uniform_range);
        }
        if (i%100==0) std::cout << "(" << i << ", " << total_winnings / (i*2) << "), " << std::flush;
    }
    return total_winnings / (hands*2);
}

float lbr_traverse_tree(GameState gs, bool exploitative_player, std::array<std::array<float, 36>, 36> range) {
    if (gs.is_terminal()) {
    	// if (gs.utility(exploitative_player) > 30 || gs.utility(exploitative_player) < -30) std::cout << "utility to exploitative_player (player " << exploitative_player << "): " << gs.utility(exploitative_player) << "\n";
        return gs.utility(exploitative_player);
    }

    if (gs.is_chance()) {
        GameState temp = gs;
        int num_actions = temp.num_chance_actions();
        temp.apply_chance_action(num_actions);
        return lbr_traverse_tree(temp, exploitative_player, range);
    }

    InfoSet info_set = gs.to_information_set();
    
    if (info_set.player != exploitative_player) {
        // Opponent's turn: use their strategy
        std::array<float, 7> strategy = get_average_strategy(info_set);
        int sampled_action = sample_action(strategy);
        range = update_range(gs, sampled_action, exploitative_player, 0, range);
        GameState child = gs;
        child.apply_index(sampled_action);
        return lbr_traverse_tree(child, exploitative_player, range);
    }

    // Active player's turn: compute best response
    int lbr = local_best_response(gs, exploitative_player, range);
    // std::cout << gs.to_string() << "lbr: " << lbr << "\n\n";
    GameState child = gs;
    child.apply_index(lbr);
	return lbr_traverse_tree(child, exploitative_player, range);
}

void print_opponent_range(const std::array<std::array<float, 36>, 36> range) {

	std::array<std::array<float, 9>, 9> p_id_range;

	for (int i=0; i<81; i++) {
		p_id_range[i/9][i%9]=0.0f;
	}

	std::cout << "Range:\n";
	for (int p1=0; p1<36; p1++) {
		for (int p2=0; p2<36; p2++) {
			if (p1==p2) continue;
			int p = pocket_id(p1, p2);
			int r = pocket_id_to_row_col(p)[0];
			int c = pocket_id_to_row_col(p)[1];

			p_id_range[r][c] += range[p1][p2];
		}
	}

	for (int r=0; r<10; r++) {
        for (int c=0; c<10; c++) {
            if (c==0 && r==0) {
                std::cout << "    ";
            } else if (r==0 && c>0) {
                std::cout << CARD_NAMES[9-c] << "s   ";
            } else if (c==0 && r>0) {
                std::cout << CARD_NAMES[9-r] << " ";
            } else {
            	// float f = 100.0f * range[r-1][c-1];
                // std::cout << FIXED_FLOAT(f) << " ";
                std::cout << FIXED_FLOAT(100*p_id_range[r-1][c-1]) << " ";
            }
        }
        std::cout << "\n";
    }
}

std::array<std::array<float, 36>, 36> update_range(GameState gs, int action, bool exploitative_player, bool exclusive, std::array<std::array<float, 36>, 36> range) {
    float total_weight = 0.0f;
    std::array<std::array<float, 36>, 36> updated_range = range;

    uint32_t mask = (exploitative_player == 0) ? 0b111111111000000000111111111
                                     		   : 0b111111111111111111000000000;

    gs.suita &= mask;
    gs.suitb &= mask;
    gs.suitc &= mask;
    gs.suitd &= mask;

    std::array<uint16_t, 4> suit_cards_all = {
        (uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
        (uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
        (uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
        (uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
    };

    GameState temp = gs;

    // Add turn and river cards to suit_cards_all
    if (gs.turn_seen) suit_cards_all[(gs.turn >> 4) & 0b11] |= (0b1 << (gs.turn & 0b1111));
    if (gs.rivr_seen) suit_cards_all[(gs.rivr >> 4) & 0b11] |= (0b1 << (gs.rivr & 0b1111));
    
    for (int c1 = 0; c1 < 36; c1++) {
        for (int c2 = 0; c2 < 36; c2++) {
            if (c1 == c2) continue;
            if ((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)]) || 
                (suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])) {
            	updated_range[c1][c2] = 0.0f; // this combination of hole cards is not possible
            	continue;
            }
            
            std::array<uint32_t, 4> suit_cards = {gs.suita, gs.suitb, gs.suitc, gs.suitd};
            suit_cards[c1/9] |= (gs.player == 0) ? (0b1 << (c1%9)) : (0b1 << (9+(c1%9)));
            suit_cards[c2/9] |= (gs.player == 0) ? (0b1 << (c2%9)) : (0b1 << (9+(c2%9)));

            gs.suita = suit_cards[0];
            gs.suitb = suit_cards[1];
            gs.suitc = suit_cards[2];
            gs.suitd = suit_cards[3];

            InfoSet is = gs.to_information_set();

            float action_probability = get_average_strategy(is)[action];
            
            updated_range[c1][c2] *= (exclusive) ? (1-action_probability) 
            									 : (action_probability);

            total_weight += updated_range[c1][c2];

            gs = temp;
        }
    }
    
    // Normalize
    if (total_weight > 0) {
        for (int c1 = 0; c1 < 36; c1++) {
            for (int c2 = 0; c2 < 36; c2++) {
                updated_range[c1][c2] /= total_weight;
            }
        }
    }
    return updated_range;
}

float calculate_fold_probability(GameState gs, const std::array<std::array<float, 36>, 36>& range) {

    float fold_probability = 0.0f;
    float total_weight = 0.0f;

    uint32_t mask = (gs.player == 0) ? 0b111111111111111111000000000
                                     : 0b111111111000000000111111111;
    gs.suita &= mask;
    gs.suitb &= mask;
    gs.suitc &= mask;
    gs.suitd &= mask;

    std::array<uint16_t, 4> suit_cards_all = {
        (uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
        (uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
        (uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
        (uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
    };

    GameState temp = gs;

    // Add turn and river cards to suit_cards_all
    if (gs.turn_seen) suit_cards_all[(gs.turn >> 4) & 0b11] |= (0b1 << (gs.turn & 0b1111));
    if (gs.rivr_seen) suit_cards_all[(gs.rivr >> 4) & 0b11] |= (0b1 << (gs.rivr & 0b1111));

    for (int c1 = 0; c1 < 36; c1++) {
        for (int c2 = 0; c2 < 36; c2++) {
            if (c1 == c2) continue;
            if ((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)]) || 
                (suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])) continue;

            float hand_weight = range[c1][c2];
            if (hand_weight == 0) continue;

            std::array<uint32_t, 4> suit_cards = {gs.suita, gs.suitb, gs.suitc, gs.suitd};
            suit_cards[c1/9] |= (gs.player==0) ? (0b1 << (c1%9)) : (0b1 << (9+(c1%9)));
            suit_cards[c2/9] |= (gs.player==0) ? (0b1 << (c2%9)) : (0b1 << (9+(c2%9)));

            gs.suita = suit_cards[0];
            gs.suitb = suit_cards[1];
            gs.suitc = suit_cards[2];
            gs.suitd = suit_cards[3];

            InfoSet is = gs.to_information_set();

            float fold_prob = get_average_strategy(is)[0];
            // if (c1%5==0 && c2%8==0) {
            // 	std::cout << gs.to_string();
            // 	std::cout << "Hand: " << c1 << ", " << c2 << "\nInfoSet: " << is.to_string() << "Fold prob: " << fold_prob << "\n\n";
            // }
            fold_probability += hand_weight * fold_prob;

            // Update the range
            total_weight += range[c1][c2];

            gs = temp;
        }
    }

    // Normalize the fold probability
    fold_probability /= total_weight;

    return fold_probability;
}

int local_best_response(GameState gs, bool exploitative_player, std::array<std::array<float, 36>, 36> opponent_range) {

    GameState temp = gs;
    float win_probability = win_probability_rollout(gs, exploitative_player, opponent_range, 15);
    float asked = gs.biggest_bet - gs.biggest_mutual_bet;
    std::array<float, 7> action_utils = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    if (gs.action_to_index(7) != -1) {
    	// call is possible
    	action_utils[gs.action_to_index(7)] = win_probability * (gs.pot_size + gs.biggest_bet + gs.biggest_mutual_bet) - (1 - win_probability) * asked; // utility of call
    	// std::cout << "Call" << ":\n"
        //       << "  win_probability: " << win_probability << "\n"
        //       << "  utility: " << action_utils[6] << "\n\n";
    }

    int actions = gs.num_actions();

    // iterate over all considered bets/raises
    for (int a=1; (a<6) && (a<actions); a++) {
    	if (gs.index_to_action(a)==7) continue;
    	gs.apply_index(a);

        float fold_probability = calculate_fold_probability(gs, opponent_range);
        std::array<std::array<float, 36>, 36> updated_opponent_range = update_range(gs, 0, exploitative_player, 1, opponent_range);

        // std::cout << "updated_opponent_range:\n";
        // print_opponent_range(updated_opponent_range);
        // std::cout << "\n";

        win_probability = win_probability_rollout(gs, exploitative_player, updated_opponent_range, 15);
        // std::cout << "win_probability: " << win_probability << "\nfold_probability: " << fold_probability << "\n";

    	action_utils[a] = fold_probability * (gs.pot_size + gs.biggest_mutual_bet + temp.biggest_mutual_bet) 
    					  + (1 - fold_probability) * (win_probability * (gs.pot_size + gs.biggest_bet + temp.biggest_mutual_bet) 
    					  						      - (1 - win_probability) * (gs.biggest_bet - temp.biggest_mutual_bet));

    	// std::cout << "Action " << a << ":\n"
        //       << "  win_probability: " << win_probability << "\n"
        //       << "  fold_probability: " << fold_probability << "\n"
        //       << "  utility: " << action_utils[a] << "\n\n";

        gs = temp;
    }

    float max_util = -1;
    int best_action = -1;
    for (int a=1; a<actions; a++) {
        if (action_utils[a] > max_util) {
            max_util = action_utils[a];
            best_action = a;
        }
    }

    // std::cout << "action_utils: " << action_utils << "\n";

	if (max_util > 1e-6) return best_action;
    return 0;
}

// float win_probability_rollout(GameState gs, bool exploitative_player, const std::array<std::array<float, 36>, 36>& opponent_range, int samples) {
    
//     float total_wp = 0.0f;
//     float total_weight = 0.0f;

//     for (int i = 0; i < samples; i++) {
//         // Mask out opponent's actual cards
//         uint32_t mask = (exploitative_player == 0) ? 0b111111111000000000111111111
//                                                    : 0b111111111111111111000000000;
//         gs.suita &= mask;
//         gs.suitb &= mask;
//         gs.suitc &= mask;
//         gs.suitd &= mask;

//         GameState temp = gs;

//         std::array<uint16_t, 4> suit_cards_all = {
//         	(uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
//         	(uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
//         	(uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
//         	(uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
//     	};

//         // Add turn and river cards to suit_cards_all
//         if (gs.turn_seen) suit_cards_all[(gs.turn >> 4) & 0b11] |= (0b1 << (gs.turn & 0b1111));
//         if (gs.rivr_seen) suit_cards_all[(gs.rivr >> 4) & 0b11] |= (0b1 << (gs.rivr & 0b1111));

//         float iteration_wp = 0.0f;
//         float iteration_weight = 0.0f;

//         for (int c1 = 0; c1 < 36; c1++) {
//             for (int c2 = 0; c2 < 36; c2++) {
//                 if (c2 == c1) continue;
//                 if ((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)]) || 
//                     (suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])) continue;

//                 float hand_weight = opponent_range[c1][c2];
//                 if (hand_weight == 0) continue;

//                 std::array<uint32_t, 4> suit_cards = {gs.suita, gs.suitb, gs.suitc, gs.suitd};
//                 suit_cards[c1/9] |= (exploitative_player) ? (0b1 << (c1%9)) : (0b1 << (9+(c1%9)));
//                 suit_cards[c2/9] |= (exploitative_player) ? (0b1 << (c2%9)) : (0b1 << (9+(c2%9)));

//                 gs.suita = suit_cards[0];
//                 gs.suitb = suit_cards[1];
//                 gs.suitc = suit_cards[2];
//                 gs.suitd = suit_cards[3];

//                 // Deal remaining community cards if needed
// 		        if (!gs.flop_seen) gs.apply_chance_action(32), gs.apply_chance_action(31), gs.apply_chance_action(30);
// 		        if (!gs.turn_seen) gs.apply_chance_action(29);
// 		        if (!gs.rivr_seen) gs.apply_chance_action(28);

//                 int exploitative_player_best = gs.best_hand(exploitative_player);
//                 int opponent_best = gs.best_hand(!exploitative_player);

//                 if (exploitative_player_best > opponent_best) {
//                     iteration_wp += hand_weight;
//                 } else if (exploitative_player_best == opponent_best) {
//                     iteration_wp += 0.5f * hand_weight;
//                 }

//                 iteration_weight += hand_weight;

//                 gs = temp;
//             }
//         }

//         if (iteration_weight > 1e-6) {
//             total_wp += iteration_wp / iteration_weight;
//             total_weight += 1.0f;
//         }
//     }

//     return (total_weight > 0) ? total_wp / total_weight : 0.5f;
// }

float win_probability_rollout(GameState gs, bool exploitative_player, const std::array<std::array<float, 36>, 36>& opponent_range, int samples) {
    float total_wp = 0.0f;
    float total_weight = 0.0f;
    
    // Mask out opponent's actual cards
    uint32_t mask = (exploitative_player == 0) ? 0b111111111000000000111111111
                                               : 0b111111111111111111000000000;
    gs.suita &= mask;
    gs.suitb &= mask;
    gs.suitc &= mask;
    gs.suitd &= mask;
    
    GameState temp = gs;
    std::array<uint16_t, 4> suit_cards_all = {
        (uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
        (uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
        (uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
        (uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
    };
    
    // Add turn and river cards to suit_cards_all
    if (gs.turn_seen) suit_cards_all[(gs.turn >> 4) & 0b11] |= (0b1 << (gs.turn & 0b1111));
    if (gs.rivr_seen) suit_cards_all[(gs.rivr >> 4) & 0b11] |= (0b1 << (gs.rivr & 0b1111));

    for (int c1 = 0; c1 < 36; c1++) {
        for (int c2 = 0; c2 < 36; c2++) {
            if (c2 == c1) continue;
            if ((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)]) || 
                (suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])) continue;
            
            float hand_weight = opponent_range[c1][c2];
            if (hand_weight == 0) continue;

            std::array<uint32_t, 4> suit_cards = {gs.suita, gs.suitb, gs.suitc, gs.suitd};
            suit_cards[c1/9] |= (exploitative_player) ? (0b1 << (c1%9)) : (0b1 << (9+(c1%9)));
            suit_cards[c2/9] |= (exploitative_player) ? (0b1 << (c2%9)) : (0b1 << (9+(c2%9)));
            
            float hand_wp = 0.0f;
            for (int i = 0; i < samples; i++) {
                GameState sample_gs = temp;
                sample_gs.suita = suit_cards[0];
                sample_gs.suitb = suit_cards[1];
                sample_gs.suitc = suit_cards[2];
                sample_gs.suitd = suit_cards[3];

                // Deal remaining community cards if needed
                if (!sample_gs.flop_seen) sample_gs.apply_chance_action(32), sample_gs.apply_chance_action(31), sample_gs.apply_chance_action(30);
                if (!sample_gs.turn_seen) sample_gs.apply_chance_action(29);
                if (!sample_gs.rivr_seen) sample_gs.apply_chance_action(28);

                int exploitative_player_best = sample_gs.best_hand(exploitative_player);
                int opponent_best = sample_gs.best_hand(!exploitative_player);

                if (exploitative_player_best > opponent_best) {
                    hand_wp += 1.0f;
                } else if (exploitative_player_best == opponent_best) {
                    hand_wp += 0.5f;
                }
            }
            
            hand_wp /= samples;
            total_wp += hand_wp * hand_weight;
            total_weight += hand_weight;
        }
    }

    return (total_weight > 1e-6) ? total_wp / total_weight : 0.5f;
}
