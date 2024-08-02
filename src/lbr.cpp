// #include "lbr.h"

// float calculate_exploitability(int iterations, std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> player_strategy_sum) {
//     float total_winnings = 0;
//     for (int i=0; i<iterations; i++) {
//         for (int player=0; player<2; player++) {
//             GameState gs = generate_random_initial_state();

//             // initialize uniform range
//             // 

//             total_winnings += lbr_traverse_tree(gs, player, );
//         }
//         if ((i+1)%100==0) std::cout << "Average exploitability at i=" << i+1 << ": " << total_winnings/(2*(i+1)) << "\n"; 
//     }
//     return total_winnings / (iterations*2);
// }

// float lbr_traverse_tree(GameState gs, bool active_player, std::array<float, 81> opponent_range) {
//     if (gs.is_terminal()) {
//         return gs.utility(active_player) * q;
//     }

//     if (gs.is_chance()) {
//         GameState temp = gs;
//         int num_actions = temp.num_chance_actions();
//         temp.apply_chance_action(num_actions);
//         return lbr_traverse_tree(temp, active_player, q);
//     }

//     InfoSet info_set = gs.to_information_set();
//     int actions = info_set.num_actions;
    
//     if (info_set.player != active_player) {
//         // Opponent's turn: use their strategy
//         std::array<float, 7> strategy = get_strategy(info_set);
//         int sampled_action = sample_action(strategy);
//         GameState child = gs;
//         child.apply_index(sampled_action);
//         return lbr_traverse_tree(child, active_player, q * strategy[sampled_action]);
//     }

//     // Active player's turn: compute best response
//     float best_value = -999999;
//     for (int a=0; a<actions; a++) {
//         GameState child = gs;
//         child.apply_index(a);
//         float action_value = lbr_traverse_tree(child, active_player, q);
//         best_value = std::max(best_value, action_value);
//     }

//     return best_value;
// }

// int local_best_response(GameState gs, bool active_player, std::array<float, 630> opponent_range) {
//     float win_probability = win_probability_rollout(gs, active_player, opponent_range);
//     float asked = 0; // fix this!!
//     std::array<float, 3> action_utils = {0.0f, 0.0f, 0.0f};

//     action_utils[1] = win_probability * gs.pot_size() - (1 - win_probability) * asked; // utility of call

//     int actions = gs.num_actions();

//     for (int a=1; a<actions; a++) {
//         float fold_probability = 0;
//         std::array<float, 81> updated_opponent_range;

//         for (int h1=0; h1<36; h1++) {
//             for (int h2=h1+1; h2<36; h2++) {
//                 // update gs to incorporate hole cards
//                 int p_id = gs.p_id(0);//fix active player

//                 std::array<float, 3> strategy = get_strategy(gs.to_information_set());
//                 fold_probability += opponent_range[p_id] * strategy[0];
//                 updated_opponent_range[p_id] = opponent_range[p_id] * (1 - strategy[0]);
//             }
//         }

//         // normalize updated_opponent_range

//         win_probability = win_probability_rollout(gs, active_player, updated_opponent_range);

//         float bet_size = 0.0f; // fill in function

//         action_utils[a] = fold_probability * gs.pot_size() + (1 - fold_probability) * (win_probability * (gs.pot_size() + bet_size) + (1 - win_probability) * (asked + bet_size));
//     }

//     float max_util = -999999;
//     int best_action = -1;
//     for (int a=1; a<actions; a++) {
//         if (action_utils[a] > max_util) {
//             max_util = action_utils[a];
//             best_action = a;
//         }
//     }

//     if (max_util > 0) return best_action;
//     return 0;
// }

// float win_probability_rollout(GameState gs, bool active_player, std::array<std::array<float, 36>, 36> opponent_range) {

//     // mask out opponent's actual cards
//     uint32_t mask = (active_player == 0) ? 0b111111111000000000111111111
//                                          : 0b111111111111111111000000000;

//     gs.suita &= mask;
//     gs.suitb &= mask;
//     gs.suitc &= mask;
//     gs.suitd &= mask;

//     for (int p1=0; p1<36; p1++) {
//         for (int p2=0; (p2!=p1) && (p2<36); p2++) {
            

//             if (!gs.flop_seen) {
//                 gs.apply_chance_action(32);
//                 gs.apply_chance_action(31);
//                 gs.apply_chance_action(30);
//             }

//             if (!gs.turn_seen) {
//                 gs.apply_chance_action(29);
//             }

//             if (!gs.rivr_seen) {
//                 gs.apply_chance_action(28);
//             }

//             gs.flop_seen = true;
//             gs.turn_seen = true;
//             gs.rivr_seen = true;

//             int active_player_best_hand = gs.best_hand(0);
//         }
//     }


//     for (int c1=0; c1<36; c1++) {
//         for (int c2=c1+1; c2<36; c2++) {
//             if ((c2!=c1) &&
//                 (!((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)])==SINGLE_MASKS[8-(c1%9)])) &&
//                 (!((suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])==SINGLE_MASKS[8-(c2%9)]))) {

//                 std::array<uint32_t, 4> suit_cards = {suita, suitb, suitc, suitd};
//                 uint32_t tempa = suita;
//                 uint32_t tempb = suitb;
//                 uint32_t tempc = suitc;
//                 uint32_t tempd = suitd;

//                 suit_cards[c1/9] |= (0b1 << (9+(c1%9)));
//                 suit_cards[c2/9] |= (0b1 << (9+(c2%9)));
//                 turn_seen = true;
//                 rivr_seen = true;
                
//                 suita = suit_cards[0];
//                 suitb = suit_cards[1];
//                 suitc = suit_cards[2];
//                 suitd = suit_cards[3];

//                 int player_best = best_hand(0);
//                 int opponent_best = best_hand(1);

//                 if (player_best > opponent_best) {
//                     wins += 1.0f;   
//                 } else if (player_best == opponent_best) {
//                     wins += 0.5f;
//                 }

//                 total += 1;

//                 suita = tempa;
//                 suitb = tempb;
//                 suitc = tempc;
//                 suitd = tempd;
//             }
//         }
//     }
    
// }