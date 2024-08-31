#include "file_io.h"

void read_range_file(const std::string& filename, float range[NUM_RANKS][NUM_RANKS]) {
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    
    // Initialize array to 0
    for (int i = 0; i < NUM_RANKS; i++) {
        for (int j = 0; j < NUM_RANKS; j++) {
            range[i][j] = 0.0;
        }
    }
    
    std::getline(file, line);
    std::istringstream iss(line);
    std::string token;
    
    std::map<char, int> rankToIndex = {
        {'A', 0}, {'K', 1}, {'Q', 2}, {'J', 3}, {'T', 4},
        {'9', 5}, {'8', 6}, {'7', 7}, {'6', 8}
    };
    
    while (std::getline(iss, token, ',')) {
        size_t colonPos = token.find(':');
        if (colonPos != std::string::npos) {
            std::string hand = token.substr(0, colonPos);
            float value = std::stod(token.substr(colonPos + 1));
            
            int row = rankToIndex[hand[0]];
            int col = rankToIndex[hand[1]];
            
            if (hand.length() == 3 && hand[2] == 's') {
                // Suited hand
                range[row][col] = value;
            } else if (hand.length() == 2) {
                // Pair
                range[row][col] = value;
            } else if (hand.length() == 3 && hand[2] == 'o') {
                // Offsuit hand
                range[col][row] = value;
            }
        }
    }
    
    file.close();
}

// Helper function to print the range
void printRange(const std::vector<std::vector<float>>& range) {
    for (const auto& row : range) {
        for (float val : row) {
            std::cout << val << "\t";
        }
        std::cout << std::endl;
    }
}