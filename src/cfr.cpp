#include "cfr.h"
#include "constants.h"

std::atomic<float> util[2] = {0, 0};
std::mutex regret_sum_mutex;
std::mutex strategy_sum_mutex;
extern std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE> regret_sum;
extern std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE> strategy_sum;

std::ostream& operator<<(std::ostream& os, const std::array<float, 3>& arr) {
    os << "[";
    for (size_t i = 0; i < arr.size(); ++i) {
        os << arr[i];
        if (i != arr.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

void save_cfr_data(const std::string& filename,
                   const std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   const std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& strategy_sum) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for writing: " + filename);
    }

    // Save regret_sum
    file.write(reinterpret_cast<const char*>(regret_sum.data()), 
               sizeof(float) * 3 * STRATEGY_ARRAY_SIZE);

    // Save strategy_sum
    file.write(reinterpret_cast<const char*>(strategy_sum.data()), 
               sizeof(float) * 3 * STRATEGY_ARRAY_SIZE);

    file.close();
}

void load_cfr_data(const std::string& filename,
                   std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& regret_sum,
                   std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE>& strategy_sum) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for reading: " + filename);
    }

    // Load regret_sum
    file.read(reinterpret_cast<char*>(regret_sum.data()), 
              sizeof(float) * 3 * STRATEGY_ARRAY_SIZE);

    // Load strategy_sum
    file.read(reinterpret_cast<char*>(strategy_sum.data()), 
              sizeof(float) * 3 * STRATEGY_ARRAY_SIZE);

    file.close();
}

std::array<float, 3> get_strategy(const InfoSet& info_set) {
    size_t hash = info_set.hash();

    std::array<float, 3> regrets = regret_sum[hash];
    std::array<float, 3> strategy = {0.0f, 0.0f, 0.0f};
    int actions;

    try {
        actions = info_set.num_actions();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    float normalizing_sum = 0;

    // get positive regrets
    for (int i=0; i<actions; i++) {
        strategy[i] = std::max(0.0f, regrets[i]);
        normalizing_sum += strategy[i];
    }
    
    if (normalizing_sum > 0.0001) {
        for (int i=0; i<actions; i++) strategy[i] /= normalizing_sum;
    } else {
        float uniform_prob = 1.0f / actions;
        for (int i=0; i<actions; i++) strategy[i] = uniform_prob;
    }

    return strategy;
}

std::array<float, 3> get_average_strategy(const InfoSet& info_set) {
    size_t hash = info_set.hash();

    std::array<float, 3> sum = strategy_sum[hash];
    int actions = info_set.num_actions();
    float normalizing_sum = 0;

    for (int i=0; i<actions; i++) {
        normalizing_sum += sum[i];
    }
    
    if (normalizing_sum > 0.001) {
        for (int i=0; i<actions; i++) sum[i] /= normalizing_sum;
    } else {
        for (int i=0; i<actions; i++) sum[i] = 1.0 / actions;
    }

    return sum;
}

int sample_action(std::array<float, 3> strategy) {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    float random_value = dis(gen);
    float cumulative_sum = 0.0f;

    for (int i=0; i<3; ++i) {
        cumulative_sum += strategy[i];
        if (random_value <= cumulative_sum) {
            return i;
        }
    }

    return 0;
}

void print_nonzero_strategy(int n, std::string filename) {

    load_cfr_data(filename, regret_sum, strategy_sum);
    
    for (int i=0; i<n; i++) {
        GameState gs = generate_random_initial_state();

        gs.apply_action(1);
        gs.apply_chance_action(32);
        gs.apply_chance_action(31);
        gs.apply_chance_action(30);
        // gs.apply_action(0);
        // gs.apply_action(0);
        // gs.apply_chance_action(29);
        // gs.apply_action(0);
        // gs.apply_action(1);
        // gs.apply_action(1);
        // gs.apply_chance_action(28);
        // gs.apply_action(1);
        // gs.apply_action(2);

        InfoSet info_set = gs.to_information_set();

        std::array<float, 3> strategy = get_strategy(info_set);

        std::cout << info_set.to_string();
        std::cout << "Strategy:         " << strategy << "\n";
        std::cout << "Average strategy: " << get_average_strategy(info_set) << "\n";
        std::cout << "Strategy_sum:     " << strategy_sum[info_set.hash()] << "\n";
        std::cout << "Regret_sum:       " << regret_sum[info_set.hash()] << "\n\n";
    }
}

float as_mccfr(int iterations) {
    std::array<float, 3> init = {0.0f, 0.0f, 0.0f};
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
        threads.emplace_back(as_mccfr_worker, start, end);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    save_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    std::cout << "\n" << iterations << " iterations complete.\n\n";

    return util[0] / iterations;
}

void as_mccfr_worker(int start, int end) {
    for (int t=start; t<end; t++) {
        for (int player = 0; player < 2; player++) {
            GameState gs = generate_random_initial_state();
            float player_util = as_traverse_tree(gs, player, 1);
            float current = util[player].load(std::memory_order_relaxed);
            while (!util[player].compare_exchange_weak(current, current + player_util,
                                                       std::memory_order_relaxed,
                                                       std::memory_order_relaxed)) {
            }
        }
        if ((t+1)%100000==0) std::cout << "t=" << t+1 << "\n";
    }
}

float as_traverse_tree(GameState gs, bool active_player, float q) {
    // Terminal
    if (gs.is_terminal()) return gs.utility(active_player) / q;

    // Chance
    if (gs.is_chance()) {
        GameState temp = gs;
        temp.apply_chance_action(temp.num_chance_actions());
        return as_traverse_tree(temp, active_player, q);
    }

    InfoSet info_set = gs.to_information_set();
    int actions = info_set.num_actions();
    size_t hash = info_set.hash();
    std::array<float, 3> strategy = get_strategy(info_set);

    if (info_set.player != active_player) {
        for (int a=0; a<actions; a++) {
            std::lock_guard<std::mutex> lock(strategy_sum_mutex);
            strategy_sum[hash][a] += strategy[a] / q;
        }

        // Sample inactive player's action and compute utility for it
        int sampled_action = sample_action(strategy);
        GameState child = gs;
        child.apply_action(sampled_action);

        return as_traverse_tree(child, active_player, q);
    }

    // Active player's turn

    std::array<float, 3> util = {0.0f, 0.0f, 0.0f};
    float node_util = 0;
    float sum_over_all_actions = 0;
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    for (int a=0; a<actions; a++) {
        sum_over_all_actions += strategy_sum[hash][a];
    }
    
    for (int a=0; a<actions; a++) {
        float res = (BETA + TAU * strategy_sum[hash][a]) / 
                    (BETA + sum_over_all_actions);
        float p = std::max(EPSILON, res);

        if (dis(gen) < p) {
            GameState child = gs;
            child.apply_action(a);
            util[a] = as_traverse_tree(child, active_player, q * std::min(1.0f, p));
        }

        node_util += strategy[a] * util[a];
    }

    std::lock_guard<std::mutex> lock(strategy_sum_mutex);
    for (int a=0; a<actions; a++) {
        regret_sum[hash][a] += util[a] - node_util;
    }

    return node_util;
}

float calculate_exploitability(int iterations) {
    float ex = 0.0f;
    return ex;
}

void sample_games(int iterations) {
    for (int i=0; i<iterations; i++) {

        std::cout << "Sample game #" << i+1 << "\n";

        GameState gs = generate_random_initial_state();

        while (!gs.is_terminal()) {
            if (gs.is_chance()) {
                gs.apply_chance_action(gs.num_chance_actions());
            }

            else {
                InfoSet info_set = gs.to_information_set();
                std::array<float, 3> strategy = get_average_strategy(info_set);

                std::cout << gs.to_string();
                std::cout << "Strategy: " << strategy << "\n";

                int sampled_action = sample_action(strategy);
                std::cout << "Sampled action: " << sampled_action << "\n\n";

                std::cout << "As information set:\n" << info_set.to_string() << "\n";

                gs.apply_action(sampled_action);
            }
        }
        std::cout << "Sample game #" << i+1 << " over.\n\n";
    }
}

// float as_traverse_tree(GameState gs, bool active_player, float q) {
//     // Terminal
//     if (gs.is_terminal()) return gs.utility(active_player) / q;

//     // Chance
//     if (gs.is_chance()) {
//         GameState temp = gs;
//         temp.apply_chance_action(temp.num_chance_actions());
//         return as_traverse_tree(temp, active_player, q);
//     }

//     InfoSet info_set = gs.to_information_set();
//     int actions = info_set.num_actions();
//     size_t hash = info_set.hash();
//     std::array<float, 3> strategy = get_strategy(info_set);

//     if (info_set.player != active_player) {
//         for (int a=0; a<actions; a++) {
//             strategy_sum[hash][a] += strategy[a] / q;
//         }

//         // Sample inactive player's action and compute utility for it
//         int sampled_action = sample_action(strategy);
//         GameState child = gs;
//         child.apply_action(sampled_action);

//         return as_traverse_tree(child, active_player, q);
//     }

//     // Active player's turn

//     std::array<float, 3> util = {0.0f, 0.0f, 0.0f};
//     float node_util = 0;
//     float sum_over_all_actions = 0;
//     static thread_local std::random_device rd;
//     static thread_local std::mt19937 gen(rd());
//     static thread_local std::uniform_real_distribution<float> dis(0.0f, 1.0f);

//     for (int a=0; a<actions; a++) {
//         sum_over_all_actions += strategy_sum[hash][a];
//     }
    
//     for (int a=0; a<actions; a++) {
//         float res = (BETA + TAU * strategy_sum[hash][a]) / 
//                     (BETA + sum_over_all_actions);
//         float p = std::max(EPSILON, res);

//         if (dis(gen) < p) {
//             GameState child = gs;
//             child.apply_action(a);
//             util[a] = as_traverse_tree(child, active_player, q * std::min(1.0f, p));
//         }

//         node_util += strategy[a] * util[a];
//     }

//     for (int a=0; a<actions; a++) {
//         regret_sum[hash][a] += util[a] - node_util;
//     }

//     return node_util;
// }

// float as_mccfr(int iterations) {
//     float util[2] = {0, 0};

//     // Initialize all elements to {0.0f, 0.0f, 0.0f}
//     std::array<float, 3> init = {0.0f, 0.0f, 0.0f};
//     for (int i=0; i<STRATEGY_ARRAY_SIZE; i++) {
//         strategy_sum[i] = init;
//         regret_sum[i] = init;
//     }

//     try {
//         load_cfr_data("as_latest_checkpoint.dat", regret_sum, strategy_sum);
//     } catch (const std::runtime_error& e) {
//         std::cerr << "Error: " << e.what() << std::endl;
//     }

//     for (int t=0; t<iterations; t++) {
//         for (int player=0; player<2; player++) {
//             GameState gs = generate_random_initial_state();
//             if (player==0) util[0] += as_traverse_tree(gs, player, 1);
//             if (player==1) util[1] += as_traverse_tree(gs, player, 1);
//         }

//         if ((t+1)%20000==0) {
//             std::cout << "Iteration i=" << t+1 << ": " << util[0]/(t+1) << " (utility to SB)\n";
//             std::cout << "                   " << util[1]/(t+1) << " (utility to BB)\n";
//         }

//         if ((t+1)%500000==0) {
//             std::ostringstream filename;
//             filename << "as_latest_checkpoint.dat";
//             save_cfr_data(filename.str(), regret_sum, strategy_sum);
//         }
//     }

//     save_cfr_data("as_latest_checkpoint.dat", regret_sum, strategy_sum);

//     return util[0] / iterations;
// }