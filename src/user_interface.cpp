#include "user_interface.h"
#include "constants.h"

void user_interface() {

    /************************** Card input *************************/
    std::array<uint8_t, 5> board_cards = get_board_cards();

    /************************** Pot Input **************************/
    float pot_size = get_pot_size();

    // input bet sizes (in progress)

    /************************* Range Input *************************/
    // in progress, currently just uniform

    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> range;
    for (int r=0; r<NUM_CARDS; r++) range[r].fill(0.0f);

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            range[c1 - 1][c2 - 1] = 1.0f / (36.0f * 35.0f / 2.0f);
        }   
    }

    /************************ Tree Building ************************/
    GameState root_state = initial_state(pot_size, board_cards);

    if (!prompt_tree_building(root_state)) {
        std::cout << "Exiting.\n";
        return;
    }

    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "Building tree... " << std::flush;

    generate_rank_table(root_state);
    generate_terminal_node_evaluation_tables(root_state);
    Tree tree = Tree(root_state);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << "finished. " << total_time << " seconds.\n";

    /*************************** CFR+ ***************************/
    
    CFRParameters parameters = get_cfr_parameters();
    cfr_plus(tree, parameters, range, range);

    /********************** Display Ranges **********************/

    std::vector<int> history = {};
    print_range(tree, root_state, history);

    /*********************** Explore Tree ***********************/
    // (in progress)
}

std::array<uint8_t, 5> get_board_cards() {
    std::vector<std::string> cards;
    std::array<uint8_t, 5> cards_int;
    std::string input;
    std::regex cardPattern("^[AKQJT9876][hdcs]$");
    
    while (true) {
        std::cout << "Enter 3-5 cards (e.g., Ah Kd 7c), separated by spaces: ";
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string card;
        cards.clear();
        
        while (iss >> card) {
            if (std::regex_match(card, cardPattern)) {
                cards.push_back(card);
            } else {
                std::cout << "Invalid card format. Please format cards like Ah, Kd, or 7c.\n";
                cards.clear();
                break;
            }
        }
        
        if (cards.size() >= 3 && cards.size() <= 5) {
            break;  // Valid input, exit loop
        } else {
            std::cout << "Please enter between 3 and 5 valid cards.\n";
        }
    }

    std::unordered_map<char, int> rank_to_int = {
        {'6', 0}, {'7', 1}, {'8', 2}, {'9', 3}, {'T', 4},
        {'J', 5}, {'Q', 6}, {'K', 7}, {'A', 8}
    };
    std::unordered_map<char, int> suit_to_int = {
        {'s', 0}, {'h', 1}, {'d', 2}, {'c', 3}
    };
    
    // Convert cards to ints
    for (int i=0; i<cards.size(); i++) {
        char rank = cards[i][0];
        char suit = cards[i][1];
        int index = suit_to_int[suit] * 9 + rank_to_int[rank] + 1;
        cards_int[i] = index;
    }

    return cards_int;
}

float get_pot_size() {
    float pot_size;
    while (true) {
        std::cout << "Enter the pot size: ";
        if (std::cin >> pot_size) {
            if (pot_size > 0) {
                return pot_size;
            } else {
                std::cout << "Pot size must be greater than 0. Please try again.\n";
            }
        } else {
            std::cout << "Invalid input. Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

std::string bytes_to_readable_string(
    unsigned long long bytes) {

    const unsigned long long KB = 1024;
    const unsigned long long MB = KB * 1024;
    const unsigned long long GB = MB * 1024;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    if (bytes >= GB) {
        ss << static_cast<double>(bytes) / GB << " GB";
    } else if (bytes >= MB) {
        ss << static_cast<double>(bytes) / MB << " MB";
    } else if (bytes >= KB) {
        ss << static_cast<double>(bytes) / KB << " KB";
    } else {
        ss << bytes << " bytes";
    }

    return ss.str();
}

bool prompt_tree_building(const GameState& initial_state) {
    unsigned long long estimated_memory = estimate_tree_memory(initial_state);
    std::cout << "Estimated memory usage is " << bytes_to_readable_string(estimated_memory) 
              << ". Would you like to continue to build the tree? (Y/n) ";

    // Clear any leftover characters in the input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string response;
    std::getline(std::cin, response);

    // If the response is empty or starts with 'Y' or 'y', proceed with tree building
    if (response.empty() || std::tolower(response[0]) == 'y') {
        return true;
    } else {
        std::cout << "Tree building cancelled.\n";
        return false;
    }
}

CFRParameters get_cfr_parameters() {
    CFRParameters parameters;

    const float MIN_EXPLOITABILITY = 0.0001f;
    const float MAX_EXPLOITABILITY = 1.0f; 

    // Get exploitability goal
    while (true) {
        std::cout << "Enter exploitability goal (recommended: between 0.05-0.5): ";
        if (std::cin >> parameters.exploitability_goal && 
            parameters.exploitability_goal >= MIN_EXPLOITABILITY && 
            parameters.exploitability_goal <= MAX_EXPLOITABILITY) {
            break;
        }
        std::cout << "Invalid input. Please enter a number between " << MIN_EXPLOITABILITY << " and " << MAX_EXPLOITABILITY << ".\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Clear input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const int MIN_ITERATIONS = 1;
    const int MAX_ITERATIONS = 1e+9; 

    // Get max number of iterations
    while (true) {
        std::cout << "Enter maximum number of iterations (" << MIN_ITERATIONS << " - " << MAX_ITERATIONS << "): ";
        if (std::cin >> parameters.max_iterations && 
            parameters.max_iterations >= MIN_ITERATIONS && 
            parameters.max_iterations <= MAX_ITERATIONS) {
            break;
        }
        std::cout << "Invalid input. Please enter a number between " << MIN_ITERATIONS << " and " << MAX_ITERATIONS << ".\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const int MIN_LOG_INTERVAL = 1;
    const int MAX_LOG_INTERVAL = 1000; 

    // Get log interval
    while (true) {
        std::cout << "Enter log interval: ";
        if (std::cin >> parameters.log_interval && 
            parameters.log_interval >= MIN_LOG_INTERVAL && 
            parameters.log_interval <= MAX_LOG_INTERVAL) {
            break;
        }
        std::cout << "Invalid input. Please enter a number between " << MIN_LOG_INTERVAL << " and " << MAX_LOG_INTERVAL << ".\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << "\n";

    return parameters;
}