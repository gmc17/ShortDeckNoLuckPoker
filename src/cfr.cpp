#include "cfr.h"

std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> regret_sum;
std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE> strategy_sum;

void save_cfr_data(const std::string& filename,
                        const std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& regret_sum,
                        const std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& strategy_sum) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }

    // Save regret_sum
    file.write(reinterpret_cast<const char*>(regret_sum.data()), 
               sizeof(float) * 7 * STRATEGY_ARRAY_SIZE);

    // Save strategy_sum
    file.write(reinterpret_cast<const char*>(strategy_sum.data()), 
               sizeof(float) * 7 * STRATEGY_ARRAY_SIZE);

    file.close();
}

void load_cfr_data(const std::string& filename,
                        std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& regret_sum,
                        std::array<std::array<float, 7>, STRATEGY_ARRAY_SIZE>& strategy_sum) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }

    // Load regret_sum
    file.read(reinterpret_cast<char*>(regret_sum.data()), 
              sizeof(float) * 7 * STRATEGY_ARRAY_SIZE);

    // Load strategy_sum
    file.read(reinterpret_cast<char*>(strategy_sum.data()), 
              sizeof(float) * 7 * STRATEGY_ARRAY_SIZE);

    file.close();
}

std::array<float, 7> get_strategy(const InfoSet& info_set) {
    size_t hash = info_set.hash();
    int actions = info_set.num_actions;

    std::array<float, 7> regrets = regret_sum[hash];
    std::array<float, 7> strategy = {0.0f};
    float normalizing_sum = 0.0f;

    // get positive regrets
    for (int i=0; i<actions; i++) {
        strategy[i] = std::max(0.0f, regrets[i]);
        normalizing_sum += strategy[i];
    }
    
    if (normalizing_sum > 1e-9f) {
        for (int i=0; i<actions; i++) strategy[i] /= normalizing_sum;
    } else {
        float uniform_prob = 1.0f / actions;
        for (int i=0; i<actions; i++) strategy[i] = uniform_prob;
    }

    return strategy;
}

std::array<float, 7> get_average_strategy(const InfoSet& info_set) {
    size_t hash = info_set.hash();
    int actions = info_set.num_actions;

    std::array<float, 7> avg_strategy = {0.0f};
    float normalizing_sum = 0.0f;

    for (int i=0; i<actions; i++) {
        normalizing_sum += strategy_sum[hash][i];
    }
    
    if (normalizing_sum > 1e-9f) {
        for (int i=0; i<actions; i++) {
            avg_strategy[i] = strategy_sum[hash][i] / normalizing_sum;
        }
    } else {
        float uniform_prob = 1.0f / actions;
        for (int i=0; i<actions; i++) avg_strategy[i] = uniform_prob;
    }

    return avg_strategy;
}

int sample_action(const std::array<float, 7>& strategy) {

    float random_value = get_uniform_distribution()(get_random_generator());
    float cumulative_sum = 0.0f;

    for (int i=0; i<7; ++i) {
        cumulative_sum += strategy[i];
        if (random_value <= cumulative_sum) {
            return i;
        }
    }

    return 0;
}

void as_mccfr(int iterations, std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                   std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                   std::array<uint8_t, 5> board_cards,
                                   float pot_size) {

    double total_time = 0.0;
    auto start = std::chrono::high_resolution_clock::now();

    std::array<float, 7> init = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    for (int i=0; i<STRATEGY_ARRAY_SIZE; i++) {
        strategy_sum[i] = init;
        regret_sum[i] = init;
    }

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    const int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunk_size = iterations / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? iterations : start + chunk_size;
        threads.emplace_back(as_mccfr_worker, start, end, op_range, ip_range, board_cards, pot_size);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    save_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << iterations << " iterations of Parallel AS-MCCFR: "
              << total_time << " seconds. " << iterations/total_time << " iterations/second." << "\n";
}

void as_mccfr_worker(int start, int end, std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                              std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                              std::array<uint8_t, 5> board_cards,
                                              float pot_size) {
    for (int t=start; t<end; t++) {    
        int iteration = t - start + 1.0f;    
        for (int player=0; player<2; player++) {
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

            as_traverse_tree(state, player, 1.0f);
        }
        if (iteration%1000000==0) std::cout << "iteration " << iteration << " complete.\n";
    }
}

float as_traverse_tree(GameState& gs, bool active_player, float q) {
    // Terminal
    if (gs.is_terminal) return gs.utility(active_player) / q;

    // Chance
    if (gs.is_chance()) {
        uint8_t c = get_card_distribution()(get_random_generator());
        while (gs.has_card(c)) 
            c = get_card_distribution()(get_random_generator());

        bool player_temp = gs.player; float pot_size_temp = gs.pot_size;
        gs.deal_card(c);
        float res = as_traverse_tree(gs, active_player, q);
        gs.undo(player_temp, pot_size_temp);

        return res;
    }

    InfoSet info_set = gs.to_information_set();
    int actions = info_set.num_actions;
    size_t hash = info_set.hash();
    std::array<float, 7> strategy = get_strategy(info_set);

    if (info_set.player != active_player) {
        for (int a=0; a<actions; a++) {
            strategy_sum[hash][a] += strategy[a] / q;
        }

        // Sample inactive player's action and compute utility for it
        int sampled_action = sample_action(strategy);
        
        bool player_temp = gs.player; float pot_size_temp = gs.pot_size;
        gs.apply_index(sampled_action);
        float res = as_traverse_tree(gs, active_player, q);
        gs.undo(player_temp, pot_size_temp);

        return res;
    }

    // Active player's turn

    std::array<float, 7> action_utils = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float node_util = 0;
    float sum_over_all_actions = 0;

    for (int a=0; a<actions; a++) {
        sum_over_all_actions += strategy_sum[hash][a];
    }
    
    for (int a=0; a<actions; a++) {
        float res = (BETA + TAU * strategy_sum[hash][a]) / 
                    (BETA + sum_over_all_actions);
        float p = std::max(EPSILON, res);
        float r = get_uniform_distribution()(get_random_generator());

        if (r < p) {
            GameState child = gs;
            bool player_temp = gs.player; float pot_size_temp = gs.pot_size;
            gs.apply_index(a);
            action_utils[a] = as_traverse_tree(gs, active_player, q * std::min(1.0f, p));
            gs.undo(player_temp, pot_size_temp);
        }

        node_util += strategy[a] * action_utils[a];
    }

    for (int a=0; a<actions; a++) {
        regret_sum[hash][a] = regret_sum[hash][a] + action_utils[a] - node_util;
    }

    return node_util;
}

void add_2d_arrays_simd(std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
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

void multiply_2d_arrays_simd(std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& res,
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

void fast_initialize_2d_array(std::array<float, NUM_CARDS>& array) {
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

void cfr_plus_parallel(int iterations, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range,
                                       const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                       const std::array<uint8_t, 5> board_cards, float pot_size) {

    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    std::array<float, 7> init = {0.0f};
    for (int i=0; i<STRATEGY_ARRAY_SIZE; i++) {
        strategy_sum[i] = init;
        regret_sum[i] = init;
    }

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    cfr_plus_worker_parallel(0, iterations, op_range, ip_range, board_cards, pot_size);

    save_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << iterations << " iterations of CFR+: "
              << total_time << " seconds. " << iterations/total_time << " iterations/second." << "\n";
}

void cfr_plus_worker_parallel(int start, int end, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                                  std::array<uint8_t, 5> board_cards, float pot_size) {

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

    for (int t=start; t<end; t++) {

        float delay = 0.0f;
        float iteration = t - start + 1.0f;
        float weight = std::max(iteration - delay, 0.0f);
        const auto ones = create_ones_array();

        int num_threads = get_cpu_cores();
        ThreadPool pool(num_threads);

        GameState temp = state;
        cfr_plus_traverse_tree_fast_parallel(temp, 0, weight, ones, ip_range, op_range, pool);
        temp = state;
        cfr_plus_traverse_tree_fast_parallel(temp, 1, weight, ones, op_range, ip_range, pool);

        // dcfr_traverse_tree_fast(state, 0, weight, ones, ip_range, op_range);
        // dcfr_traverse_tree_fast(state, 1, weight, ones, op_range, ip_range);
        
        // lcfr_traverse_tree_fast(state, 0, weight, ones, ip_range, op_range);
        // lcfr_traverse_tree_fast(state, 1, weight, ones, op_range, ip_range);

        if ((t-start+1)%1==0) std::cout << "t=" << t-start+1 << "\n";
    }
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> cfr_plus_traverse_tree_fast_parallel(
    GameState& state, bool traversing_player, float weight,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range,
    ThreadPool& pool) {

    // Terminal case remains the same
    if (state.is_terminal) return expected_utility_fast_cfr_plus(state, traversing_player, reach_probabilities, inactive_player_range, traversing_player_range);

    auto info_set_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*info_set_utilities)[r].fill(0.0f);

    if (state.is_chance()) {
        int num = state.num_in_deck();
        std::vector<std::future<std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>>> futures;
        
        for (int c = 1; c <= NUM_CARDS; c++) {
            if (state.has_card(c)) continue;
            futures.push_back(pool.enqueue([&, c]() {
                GameState thread_state = state;
                thread_state.deal_card(c);
                auto new_reach_probabilities = update_chance_reach_probabilities_cfr_plus(thread_state, num, reach_probabilities);
                return cfr_plus_traverse_tree_fast_parallel(thread_state, traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
            }));
        }

        // Collect and combine results
        for (auto& future : futures) {
            auto result = future.get();
            add_2d_arrays_simd(*info_set_utilities, *result);
        }

        return info_set_utilities;
    }

    InfoSet is = state.to_information_set();
    int actions = state.num_actions();

    auto action_vals = std::make_unique<std::array<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, 7>>();

    if (state.player == traversing_player) {
        for (int a=0; a<actions; a++) {
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = cfr_plus_traverse_tree_fast_parallel(state, traversing_player, weight, reach_probabilities, inactive_player_range, traversing_player_range, pool);
            state.undo(player_temp, pot_size_temp);

            (*action_vals)[a] = *temp;
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += strategy[a] * (*action_vals)[a][c1 - 1][c2 - 1];
                    }
                }
            }
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    for (int a=0; a<actions; a++) {
                        is.cr1 = c1; is.cr2 = c2;
                        int hash = is.hash();
                        regret_sum[hash][a] = std::max(regret_sum[hash][a] + (*action_vals)[a][c1 - 1][c2 - 1] - (*info_set_utilities)[c1 - 1][c2 - 1], 0.0f);
                    }
                }
            }
        }
    } else {
        for (int a=0; a<actions; a++) {
            auto new_reach_probabilities = update_reach_probabilities_cfr_plus(state, a, reach_probabilities);

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = cfr_plus_traverse_tree_fast_parallel(state, traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range, pool);
            state.undo(player_temp, pot_size_temp);

            add_2d_arrays_simd(*info_set_utilities, *temp);
        }

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    int hash = is.hash();
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        strategy_sum[hash][a] = strategy_sum[hash][a] + weight * reach_probabilities[c1 - 1][c2 - 1] * strategy[a];
                    }
                }
            }
        }
    }

    return info_set_utilities;
}






void cfr_plus(int iterations, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range,
                                   const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                   const std::array<uint8_t, 5> board_cards, float pot_size) {

    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    std::array<float, 7> init = {0.0f};
    for (int i=0; i<STRATEGY_ARRAY_SIZE; i++) {
        strategy_sum[i] = init;
        regret_sum[i] = init;
    }

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    const int num_threads = 1;
    std::vector<std::thread> threads;
    int chunk_size = iterations / num_threads;

    for (int thread_id = 0; thread_id < num_threads; thread_id++) {
        int start = thread_id * chunk_size;
        int end = (thread_id == num_threads - 1) ? iterations : start + chunk_size;
        threads.emplace_back(cfr_plus_worker, start, end, op_range, ip_range, board_cards, pot_size, thread_id);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    save_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << iterations << " iterations of CFR+: "
              << total_time << " seconds. " << iterations/total_time << " iterations/second." << "\n";
}

void cfr_plus_worker(int start, int end, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> op_range, 
                                              const std::array<std::array<float, NUM_CARDS>, NUM_CARDS> ip_range, 
                                              std::array<uint8_t, 5> board_cards, float pot_size, int thread_id) {
    for (int t=start; t<end; t++) {

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

        float delay = 0.0f;
        float iteration = t - start + 1.0f;
        float weight = std::max(iteration - delay, 0.0f);
        const auto ones = create_ones_array();

        cfr_plus_traverse_tree_fast(state, 0, weight, ones, ip_range, op_range);
        cfr_plus_traverse_tree_fast(state, 1, weight, ones, op_range, ip_range);

        // dcfr_traverse_tree_fast(state, 0, weight, ones, ip_range, op_range);
        // dcfr_traverse_tree_fast(state, 1, weight, ones, op_range, ip_range);
        
        // lcfr_traverse_tree_fast(state, 0, weight, ones, ip_range, op_range);
        // lcfr_traverse_tree_fast(state, 1, weight, ones, op_range, ip_range);

        if ((t-start+1)%1==0) std::cout << "t=" << t-start+1 << "\n";
    }
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> cfr_plus_traverse_tree_fast(GameState& state, bool traversing_player, float weight,
                                                                                                 const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
                                                                                                 const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                                 const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range) {
    // Terminal
    if (state.is_terminal) return expected_utility_fast_cfr_plus(state, traversing_player, reach_probabilities, inactive_player_range, traversing_player_range);

    auto info_set_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*info_set_utilities)[r].fill(0.0f);

    // Chance
    if (state.is_chance()) {
        int num = state.num_in_deck();

        for (int c=1; c<=NUM_CARDS; c++) {
            if (state.has_card(c)) continue;

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.deal_card(c);
            auto new_reach_probabilities = update_chance_reach_probabilities_cfr_plus(state, num, reach_probabilities);
            auto temp = cfr_plus_traverse_tree_fast(state, traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(state.has_card(c1) || state.has_card(c2) || (c1==c) || (c2==c))) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += (*temp)[c1 - 1][c2 - 1];
                    }
                }
            }
        }
        return info_set_utilities;
    }

    InfoSet is = state.to_information_set();
    int actions = state.num_actions();

    auto action_vals = std::make_unique<std::array<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, 7>>();
    for (int a=0; a<7; a++) {
        for (int r=0; r<NUM_CARDS; r++) (*action_vals)[a][r].fill(0.0f);
    }

    if (state.player == traversing_player) {
        for (int a=0; a<actions; a++) {
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = cfr_plus_traverse_tree_fast(state, traversing_player, weight, reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);

            (*action_vals)[a] = *temp;
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += strategy[a] * (*action_vals)[a][c1 - 1][c2 - 1];
                    }
                }
            }
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    for (int a=0; a<actions; a++) {
                        is.cr1 = c1; is.cr2 = c2;
                        int hash = is.hash();
                        regret_sum[hash][a] = std::max(regret_sum[hash][a] + (*action_vals)[a][c1 - 1][c2 - 1] - (*info_set_utilities)[c1 - 1][c2 - 1], 0.0f);
                    }
                }
            }
        }
    } else {
        for (int a=0; a<actions; a++) {
            auto new_reach_probabilities = update_reach_probabilities_cfr_plus(state, a, reach_probabilities);

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = cfr_plus_traverse_tree_fast(state, traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);
        
            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(state.has_card(c1) || state.has_card(c2))) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += (*temp)[c1 - 1][c2 - 1];
                    }
                }
            }
        }

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    int hash = is.hash();
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        strategy_sum[hash][a] = strategy_sum[hash][a] + weight * reach_probabilities[c1 - 1][c2 - 1] * strategy[a];
                    }
                }
            }
        }
    }

    return info_set_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> lcfr_traverse_tree_fast(GameState& state, bool traversing_player, float weight,
                                                                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
                                                                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range) {
    // Terminal
    if (state.is_terminal) return expected_utility_fast_cfr_plus(state, traversing_player, reach_probabilities, inactive_player_range, traversing_player_range);

    auto info_set_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*info_set_utilities)[r].fill(0.0f);

    // Chance
    if (state.is_chance()) {
        int num = state.num_in_deck();

        for (int c=1; c<=NUM_CARDS; c++) {
            if (state.has_card(c)) continue;

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.deal_card(c);
            auto new_reach_probabilities = update_chance_reach_probabilities_cfr_plus(state, num, reach_probabilities);
            auto temp = lcfr_traverse_tree_fast(state, traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(state.has_card(c1) || state.has_card(c2) || (c1==c) || (c2==c))) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += (*temp)[c1 - 1][c2 - 1];
                    }
                }
            }
        }
        return info_set_utilities;
    }

    InfoSet is = state.to_information_set();
    int actions = is.num_actions;

    auto action_vals = std::make_unique<std::array<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, 7>>();
    for (int a=0; a<7; a++) {
        for (int r=0; r<NUM_CARDS; r++) (*action_vals)[a][r].fill(0.0f);
    }

    if (state.player == traversing_player) {
        for (int a=0; a<actions; a++) {
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = lcfr_traverse_tree_fast(state, traversing_player, weight, reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);

            (*action_vals)[a] = *temp;
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += strategy[a] * (*action_vals)[a][c1 - 1][c2 - 1];
                    }
                }
            }
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    for (int a=0; a<actions; a++) {
                        is.cr1 = c1; is.cr2 = c2;
                        int hash = is.hash();
                        regret_sum[hash][a] += weight * ((*action_vals)[a][c1 - 1][c2 - 1] - (*info_set_utilities)[c1 - 1][c2 - 1]);
                    }
                }
            }
        }
    } else {
        for (int a=0; a<actions; a++) {
            auto new_reach_probabilities = update_reach_probabilities_cfr_plus(state, a, reach_probabilities);

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = lcfr_traverse_tree_fast(state, traversing_player, weight, *new_reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);
        
            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(state.has_card(c1) || state.has_card(c2))) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += (*temp)[c1 - 1][c2 - 1];
                    }
                }
            }
        }

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    int hash = is.hash();
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        strategy_sum[hash][a] += weight * reach_probabilities[c1 - 1][c2 - 1] * strategy[a];
                    }
                }
            }
        }
    }

    return info_set_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> dcfr_traverse_tree_fast(GameState& state, bool traversing_player, int iteration,
                                                                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities,
                                                                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                                  const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range) {
    // Terminal
    if (state.is_terminal) return expected_utility_fast_cfr_plus(state, traversing_player, reach_probabilities, inactive_player_range, traversing_player_range);

    auto info_set_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*info_set_utilities)[r].fill(0.0f);

    // Chance
    if (state.is_chance()) {
        int num = state.num_in_deck();

        for (int c=1; c<=NUM_CARDS; c++) {
            if (state.has_card(c)) continue;

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.deal_card(c);
            auto new_reach_probabilities = update_chance_reach_probabilities_cfr_plus(state, num, reach_probabilities);
            auto temp = dcfr_traverse_tree_fast(state, traversing_player, iteration, *new_reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);

            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(state.has_card(c1) || state.has_card(c2) || (c1==c) || (c2==c))) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += (*temp)[c1 - 1][c2 - 1];
                    }
                }
            }
        }
        return info_set_utilities;
    }

    InfoSet is = state.to_information_set();
    int actions = is.num_actions;

    auto action_vals = std::make_unique<std::array<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, 7>>();
    for (int a=0; a<7; a++) {
        for (int r=0; r<NUM_CARDS; r++) (*action_vals)[a][r].fill(0.0f);
    }

    if (state.player == traversing_player) {
        for (int a=0; a<actions; a++) {
            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = dcfr_traverse_tree_fast(state, traversing_player, iteration, reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);

            (*action_vals)[a] = *temp;
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    std::array<float, 7> strategy = get_strategy(is);

                    for (int a=0; a<actions; a++) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += strategy[a] * (*action_vals)[a][c1 - 1][c2 - 1];
                    }
                }
            }
        }
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    for (int a=0; a<actions; a++) {
                        is.cr1 = c1; is.cr2 = c2;
                        int hash = is.hash();
                        regret_sum[hash][a] += ((*action_vals)[a][c1 - 1][c2 - 1] - (*info_set_utilities)[c1 - 1][c2 - 1]);

                        float t_to_alpha = std::pow(iteration, DCFR_ALPHA);
                        float t_to_beta = std::pow(iteration, DCFR_BETA);
                        float weight = (regret_sum[hash][a] > 0) ? (t_to_alpha / (t_to_alpha + 1.0f)) : (t_to_beta / (t_to_beta + 1.0f));
                        regret_sum[hash][a] *= weight;
                    }
                }
            }
        }
    } else {
        for (int a=0; a<actions; a++) {
            auto new_reach_probabilities = update_reach_probabilities_cfr_plus(state, a, reach_probabilities);

            bool player_temp = state.player; float pot_size_temp = state.pot_size;
            state.apply_index(a);
            auto temp = dcfr_traverse_tree_fast(state, traversing_player, iteration, *new_reach_probabilities, inactive_player_range, traversing_player_range);
            state.undo(player_temp, pot_size_temp);
        
            for (int c1=1; c1<=NUM_CARDS; c1++) {
                for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                    if (!(state.has_card(c1) || state.has_card(c2))) {
                        (*info_set_utilities)[c1 - 1][c2 - 1] += (*temp)[c1 - 1][c2 - 1];
                    }
                }
            }
        }

        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    is.cr1 = c1; is.cr2 = c2;
                    int hash = is.hash();
                    std::array<float, 7> strategy = get_strategy(is);

                    float weight = std::pow((iteration / (iteration + 1.0f)), DCFR_GAMMA);
                    for (int a=0; a<actions; a++) {
                        strategy_sum[hash][a] = strategy_sum[hash][a] + weight * reach_probabilities[c1 - 1][c2 - 1] * strategy[a];
                    }
                }
            }
        }
    }

    return info_set_utilities;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_reach_probabilities_cfr_plus(GameState state, int action, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    auto res = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*res)[r].fill(0.0f);

    // Mask out acting player's cards
    if (state.player==0) { state.op1 = 0; state.op2 = 0; }
    else { state.ip1 = 0; state.ip2 = 0; }

    InfoSet is = state.to_information_set();

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(state.has_card(c1) || state.has_card(c2))) {
                // Assign acting player their cards
                is.cr1 = c1;
                is.cr2 = c2;

                float action_probability = get_strategy(is)[action];
                
                (*res)[c1 - 1][c2 - 1] = (action_probability) * reach_probabilities[c1 - 1][c2 - 1];
            }            
        }
    }
    return res;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> update_chance_reach_probabilities_cfr_plus(const GameState& state, int num, const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& reach_probabilities) {
    auto res = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*res)[r].fill(0.0f);

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(state.has_card(c1) || state.has_card(c2))) {
                (*res)[c1 - 1][c2 - 1] = (1.0f / num) * reach_probabilities[c1 - 1][c2 - 1];
            }
        }
    }
    return res;
}

std::unique_ptr<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> expected_utility_fast_cfr_plus(const GameState& state, bool traversing_player, 
                                                                                                    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> reach_probabilities,
                                                                                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& inactive_player_range,
                                                                                                    const std::array<std::array<float, NUM_CARDS>, NUM_CARDS>& traversing_player_range) {
    auto expected_utilities = std::make_unique<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>>();
    for (int r=0; r<NUM_CARDS; r++) (*expected_utilities)[r].fill(0.0f);

    multiply_2d_arrays_simd(reach_probabilities, inactive_player_range);

    std::array<float, NUM_CARDS> row_reach_sums; row_reach_sums.fill(0.0f);
    std::array<float, NUM_CARDS> col_reach_sums; col_reach_sums.fill(0.0f);
    float total_reach = 0.0f;

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(state.has_card(c1) || state.has_card(c2))) {
                row_reach_sums[c1 - 1] += reach_probabilities[c1 - 1][c2 - 1];
                col_reach_sums[c2 - 1] += reach_probabilities[c1 - 1][c2 - 1];
                total_reach += reach_probabilities[c1 - 1][c2 - 1];
            }
        }
    }

    if (state.is_fold()) {
        float fold_utility = (state.player == traversing_player) ? state.pot_size * -0.5f : state.pot_size * 0.5f;
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                if (!(state.has_card(c1) || state.has_card(c2))) {
                    float r = total_reach - row_reach_sums[c1 - 1]
                                          - col_reach_sums[c2 - 1]
                                          + reach_probabilities[c1 - 1][c2 - 1];
                    (*expected_utilities)[c1 - 1][c2 - 1] = fold_utility * r * traversing_player_range[c1 - 1][c2 - 1];
                }
            }
        } 
        // std::cout << "FOLD:\n";
        // std::cout << state.to_string() << "\n";
        // print_opponent_reach_probabilities(*expected_utilities);
        // std::cout << "\n";
        return expected_utilities;
    }

    std::unordered_map<int, std::vector<std::tuple<float, int, int>>> strength_map;
    
    // Prepare all possible hands
    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (!(state.has_card(c1) || state.has_card(c2))) {
                GameState new_state = state;
                new_state.op1 = c1; new_state.op2 = c2;
                int strength = new_state.best_hand_fast();

                // std::cout << "best_hand_fast:   " << strength << "\n";
                // std::cout << "best_hand normal: " << new_state.best_hand(0) << "\n";
                // std::cout << new_state.to_string() << "\n";
                float reach = reach_probabilities[c1 - 1][c2 - 1];

                strength_map[strength].emplace_back(reach, c1, c2);
            }
        }
    }

    std::vector<int> sorted_strengths;
    sorted_strengths.reserve(strength_map.size());

    for (const auto& [strength, _] : strength_map) {
        sorted_strengths.push_back(strength);
    }

    std::sort(sorted_strengths.begin(), sorted_strengths.end());

    std::array<float, NUM_CARDS> row_reach_sums_worse;
    std::array<float, NUM_CARDS> col_reach_sums_worse; 
    std::array<float, NUM_CARDS> row_reach_sums_equal; 
    std::array<float, NUM_CARDS> col_reach_sums_equal; 

    fast_initialize_array(row_reach_sums_equal);
    fast_initialize_array(col_reach_sums_equal);
    fast_initialize_array(row_reach_sums_worse);
    fast_initialize_array(col_reach_sums_worse);

    float cumulative_reach_probabilities_of_worse = 0.0f;
    float cumulative_reach_probabilities_of_equal = 0.0f;
    float utility = state.pot_size * 0.5f;
    
    for (int strength : sorted_strengths) {
        // We've moved to a new hand rank level
        cumulative_reach_probabilities_of_worse += cumulative_reach_probabilities_of_equal;
        cumulative_reach_probabilities_of_equal = 0.0f;

        // row_reach_sums_equal.fill(0.0f);
        // col_reach_sums_equal.fill(0.0f);

        fast_initialize_array(row_reach_sums_equal);
        fast_initialize_array(col_reach_sums_equal);

        for (const auto& [reach, c1, c2] : strength_map[strength]) {
            cumulative_reach_probabilities_of_equal += reach;
            row_reach_sums_equal[c1 - 1] += reach;
            col_reach_sums_equal[c2 - 1] += reach;
        }

        for (const auto& [reach, c1, c2] : strength_map[strength]) {

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

            // std::cout << "hand strength for " << CARD_NAMES[(c1-1)%9] << SUIT_NAMES[(c1-1)/9] << CARD_NAMES[(c2-1)%9] << SUIT_NAMES[(c2-1)/9] << ": " << strength << "\n";
            // std::cout << "cumulative_reach_probabilities_of_worse_no_overlaps for " << CARD_NAMES[(c1-1)%9] << SUIT_NAMES[(c1-1)/9] << CARD_NAMES[(c2-1)%9] << SUIT_NAMES[(c2-1)/9] << ": " << cumulative_reach_probabilities_of_worse_no_overlaps << "\n";
            // std::cout << "cumulative_reach_probabilities_of_equal_no_overlaps for " << CARD_NAMES[(c1-1)%9] << SUIT_NAMES[(c1-1)/9] << CARD_NAMES[(c2-1)%9] << SUIT_NAMES[(c2-1)/9] << ": " << cumulative_reach_probabilities_of_equal_no_overlaps << "\n";
            // std::cout << "cumulative_reach_probabilities_of_bettr_no_overlaps for " << CARD_NAMES[(c1-1)%9] << SUIT_NAMES[(c1-1)/9] << CARD_NAMES[(c2-1)%9] << SUIT_NAMES[(c2-1)/9] << ": " << cumulative_reach_probabilities_of_bettr_no_overlaps << "\n";
            // std::cout << "cumulative_reach_probabilities_of_hands_no_overlaps for " << CARD_NAMES[(c1-1)%9] << SUIT_NAMES[(c1-1)/9] << CARD_NAMES[(c2-1)%9] << SUIT_NAMES[(c2-1)/9] << ": " << cumulative_reach_probabilities_of_hands_no_overlaps << "\n";
            // std::cout << "so expected utility = " << cumulative_reach_probabilities_of_worse_no_overlaps * utility - cumulative_reach_probabilities_of_bettr_no_overlaps * utility << "\n";
            // std::cout << state.to_string() << "\n";

            (*expected_utilities)[c1 - 1][c2 - 1] = utility * traversing_player_range[c1 - 1][c2 - 1] * (cumulative_reach_probabilities_of_worse_no_overlaps - cumulative_reach_probabilities_of_bettr_no_overlaps);
        }

        for (const auto& [reach, c1, c2] : strength_map[strength]) {
            row_reach_sums_worse[c1 - 1] += reach;
            col_reach_sums_worse[c2 - 1] += reach;
        }
    }

    // std::cout << "FULL EXPECTED UTILITIES:\n";
    // std::cout << state.to_string() << "\n";
    // print_opponent_reach_probabilities(*expected_utilities);
    // std::cout << "\n";

    return expected_utilities;
}