#include "game_state.h"
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>

GameState::GameState(uint32_t suita, 
                     uint32_t suitb, 
                     uint32_t suitc, 
                     uint32_t suitd, 
                     uint8_t  turn,
                     uint8_t  rivr,
                     uint8_t  flop_history, 
                     uint8_t  turn_history, 
                     uint8_t  rivr_history,
                     bool is_information_set,
                     bool player)
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
                     is_information_set(is_information_set),
                     player(player)

{
    if (suita < suitb || suitb < suitc || suitc < suitd) throw std::invalid_argument("Suits not in correct order.");
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
/**
    * @param 'flop_history' 8-bit integer representing betting history on the flop:
    *                       - Bit 0: Flag for flop appearance (0: not yet, 1: appeared)
    *                       - Bits 1-2: Player 0's initial action
    *                                -->  01: check
    *                                -->  10: bet
    *                                -->  11: fold (preflop)
    *                       - Bits 3-4: Player 1's response
    *                                -->  01: check/call
    *                                -->  10: bet/raise
    *                                -->  11: fold
    *                       - Bits 5-6: Player 0's response to bet/raise
    *                                -->  01: call
    *                                -->  10: raise (check raise)
    *                                -->  11: fold
    *                       - Bit 7: Player 1's response to check raise
    *                                -->   1: call
    *                                -->   all 8 1's: fold
    * @param 'turn_history' 8-bit integer representing betting history on the turn:
    *                       - Bit 0: Flag for turn appearance (0: not yet, 1: appeared)
    *                       - Bits 1-2: Player 0's action
    *                                -->  01: check
    *                                -->  10: bet
    *                       - Bits 3-4: Player 1's response
    *                                -->  01: check
    *                                -->  10: call/bet
    *                                -->  11: fold
    *                       - Bits 5-7: Unused
    * @param 'rivr_history' 8-bit integer representing betting history on the river:
    *                       - Bit 0: Flag for river appearance (0: not yet, 1: appeared)
    *                       - Bits 1-2: Player 0's initial action
    *                                -->  01: check
    *                                -->  10: bet
    *                       - Bits 3-4: Player 1's response
    *                                -->  01: check/call
    *                                -->  10: bet/raise
    *                                -->  11: fold
    *                       - Bits 5-6: Player 0's response to raise
    *                                -->  01: call
    *                                -->  11: fold
    *                       - Bit 7: Player 1's response to check raise
    *                                -->   1: call
    *                                -->   all 8 1's: fold
    */
bool GameState::is_terminal_node() const {

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t river_action = rivr_history >> 1;

    // Flop terminals
    if (flop_action == 0b11 ||          // fold preflop
        flop_action == 0b1110 ||        // bet fold
        flop_action == 0b111010 ||      // bet raise fold
        flop_action == 0b111001 ||      // check bet fold
        flop_action == 0b1111111)       // check bet raise fold
        return true;

    // Turn terminals
    if (turn_action == 0b1110 ||        // bet fold
        turn_action == 0b111001)        // check bet fold
        return true;

    // River terminals
    if (river_action == 0b101 ||        // check check
        river_action == 0b110 ||        // bet call
        river_action == 0b1110 ||       // bet fold
        river_action == 0b111010 ||     // bet raise fold
        river_action == 0b11010 ||      // bet raise call
        river_action == 0b1111111 ||    // check bet raise fold (all 1's)
        river_action == 0b1101001 ||    // check bet raise call
        river_action == 0b11001)        // check bet call
        return true;

    return false;
}

int GameState::best_hand(bool player) const {
    
}

bool GameState::showdown() const {
    /**
     * 0: high card
     * 1: pair
     * 2: two pair
     * 3: three of a kind
     * 4: straight
     * 5: flush
     * 6: full house
     * 7: quads
     * 8: straight flush
     */
}

int GameState::pot_size() const {

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t river_action = rivr_history >> 1;

    int pot = 20;

    //   flop bet call             flop check bet call
    if ((flop_action == 0b110) || (flop_action == 0b11001)) pot *= 3;

    //   flop bet raise call        flop check bet raise call
    if ((flop_action == 0b1101) || (flop_action == 0b1101001)) pot *= 6;

    //   turn bet call             turn check bet call
    if ((turn_action == 0b110) || (turn_action == 0b11001)) pot *= 3;

    //   river bet call             river check bet call
    if ((river_action == 0b110) || (river_action == 0b11001)) pot *= 2;

    //   river bet raise call          river check bet raise call
    if ((river_action == 0b110100) || (river_action == 0b1101001)) pot = 800;

    return pot;
}

int GameState::utility(int player) const {
    if (!is_terminal_node()) throw std::invalid_argument("Non-terminal node passed to utility function.");

    //  fold preflop
    if (flop_history == 0b110) return (player == 0) ? -5 : 5; 

    //  bet fold
    if (flop_history == 0b11100) return (player == 0) ? 10 : -10;

    //  bet raise fold
    if (flop_history == 0b1110100) return (player == 0) ? -20 : 20;
    
    //  bet fold
    if (turn_history == 0b11100) return (player == 0) ? pot_size() : -1 * pot_size();
    
    //  check check
    if (rivr_history == 0b1010) return (player == 0) ? 0 : 0;

    //  bet call
    if (rivr_history == 0b1100) return 0;

    //  bet fold
    if (rivr_history == 0b11100) return 0;

    //  bet raise fold
    if (rivr_history == 0b1110100) return 0;

    //  bet raise call
    if (rivr_history == 0b110100) return 0;

    return 1;
}

int GameState::get_num_actions() const {
    // Implementation here
    return 0;
}