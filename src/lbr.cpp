#include "lbr.h"

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

// int local_best_response(GameState gs, bool active_player, std::array<std::array<float, 36>, 36> opponent_range) {
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

float win_probability_rollout(GameState gs, bool exploitative_player, const std::array<std::array<float, 36>, 36>& opponent_range, int samples) {
    GameState temp = gs;
    float total_wp = 0.0f;
    float total_weight = 0.0f;

    for (int i = 0; i < samples; i++) {
        gs = temp;

        // Mask out opponent's actual cards
        uint32_t mask = (exploitative_player == 0) ? 0b111111111000000000111111111
                                                   : 0b111111111111111111000000000;
        gs.suita &= mask;
        gs.suitb &= mask;
        gs.suitc &= mask;
        gs.suitd &= mask;

        // Deal remaining community cards if needed
        if (!gs.flop_seen) gs.apply_chance_action(32), gs.apply_chance_action(31), gs.apply_chance_action(30);
        if (!gs.turn_seen) gs.apply_chance_action(29);
        if (!gs.rivr_seen) gs.apply_chance_action(28);

        std::array<uint16_t, 4> suit_cards_all = {
            (uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
            (uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
            (uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
            (uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
        };

        // Add turn and river cards to suit_cards_all
        suit_cards_all[(gs.turn >> 4) & 0b11] |= (0b1 << (gs.turn & 0b1111));
        suit_cards_all[(gs.rivr >> 4) & 0b11] |= (0b1 << (gs.rivr & 0b1111));

        float iteration_wp = 0.0f;
        float iteration_weight = 0.0f;

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

                gs.suita = suit_cards[0];
                gs.suitb = suit_cards[1];
                gs.suitc = suit_cards[2];
                gs.suitd = suit_cards[3];

                int exploitative_player_best = gs.best_hand(exploitative_player);
                int opponent_best = gs.best_hand(!exploitative_player);

                if (exploitative_player_best > opponent_best) {
                    iteration_wp += hand_weight;
                } else if (exploitative_player_best == opponent_best) {
                    iteration_wp += 0.5f * hand_weight;
                }

                iteration_weight += hand_weight;

                // Reset suits for next iteration
                gs.suita = suit_cards[0] & mask;
                gs.suitb = suit_cards[1] & mask;
                gs.suitc = suit_cards[2] & mask;
                gs.suitd = suit_cards[3] & mask;
            }
        }

        if (iteration_weight > 0) {
            total_wp += iteration_wp / iteration_weight;
            total_weight += 1.0f;
        }
    }

    return (total_weight > 0) ? total_wp / total_weight : 0.5f;
}

// float win_probability_rollout(GameState gs, bool exploitative_player, const std::array<std::array<float, 36>, 36>& strategy_range, int samples) {
//     GameState temp = gs;
//     float total_wp = 0.0f;
//     float total_weight = 0.0f;

//     for (int i = 0; i < samples; i++) {
//         gs = temp;
//         uint32_t mask = (exploitative_player == 0) ? 0b111111111000000000111111111
//                                                    : 0b111111111111111111000000000;
//         gs.suita &= mask;
//         gs.suitb &= mask;
//         gs.suitc &= mask;
//         gs.suitd &= mask;

//         // Deal remaining community cards if needed
//         if (!gs.flop_seen) gs.apply_chance_action(32), gs.apply_chance_action(31), gs.apply_chance_action(30);
//         if (!gs.turn_seen) gs.apply_chance_action(29);
//         if (!gs.rivr_seen) gs.apply_chance_action(28);

//         std::array<uint16_t, 4> suit_cards_all = {
//             (uint16_t)(((gs.suita >> 18) | (gs.suita >> 9) | gs.suita) & 0b111111111),
//             (uint16_t)(((gs.suitb >> 18) | (gs.suitb >> 9) | gs.suitb) & 0b111111111),
//             (uint16_t)(((gs.suitc >> 18) | (gs.suitc >> 9) | gs.suitc) & 0b111111111),
//             (uint16_t)(((gs.suitd >> 18) | (gs.suitd >> 9) | gs.suitd) & 0b111111111)
//         };

//         // Add turn and river cards to suit_cards_all
//         suit_cards_all[(gs.turn >> 4) & 0b11] |= (0b1 << (gs.turn & 0b1111));
//         suit_cards_all[(gs.rivr >> 4) & 0b11] |= (0b1 << (gs.rivr & 0b1111));

//         float iteration_wp = 0.0f;
//         float iteration_weight = 0.0f;
//         int valid_combinations = 0;

//         for (int c1 = 0; c1 < 36; c1++) {
//             for (int c2 = 0; c2 < 36; c2++) {
//                 if (c2 == c1) continue;
//                 if ((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)]) || 
//                     (suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])) continue;

//                 float hand_weight = strategy_range[c1][c2];
//                 if (hand_weight == 0) continue;

//                 std::array<uint32_t, 4> suit_cards = {gs.suita, gs.suitb, gs.suitc, gs.suitd};


//                 suit_cards[c1/9] |= (exploitative_player) ? (0b1 << (c1%9)) : (0b1 << (9+(c1%9)));
//                 suit_cards[c2/9] |= (exploitative_player) ? (0b1 << (c2%9)) : (0b1 << (9+(c2%9)));

//                 gs.suita = suit_cards[0];
//                 gs.suitb = suit_cards[1];
//                 gs.suitc = suit_cards[2];
//                 gs.suitd = suit_cards[3];

//                 int exploitative_player_best = gs.best_hand(exploitative_player);
//                 int opponent_best = gs.best_hand(!exploitative_player);

//                 if (exploitative_player_best > opponent_best) {
//                     iteration_wp += hand_weight;
//                 } else if (exploitative_player_best == opponent_best) {
//                     iteration_wp += 0.5f * hand_weight;
//                 }

//                 iteration_weight += hand_weight;
//                 valid_combinations++;

//                 // Reset suits for next iteration
//                 gs.suita = suit_cards[0] & mask;
//                 gs.suitb = suit_cards[1] & mask;
//                 gs.suitc = suit_cards[2] & mask;
//                 gs.suitd = suit_cards[3] & mask;
//             }
//         }

//         if (iteration_weight > 0) {
//             float sample_wp = iteration_wp / iteration_weight;
//             total_wp += sample_wp;
//             total_weight += 1.0f;
//             std::cout << "Sample " << i << ": WP = " << sample_wp << ", Valid combinations: " << valid_combinations << std::endl;
//         } else {
//             std::cout << "Sample " << i << ": No valid combinations" << std::endl;
//         }
//     }

//     float final_wp = (total_weight > 0) ? total_wp / total_weight : 0.5f;
//     std::cout << "Final WP: " << final_wp << ", Total weight: " << total_weight << std::endl;
//     return final_wp;
// }