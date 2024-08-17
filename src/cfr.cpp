#include "cfr.h"

void cfr_plus(
    Tree& tree,
    CFRParameters parameters,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& op_range, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& ip_range) {
    
    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    const auto ones = create_ones_array();
    int num_threads = get_cpu_cores();
    ThreadPool pool(num_threads);

    int t = 1;
    float current_exploitability = 10;
    while (t <= parameters.max_iterations && 
           parameters.exploitability_goal < current_exploitability) {

        float delay = 0.0f;
        float weight = std::max(t - delay, 0.0f);
        
        cfr_plus_traverse_tree(tree.get_root(), 0, 0, weight, ones, ip_range, op_range, pool);
        cfr_plus_traverse_tree(tree.get_root(), 1, 0, weight, ones, op_range, ip_range, pool);

        if (t % parameters.log_interval==0) {
            std::cout << "Iteration " << t << " complete.\n";
            current_exploitability = calculate_exploitability(tree, op_range, ip_range);
            std::cout << "\n";
        }

        t++;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << t << " iterations of CFR+: "
              << total_time << " seconds. " << t/total_time << " iterations/second." << "\n";
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> cfr_plus_traverse_tree( 
    Tree::Node* node,
    bool traversing_player, 
    bool chance_layer_seen,
    float weight,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range,
    ThreadPool& pool) {

    if (!node)
        return std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();

    if (node->is_terminal_node()) 
        return terminal_node_utility(node, traversing_player, reach_probabilities, inactive_player_range, traversing_player_range);

    auto info_set_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();

    if (node->is_chance_node()) {
        auto& chance_node = node->as_chance_node();
        int num = chance_node.num;
        std::vector<std::future<std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>>> futures;
        
        for (int c=1; c<=NUM_CARDS; c++) {
            if (!chance_node.has_card(c)) {
                if (!chance_layer_seen) {
                    futures.push_back(pool.enqueue([&, c, num]() {
                        auto new_reach_probabilities = update_chance_reach_probabilities(chance_node.children[c - 1].get(), num, reach_probabilities);
                        auto result = cfr_plus_traverse_tree(chance_node.children[c - 1].get(), traversing_player, 1, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
                        for (int c1=1; c1<=NUM_CARDS; c1++) {
                            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                                if (chance_node.has_card(c1) || chance_node.has_card(c2) || (c1==c) || (c2==c)) {
                                    (*result)[c1 - 1][c2 - 1] = 0.0f;
                                }
                            }
                        }
                        return result;
                    }));
                } else {
                    auto new_reach_probabilities = update_chance_reach_probabilities(chance_node.children[c - 1].get(), num, reach_probabilities);
                    auto result = cfr_plus_traverse_tree(chance_node.children[c - 1].get(), traversing_player, 1, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
                    for (int c1=1; c1<=NUM_CARDS; c1++) {
                        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                            if (chance_node.has_card(c1) || chance_node.has_card(c2) || (c1==c) || (c2==c)) {
                                (*result)[c1 - 1][c2 - 1] = 0.0f;
                            }
                        }
                    }
                    add_2d_arrays_simd(*info_set_utilities, *result);
                }
            }
        }

        // combine thread results
        for (auto& future : futures) {
            auto result = future.get();
            add_2d_arrays_simd(*info_set_utilities, *result);
        }

        return info_set_utilities;
    }

    auto& decision_node = node->as_decision_node();
    int actions = decision_node.actions;
    
    if (decision_node.player == traversing_player) {
        auto action_vals = std::make_unique<std::array<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, 7>>();

        for (int a=0; a<actions; a++) {
            auto temp = cfr_plus_traverse_tree(decision_node.children[a].get(), traversing_player, chance_layer_seen, weight, reach_probabilities, inactive_player_range, traversing_player_range, pool);
            (*action_vals)[a] = *temp;
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                    auto strategy = decision_node.get_strategy(c1, c2);
                    for (int a=0; a<actions; a++) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += strategy[a] * (*action_vals)[a][c1 - 1][c2 - 1];
                    }
                }
            }
        }
        for (int a=0; a<actions; a++) {
            decision_node.accumulate_regret(a, (*action_vals)[a], *info_set_utilities);
        }

    } else {
        for (int a=0; a<actions; a++) {
            auto new_reach_probabilities = update_reach_probabilities(node, a, reach_probabilities);
            auto temp = cfr_plus_traverse_tree(decision_node.children[a].get(), traversing_player, chance_layer_seen, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
            add_2d_arrays_simd(*info_set_utilities, *temp);
        }
        if (weight > 0) {
            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                        auto strategy = decision_node.get_strategy(c1, c2);
                        decision_node.update_strategy_sum(c1, c2, strategy, weight);
                    }
                }
            }
        }
    }

    return info_set_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> terminal_node_utility(
    const Tree::Node* node, 
    bool traversing_player, 
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range) {

    auto expected_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();

    multiply_2d_arrays_simd(reach_probabilities, inactive_player_range);

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
                    (*expected_utilities)[c1 - 1][c2 - 1] = fold_utility * r * traversing_player_range[c1 - 1][c2 - 1];
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

            (*expected_utilities)[c1 - 1][c2 - 1] = utility * traversing_player_range[c1 - 1][c2 - 1] * (cumulative_reach_probabilities_of_worse_no_overlaps - cumulative_reach_probabilities_of_bettr_no_overlaps);
        }

        for (const auto& [c1, c2] : strength_map[strength]) {
            float reach = reach_probabilities[c1 - 1][c2 - 1];
            row_reach_sums_worse[c1 - 1] += reach;
            col_reach_sums_worse[c2 - 1] += reach;
        }
    }
    return expected_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_reach_probabilities(
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
                float action_probability = decision_node.get_strategy(c1, c2)[action];
                (*res)[c1 - 1][c2 - 1] = action_probability * reach_probabilities[c1 - 1][c2 - 1];
            }            
        }
    }

    return res;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_chance_reach_probabilities(
    const Tree::Node* node, 
    int num, 
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {

    auto res = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();

    if (node->is_decision_node()) {
        const auto& decision_node = node->as_decision_node();
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(decision_node.has_card(c1) || decision_node.has_card(c2))) {
                    (*res)[c1 - 1][c2 - 1] = (1.0f / num) * reach_probabilities[c1 - 1][c2 - 1];
                }
            }
        }
    } else if (node->is_chance_node()) {
        const auto& chance_node = node->as_chance_node();
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(chance_node.has_card(c1) || chance_node.has_card(c2))) {
                    (*res)[c1 - 1][c2 - 1] = (1.0f / num) * reach_probabilities[c1 - 1][c2 - 1];
                }
            }
        }
    } else if (node->is_terminal_node()) {
        const auto& terminal_node = node->as_terminal_node();
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(terminal_node.has_card(c1) || terminal_node.has_card(c2))) {
                    (*res)[c1 - 1][c2 - 1] = (1.0f / num) * reach_probabilities[c1 - 1][c2 - 1];
                }
            }
        }
    }
    
    return res;
}