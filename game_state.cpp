#include "game_state.h"
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>
#include <initializer_list>
#include <iostream>

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
    // if (suita < suitb || suitb < suitc || suitc < suitd) throw std::invalid_argument("Suits not in correct order.");
}

std::string GameState::to_string() const {
    std::stringstream ss;

    std::array<uint32_t, 4> suits = {suita, suitb, suitc, suitd};
    std::array<std::string,  4> suit_names = {"s", "h", "d", "c"};
    std::array<std::string, 13> card_names = {"6", "7", "8", "9",
                                              "10", "J", "Q", "K",
                                              "A"};
    
    // Small blind cards
    ss << "SB cards: ";

    for (int i=0; i<4; i++) {
        std::vector<int> hole_cards;

        for (int k=0; k<9; k++) {
            if (suits[i] % 2) hole_cards.push_back(k);
            suits[i] = suits[i] >> 1;
        }

        if (hole_cards.size() > 0) {
            for (int j=0; j<hole_cards.size(); j++) {
                ss << card_names[hole_cards[j]] << suit_names[i];
            }
        }
    }
    ss << "\n";

    // Big blind cards
    ss << "BB cards: ";

    for (int i=0; i<4; i++) {
        std::vector<int> hole_cards;

        for (int k=0; k<9; k++) {
            if (suits[i] % 2) hole_cards.push_back(k);
            suits[i] = suits[i] >> 1;
        }

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

// Ordered best-to-worst
std::array<int, 6> straight_masks = {0b111110000, 0b011111000, 0b001111100, 0b000111110, 0b000011111, 0b100001111};
std::array<int, 9> single_masks = {0b100000000, 0b010000000, 0b001000000, 0b000100000, 0b000010000, 0b000001000, 0b000000100, 0b000000010, 0b000000001};

int GameState::best_hand(bool p) const {
    uint16_t suita_player;
    uint16_t suitb_player;
    uint16_t suitc_player;
    uint16_t suitd_player;

    if (p==0) suita_player = suita & 0b111111111;
    else suita_player = (suita>>9) & 0b111111111;
    if (p==0) suitb_player = suitb & 0b111111111;
    else suitb_player = (suitb>>9) & 0b111111111;
    if (p==0) suitc_player = suitc & 0b111111111;
    else suitc_player = (suitc>>9) & 0b111111111;
    if (p==0) suitd_player = suitd & 0b111111111;
    else suitd_player = (suitd>>9) & 0b111111111;

    uint16_t suita_all = (suita>>18) | suita_player;
    uint16_t suitb_all = (suitb>>18) | suitb_player;
    uint16_t suitc_all = (suitc>>18) | suitc_player;
    uint16_t suitd_all = (suitd>>18) | suitd_player;

    /************************* Straight flush *************************/

    for (int i=0; i<6; i++) {
        if (((suita_all & straight_masks[i]) == straight_masks[i]) ||
            ((suitb_all & straight_masks[i]) == straight_masks[i]) ||
            ((suitc_all & straight_masks[i]) == straight_masks[i]) || 
            ((suitd_all & straight_masks[i]) == straight_masks[i])) {
            return 800000 + (6-i)*10000;
        }
    }

    /************************* Four of a kind *************************/
    
    int best = -1;
    int kicker = -1;

    for (int i=0; i<9; i++) {
        if ((suita_all & single_masks[i]) && 
            (suitb_all & single_masks[i]) &&
            (suitc_all & single_masks[i]) &&
            (suitd_all & single_masks[i])) {
            best = i;
            for (int k=8; k>=0; k--) {
                if ((k != i) && 
                    ((suita_all & single_masks[k]) || 
                     (suitb_all & single_masks[k]) ||
                     (suita_all & single_masks[k]) ||
                     (suita_all & single_masks[k]))) {
                    kicker = k;
                    break;
                }
            }
        }
    }

    if (kicker != -1 && best != -1) return 700000 + best*10000 + kicker*1000;

    /************************* Full House *************************/

    int trips = -1;
    int pair = -1;

    // Find highest trips
    for (int i=0; i<9; i++) {
        int count = ((suita_all & single_masks[i]) ? 1 : 0) +
                    ((suitb_all & single_masks[i]) ? 1 : 0) +
                    ((suitc_all & single_masks[i]) ? 1 : 0) +
                    ((suitd_all & single_masks[i]) ? 1 : 0);
        if (count >= 3) {
            trips = 9-i;
            break;
        }
    }

    // Find highest pair
    if (trips != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == trips) continue; // skip trips we already found
            int count = ((suita_all & single_masks[i]) ? 1 : 0) +
                        ((suitb_all & single_masks[i]) ? 1 : 0) +
                        ((suitc_all & single_masks[i]) ? 1 : 0) +
                        ((suitd_all & single_masks[i]) ? 1 : 0);
            if (count >= 2) {
                pair = 9-i;
                break;
            }
        }
    }

    if (pair != -1 && trips != -1) return 600000 + trips*10000 + pair*1000;

    /************************* Flush *************************/
    
    best = -1;

    std::array<uint16_t, 4> suit_cards = {suita_all, suitb_all, suitc_all, suitd_all};

    for (int i=0; i<4; i++) {
        if (__builtin_popcount(suit_cards[i]) >= 5) {
            // find highest card in the flush
            for (int k=0; k<9; k++) {
                if (suit_cards[i] & single_masks[k]) {
                    best = 9-k;
                    break;
                }
            }
        }
    }

    if (best != -1) return 500000 + best*10000;

    /************************* Straight *************************/

    uint8_t all_cards = suita_all | suitb_all | suitc_all | suitd_all;

    for (int i=0; i<6; i++) {
        if ((all_cards & straight_masks[i]) == straight_masks[i]) {
            return 400000 + (6-i)*10000;
        }
    }

    /************************* Three of a kind *************************/

    trips = -1;
    int kicker1 = -1;
    int kicker2 = -1;

    // Find highest trips
    for (int i=0; i<9; i++) {
        int count = ((suita_all & single_masks[i]) ? 1 : 0) +
                    ((suitb_all & single_masks[i]) ? 1 : 0) +
                    ((suitc_all & single_masks[i]) ? 1 : 0) +
                    ((suitd_all & single_masks[i]) ? 1 : 0);
        if (count >= 3) {
            trips = 9-i;
            break;
        }
    }

    // Find highest kickers
    if (trips != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == trips) continue; // skip trips we already found
            if (all_cards & single_masks[i]) {
                if (kicker1 == -1) {
                    kicker1 = 9-i;
                } else if (kicker2 == -1) {
                    kicker2 = 9-i;
                    break;
                }
            }
        }
    }

    if (kicker2 != -1 && kicker1 != -1 && trips != -1) {
        return 300000 + trips*10000 + kicker1*1000 + kicker2*100;
    }

    /************************* Two Pair *************************/

    int pair1 = -1;
    int pair2 = -1;
    kicker = -1;

    // Find highest pairs
    for (int i=0; i<9; i++) {
        int count = ((suita_all & single_masks[i]) ? 1 : 0) +
                    ((suitb_all & single_masks[i]) ? 1 : 0) +
                    ((suitc_all & single_masks[i]) ? 1 : 0) +
                    ((suitd_all & single_masks[i]) ? 1 : 0);
        if (count >= 2) {
            if (pair1==-1) {
                pair1 = 9-i;
            } else if (pair2==-1) {
                pair2 = 9-i;
                break;
            }
        }
    }    

    // Find best kicker
    if (pair1 != -1 && pair2 != -1) {
        for (int i=0; i<9; i++) {
            if (((9-i)!=pair1) && ((9-i)!=pair2)) {
                if (all_cards & single_masks[i]) {
                    kicker = 9-i;
                    break;
                }
            }
        }
    }

    if (kicker != -1 && pair2 != -1 && pair1 != -1) {
        return 200000 + pair1*10000 + pair2*1000 + kicker*100;
    }

    /************************* Pair *************************/

    pair = -1;
    kicker1 = -1;
    kicker2 = -1;
    int kicker3 = -1;

    // Find highest pair
    for (int i=0; i<9; i++) {
        int count = ((suita_all & single_masks[i]) ? 1 : 0) +
                    ((suitb_all & single_masks[i]) ? 1 : 0) +
                    ((suitc_all & single_masks[i]) ? 1 : 0) +
                    ((suitd_all & single_masks[i]) ? 1 : 0);
        if (count >= 2) {
            pair = 9-i;
            break;
        }
    }    

    // Find best kickers
    if (pair != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == pair) continue;
            if (all_cards & single_masks[i]) {
                if (kicker1 == -1) {
                    kicker1 = 9-i;
                } else if (kicker2 == -1) {
                    kicker2 = 9-i;
                } else if (kicker3 == -1) {
                    kicker3 = 9-i;
                    break;
                }
            }
        }
    }

    if (kicker3 != -1 && kicker2 != -1 && kicker1 != -1 && pair != -1) {
        return 100000 + pair*10000 + kicker1*1000 + kicker2*100 + kicker3*10;
    }

    /************************* High Card *************************/

    kicker1 = -1;
    kicker2 = -1;
    kicker3 = -1;
    kicker = -1;
    pair = -1;

    // Find best kickers
    for (int i=0; i<9; i++) {
        if (all_cards & single_masks[i]) {
            if (kicker1 == -1) {
                kicker1 = 9-i;
            } else if (kicker2 == -1) {
                kicker2 = 9-i;
            } else if (kicker3 == -1) {
                kicker3 = 9-i;
            } else if (kicker == -1) {
                kicker = 9-i;
            } else if (pair == -1) {
                pair = 9-i;
                break;
            }
        }
    }

    if (kicker1 != -1 && kicker2 != -1 && kicker3 != -1 && kicker != -1 && pair != -1) {
        return kicker1*10000 + kicker2*1000 + kicker3*100 + kicker*10 + pair;
    }

    throw std::invalid_argument("Player has less than 5 cards; cannot calculate best hand.");

    return -1;
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
    return 0;
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