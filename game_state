#include "game_state.h"
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>

GameState::GameState(const uint32_t suita, 
                     const uint32_t suitb, 
                     const uint32_t suitc, 
                     const uint32_t suitd, 
                     const uint8_t  turn,
                     const uint8_t  rivr,
                     const uint8_t  flop_history, 
                     const uint8_t  turn_history, 
                     const uint8_t  rivr_history,
                     const bool player)
                     : 
                     suita(suita),
                     suitb(suitb),
                     suitc(suitc),
                     suitd(suitd),
                     turn(turn),
                     rivr(rivr),
                     flop_history(flop_history),
                     turn_history(turn_history),
                     rivr_history(rivr_history),
                     player(player)

{
    if (suita < suitb || suitb < suitc || suitc < suitd) {
        throw std::invalid_argument("Suits not in correct order.");
    }
}

std::string GameState::to_string() const {
    std::stringstream ss;

    std::array<uint32_t, 4> suits = {suita, suitb, suitc, suitd};
    std::array<std::string,  4> suit_names = {"s", "h", "d", "c"};
    std::array<std::string, 13> card_names = {"6", "7", "8", "9",
                                              "10", "J", "Q", "K",
                                              "A"};
    
    // Hole cards
    ss << "Hole cards: ";

    for (int i=0; i<4; i++) {
        std::vector<int> hole_cards;

        if (player==1) for (int k=0; k<9; k++) suits[i] = suits[i] >> 1;

        for (int k=0; k<9; k++) {
            if (suits[i] % 2) hole_cards.push_back(k);
            suits[i] = suits[i] >> 1;
        }

        if (player==0) for (int k=0; k<9; k++) suits[i] = suits[i] >> 1;

        if (hole_cards.size() > 0) {
            for (int j=0; j<hole_cards.size(); j++) {
                ss << card_names[hole_cards[j]] << suit_names[i];
            }
        }
    }
    ss << "\n";

    // Community cards
    ss << "Community cards: ";
    
    for (int i=0; i<4; i++) {
        std::vector<int> comm_cards;

        for (int k=0; k<9; k++) {
            if (suits[i] % 2) comm_cards.push_back(k);
            suits[i] = suits[i] >> 1;
        }

        if (comm_cards.size() > 0) {
            for (int j=0; j<comm_cards.size(); j++) {
                ss << card_names[comm_cards[j]] << suit_names[i];
            }
        }
    }
    ss << "\n";
    
    return ss.str();
}

bool GameState::operator==(const GameState& other) const {
    return suita == other.suita && suitb == other.suitb && 
           suitc == other.suitc && suitd == other.suitd && player == other.player;
}

int GameState::get_num_actions() const {
    // Implementation here
    return 0;
}