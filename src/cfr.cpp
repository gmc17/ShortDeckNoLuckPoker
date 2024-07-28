#include "cfr.h"
#include "constants.h"

std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE> regret_sum;
std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE> strategy_sum;

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

    std::cout << "First few elements after loading:\n";
    for (int i = 0; i < std::min(10, static_cast<int>(STRATEGY_ARRAY_SIZE)); ++i) {
        std::cout << "regret_sum[" << i << "]: " 
                  << regret_sum[i][0] << ", " << regret_sum[i][1] << ", " << regret_sum[i][2] << "\n";
        std::cout << "strategy_sum[" << i << "]: " 
                  << strategy_sum[i][0] << ", " << strategy_sum[i][1] << ", " << strategy_sum[i][2] << "\n";
    }
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

std::array<float, 3> get_strategy(const GameState& info_set) {
    size_t hash = hash_gamestate(info_set);

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

std::array<float, 3> get_average_strategy(const GameState& info_set) {
    size_t hash = hash_gamestate(info_set);

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

float traverse_tree(GameState gs, bool active_player, float p0, float p1) {
    // Terminal
    if (gs.is_terminal()) {
        // std::cout << gs.to_string();
        // std::cout << "Utility to player " << active_player << ": " << gs.utility(active_player) << "\n\n";
        return gs.utility(active_player);
    }

    // Chance
    if (gs.is_chance()) {
        float util = 0;
        for (int i=0; i<NUM_CHANCE_SAMPLES; i++) {
            int actions = gs.num_chance_actions();
            GameState temp = gs;
            temp.apply_chance_action(actions);
            util += traverse_tree(temp, active_player, p0 * (1.0f/actions), p1 * (1.0f/actions));
        }
        return util / NUM_CHANCE_SAMPLES;
    }

    GameState info_set = gs;
    info_set.to_information_set();
    int actions;
    size_t hash = hash_gamestate(info_set);

    try {
        actions = info_set.num_actions();
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    if (info_set.player==active_player) {
        std::array<float, 3> util = {0.0f, 0.0f, 0.0f};
        float node_util = 0;
        std::array<float, 3> strategy = get_strategy(info_set);
        
        for (int i=0; i<actions; i++) {
            GameState temp = gs;
            temp.apply_action(i);
            float child_util = (active_player == 0) 
                             ? traverse_tree(temp, active_player, p0 * strategy[i], p1) 
                             : traverse_tree(temp, active_player, p0, p1 * strategy[i]);
            util[i] = child_util;
            node_util += strategy[i] * child_util;
        }

        for (int i=0; i<actions; i++) {

            regret_sum[hash][i] += (active_player == 0)
                                     ? (util[i] - node_util) * p1
                                     : (util[i] - node_util) * p0;
            
            strategy_sum[hash][i] += (active_player == 0)
                                       ? strategy[i] * p0
                                       : strategy[i] * p1;
        }

        return node_util;

    } else if (info_set.player != active_player) {
        std::array<float, 3> strategy = get_strategy(info_set);
        int sampled_action = sample_action(strategy);
        GameState temp = gs;
        temp.apply_action(sampled_action);

        float util = (active_player == 0) 
             ? traverse_tree(temp, active_player, p0, p1 * strategy[sampled_action]) 
             : traverse_tree(temp, active_player, p0 * strategy[sampled_action], p1) ;

        for (int i=0; i<actions; i++) {
            strategy_sum[hash][i] += (active_player == 0)
                                       ? strategy[i] * p0
                                       : strategy[i] * p1;
        }

        return util;
    }

    return 0.0f;
}

float mccfr(int iterations) {
    float util[2] = {0, 0};

    // Initialize all elements to {0.0f, 0.0f, 0.0f}
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

    for (int t=0; t<iterations; t++) {
        for (int player=0; player<2; player++) {
            GameState gs = generate_random_initial_state();
            if (player==0) util[0] += traverse_tree(gs, player, 1, 1);
    
            if (player==1) {
                GameState info_set = gs;
                info_set.to_information_set();
                
                float preflop_call_odds = get_strategy(info_set)[1];
                gs.apply_action(1);

                gs.apply_chance_action(32);
                gs.apply_chance_action(31);
                gs.apply_chance_action(30);

                info_set = gs;
                info_set.to_information_set();

                int sampled_action = sample_action(get_strategy(info_set));
                gs.apply_action(sampled_action);

                util[1] += traverse_tree(gs, player, preflop_call_odds * get_strategy(info_set)[sampled_action], 1);
            }
        }
        if (t%1000==0) {
            std::cout << "Iteration i=" << t << ": " << util[0]/iterations << " (utility to SB)\n";
            std::cout << "                 " << util[1]/iterations << " (utility to BB)\n";
        }

        if ((t+1)%2500==0) {
            std::ostringstream filename;
            filename << "latest_checkpoint.dat";
            save_cfr_data(filename.str(), regret_sum, strategy_sum);
        }
    }


    for (int i=0; i<2; i++) {

        std::cout << "Sample game #" << i+1 << "\n";

        GameState gs = generate_random_initial_state();

        while (!gs.is_terminal()) {
            if (gs.is_chance()) {
                gs.apply_chance_action(gs.num_chance_actions());
            }

            else {
                GameState info_set = gs;
                info_set.to_information_set();
                std::array<float, 3> strategy = get_average_strategy(info_set);

                std::cout << gs.to_string();
                std::cout << "Strategy: " << strategy << "\n";

                int sampled_action = sample_action(strategy);
                std::cout << "Sampled action: " << sampled_action << "\n\n";

                gs.apply_action(sampled_action);
            }
        }
        std::cout << "Sample game #" << i+1 << " over.\n";
    }

    return util[0] / iterations;
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

        gs.to_information_set();

        std::array<float, 3> strategy = get_strategy(gs);

        std::cout << gs.to_string();
        std::cout << "Strategy:         " << strategy << "\n";
        std::cout << "Average strategy: " << get_average_strategy(gs) << "\n";
        std::cout << "Strategy_sum:     " << strategy_sum[hash_gamestate(gs)] << "\n";
        std::cout << "Regret_sum:       " << regret_sum[hash_gamestate(gs)] << "\n\n";
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

    GameState info_set = gs;
    info_set.to_information_set();
    int actions = info_set.num_actions();
    size_t hash = hash_gamestate(info_set);
    std::array<float, 3> strategy = get_strategy(info_set);

    if (info_set.player != active_player) {
        for (int a=0; a<actions; a++) {
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

    for (int a=0; a<actions; a++) {
        regret_sum[hash][a] += util[a] - node_util;
    }

    return node_util;
}

float as_mccfr(int iterations) {
    float util[2] = {0, 0};

    // Initialize all elements to {0.0f, 0.0f, 0.0f}
    std::array<float, 3> init = {0.0f, 0.0f, 0.0f};
    for (int i=0; i<STRATEGY_ARRAY_SIZE; i++) {
        strategy_sum[i] = init;
        regret_sum[i] = init;
    }

    try {
        load_cfr_data("as_latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    for (int t=0; t<iterations; t++) {
        for (int player=0; player<2; player++) {
            GameState gs = generate_random_initial_state();
            if (player==0) util[0] += as_traverse_tree(gs, player, 1);
            if (player==1) util[1] += as_traverse_tree(gs, player, 1);
        }

        if (t%20000==0) {
            std::cout << "Iteration i=" << t << ": " << util[0]/iterations << " (utility to SB)\n";
            std::cout << "                 " << util[1]/iterations << " (utility to BB)\n";
        }

        if ((t+1)%100000==0) {
            std::ostringstream filename;
            filename << "as_latest_checkpoint.dat";
            save_cfr_data(filename.str(), regret_sum, strategy_sum);
        }
    }


    for (int i=0; i<2; i++) {

        std::cout << "Sample game #" << i+1 << "\n";

        GameState gs = generate_random_initial_state();

        while (!gs.is_terminal()) {
            if (gs.is_chance()) {
                gs.apply_chance_action(gs.num_chance_actions());
            }

            else {
                GameState info_set = gs;
                info_set.to_information_set();
                std::array<float, 3> strategy = get_average_strategy(info_set);

                std::cout << gs.to_string();
                std::cout << "Strategy: " << strategy << "\n";

                int sampled_action = sample_action(strategy);
                std::cout << "Sampled action: " << sampled_action << "\n\n";

                gs.apply_action(sampled_action);
            }
        }
        std::cout << "Sample game #" << i+1 << " over.\n\n";
    }

    return util[0] / iterations;
}