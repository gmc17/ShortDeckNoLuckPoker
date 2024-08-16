#include "helpers.h"

// void add_2d_arrays_simd(
//     std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
//     const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b) {
//     constexpr int SIMD_WIDTH = 4; // Number of floats in __m128
//     constexpr int TOTAL_ELEMENTS = NUM_CARDS * NUM_CARDS;
//     constexpr int SIMD_ITERATIONS = TOTAL_ELEMENTS / SIMD_WIDTH;
//     float* res_ptr = reinterpret_cast<float*>(res.data());
//     const float* b_ptr = reinterpret_cast<const float*>(b.data());

//     for (int i = 0; i < SIMD_ITERATIONS; ++i) {
//         __m128 vr = _mm_loadu_ps(res_ptr + i * SIMD_WIDTH);
//         __m128 vb = _mm_loadu_ps(b_ptr + i * SIMD_WIDTH);
//         __m128 vs = _mm_add_ps(vr, vb);
//         _mm_storeu_ps(res_ptr + i * SIMD_WIDTH, vs);
//     }

//     // Handle remaining elements if TOTAL_ELEMENTS is not divisible by SIMD_WIDTH
//     for (int i = SIMD_ITERATIONS * SIMD_WIDTH; i < TOTAL_ELEMENTS; ++i) {
//         res_ptr[i] = res_ptr[i] + b_ptr[i];
//     }
// }

void add_2d_arrays_simd(
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b) {
    constexpr int SIMD_WIDTH = 8; // Number of floats in __m256
    constexpr int TOTAL_ELEMENTS = NUM_CARDS * NUM_CARDS;
    constexpr int SIMD_ITERATIONS = TOTAL_ELEMENTS / SIMD_WIDTH;
    float* res_ptr = reinterpret_cast<float*>(res.data());
    const float* b_ptr = reinterpret_cast<const float*>(b.data());
    for (int i = 0; i < SIMD_ITERATIONS; ++i) {
        __m256 vr = _mm256_loadu_ps(res_ptr + i * SIMD_WIDTH);
        __m256 vb = _mm256_loadu_ps(b_ptr + i * SIMD_WIDTH);
        __m256 vs = _mm256_add_ps(vr, vb);
        _mm256_storeu_ps(res_ptr + i * SIMD_WIDTH, vs);
    }
    // Handle remaining elements if TOTAL_ELEMENTS is not divisible by SIMD_WIDTH
    for (int i = SIMD_ITERATIONS * SIMD_WIDTH; i < TOTAL_ELEMENTS; ++i) {
        res_ptr[i] = res_ptr[i] + b_ptr[i];
    }
}

void multiply_2d_arrays_simd(
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b) {
    constexpr int SIMD_WIDTH = 8; // Number of floats in __m256
    constexpr int TOTAL_ELEMENTS = NUM_CARDS * NUM_CARDS;
    constexpr int SIMD_ITERATIONS = TOTAL_ELEMENTS / SIMD_WIDTH;
    float* res_ptr = reinterpret_cast<float*>(res.data());
    const float* b_ptr = reinterpret_cast<const float*>(b.data());
    for (int i = 0; i < SIMD_ITERATIONS; ++i) {
        __m256 vr = _mm256_loadu_ps(res_ptr + i * SIMD_WIDTH);
        __m256 vb = _mm256_loadu_ps(b_ptr + i * SIMD_WIDTH);
        __m256 vm = _mm256_mul_ps(vr, vb);  // Changed from add_ps to mul_ps
        _mm256_storeu_ps(res_ptr + i * SIMD_WIDTH, vm);
    }
    // Handle remaining elements if TOTAL_ELEMENTS is not divisible by SIMD_WIDTH
    for (int i = SIMD_ITERATIONS * SIMD_WIDTH; i < TOTAL_ELEMENTS; ++i) {
        res_ptr[i] = res_ptr[i] * b_ptr[i];
    }
}

// void multiply_2d_arrays_simd(
//     std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
//     const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& b) {
//     constexpr int SIMD_WIDTH = 4; // Number of floats in __m128
//     constexpr int TOTAL_ELEMENTS = NUM_CARDS * NUM_CARDS;
//     constexpr int SIMD_ITERATIONS = TOTAL_ELEMENTS / SIMD_WIDTH;
//     float* res_ptr = reinterpret_cast<float*>(res.data());
//     const float* b_ptr = reinterpret_cast<const float*>(b.data());

//     for (int i = 0; i < SIMD_ITERATIONS; ++i) {
//         __m128 vr = _mm_loadu_ps(res_ptr + i * SIMD_WIDTH);
//         __m128 vb = _mm_loadu_ps(b_ptr + i * SIMD_WIDTH);
//         __m128 vm = _mm_mul_ps(vr, vb);
//         _mm_storeu_ps(res_ptr + i * SIMD_WIDTH, vm);
//     }

//     // Handle remaining elements if TOTAL_ELEMENTS is not divisible by SIMD_WIDTH
//     for (int i = SIMD_ITERATIONS * SIMD_WIDTH; i < TOTAL_ELEMENTS; ++i) {
//         res_ptr[i] = res_ptr[i] * b_ptr[i];
//     }
// }

void fast_initialize_array(std::array<float, NUM_CARDS>& array) {
    constexpr int SIMD_WIDTH = 8; // Number of floats in __m256
    constexpr int SIMD_ITERATIONS = NUM_CARDS / SIMD_WIDTH;
    
    __m256 zero_vector = _mm256_setzero_ps();

    for (int i = 0; i < SIMD_ITERATIONS; ++i) {
        _mm256_storeu_ps(&array[i * SIMD_WIDTH], zero_vector);
    }

    // Handle any remaining elements
    for (size_t i = SIMD_ITERATIONS * SIMD_WIDTH; i < NUM_CARDS; ++i) {
        array[i] = 0.0f;
    }
}

void generate_rank_table(GameState state) {
    if (state.trn==0) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) {
                    for (int c4=c3+1; c4<=NUM_CARDS; c4++) {
                        GameState temp = state;
                        if (!(temp.has_card(c1) || temp.has_card(c2) || temp.has_card(c3) || temp.has_card(c4))) {
                            temp.op1 = c1;
                            temp.op2 = c2;
                            temp.trn = c3;
                            temp.rvr = c4;
                            int hand_rank = temp.best_hand(0);

                            rank_table[c1 - 1][c2 - 1][c3 - 1][c4 - 1] = hand_rank;
                            rank_table[c1 - 1][c2 - 1][c4 - 1][c3 - 1] = hand_rank;
                            rank_table[c1 - 1][c3 - 1][c2 - 1][c4 - 1] = hand_rank;
                            rank_table[c1 - 1][c3 - 1][c4 - 1][c2 - 1] = hand_rank;
                            rank_table[c1 - 1][c4 - 1][c2 - 1][c3 - 1] = hand_rank;
                            rank_table[c1 - 1][c4 - 1][c3 - 1][c2 - 1] = hand_rank;

                            rank_table[c2 - 1][c1 - 1][c3 - 1][c4 - 1] = hand_rank;
                            rank_table[c2 - 1][c1 - 1][c4 - 1][c3 - 1] = hand_rank;
                            rank_table[c2 - 1][c3 - 1][c1 - 1][c4 - 1] = hand_rank;
                            rank_table[c2 - 1][c3 - 1][c4 - 1][c1 - 1] = hand_rank;
                            rank_table[c2 - 1][c4 - 1][c1 - 1][c3 - 1] = hand_rank;
                            rank_table[c2 - 1][c4 - 1][c3 - 1][c1 - 1] = hand_rank;

                            rank_table[c3 - 1][c1 - 1][c2 - 1][c4 - 1] = hand_rank;
                            rank_table[c3 - 1][c1 - 1][c4 - 1][c2 - 1] = hand_rank;
                            rank_table[c3 - 1][c2 - 1][c1 - 1][c4 - 1] = hand_rank;
                            rank_table[c3 - 1][c2 - 1][c4 - 1][c1 - 1] = hand_rank;
                            rank_table[c3 - 1][c4 - 1][c1 - 1][c2 - 1] = hand_rank;
                            rank_table[c3 - 1][c4 - 1][c2 - 1][c1 - 1] = hand_rank;

                            rank_table[c4 - 1][c1 - 1][c2 - 1][c3 - 1] = hand_rank;
                            rank_table[c4 - 1][c1 - 1][c3 - 1][c2 - 1] = hand_rank;
                            rank_table[c4 - 1][c2 - 1][c1 - 1][c3 - 1] = hand_rank;
                            rank_table[c4 - 1][c2 - 1][c3 - 1][c1 - 1] = hand_rank;
                            rank_table[c4 - 1][c3 - 1][c1 - 1][c2 - 1] = hand_rank;
                            rank_table[c4 - 1][c3 - 1][c2 - 1][c1 - 1] = hand_rank;
                        }
                    }
                }
            }
        }
    } else if (state.rvr==0) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) {
                    GameState temp = state;
                    if (!(temp.has_card(c1) || temp.has_card(c2) || temp.has_card(c3))) {
                        temp.op1 = c1;
                        temp.op2 = c2;
                        temp.rvr = c3;
                        int hand_rank = temp.best_hand(0);

                        rank_table[c1 - 1][c2 - 1][temp.trn - 1][c3 - 1] = hand_rank;
                        rank_table[c1 - 1][c3 - 1][temp.trn - 1][c2 - 1] = hand_rank;
                        rank_table[c2 - 1][c1 - 1][temp.trn - 1][c3 - 1] = hand_rank;
                        rank_table[c2 - 1][c3 - 1][temp.trn - 1][c1 - 1] = hand_rank;
                        rank_table[c3 - 1][c1 - 1][temp.trn - 1][c2 - 1] = hand_rank;
                        rank_table[c3 - 1][c2 - 1][temp.trn - 1][c1 - 1] = hand_rank;
                    }
                }
            }
        }
    } else {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                GameState temp = state;
                if (!(temp.has_card(c1) || temp.has_card(c2))) {
                    temp.op1 = c1;
                    temp.op2 = c2;
                    int hand_rank = state.best_hand(0);

                    rank_table[c1 - 1][c2 - 1][temp.trn - 1][temp.rvr - 1] = hand_rank;
                    rank_table[c2 - 1][c1 - 1][temp.trn - 1][temp.rvr - 1] = hand_rank;
                }
            }
        }
    }
}

void test_rank_table(GameState state) {

    int samples = 50000;
    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    generate_rank_table(state);

    for (int i=0; i<samples; i++) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) { 
                    state.op1 = 0; state.op2 = 0; state.rvr = 0;
                    if (state.has_card(c1) || state.has_card(c2) || state.has_card(c3)) continue;                    
                    state.op1 = c1;
                    state.op2 = c2;
                    state.rvr = c3;
                    state.flp_seen = true;
                    state.trn_seen = true;
                    state.rvr_seen = true;
                    state.pfp_history = 0b111001;
                    state.flp_history = 0b1001;
                    state.best_hand_fast();
                }
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << samples << " samples of best_hand_fast: "
          << total_time << " seconds. " << 4960 * samples/total_time << " hands/second." << "\n";

    total_time = 0.0f;
    start = std::chrono::high_resolution_clock::now();

    for (int i=0; i<samples; i++) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) { 
                    state.op1 = 0; state.op2 = 0; state.rvr = 0;
                    if (state.has_card(c1) || state.has_card(c2) || state.has_card(c3)) continue;                    
                    state.op1 = c1;
                    state.op2 = c2;
                    state.rvr = c3;
                    state.flp_seen = true;
                    state.trn_seen = true;
                    state.rvr_seen = true;
                    state.pfp_history = 0b111001;
                    state.flp_history = 0b1001;
                    state.best_hand(0);
                }
            }
        }
    }

    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    total_time += elapsed.count();

    std::cout << samples << " samples of best_hand: "
          << total_time << " seconds. " << 4960 * samples/total_time << " hands/second." << "\n";
}

void generate_terminal_node_evaluation_tables(GameState state) {
    strength_map_table.resize(NUM_CARDS, std::vector<std::unordered_map<int, std::vector<std::tuple<int, int>>>>(NUM_CARDS));
    sorted_strengths_table.resize(NUM_CARDS, std::vector<std::vector<int>>(NUM_CARDS));

    if (state.trn==0) {
        for (int trn=1; trn<=NUM_CARDS; trn++) {
            for (int rvr=trn+1; rvr<=NUM_CARDS; rvr++) {
                GameState temp = state;
                if (!(temp.has_card(trn) || temp.has_card(rvr))) {
                    temp.trn = trn; temp.rvr = rvr;
                   
                    std::unordered_map<int, std::vector<std::tuple<int, int>>> strength_map;

                    for (int c1=1; c1<=NUM_CARDS; c1++) {
                        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                            temp.op1 = 0; temp.op2 = 0;
                            if (!(temp.has_card(c1) || state.has_card(c2))) {
                                temp.op1 = c1; temp.op2 = c2;
                                int strength = temp.best_hand(0);

                                strength_map[strength].emplace_back(c1, c2);
                            }
                        }
                    }

                    std::vector<int> sorted_strengths;
                    sorted_strengths.reserve(strength_map.size());

                    for (const auto& [strength, _] : strength_map) {
                        sorted_strengths.push_back(strength);
                    }

                    std::sort(sorted_strengths.begin(), sorted_strengths.end());

                    strength_map_table[trn - 1][rvr - 1] = strength_map;
                    strength_map_table[rvr - 1][trn - 1] = strength_map;
                    sorted_strengths_table[trn - 1][rvr - 1] = sorted_strengths;
                    sorted_strengths_table[rvr - 1][trn - 1] = sorted_strengths;
                }
            }
        }
    } else if (state.rvr==0) {
        for (int rvr=1; rvr<=NUM_CARDS; rvr++) {
            GameState temp = state;
            if (!temp.has_card(rvr)) {
                temp.rvr = rvr;
                std::unordered_map<int, std::vector<std::tuple<int, int>>> strength_map;

                for (int c1=1; c1<=NUM_CARDS; c1++) {
                    for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                        temp.op1 = 0; temp.op2 = 0;
                        if (!(temp.has_card(c1) || temp.has_card(c2))) {
                            temp.op1 = c1; temp.op2 = c2;

                            int strength = temp.best_hand(0);

                            strength_map[strength].emplace_back(c1, c2);
                        }
                    }
                }

                std::vector<int> sorted_strengths;
                // sorted_strengths.reserve(strength_map.size());
                
                for (const auto& [strength, _] : strength_map) {
                    sorted_strengths.push_back(strength);
                }

                std::sort(sorted_strengths.begin(), sorted_strengths.end());

                strength_map_table[temp.trn - 1][rvr - 1] = strength_map;
                sorted_strengths_table[temp.trn - 1][rvr - 1] = sorted_strengths;
            }
        }
    } else {
        std::unordered_map<int, std::vector<std::tuple<int, int>>> strength_map;

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                state.op1 = 0; state.op2 = 0;
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    state.op1 = c1; state.op2 = c2;
                    int strength = state.best_hand(0);
                    strength_map[strength].emplace_back(c1, c2);
                }
            }
        }

        std::vector<int> sorted_strengths;
        sorted_strengths.reserve(strength_map.size());
        
        for (const auto& [strength, _] : strength_map) {
            sorted_strengths.push_back(strength);
        }

        std::sort(sorted_strengths.begin(), sorted_strengths.end());

        strength_map_table[state.trn - 1][state.rvr - 1] = strength_map;
        sorted_strengths_table[state.trn - 1][state.rvr - 1] = sorted_strengths;
    }
}

void print_reach_probabilities(const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {

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



void print_range(
    const Tree& tree,
    const std::vector<int>& actions) {

    Tree::Node* node = tree.get_node(actions);

    if (!node->is_decision_node()) {
        std::cout << "Error: Non-decision node passed to print_range function.\n";
        return;
    }

    auto& decision_node = node->as_decision_node();

    std::cout << "\n******************* Board ********************\n";

    for (int c=1; c<=NUM_CARDS; c++) {
        if (decision_node.card_marker[c - 1]) {
            std::cout << CARD_NAMES[rank(c)] << SUIT_NAMES[suit(c)];
        }
    }

    std::string player_name = (decision_node.player==0) ? "OOP" : "IP";
    std::cout << "\n\n***************** " << player_name << " Range: ******************\n";

    for (int a=0; a<decision_node.actions; a++) {
        std::array<std::array<float, 9>, 9> range;
        std::array<std::array<float, 9>, 9> count;

        for (int r=0; r<9; r++) {
            for (int c=0; c<9; c++) {
                range[r][c] = count[r][c] = 0.0f;
            }
        }

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!((decision_node.has_card(c1)) || (decision_node.has_card(c2)))) {

                    std::array<float, 7> strategy = decision_node.get_average_strategy(c1, c2);
                    std::array<int, 2> row_col = pocket_id_to_row_col(pocket_id(c1, c2));
                    
                    range[row_col[0]][row_col[1]] += strategy[a];
                    count[row_col[0]][row_col[1]] += 1.0f;
                }
            }
        }

        std::cout << "Action: " << a << "\n";

        for (int r=0; r<10; r++) {
            for (int c=0; c<10; c++) {
                if (c==0 && r==0) {
                    std::cout << "    ";
                } else if (r==0 && c>0) {
                    std::cout << CARD_NAMES[9-c] << "s   ";
                } else if (c==0 && r>0) {
                    std::cout << CARD_NAMES[9-r] << " ";
                } else {
                    std::cout << FIXED_FLOAT(range[r-1][c-1]/count[r-1][c-1]) << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }  
}