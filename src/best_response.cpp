#include "best_response.h"

void print_opponent_reach_probabilities(const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {

    // std::array<std::array<float, 9>, 9> p_id_reach_probabilities;

    // for (int i=0; i<9; i++) {
    //     p_id_reach_probabilities[i] = {0.0f};
    // }

    // std::cout << "Reach Probabilities:\n";
    // for (int c1=1; c1<=NUM_CARDS; c1++) {
    //     for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
    //         if (gs.has_card(c1) || gs.has_card(c2) || c1==c2) continue;
    //         int p = pocket_id(c1, c2);
    //         int r = pocket_id_to_row_col(p)[0];
    //         int c = pocket_id_to_row_col(p)[1];

    //         p_id_reach_probabilities[r][c] += reach_probabilities[c1 - 1][c2 - 1];
    //     }
    // }

    for (int c1=0; c1<=NUM_CARDS; c1++) {
        for (int c2=0; c2<=NUM_CARDS; c2++) {
            if (c1==0 && c2==0) {
                std::cout << "         ";
            } else if (c1==0 && c2>0) {
                std::cout << CARD_NAMES[(c2-1)%9] << "s        ";
            } else if (c2==0 && c1>0) {
                std::cout << CARD_NAMES[(c1-1)%9] << " ";
            } else {
                std::cout << std::scientific << std::setprecision(2);
                if (reach_probabilities[c1 - 1][c2 - 1] >= 0) std::cout << " ";
                std::cout << reach_probabilities[c1 - 1][c2 - 1] << " ";
            }
        }
        std::cout << "\n";
    }

    // for (int r=0; r<10; r++) {
    //     for (int c=0; c<10; c++) {
    //         if (c==0 && r==0) {
    //             std::cout << "         ";
    //         } else if (r==0 && c>0) {
    //             std::cout << CARD_NAMES[9-c] << "s        ";
    //         } else if (c==0 && r>0) {
    //             std::cout << CARD_NAMES[9-r] << " ";
    //         } else {
    //          // float f = 100.0f * range[r-1][c-1];
    //             // std::cout << FIXED_FLOAT(f) << " ";
    //             std::cout << std::scientific << std::setprecision(1);
    //             std::cout << p_id_reach_probabilities[r - 1][c - 1] << " ";
    //         }
    //     }
    //     std::cout << "\n";
    // }
}

float calculate_exploitability_fast(const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_reach_probabilities, 
                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_reach_probabilities,
                                    const std::array<uint8_t, 5> board_cards, 
                                    float pot) {

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    std::array<float, 2> total_exploitability; total_exploitability.fill(0.0f);

    GameState state;
    state.pfp_history = 0b111001;
    state.flp_history = 0b1001;
    state.pot_size = pot;
    state.flp_seen = true;
    state.trn_seen = true;
    state.bets.push(0.0f);
    
    state.fp1 = board_cards[0];
    state.fp2 = board_cards[1];
    state.fp3 = board_cards[2];
    state.trn = board_cards[3];
    state.rvr = board_cards[4];

    generate_rank_table(state);

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_info_set_utilities = br_traverse_tree_fast(state, 0, ip_reach_probabilities);
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_info_set_utilities = br_traverse_tree_fast(state, 1, op_reach_probabilities);

    // print_opponent_reach_probabilities(op_info_set_utilities);
    // print_opponent_reach_probabilities(ip_info_set_utilities);

    for (int c1=1; c1<=NUM_CARDS; c1++) { 
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (state.has_card(c1) || state.has_card(c2)) continue;
            total_exploitability[0] += op_reach_probabilities[c1 - 1][c2 - 1] * op_info_set_utilities[c1 - 1][c2 - 1]; 
        }
    }

    for (int c1=1; c1<=NUM_CARDS; c1++) { 
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (state.has_card(c1) || state.has_card(c2)) continue;
            total_exploitability[1] += ip_reach_probabilities[c1 - 1][c2 - 1] * ip_info_set_utilities[c1 - 1][c2 - 1]; 
        }
    }
    
    std::cout << "Exploitability of OOP: " << total_exploitability[1] << "\n";
    std::cout << "Exploitability of IP:  " << total_exploitability[0] << "\n";
    std::cout << "Total exploitability:  " << (total_exploitability[0] + total_exploitability[1]) / 2.0f << "\n";
    
    return (total_exploitability[0] + total_exploitability[1]) / 2.0f;
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> br_traverse_tree_fast(GameState& gs, bool exploitative_player, 
                                                                          const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_reach_probabilities) {
    if (gs.is_terminal) {
        return expected_utility_fast(gs, exploitative_player, strategy_reach_probabilities);
    }

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> info_set_utilities;
    for (int r=0; r<NUM_CARDS; r++) info_set_utilities[r].fill(0.0f);

    if (gs.is_chance()) {
        int num = gs.num_in_deck();

        for (int c=1; c<=NUM_CARDS; c++) {
            if (gs.has_card(c)) continue;
            
            bool player_temp = gs.player; float pot_size_temp = gs.pot_size;
            gs.deal_card(c);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> new_strategy_reach_probabilities = update_chance_reach_probabilities(gs, num, strategy_reach_probabilities);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = br_traverse_tree_fast(gs, exploitative_player, new_strategy_reach_probabilities);
            gs.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (gs.has_card(c1) || gs.has_card(c2) || (c1==c) || (c2==c)) continue;
                    else info_set_utilities[c1 - 1][c2 - 1] += temp[c1 - 1][c2 - 1];
                }
            }
        }
        return info_set_utilities;
    }

    int actions = gs.num_actions();

    if (gs.player != exploitative_player) {
        for (int a=0; a<actions; a++) {
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> new_strategy_reach_probabilities = update_reach_probabilities(gs, a, strategy_reach_probabilities);
            
            bool player_temp = gs.player; float pot_size_temp = gs.pot_size;
            gs.apply_index(a);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = br_traverse_tree_fast(gs, exploitative_player, new_strategy_reach_probabilities);
            gs.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (gs.has_card(c1) || gs.has_card(c2)) info_set_utilities[c1 - 1][c2 - 1] = std::numeric_limits<float>::lowest();
                    else info_set_utilities[c1 - 1][c2 - 1] += temp[c1 - 1][c2 - 1];
                }
            }
        }
        return info_set_utilities;
    }

    for (int r=0; r<NUM_CARDS; r++) info_set_utilities[r].fill(std::numeric_limits<float>::lowest());
    
    for (int a=0; a<actions; a++) {
        bool player_temp = gs.player; float pot_size_temp = gs.pot_size;
        gs.apply_index(a);
        std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = br_traverse_tree_fast(gs, exploitative_player, strategy_reach_probabilities);
        gs.undo(player_temp, pot_size_temp);

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (gs.has_card(c1) || gs.has_card(c2)) info_set_utilities[c1 - 1][c2 - 1] = std::numeric_limits<float>::lowest();
                else info_set_utilities[c1 - 1][c2 - 1] = std::max(info_set_utilities[c1 - 1][c2 - 1], temp[c1 - 1][c2 - 1]);
            }
        }
    }
    
    return info_set_utilities;
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> expected_utility_fast(GameState gs, bool exploitative_player, 
                                                                          const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> expected_utilities;
    for (int r=0; r<NUM_CARDS; r++) expected_utilities[r].fill(std::numeric_limits<float>::lowest());

    std::array<float, NUM_CARDS> row_reach_sums; row_reach_sums.fill(0.0f);
    std::array<float, NUM_CARDS> col_reach_sums; col_reach_sums.fill(0.0f);
    float total_reach = 0.0f;

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            row_reach_sums[c1 - 1] += reach_probabilities[c1 - 1][c2 - 1];
            col_reach_sums[c2 - 1] += reach_probabilities[c1 - 1][c2 - 1];
            total_reach += reach_probabilities[c1 - 1][c2 - 1];
        }
    }

    if (gs.is_fold()) {
        float fold_utility = (gs.player == exploitative_player) ? gs.pot_size * -0.5f : gs.pot_size * 0.5f;
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (gs.has_card(c1) || gs.has_card(c2)) continue;
                float r = total_reach - row_reach_sums[c1 - 1]
                                      - col_reach_sums[c2 - 1]
                                      + reach_probabilities[c1 - 1][c2 - 1];
                expected_utilities[c1 - 1][c2 - 1] = fold_utility * r;
            }
        } 
        return expected_utilities;
    }

    std::unordered_map<int, std::vector<std::tuple<float, int, int>>> strength_map;
    
    // Prepare all possible hands
    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (gs.has_card(c1) || gs.has_card(c2)) continue;
            GameState temp_gs = gs;
            temp_gs.op1 = c1; temp_gs.op2 = c2;
            int strength = temp_gs.best_hand_fast();
            float reach = reach_probabilities[c1 - 1][c2 - 1];

            strength_map[strength].emplace_back(reach, c1, c2);
        }
    }

    std::vector<int> sorted_strengths;
    sorted_strengths.reserve(strength_map.size());

    for (const auto& [strength, _] : strength_map) {
        sorted_strengths.push_back(strength);
    }

    std::sort(sorted_strengths.begin(), sorted_strengths.end());

    std::array<float, NUM_CARDS> row_reach_sums_worse; row_reach_sums_worse.fill(0.0f);
    std::array<float, NUM_CARDS> col_reach_sums_worse; col_reach_sums_worse.fill(0.0f);
    std::array<float, NUM_CARDS> row_reach_sums_equal; row_reach_sums_equal.fill(0.0f);
    std::array<float, NUM_CARDS> col_reach_sums_equal; col_reach_sums_equal.fill(0.0f);

    float cumulative_reach_probabilities_of_worse = 0.0f;
    float cumulative_reach_probabilities_of_equal = 0.0f;
    
    for (int strength : sorted_strengths) {
        // We've moved to a new hand rank level
        cumulative_reach_probabilities_of_worse += cumulative_reach_probabilities_of_equal;
        cumulative_reach_probabilities_of_equal = 0.0f;

        row_reach_sums_equal.fill(0.0f);
        col_reach_sums_equal.fill(0.0f);

        for (const auto& [reach, c1, c2] : strength_map[strength]) {
            cumulative_reach_probabilities_of_equal += reach;
            row_reach_sums_equal[c1 - 1] += reach;
            col_reach_sums_equal[c2 - 1] += reach;
        }

        for (const auto& [reach, c1, c2] : strength_map[strength]) {

            float cumulative_reach_probabilities_of_hands_no_overlaps = clip(total_reach - row_reach_sums[c1 - 1] 
                                                                                         - col_reach_sums[c2 - 1]
                                                                                         + reach);
            
            float cumulative_reach_probabilities_of_worse_no_overlaps = clip(cumulative_reach_probabilities_of_worse - row_reach_sums_worse[c1 - 1]
                                                                                                                     - col_reach_sums_worse[c2 - 1]);

            float cumulative_reach_probabilities_of_equal_no_overlaps = clip(cumulative_reach_probabilities_of_equal - row_reach_sums_equal[c1 - 1]
                                                                                                                     - col_reach_sums_equal[c2 - 1]
                                                                                                                     + reach);

            float cumulative_reach_probabilities_of_bettr_no_overlaps = clip(cumulative_reach_probabilities_of_hands_no_overlaps - cumulative_reach_probabilities_of_equal_no_overlaps
                                                                                                                                 - cumulative_reach_probabilities_of_worse_no_overlaps);

            float utility = gs.pot_size * 0.5f;
            expected_utilities[c1 - 1][c2 - 1] = cumulative_reach_probabilities_of_worse_no_overlaps * utility - cumulative_reach_probabilities_of_bettr_no_overlaps * utility;
        }

        for (const auto& [reach, c1, c2] : strength_map[strength]) {
            row_reach_sums_worse[c1 - 1] += reach;
            col_reach_sums_worse[c2 - 1] += reach;
        }
    }

    return expected_utilities;
}







float calculate_exploitability(const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_reach_probabilities, 
                               const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_reach_probabilities,
                               const std::array<uint8_t, 5> board_cards, 
                               float pot) {

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 0.0f;
    }

    std::array<float, 2> total_exploitability = {0.0f};

    GameState gs;
    gs.pfp_history = 0b111001;
    gs.flp_history = 0b1001;
    gs.pot_size = pot;
    gs.flp_seen = true;
    gs.trn_seen = true;
    gs.bets.push(0.0f);
    
    gs.fp1 = board_cards[0];
    gs.fp2 = board_cards[1];
    gs.fp3 = board_cards[2];
    gs.trn = board_cards[3];
    gs.rvr = board_cards[4];

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_info_set_utilities;
    for (int r=0; r<NUM_CARDS; r++) op_info_set_utilities[r] = {0.0f};

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            // Take out all private cards
            gs.ip1 = 0; gs.ip2 = 0; gs.op1 = 0; gs.op2 = 0;
            if (gs.has_card(c1) || gs.has_card(c2)) continue;

            for (int exploitative_player=0; exploitative_player<=1; exploitative_player++) {
                // Skip hands that aren't part of our range
                if ((exploitative_player==0) && (op_reach_probabilities[c1-1][c2-1]<1e-6)) continue;
                if ((exploitative_player==1) && (ip_reach_probabilities[c1-1][c2-1]<1e-6)) continue;

                // Assign our private cards
                if (exploitative_player==0) { gs.op1 = c1; gs.op2 = c2; gs.ip1 = 0; gs.ip2 = 0; }
                else { gs.ip1 = c1; gs.ip2 = c2; gs.op1 = 0; gs.op2 = 0; }

                total_exploitability[exploitative_player] += (exploitative_player == 0) ? op_reach_probabilities[c1-1][c2-1] * br_traverse_tree(gs, exploitative_player, ip_reach_probabilities)
                                                                                        : ip_reach_probabilities[c1-1][c2-1] * br_traverse_tree(gs, exploitative_player, op_reach_probabilities);

                op_info_set_utilities[c1-1][c2-1] = br_traverse_tree(gs, 0, ip_reach_probabilities);
                std::cout << "c1: " << c1 << "c2: " << c2 << "\n";
            }
        }
    }

    print_opponent_reach_probabilities(op_info_set_utilities);

    std::cout << "Avg exploitability (mbb/h) of OOP: " << 1000.0f * total_exploitability[1] / (pot * 0.5f) << "\n";
    std::cout << "Avg exploitability (mbb/h) of IP:  " << 1000.0f * total_exploitability[0] / (pot * 0.5f) << "\n";
    std::cout << "Avg exploitability (raw) of OOP:   " << total_exploitability[1] << "\n";
    std::cout << "Avg exploitability (raw) of IP:    " << total_exploitability[0] << "\n";
    
    return (total_exploitability[0] + total_exploitability[1]) / 2;
}

float br_traverse_tree(GameState gs, bool exploitative_player, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {

    if (gs.is_terminal) {
        return expected_utility(gs, exploitative_player, reach_probabilities);
    }

    float node_util = 0.0f;

    if (gs.is_chance()) {
        int num = gs.num_in_deck();

        for (int c=1; c<=NUM_CARDS; c++) {
            if (gs.has_card(c)) continue;

            GameState child = gs;
            child.deal_card(c);

            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> new_reach_probabilities = update_chance_reach_probabilities(child, num, reach_probabilities);
            
            node_util += br_traverse_tree(child, exploitative_player, new_reach_probabilities);
        }
        
        return node_util;
    }

    int actions = gs.num_actions();

    if (gs.player != exploitative_player) {
        for (int a=0; a<actions; a++) {
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> new_reach_probabilities = update_reach_probabilities(gs, a, reach_probabilities);
            GameState child = gs;
            child.apply_index(a);
            node_util += br_traverse_tree(child, exploitative_player, new_reach_probabilities);
        }
        return node_util;
    }

    float action_val;
    float best_val = -9999.0f;
    
    for (int a=0; a<actions; a++) {
        GameState child = gs;
        child.apply_index(a);
        action_val = br_traverse_tree(child, exploitative_player, reach_probabilities);

        best_val = std::max(best_val, action_val);
    }
    
    return best_val;
}

float expected_utility(GameState gs, bool exploitative_player, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    float expected_utility = 0.0f;

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (exploitative_player==0) { gs.ip1 = 0; gs.ip2 = 0; }
            else { gs.op1 = 0; gs.op2 = 0; }

            if (gs.has_card(c1) || gs.has_card(c2)) continue;

            // insert opponent's cards
            if (exploitative_player==0) { gs.ip1 = c1; gs.ip2 = c2; }
            else { gs.op1 = c1; gs.op2 = c2; }

            expected_utility += reach_probabilities[c1 - 1][c2 - 1] * gs.utility(exploitative_player);   
        }
    }

    return expected_utility;
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> update_reach_probabilities(GameState gs, int action, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> res;
    for (int r=0; r<NUM_CARDS; r++) res[r].fill(0.0f);

    // Mask out acting player's cards
    if (gs.player==0) { gs.op1 = 0; gs.op2 = 0; }
    else { gs.ip1 = 0; gs.ip2 = 0; }

    InfoSet is = gs.to_information_set();

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (gs.has_card(c1) || gs.has_card(c2)) continue; // this combination of hole cards is not possible
            
            // Assign acting player their cards
            is.cr1 = c1;
            is.cr2 = c2;

            float action_probability = get_average_strategy(is)[action];
            
            res[c1 - 1][c2 - 1] = (action_probability) * reach_probabilities[c1 - 1][c2 - 1];
        }
    }
    return res;
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> update_chance_reach_probabilities(GameState gs, int num, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> res;
    for (int r=0; r<NUM_CARDS; r++) res[r].fill(0.0f);

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (gs.has_card(c1) || gs.has_card(c2)) continue; // this combination of hole cards is not possible
            
            res[c1 - 1][c2 - 1] = (1.0f / num) * reach_probabilities[c1 - 1][c2 - 1];
        }
    }
    return res;
}