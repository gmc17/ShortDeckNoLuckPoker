#include "best_response.h"

float calculate_exploitability(
    const Tree& tree,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& op_range, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& ip_range) {

    const auto ones = create_ones_array();
    int num_threads = get_cpu_cores();
    ThreadPool pool(num_threads);
    auto ip_info_set_utilities = best_response_traverse_tree(tree.get_root(), 1, 0, ones, op_range, ip_range, pool);
    auto op_info_set_utilities = best_response_traverse_tree(tree.get_root(), 0, 0, ones, ip_range, op_range, pool);

    auto& decision_node = tree.get_root()->as_decision_node();

    std::array<float, 2> total_exploitability = {0.0f};
    for (int c1=1; c1<=NUM_CARDS; c1++) { 
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                total_exploitability[0] += op_info_set_utilities[c1 - 1][c2 - 1];
                total_exploitability[1] += ip_info_set_utilities[c1 - 1][c2 - 1]; 
            } 
        }
    }
    
    // std::cout << "Exploitability of OOP: " << total_exploitability[1] << "\n";
    // std::cout << "Exploitability of IP:  " << total_exploitability[0] << "\n";
    // std::cout << "Total exploitability:  " << (total_exploitability[0] + total_exploitability[1]) / 2.0f << "\n";
    // std::cout << (total_exploitability[0] + total_exploitability[1]) / 2.0f << "\n";
    
    return (total_exploitability[0] + total_exploitability[1]) / 2.0f;
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> best_response_traverse_tree(
    Tree::Node* node,
    bool exploitative_player, 
    bool chance_layer_seen,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& exploitative_player_range,
    ThreadPool& pool) {
    
    if (node->is_terminal_node()) {
        auto temp = best_response_terminal_node_utility(node, exploitative_player, reach_probabilities, strategy_range, exploitative_player_range);
        return *temp;
    }

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> info_set_utilities;
    for (int r=0; r<NUM_CARDS; r++) info_set_utilities[r].fill(0.0f);

    // if (node->is_chance_node()) {
    //     auto& chance_node = node->as_chance_node();
    //     int num = chance_node.num;

    //     for (int c=1; c<=NUM_CARDS; c++) {
    //         if (!chance_node.has_card(c)) {
    //             auto new_reach_probabilities = update_chance_reach_probabilities(chance_node.children[c - 1].get(), num, reach_probabilities);
    //             std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = best_response_traverse_tree(chance_node.children[c - 1].get(), exploitative_player, *new_reach_probabilities, strategy_range, exploitative_player_range);

    //             for (int c1=1; c1<=NUM_CARDS; c1++) {
    //                 for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
    //                     if (!(chance_node.has_card(c1) || chance_node.has_card(c2) || (c1==c) || (c2==c))) {
    //                         info_set_utilities[c1 - 1][c2 - 1] += temp[c1 - 1][c2 - 1];
    //                     }
    //                 }
    //             }
    //         }
    //     }
    //     return info_set_utilities;
    // }

    if (node->is_chance_node()) {
        auto& chance_node = node->as_chance_node();
        int num = chance_node.num;
        std::vector<std::future<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>> futures;
        
        for (int c=1; c<=NUM_CARDS; c++) {
            if (!chance_node.has_card(c)) {
                if (!chance_layer_seen) {
                    futures.push_back(pool.enqueue([&, c, num]() {
                        auto new_reach_probabilities = update_chance_reach_probabilities(chance_node.children[c - 1].get(), num, reach_probabilities);
                        auto result = best_response_traverse_tree(chance_node.children[c - 1].get(), exploitative_player, 1, *new_reach_probabilities, strategy_range, exploitative_player_range, pool);
                        for (int c1=1; c1<=NUM_CARDS; c1++) {
                            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                                if (chance_node.has_card(c1) || chance_node.has_card(c2) || (c1==c) || (c2==c)) {
                                    result[c1 - 1][c2 - 1] = 0.0f;
                                }
                            }
                        }
                        return result;
                    }));
                } else {
                    auto new_reach_probabilities = update_chance_reach_probabilities(chance_node.children[c - 1].get(), num, reach_probabilities);
                    auto result = best_response_traverse_tree(chance_node.children[c - 1].get(), exploitative_player, 1, *new_reach_probabilities, strategy_range, exploitative_player_range, pool);
                    for (int c1=1; c1<=NUM_CARDS; c1++) {
                        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                            if (chance_node.has_card(c1) || chance_node.has_card(c2) || (c1==c) || (c2==c)) {
                                result[c1 - 1][c2 - 1] = 0.0f;
                            }
                        }
                    }
                    add_2d_arrays_simd(info_set_utilities, result);
                }
            }
        }

        // combine thread results
        for (auto& future : futures) {
            auto result = future.get();
            add_2d_arrays_simd(info_set_utilities, result);
        }

        return info_set_utilities;
    }

    auto& decision_node = node->as_decision_node();
    int actions = decision_node.actions;

    if (decision_node.player != exploitative_player) {
        for (int a=0; a<actions; a++) {
            auto new_reach_probabilities = update_reach_probabilities_using_average_strategy(node, a, reach_probabilities);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = best_response_traverse_tree(decision_node.children[a].get(), exploitative_player, chance_layer_seen, *new_reach_probabilities, strategy_range, exploitative_player_range, pool);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                        info_set_utilities[c1 - 1][c2 - 1] += temp[c1 - 1][c2 - 1];
                    } else {
                        info_set_utilities[c1 - 1][c2 - 1] = std::numeric_limits<float>::lowest();
                    }
                }
            }
        }
        return info_set_utilities;
    }

    for (int r=0; r<NUM_CARDS; r++) info_set_utilities[r].fill(std::numeric_limits<float>::lowest());
    
    for (int a=0; a<actions; a++) {
        auto temp = best_response_traverse_tree(decision_node.children[a].get(), exploitative_player, chance_layer_seen, reach_probabilities, strategy_range, exploitative_player_range, pool);

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                    info_set_utilities[c1 - 1][c2 - 1] = std::max(info_set_utilities[c1 - 1][c2 - 1], temp[c1 - 1][c2 - 1]);
                }
            }
        }
    }
    
    return info_set_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> best_response_terminal_node_utility(
    const Tree::Node* node, 
    bool traversing_player, 
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& exploitative_player_range) {

    auto expected_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*expected_utilities)[r].fill(std::numeric_limits<float>::lowest());

    multiply_2d_arrays_simd(reach_probabilities, strategy_range);

    std::array<float, NUM_CARDS> row_reach_sums; row_reach_sums.fill(0.0f);
    std::array<float, NUM_CARDS> col_reach_sums; col_reach_sums.fill(0.0f);
    float total_reach = 0.0f;

    const auto& terminal_node = node->as_terminal_node();

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(terminal_node.has_card(c1) || terminal_node.has_card(c2))) {
                row_reach_sums[c1 - 1] += reach_probabilities[c1 - 1][c2 - 1];
                col_reach_sums[c2 - 1] += reach_probabilities[c1 - 1][c2 - 1];
                total_reach += reach_probabilities[c1 - 1][c2 - 1];
            }
        }
    }

    if (terminal_node.is_fold) {
        float fold_utility = (terminal_node.folding_player == traversing_player) ? terminal_node.pot * -0.5f : terminal_node.pot * 0.5f;
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(terminal_node.has_card(c1) || terminal_node.has_card(c2))) {
                    float r = total_reach - row_reach_sums[c1 - 1]
                                          - col_reach_sums[c2 - 1]
                                          + reach_probabilities[c1 - 1][c2 - 1];
                    (*expected_utilities)[c1 - 1][c2 - 1] = fold_utility * r * exploitative_player_range[c1 - 1][c2 - 1];
                }
            }
        } 
        return expected_utilities;
    }

    auto& strength_map = strength_map_table[terminal_node.trn - 1][terminal_node.rvr - 1];
    auto& sorted_strengths = sorted_strengths_table[terminal_node.trn - 1][terminal_node.rvr - 1];

    std::array<float, NUM_CARDS> row_reach_sums_worse;
    std::array<float, NUM_CARDS> col_reach_sums_worse; 
    std::array<float, NUM_CARDS> row_reach_sums_equal; 
    std::array<float, NUM_CARDS> col_reach_sums_equal; 

    fast_initialize_array(row_reach_sums_worse);
    fast_initialize_array(col_reach_sums_worse);

    float cumulative_reach_probabilities_of_worse = 0.0f;
    float cumulative_reach_probabilities_of_equal = 0.0f;
    float utility = terminal_node.pot * 0.5f;
    
    for (int strength : sorted_strengths) {
        cumulative_reach_probabilities_of_worse += cumulative_reach_probabilities_of_equal;
        cumulative_reach_probabilities_of_equal = 0.0f;

        fast_initialize_array(row_reach_sums_equal);
        fast_initialize_array(col_reach_sums_equal);

        for (const auto& [c1, c2] : strength_map[strength]) {
            float reach = reach_probabilities[c1 - 1][c2 - 1];
            cumulative_reach_probabilities_of_equal += reach;
            row_reach_sums_equal[c1 - 1] += reach;
            col_reach_sums_equal[c2 - 1] += reach;
        }

        for (const auto& [c1, c2] : strength_map[strength]) {
            float reach = reach_probabilities[c1 - 1][c2 - 1];

            float cumulative_reach_probabilities_of_hands_no_overlaps = total_reach - row_reach_sums[c1 - 1] 
                                                                                    - col_reach_sums[c2 - 1]
                                                                                    + reach;
            
            float cumulative_reach_probabilities_of_worse_no_overlaps = cumulative_reach_probabilities_of_worse - row_reach_sums_worse[c1 - 1]
                                                                                                                - col_reach_sums_worse[c2 - 1];

            float cumulative_reach_probabilities_of_equal_no_overlaps = cumulative_reach_probabilities_of_equal - row_reach_sums_equal[c1 - 1]
                                                                                                                - col_reach_sums_equal[c2 - 1]
                                                                                                                + reach;

            float cumulative_reach_probabilities_of_bettr_no_overlaps = cumulative_reach_probabilities_of_hands_no_overlaps - cumulative_reach_probabilities_of_equal_no_overlaps
                                                                                                                            - cumulative_reach_probabilities_of_worse_no_overlaps;

            (*expected_utilities)[c1 - 1][c2 - 1] = utility * exploitative_player_range[c1 - 1][c2 - 1] * (cumulative_reach_probabilities_of_worse_no_overlaps - cumulative_reach_probabilities_of_bettr_no_overlaps);
        }

        for (const auto& [c1, c2] : strength_map[strength]) {
            float reach = reach_probabilities[c1 - 1][c2 - 1];
            row_reach_sums_worse[c1 - 1] += reach;
            col_reach_sums_worse[c2 - 1] += reach;
        }
    }
    return expected_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_reach_probabilities_using_average_strategy(
    const Tree::Node* node, 
    int action, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {

    auto res = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();

    if (!node->is_decision_node()) {
        std::cout << "Error: Non-decision node passed to update_reach function.\n";
        return res;
    }

    const auto& decision_node = node->as_decision_node();

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                float action_probability = decision_node.get_average_strategy(c1, c2)[action];
                (*res)[c1 - 1][c2 - 1] = action_probability * reach_probabilities[c1 - 1][c2 - 1];
            }            
        }
    }
    return res;
}