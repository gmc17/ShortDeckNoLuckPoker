#include "user_interface.h"
#include "constants.h"

void user_interface() {
    const int WIDTH = 70;

    print_separator(WIDTH, '=');
    print_centered("ShortDeckNoLuck", BOLD, WIDTH);
    print_separator(WIDTH, '=');

    /************************** Card input *************************/
    print_centered("Board Cards", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    std::array<uint8_t, 5> board_cards = get_board_cards();
    std::cout << std::endl;

    /************************** Pot Input **************************/
    print_centered("Pot Size", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    float pot_size = get_pot_size();
    std::cout << std::endl;

    /******************** Starting Stack Input *********************/
    print_centered("Starting Stack Size", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    STACK_SIZE = get_starting_stack_size();
    std::cout << std::endl;

    /************************* Range Input *************************/
    print_centered("Range", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    auto [ip_range, op_range] = get_ranges();
    std::cout << std::endl;

    /****************** Bet & Raise Sizings Input ******************/
    print_centered("Bet & Raise Sizes", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    get_bet_raise_sizes();
    std::cout << std::endl;

    /************************ Tree Building ************************/
    print_centered("Tree", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    GameState root_state = initial_state(pot_size, board_cards);

    if (!prompt_tree_building(root_state)) {
        std::cout << RED + "Exiting." + RESET << std::endl;
        return;
    }

    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    std::cout << GREEN << "Building tree... " << std::flush << RESET;

    generate_rank_table(root_state);
    generate_terminal_node_evaluation_tables(root_state);
    Tree tree = Tree(root_state);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << WHITE << "finished. " << total_time << " seconds." << RESET << std::endl;
    std::cout << std::endl;

    /*************************** CFR+ ***************************/
    print_centered("CFR+ Parameters", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    CFRParameters parameters = get_cfr_parameters();
    cfr_plus(tree, parameters, ip_range, op_range);
    std::cout << std::endl;

    /********************** Display Ranges **********************/
    print_centered("Player Ranges", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    std::vector<int> history = {};
    print_range(tree, root_state, history);
    std::cout << std::endl;

    /*********************** Explore Tree ***********************/
    print_centered("Tree Explorer", YELLOW, WIDTH);
    print_separator(WIDTH, '-');
    explore_tree(tree, root_state);
    std::cout << std::endl;
}

std::array<uint8_t, 5> get_board_cards() {
    std::vector<std::string> cards;
    std::array<uint8_t, 5> cards_int;
    std::string input;
    std::regex cardPattern("^[AKQJT9876][hdcs]$");

    while (true) {
        std::cout << CYAN << "Enter 3-5 cards (e.g., Ah Kd 7c), separated by spaces: " << RESET;
        std::getline(std::cin, input);
        std::istringstream iss(input);
        std::string card;
        cards.clear();

        while (iss >> card) {
            if (std::regex_match(card, cardPattern)) {
                cards.push_back(card);
            } else {
                std::cout << RED << "Invalid card format. Please format cards like Ah, Kd, or 7c." << RESET << std::endl;
                cards.clear();
                break;
            }
        }

        if (cards.size() >= 3 && cards.size() <= 5) {
            break;  // Valid input, exit loop
        } else {
            std::cout << RED << "Please enter between 3 and 5 valid cards." << RESET << std::endl;
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
        std::cout << CYAN << "Enter the pot size: " << RESET;
        if (std::cin >> pot_size) {
            if (pot_size > 0) {
                return pot_size;
            } else {
                std::cout << RED << "Pot size must be greater than 0. Please try again." << RESET << std::endl;
            }
        } else {
            std::cout << RED << "Invalid input. Please enter a valid number." << RESET << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

float get_starting_stack_size() {
    float pot_size;
    while (true) {
        std::cout << CYAN << "Enter the starting stack size: " << RESET;
        if (std::cin >> pot_size) {
            if (pot_size > 0) {
                return pot_size;
            } else {
                std::cout << RED << "Starting stack size must be greater than 0. Please try again." << RESET << std::endl;
            }
        } else {
            std::cout << RED << "Invalid input. Please enter a valid number." << RESET << std::endl;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

void get_bet_raise_sizes() {

    std::cout << GREEN << "Current bet sizes: " << RESET;
    for (const auto& size : BET_SIZES) {
        std::cout << size << " ";
    }
    std::cout << std::endl;

    std::cout << GREEN << "Current raise sizes: " << RESET;
    for (const auto& size : RAISE_SIZES) {
        std::cout << size << " ";
    }
    std::cout << std::endl;

    std::cout << CYAN << "Do you want to modify the bet and raise sizes? (Y/n): " << RESET;
    std::string response;
    std::getline(std::cin, response);
    std::cout << std::endl;

    if (response == "y" || response == "Y") {
        auto get_sizes = [](const std::string& type, std::array<float, 4>& sizes) {
            while (true) {
                std::cout << CYAN << "Enter four " << type << " sizes (space-separated): " << RESET;
                std::string input;
                std::getline(std::cin, input);
                std::istringstream iss(input);
                
                bool valid = true;
                for (int i = 0; i < 4; ++i) {
                    if (!(iss >> sizes[i])) {
                        valid = false;
                        break;
                    }
                }

                if (valid && iss.eof()) {
                    return;
                } else {
                    std::cout << RED << "Invalid input. Please enter exactly four numbers." << RESET << std::endl;
                    std::cin.clear();
                }
            }
        };

        get_sizes("bet", BET_SIZES);
        get_sizes("raise", RAISE_SIZES);

        std::cout << GREEN << "Bet and raise sizes updated successfully." << RESET << std::endl;
    } else {
        std::cout << GREEN << "Using default bet and raise sizes." << RESET << std::endl;
    }

    std::cout << GREEN << "Final bet sizes: " << RESET;
    for (const auto& size : BET_SIZES) {
        std::cout << size << " ";
    }
    std::cout << std::endl;

    std::cout << GREEN << "Final raise sizes: " << RESET;
    for (const auto& size : RAISE_SIZES) {
        std::cout << size << " ";
    }
    std::cout << std::endl;
}

void read_range_file(const std::string& filename, std::array<std::array<float, NUM_RANKS>, NUM_RANKS>& range_9x9) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::string line;
    std::getline(file, line);
    std::istringstream iss(line);
    std::string token;

    std::map<char, int> rankToIndex = {
        {'A', 8}, {'K', 7}, {'Q', 6}, {'J', 5}, {'T', 4},
        {'9', 3}, {'8', 2}, {'7', 1}, {'6', 0}
    };

    // Initialize array to 0
    for (auto& row : range_9x9) {
        row.fill(0.0f);
    }

    while (std::getline(iss, token, ',')) {
        size_t colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            std::string hand = token.substr(0, colonPos);
            float value = std::stof(token.substr(colonPos + 1));

            if (hand.length() == 2 || hand.length() == 3) {
                int row = rankToIndex[hand[0]];
                int col = rankToIndex[hand[1]];

                if (hand.length() == 2 || (hand.length() == 3 && hand[2] == 's')) {
                    // Pair or suited
                    range_9x9[row][col] = value;
                } else if (hand.length() == 3 && hand[2] == 'o') {
                    // Offsuit
                    range_9x9[col][row] = value;
                }
            }
        }
    }

    file.close();
}

std::pair<std::array<std::array<float, NUM_CARDS>, NUM_CARDS>, std::array<std::array<float, NUM_CARDS>, NUM_CARDS>> get_ranges() {
    std::array<std::array<float, NUM_RANKS>, NUM_RANKS> ip_range_9x9;
    std::array<std::array<float, NUM_RANKS>, NUM_RANKS> oop_range_9x9;
    
    // Clear any leftover input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << CYAN << "Range selection options:" << RESET << std::endl;
    std::cout << "1. Use stored ranges" << std::endl;
    std::cout << "2. Enter range files manually" << std::endl;
    std::cout << CYAN << "Desired option: " << RESET;

    std::string choice;
    std::getline(std::cin, choice);

    std::cout << std::endl;

    if (choice == "1") {
        std::vector<std::string> default_ranges;
        const std::string default_ranges_path = "default_ranges";

        // List all files in the default_ranges directory
        for (const auto& entry : std::filesystem::directory_iterator(default_ranges_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                default_ranges.push_back(entry.path().filename().string());
            }
        }

        // Sort the filenames alphabetically
        std::sort(default_ranges.begin(), default_ranges.end());

        std::cout << CYAN << "Available stored ranges:" << RESET << std::endl;
        for (size_t i = 0; i < default_ranges.size(); ++i) {
            std::cout << i + 1 << ". " << default_ranges[i] << std::endl;
        }

        auto get_default_range = [&](const std::string& position) -> std::array<std::array<float, NUM_RANKS>, NUM_RANKS> {
            while (true) {
                std::cout << CYAN << "Desired option for " << position << " player: " << RESET;
                std::string input;
                std::getline(std::cin, input);
                try {
                    int range_choice = std::stoi(input);
                    if (range_choice >= 1 && range_choice <= static_cast<int>(default_ranges.size())) {
                        std::string filename = default_ranges_path + "/" + default_ranges[range_choice - 1];
                        std::array<std::array<float, NUM_RANKS>, NUM_RANKS> range_9x9;
                        read_range_file(filename, range_9x9);
                        std::cout << GREEN << position << " range file '" << filename << "' has been processed." << RESET << std::endl;
                        return range_9x9;
                    } else {
                        std::cout << RED << "Invalid choice. Please enter a valid number." << RESET << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cout << RED << "Invalid input. Please enter a number." << RESET << std::endl;
                }
            }
        };

        ip_range_9x9 = get_default_range("IP");
        oop_range_9x9 = get_default_range("OOP");
    } else if (choice == "2") {
        auto get_single_range = [&](const std::string& position) -> std::array<std::array<float, NUM_RANKS>, NUM_RANKS> {
            while (true) {
                std::cout << CYAN << "Do you want to input a range file for the " << position << " player? (Y/n): " << RESET;
                std::string input;
                std::getline(std::cin, input);
                if (input.empty()) {
                    std::cout << RED << "Invalid input. Please enter 'y' or 'n'." << RESET << std::endl;
                    continue;
                }
                char subchoice = input[0];
                if (subchoice == 'y' || subchoice == 'Y') {
                    std::string filename;
                    std::cout << CYAN << "Enter the name of the " << position << " player's range file: " << RESET;
                    std::getline(std::cin, filename);
                    if (filename.empty()) {
                        std::cout << RED << "Invalid filename. Please try again." << RESET << std::endl;
                        continue;
                    }
                    try {
                        std::array<std::array<float, NUM_RANKS>, NUM_RANKS> range_9x9;
                        read_range_file(filename, range_9x9);
                        std::cout << GREEN << position << " range file '" << filename << "' has been processed." << RESET << std::endl;
                        return range_9x9;
                    } catch (const std::exception& e) {
                        std::cout << RED << "Failed to process the range file: " << e.what() << RESET << std::endl;
                        std::cout << "Please try again." << std::endl;
                        continue;
                    }
                } else if (subchoice == 'n' || subchoice == 'N') {
                    // Use default uniform range for 9x9
                    std::array<std::array<float, NUM_RANKS>, NUM_RANKS> range_9x9;
                    float uniform_value = 1.0f / (NUM_CARDS * (NUM_CARDS - 1.0f) * 0.5f);
                    std::fill(&range_9x9[0][0], &range_9x9[0][0] + NUM_RANKS * NUM_RANKS, uniform_value);
                    std::cout << GREEN << "Using default uniform range for " << position << " player." << RESET << std::endl;
                    return range_9x9;
                } else {
                    std::cout << RED << "Invalid input. Please enter 'y' or 'n'." << RESET << std::endl;
                }
            }
        };

        ip_range_9x9 = get_single_range("IP");
        oop_range_9x9 = get_single_range("OOP");
    } else {
        std::cout << RED << "Invalid choice. Using default uniform range." << RESET << std::endl;
        float uniform_value = 1.0f / (NUM_CARDS * (NUM_CARDS - 1.0f) * 0.5f);
        std::fill(&ip_range_9x9[0][0], &ip_range_9x9[0][0] + NUM_RANKS * NUM_RANKS, uniform_value);
        std::fill(&oop_range_9x9[0][0], &oop_range_9x9[0][0] + NUM_RANKS * NUM_RANKS, uniform_value);
    }

    return {convert_9x9_to_36x36(ip_range_9x9), convert_9x9_to_36x36(oop_range_9x9)};
}

std::array<std::array<float, NUM_CARDS>, NUM_CARDS> convert_9x9_to_36x36(const std::array<std::array<float, NUM_RANKS>, NUM_RANKS>& range_9x9) {
    std::array<std::array<float, NUM_CARDS>, NUM_CARDS> range_36x36;
    
    // Initialize 36x36 array to 0
    for (auto& row : range_36x36) {
        row.fill(0.0f);
    }

    float normalizing_sum = 0.0f;
    // 8 8 --> A A --> 8, 17, 26, 35
    // 8 7 --> A K suited --> 
    // 7 8 --> A K off --> 

    for (int r1 = 0; r1 < NUM_RANKS; r1++) {
        for (int r2 = 0; r2 < NUM_RANKS; r2++) {
            float value = range_9x9[r1][r2];
            
            // Convert rank indices to card indices
            for (int s1 = 0; s1 < NUM_SUITS; s1++) {
                for (int s2 = 0; s2 < NUM_SUITS; s2++) {
                    int c1 = r1 + s1 * NUM_RANKS;
                    int c2 = r2 + s2 * NUM_RANKS;
                    
                    if (r1 == r2) {
                        // Pair
                        if (s1 < s2) {
                            if (c1 < c2) range_36x36[c1][c2] = value;
                            else range_36x36[c2][c1] = value;
                            normalizing_sum += value;
                        }
                    } else if (r1 < r2) {
                        // Suited
                        if (s1 == s2) {
                            if (c1 < c2) range_36x36[c1][c2] = value;
                            else range_36x36[c2][c1] = value;
                            normalizing_sum += value;
                        }
                    } else {
                        // Offsuit
                        if (s1 != s2) {
                            if (c1 < c2) range_36x36[c1][c2] = value;
                            else range_36x36[c2][c1] = value;
                            normalizing_sum += value;
                        }
                    }
                }
            }
        }
    }

    if (normalizing_sum > 0.0f) {
        for (int c1=0; c1<NUM_CARDS; c1++) {
            for (int c2=0; c2<NUM_CARDS; c2++) {
                range_36x36[c1][c2] /= normalizing_sum;
            }
        }
    }
    
    return range_36x36;
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
    std::cout << YELLOW << "Estimated memory usage is " << bytes_to_readable_string(estimated_memory) 
              << "." << RESET << std::endl;
    std::cout << CYAN << "Would you like to continue to build the tree? (Y/n): " << RESET;

    std::string response;
    std::getline(std::cin, response);

    // Trim leading and trailing whitespace
    response.erase(0, response.find_first_not_of(" \t\n\r\f\v"));
    response.erase(response.find_last_not_of(" \t\n\r\f\v") + 1);

    // If the response is empty or starts with 'Y' or 'y', proceed with tree building
    if (response.empty() || std::tolower(response[0]) == 'y') {
        return true;
    } else {
        std::cout << RED << "Tree building cancelled." << RESET << std::endl;
        return false;
    }
}

CFRParameters get_cfr_parameters() {
    CFRParameters parameters;

    const float MIN_EXPLOITABILITY = 0.000001f;
    const float MAX_EXPLOITABILITY = 1.0f; 

    // Get exploitability goal
    while (true) {
        std::cout << CYAN << "Enter exploitability goal (recommended: between 0.05-0.5): " << RESET;
        if (std::cin >> parameters.exploitability_goal && 
            parameters.exploitability_goal >= MIN_EXPLOITABILITY && 
            parameters.exploitability_goal <= MAX_EXPLOITABILITY) {
            break;
        }
        std::cout << RED << "Invalid input. Please enter a number between " << MIN_EXPLOITABILITY << " and " << MAX_EXPLOITABILITY << "." << RESET << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Clear input buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const int MIN_ITERATIONS = 1;
    const int MAX_ITERATIONS = 1e+9; 

    // Get max number of iterations
    while (true) {
        std::cout << CYAN << "Enter maximum number of iterations (" << MIN_ITERATIONS << " - " << MAX_ITERATIONS << "): " << RESET;
        if (std::cin >> parameters.max_iterations && 
            parameters.max_iterations >= MIN_ITERATIONS && 
            parameters.max_iterations <= MAX_ITERATIONS) {
            break;
        }
        std::cout << RED << "Invalid input. Please enter a number between " << MIN_ITERATIONS << " and " << MAX_ITERATIONS << "." << RESET << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    const int MIN_LOG_INTERVAL = 1;
    const int MAX_LOG_INTERVAL = 1000; 

    // Get log interval
    while (true) {
        std::cout << CYAN << "Enter log interval: " << RESET;
        if (std::cin >> parameters.log_interval && 
            parameters.log_interval >= MIN_LOG_INTERVAL && 
            parameters.log_interval <= MAX_LOG_INTERVAL) {
            break;
        }
        std::cout << RED << "Invalid input. Please enter a number between " << MIN_LOG_INTERVAL << " and " << MAX_LOG_INTERVAL << "." << RESET << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::cout << std::endl;

    return parameters;
}

void print_centered(const std::string& text, const std::string& color, int width) {
    int padding = (width - text.length()) / 2;
    std::cout << color << std::string(padding, ' ') << text << RESET << std::endl;
}

void print_separator(int width, char sep) {
    std::cout << std::string(width, sep) << std::endl;
}

void display_current_state(const std::vector<GameState>& states, const std::vector<int>& actions) {
    std::cout << YELLOW << "Current state: " << RESET << "Root";
    for (size_t i = 0; i < actions.size(); ++i) {
        if (states[i].is_chance()) {
            std::cout << " → " << YELLOW << CARD_NAMES[rank(actions[i])] << SUIT_NAMES[suit(actions[i])] << RESET;
        } else {
            int action = states[i].index_to_action(actions[i]);
            std::cout << " → " << CYAN << states[i].action_to_string(action) << RESET;
        }
    }

    std::cout << "\n";
}

uint8_t parse_card_input(const std::string& input, const GameState& current_state) {
    std::regex cardPattern("^[AKQJT9876][hdcs]$");
    if (!std::regex_match(input, cardPattern)) {
        throw std::runtime_error("Invalid card format. Please format cards like Ah, Kd, or 7c.");
    }

    std::unordered_map<char, int> rank_to_int = {
        {'6', 0}, {'7', 1}, {'8', 2}, {'9', 3}, {'T', 4},
        {'J', 5}, {'Q', 6}, {'K', 7}, {'A', 8}
    };
    std::unordered_map<char, int> suit_to_int = {
        {'s', 0}, {'h', 1}, {'d', 2}, {'c', 3}
    };

    char rank = input[0];
    char suit = input[1];
    uint8_t card_int = suit_to_int[suit] * 9 + rank_to_int[rank] + 1;

    if (current_state.has_card(card_int)) {
        throw std::runtime_error("This card is already on the board. Please choose another.");
    }

    return card_int;
}

void explore_tree(const Tree& tree, const GameState& root_state) {
    std::vector<int> actions;
    std::vector<GameState> states = {root_state};
    const int WIDTH = 70;

    while (true) {
        system("clear"); // Clear the console (use "cls" for Windows)
        const GameState& current_state = states.back();
        
        // Print header
        print_separator(WIDTH, '=');
        print_centered("Tree Explorer", BOLD, WIDTH);
        print_separator(WIDTH, '=');

        // Display current state
        display_current_state(states, actions);

        if (!current_state.is_chance()) {
            // Print current range
            print_range(tree, root_state, actions);

            // Print possible actions
            std::cout << YELLOW << "Possible actions:" << RESET << "\n";
            for (int i = 0; i < current_state.num_actions(); ++i) {
                int action = current_state.index_to_action(i);
                std::cout << "  " << BOLD << (i + 1) << ":" << RESET << " " 
                          << current_state.action_to_string(action) << "\n";
            }

            // Print instructions
            print_separator(WIDTH);
            std::cout << CYAN << "Enter an action (1-" << current_state.num_actions() 
                              << "), 'b' to go back, or 'e' to exit: " << RESET;
        } else {
            std::cout << YELLOW << "This is a chance node." << CYAN << "\nEnter a card (e.g., Ah), 'b' to go back, or 'e' to exit: " << RESET;
        }

        std::string input;
        std::getline(std::cin, input);
        // Trim whitespace from input
        input.erase(0, input.find_first_not_of(" \t\n\r\f\v"));
        input.erase(input.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (input.empty()) {
            continue;  // Ignore empty input and redraw
        }
        
        if (input == "e") {
            break;
        } else if (input == "b") {
            if (!actions.empty()) {
                actions.pop_back();
                states.pop_back();
            } else {
                std::cout << RED << "Cannot go back from root state." << RESET << std::endl;
                std::cin.get();  // Wait for user input before redrawing
            }
        } else if (current_state.is_chance()) {
            try {
                uint8_t card = parse_card_input(input, current_state);
                actions.push_back(card);
                GameState new_state = current_state;
                new_state.deal_card(card);
                states.push_back(new_state);
            } catch (const std::runtime_error& e) {
                std::cout << RED << e.what() << RESET << std::endl;
                std::cin.get();  // Wait for user input before redrawing
            }
        } else {
            try {
                int action = std::stoi(input) - 1;
                if (action >= 0 && action < current_state.num_actions()) {
                    actions.push_back(action);
                    GameState new_state = current_state;
                    new_state.apply_index(action);
                    states.push_back(new_state);
                } else {
                    std::cout << RED << "Invalid action. Please enter a number between 1 and " 
                              << current_state.num_actions() << "." << RESET << std::endl;
                    std::cin.get();  // Wait for user input before redrawing
                }
            } catch (const std::invalid_argument&) {
                std::cout << RED << "Invalid input. Please enter a number, 'b', or 'e'." << RESET << std::endl;
                std::cin.get();  // Wait for user input before redrawing
            } catch (const std::out_of_range&) {
                std::cout << RED << "Number out of range. Please enter a number between 1 and " 
                          << current_state.num_actions() << "." << RESET << std::endl;
                std::cin.get();  // Wait for user input before redrawing
            }
        }
    }
}