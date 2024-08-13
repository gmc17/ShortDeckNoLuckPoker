#include "tree_cfr.h"

Tree tree_cfr_plus_parallel(int iterations, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                            const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                            std::array<uint8_t, 5> board_cards, float pot_size) {

    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    GameState state;
    state.pfp_history = 0b111001;
    state.flp_history = 0b1001;
    state.pot_size = pot_size;
    state.flp_seen = true;
    state.trn_seen = true;
    state.bets.push(0.0f);

    state.fp1 = board_cards[0];
    state.fp2 = board_cards[1];
    state.fp3 = board_cards[2];
    state.trn = board_cards[3];
    state.rvr = board_cards[4];

    generate_rank_table(state);
    Tree tree = Tree(state);

    for (int t=1; t<=iterations; t++) {

        float delay = 0.0f;
        float weight = std::max(t - delay, 0.0f);
        const auto ones = create_ones_array();

        int num_threads = get_cpu_cores();
        ThreadPool pool(num_threads);

        GameState temp = state;
        tree_cfr_plus_traverse_tree_fast_parallel(temp, tree.get_root(), 0, weight, ones, ip_range, op_range, pool);
        temp = state;
        tree_cfr_plus_traverse_tree_fast_parallel(temp, tree.get_root(), 1, weight, ones, op_range, ip_range, pool);

        if (t%5==0) std::cout << "Iteration " << t << " complete.\n";
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << iterations << " iterations of Tree CFR+: "
              << total_time << " seconds. " << iterations/total_time << " iterations/second." << "\n";

    return tree;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> tree_cfr_plus_traverse_tree_fast_parallel(
    GameState& state, 
    Tree::Node* node,
    bool traversing_player, 
    float weight,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range,
    ThreadPool& pool) {

    // Terminal case remains the same
    if (state.is_terminal) return expected_utility_fast_cfr_plus(state, traversing_player, reach_probabilities, inactive_player_range, traversing_player_range);

    auto info_set_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*info_set_utilities)[r].fill(0.0f);

    if (state.is_chance()) {
        auto& chance_node = node->as_chance_node();
        int num = state.num_in_deck();
        std::vector<std::future<std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>>> futures;
        
        for (int c = 1; c <= NUM_CARDS; c++) {
            if (state.has_card(c)) continue;
            futures.push_back(pool.enqueue([&, c]() {
                GameState thread_state = state;
                thread_state.deal_card(c);
                auto new_reach_probabilities = update_chance_reach_probabilities_cfr_plus(thread_state, num, reach_probabilities);
                return tree_cfr_plus_traverse_tree_fast_parallel(thread_state, chance_node.children[c - 1].get(), traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
            }));
        }

        // Collect and combine results
        for (auto& future : futures) {
            auto result = future.get();
            add_2d_arrays_simd(*info_set_utilities, *result);
        }

        return info_set_utilities;
    }

    auto& decision_node = node->as_decision_node();
    int actions = state.num_actions();
    auto action_vals = std::make_unique<std::array<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, 7>>();

    if (state.player == traversing_player) {
        for (int a=0; a<actions; a++) {
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = tree_cfr_plus_traverse_tree_fast_parallel(state, decision_node.children[a].get(), traversing_player, weight, reach_probabilities, inactive_player_range, traversing_player_range, pool);
            state.undo(player_temp, pot_size_temp);

            (*action_vals)[a] = *temp;
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {

                    std::array<float, 7> strategy = decision_node.get_strategy(c1, c2);

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
            auto new_reach_probabilities = tree_update_reach_probabilities_cfr_plus(state, node, a, reach_probabilities);

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = tree_cfr_plus_traverse_tree_fast_parallel(state, decision_node.children[a].get(), traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
            state.undo(player_temp, pot_size_temp);

            add_2d_arrays_simd(*info_set_utilities, *temp);
        }

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    decision_node.update_strategy_sum(c1, c2, decision_node.get_strategy(c1, c2), weight);
                }
            }
        }
    }

    return info_set_utilities;
}

float tree_calculate_exploitability_fast(const Tree& tree,
                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_reach_probabilities, 
                                         const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_reach_probabilities,
                                         const std::array<uint8_t, 5> board_cards, 
                                         float pot) {

    std::array<float, 2> total_exploitability = {0.0f};

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

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_info_set_utilities = tree_br_traverse_tree_fast(state, tree.get_root(), 0, ip_reach_probabilities);
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_info_set_utilities = tree_br_traverse_tree_fast(state, tree.get_root(), 1, op_reach_probabilities);

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

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> tree_br_traverse_tree_fast(GameState& state, 
                                                                               Tree::Node* node,
                                                                               bool exploitative_player, 
                                                                               const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& strategy_reach_probabilities) {
    
    if (state.is_terminal) return expected_utility_fast(state, exploitative_player, strategy_reach_probabilities);

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> info_set_utilities;
    for (int r=0; r<NUM_CARDS; r++) info_set_utilities[r].fill(0.0f);

    if (state.is_chance()) {
        auto& chance_node = node->as_chance_node();
        int num = state.num_in_deck();

        for (int c=1; c<=NUM_CARDS; c++) {
            if (state.has_card(c)) continue;
            
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.deal_card(c);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> new_strategy_reach_probabilities = update_chance_reach_probabilities(state, num, strategy_reach_probabilities);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = tree_br_traverse_tree_fast(state, chance_node.children[c - 1].get(), exploitative_player, new_strategy_reach_probabilities);
            state.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (state.has_card(c1) || state.has_card(c2) || (c1==c) || (c2==c)) continue;
                    else info_set_utilities[c1 - 1][c2 - 1] += temp[c1 - 1][c2 - 1];
                }
            }
        }
        return info_set_utilities;
    }

    auto& decision_node = node->as_decision_node();
    int actions = state.num_actions();

    if (state.player != exploitative_player) {
        for (int a=0; a<actions; a++) {
            auto new_strategy_reach_probabilities = tree_update_reach_probabilities(state, node, a, strategy_reach_probabilities);
            
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = tree_br_traverse_tree_fast(state, decision_node.children[a].get(), exploitative_player, new_strategy_reach_probabilities);
            state.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (state.has_card(c1) || state.has_card(c2)) info_set_utilities[c1 - 1][c2 - 1] = std::numeric_limits<float>::lowest();
                    else info_set_utilities[c1 - 1][c2 - 1] += temp[c1 - 1][c2 - 1];
                }
            }
        }
        return info_set_utilities;
    }

    for (int r=0; r<NUM_CARDS; r++) info_set_utilities[r].fill(std::numeric_limits<float>::lowest());
    
    for (int a=0; a<actions; a++) {
        bool player_temp = state.player; float pot_size_temp = state.pot_size;
        state.apply_index(a);
        std::array<std::array<float, NUM_CARDS>, NUM_CARDS> temp = tree_br_traverse_tree_fast(state, decision_node.children[a].get(), exploitative_player, strategy_reach_probabilities);
        state.undo(player_temp, pot_size_temp);

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (state.has_card(c1) || state.has_card(c2)) info_set_utilities[c1 - 1][c2 - 1] = std::numeric_limits<float>::lowest();
                else info_set_utilities[c1 - 1][c2 - 1] = std::max(info_set_utilities[c1 - 1][c2 - 1], temp[c1 - 1][c2 - 1]);
            }
        }
    }
    
    return info_set_utilities;
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> tree_update_reach_probabilities(const GameState& state,
                                                                                    const Tree::Node* node, 
                                                                                    int action, 
                                                                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> res;
    for (int r=0; r<NUM_CARDS; r++) res[r].fill(0.0f);

    if (!node->is_decision_node()) {
        // Handle error or return, as this should be a decision node
        std::cout << "Error: Non-decision node passed to update_reach function.\n";
        return res;
    }

    const auto& decision_node = node->as_decision_node();

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(state.has_card(c1) || state.has_card(c2))) {

                float action_probability = decision_node.get_strategy(c1, c2)[action];
                
                res[c1 - 1][c2 - 1] = (action_probability) * reach_probabilities[c1 - 1][c2 - 1];
            }            
        }
    }
    return res;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> tree_update_reach_probabilities_cfr_plus(const GameState& state,
                                                                                                              const Tree::Node* node, 
                                                                                                              int action, 
                                                                                                              const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    auto res = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*res)[r].fill(0.0f);

    if (!node->is_decision_node()) {
        // Handle error or return, as this should be a decision node
        std::cout << "Error: Non-decision node passed to update_reach function.\n";
        return res;
    }

    const auto& decision_node = node->as_decision_node();

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(state.has_card(c1) || state.has_card(c2))) {

                float action_probability = decision_node.get_strategy(c1, c2)[action];
                
                (*res)[c1 - 1][c2 - 1] = (action_probability) * reach_probabilities[c1 - 1][c2 - 1];
            }            
        }
    }
    return res;
}