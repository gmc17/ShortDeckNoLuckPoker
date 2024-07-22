#include "game_state.h"
#include <sstream>
#include <vector>
#include <array>
#include <stdexcept>
#include <initializer_list>
#include <iostream>
#include <bitset>

GameState::GameState(uint32_t suita, 
                     uint32_t suitb, 
                     uint32_t suitc, 
                     uint32_t suitd, 
                     uint8_t  turn,
                     uint8_t  rivr,
                     uint8_t  flop_history, 
                     uint8_t  turn_history, 
                     uint8_t  rivr_history,
                     bool call_preflop,
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
                     call_preflop(call_preflop),
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
    ss << "SB cards:   ";

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
    ss << "BB cards:   ";

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

    // Flop cards
    ss << "Flop cards: ";
    
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

    if (turn_history % 2) {
        ss << "Turn card:  ";
        
        int turn_rank = turn & 0b1111;
        int turn_suit = (turn>>4) & 0b11;

        ss << card_names[turn_rank] << suit_names[turn_suit];
    }

    ss << "\n";

    if (rivr_history % 2) {
        ss << "River card: ";
        
        int rivr_rank = rivr & 0b1111;
        int rivr_suit = (rivr>>4) & 0b11;

        ss << card_names[rivr_rank] << suit_names[rivr_suit];
    }

    ss << "\n";
    
    return ss.str();
}

bool GameState::operator==(const GameState& other) const {
    return suita == other.suita && 
           suitb == other.suitb && 
           suitc == other.suitc && 
           suitd == other.suitd &&
           turn == other.turn &&
           rivr == other.rivr &&
           flop_history == other.flop_history &&
           turn_history == other.turn_history &&
           rivr_history == other.rivr_history &&
           call_preflop == other.call_preflop &&
           is_information_set == other.is_information_set &&
           player == other.player;
}

bool GameState::is_terminal_node() const {

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t rivr_action = rivr_history >> 1;

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
    if (rivr_action == 0b101 ||        // check check
        rivr_action == 0b110 ||        // bet call
        rivr_action == 0b1110 ||       // bet fold
        rivr_action == 0b111010 ||     // bet raise fold
        rivr_action == 0b11010 ||      // bet raise call
        rivr_action == 0b1111111 ||    // check bet raise fold (all 1's)
        rivr_action == 0b1101001 ||    // check bet raise call
        rivr_action == 0b11001 ||      // check bet call
        rivr_action == 0b111001)       // check bet fold
        return true;

    return false;
}

bool GameState::is_chance_node() const {
    //  Preflop
    if (call_preflop == 1 && 
        flop_history == 0) 
        return true;

    //  Flop
    if ((flop_history == 0b1011 ||        // check check
         flop_history == 0b1101 ||        // bet call
         flop_history == 0b110101 ||      // bet raise call
         flop_history == 0b11010011 ||    // check bet raise call
         flop_history == 0b110011) &&     // check bet call
         turn_history % 2 == 0)           // ensure turn has not already happened       
        return true;

    //  Turn
    if ((turn_history == 0b101 ||          // check check
         turn_history == 0b110) &&         // bet call
         rivr_history % 2 == 0)           // ensure river has not already happened  
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

    if (turn_history % 2) {
        int turn_rank = turn & 0b1111;
        int turn_suit = (turn>>4) & 0b11;
        
        if (turn_suit == 0) suita_all |= (0b1<<turn_rank);
        if (turn_suit == 0b1) suitb_all |= (0b1<<turn_rank);
        if (turn_suit == 0b10) suitc_all |= (0b1<<turn_rank);
        if (turn_suit == 0b11) suitd_all |= (0b1<<turn_rank);
    }
    
    if (rivr_history % 2) {
        int rivr_rank = rivr & 0b1111;
        int rivr_suit = (rivr>>4) & 0b11;
        
        if (rivr_suit == 0) suita_all |= (0b1<<rivr_rank);
        if (rivr_suit == 0b1) suitb_all |= (0b1<<rivr_rank);
        if (rivr_suit == 0b10) suitc_all |= (0b1<<rivr_rank);
        if (rivr_suit == 0b11) suitd_all |= (0b1<<rivr_rank);
    }

    /************************* Straight flush *************************/

    for (int i=0; i<6; i++) {
        if (((suita_all & straight_masks[i]) == straight_masks[i]) ||
            ((suitb_all & straight_masks[i]) == straight_masks[i]) ||
            ((suitc_all & straight_masks[i]) == straight_masks[i]) || 
            ((suitd_all & straight_masks[i]) == straight_masks[i])) {
            return 900000 + (6-i)*10000;
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

    if (kicker != -1 && best != -1) return 800000 + best*10000 + kicker*1000;

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

    if (pair != -1 && trips != -1) return 700000 + trips*10000 + pair*1000;

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

    if (best != -1) return 600000 + best*10000;

    /************************* Straight *************************/

    uint8_t all_cards = suita_all | suitb_all | suitc_all | suitd_all;

    for (int i=0; i<6; i++) {
        if ((all_cards & straight_masks[i]) == straight_masks[i]) {
            return 500000 + (6-i)*10000;
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
        return 400000 + trips*10000 + kicker1*1000 + kicker2*100;
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
        return 300000 + pair1*10000 + pair2*1000 + kicker*100;
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
        return 200000 + pair*10000 + kicker1*1000 + kicker2*100 + kicker3*10;
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
        return 100000 + kicker1*10000 + kicker2*1000 + kicker3*100 + kicker*10 + pair;
    }

    throw std::invalid_argument("Player has less than 5 cards; cannot calculate best hand.");

    return -1;
}

int GameState::showdown() const {
    if (best_hand(0) == best_hand(1)) return 0;

    return (best_hand(0) > best_hand(1)) ? pot_size()/2 : -1 * pot_size()/2;
}

int GameState::pot_size() const {

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t rivr_action = rivr_history >> 1;

    int pot = 20;

    //   flop bet call             flop check bet call
    if ((flop_action == 0b110) || (flop_action == 0b11001)) pot *= 3;

    //   flop bet raise call        flop check bet raise call
    if ((flop_action == 0b1101) || (flop_action == 0b1101001)) pot *= 6;

    //   turn bet call             turn check bet call
    if ((turn_action == 0b110) || (turn_action == 0b11001)) pot *= 3;

    //   river bet call             river check bet call
    if ((rivr_action == 0b110) || (rivr_action == 0b11001)) pot *= 2;

    //   river bet raise call          river check bet raise call
    if ((rivr_action == 0b110100) || (rivr_action == 0b1101001)) pot = 800;

    return pot;
}

int GameState::utility(int player) const {
    if (!is_terminal_node()) throw std::invalid_argument("Non-terminal node passed to utility function.");

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t rivr_action = rivr_history >> 1;

    //  fold preflop
    if (flop_action == 0b11) return (player == 0) ? -5 : 5; 

    //  bet fold
    if (flop_action == 0b1110) return (player == 0) ? 10 : -10;

    //  bet raise fold
    if (flop_action == 0b111010) return (player == 0) ? -30 : 30;

    //  check bet raise fold
    if (flop_action == 0b1111111) return (player == 0) ? 30 : -30;

    //  check bet fold
    if (flop_action == 0b111001) return (player == 0) ? -10 : 10;
    
    //  bet fold
    if (turn_action == 0b1110) return (player == 0) ? pot_size()/2 : -1 * pot_size()/2;
    
    //  check check
    if (rivr_action == 0b101) return (player == 0) ? showdown() : -1 * showdown();

    //  bet call
    if (rivr_action == 0b110) return (player == 0) ? showdown() : -1 * showdown();

    //  bet fold
    if (rivr_action == 0b1110) return (player == 0) ? pot_size()/2 : -1 * pot_size()/2;

    //  bet raise fold
    if (rivr_action == 0b111010) return (player == 0) ? -1 * pot_size()/2 : pot_size()/2;;

    //  bet raise call
    if (rivr_action == 0b11010) return (player == 0) ? showdown() : -1 * showdown();

    //  check bet fold
    if (rivr_action == 0b111001) return (player == 0) ? -1 * pot_size()/2 : showdown();

    //  check bet call
    if (rivr_action == 0b11001) return (player == 0) ? showdown() : -1 * showdown();

    //  check bet raise fold
    if (rivr_action == 0b1111111) return (player == 0) ? pot_size()/2 : -1 * pot_size()/2;

    //  check bet raise call
    if (rivr_action == 0b1101001) return (player == 0) ? showdown() : -1 * showdown();
     
    throw std::invalid_argument("Terminal history not able to be evaluated for utility.");
    
    return -1;
}

int GameState::num_actions() const {
    
    // in response to check: [check, bet]
    // in response to bet: [fold, call, raise]
    // in response to raise: [fold, call]

    /************************* Preflop *************************/
    if (flop_history == 0) return 2;

    /************************* Flop *************************/
    if (flop_history == 0b1) return 2; // first flop action
    if (flop_history == 0b11) return 2; // check
    if (flop_history == 0b101) return 3; // bet
    if (flop_history == 0b10101) return 2; // bet raise
    if (flop_history == 0b10011) return 3; // check bet
    if (flop_history == 0b1010011) return 2; // check bet raise

    /************************* Turn *************************/
    if (turn_history == 0b1) return 2; // first turn action
    if (turn_history == 0b11) return 2; // check
    if (turn_history == 0b101) return 2; // bet

    /************************* River *************************/
    if (rivr_history == 1) return 2; // first river action
    if (rivr_history == 0b11) return 2; // check
    if (rivr_history == 0b101) return 3; // bet
    if (rivr_history == 0b10101) return 2; // bet raise
    if (rivr_history == 0b10011) return 3; // check bet
    if (rivr_history == 0b1010011) return 2; // check bet raise

    throw std::invalid_argument("Tried to find number of actions for non-action node.");

    return -1;
}

int GameState::num_chance_actions() const {
    //  Preflop
    if (call_preflop == 1 && 
        flop_history == 0) 
        return (34*33*32)/6;

    //  Flop
    if ((flop_history == 0b1011 ||        // check check
         flop_history == 0b1101 ||        // bet call
         flop_history == 0b110101 ||      // bet raise call
         flop_history == 0b11010011 ||    // check bet raise call
         flop_history == 0b110011) &&     // check bet call
         turn_history % 2 == 0)           // ensure turn has not already happened       
        return 31;

    //  Turn
    if ((turn_history == 0b101 ||          // check check
         turn_history == 0b110) &&         // bet call
         rivr_history % 2 == 0)           // ensure river has not already happened  
        return 30;

    return 0;
}

void GameState::apply_action(int action) {
    
    /************************* Preflop *************************/

    if (call_preflop == 0 && flop_history == 1) {
        if (action == 0) {
            flop_history = 0b11;  // fold
            player = 1;
        } else if (action == 1) {
            flop_history = 0b101; // call
            player = 0;
            call_preflop = 1;
        }
    } 

    /************************* Flop *************************/

    if (flop_history == 1) {
        if (action == 0) {
            flop_history = 0b11;  // check
            player = 1;
        } else if (action == 1) {
            flop_history = 0b101; // bet
            player = 1;
        }
    }

    if (flop_history == 0b11) {
        if (action == 0) {
            flop_history = 0b1011;  // check check
            player = 0;
        } else if (action == 1) {
            flop_history = 0b10011; // check bet
            player = 0;
        }
    }

    if (flop_history == 0b101) {
        if (action == 0) {
            flop_history = 0b11101;  // bet fold
            player = 0;
        } else if (action == 1) {
            flop_history = 0b1101; // bet call
            player = 0;
        } else if (action == 2) {
            flop_history = 0b10101; // bet raise
            player = 0;
        }
    }

    if (flop_history == 0b10101) {
        if (action == 0) {
            flop_history = 0b1110101;  // bet raise fold
            player = 1;
        } else if (action == 1) {
            flop_history = 0b110101; // bet raise call
            player = 0;
        }
    }

    if (flop_history == 0b10011) {
        if (action == 0) {
            flop_history = 0b1110011;  // check bet fold
            player = 1;
        } else if (action == 1) {
            flop_history = 0b110011; // check bet call
            player = 0;
        } else if (action == 2) {
            flop_history = 0b1010011; // check bet raise
            player = 1;
        }
    }

    if (flop_history == 0b1010011) {
        if (action == 0) {
            flop_history = 0b11111111;  // check bet raise fold
            player = 1;
        } else if (action == 1) {
            flop_history = 0b11010011; // check bet raise call
            player = 0;
        }
    }
    
    // /************************* Turn *************************/

    if (turn_history == 1) {
        if (action == 0) {
            turn_history = 0b11;  // check
            player = 1;
        } else if (action == 1) {
            turn_history = 0b101; // bet
            player = 1;
        }
    }

    if (turn_history == 0b11) {
        if (action == 0) {
            turn_history = 0b1011;  // check check
            player = 0;
        } else if (action == 1) {
            turn_history = 0b10011; // check bet
            player = 0;
        }
    }

    if (turn_history == 0b101) {
        if (action == 0) {
            turn_history = 0b11101;  // bet fold
            player = 0;
        } else if (action == 1) {
            turn_history = 0b1101; // bet call
            player = 0;
        }
    }

    // /************************* River *************************/
    
    if (rivr_history == 1) {
        if (action == 0) {
            rivr_history = 0b11;  // check
            player = 1;
        } else if (action == 1) {
            rivr_history = 0b101; // bet
            player = 1;
        }
    }

    if (rivr_history == 0b11) {
        if (action == 0) {
            rivr_history = 0b1011;  // check check
            player = 0;
        } else if (action == 1) {
            rivr_history = 0b10011; // check bet
            player = 0;
        }
    }

    if (rivr_history == 0b101) {
        if (action == 0) {
            rivr_history = 0b11101;  // bet fold
            player = 0;
        } else if (action == 1) {
            rivr_history = 0b1101; // bet call
            player = 0;
        } else if (action == 2) {
            rivr_history = 0b10101; // bet raise
            player = 0;
        }
    }

    if (rivr_history == 0b10101) {
        if (action == 0) {
            rivr_history = 0b1110101;  // bet raise fold
            player = 1;
        } else if (action == 1) {
            rivr_history = 0b110101; // bet raise call
            player = 0;
        }
    }

    if (rivr_history == 0b10011) {
        if (action == 0) {
            rivr_history = 0b1110011;  // check bet fold
            player = 1;
        } else if (action == 1) {
            rivr_history = 0b110011; // check bet call
            player = 0;
        } else if (action == 2) {
            rivr_history = 0b1010011; // check bet raise
            player = 1;
        }
    }

    if (rivr_history == 0b1010011) {
        if (action == 0) {
            rivr_history = 0b11111111;  // check bet raise fold
            player = 1;
        } else if (action == 1) {
            rivr_history = 0b11010011; // check bet raise call
            player = 0;
        }
    }
}

void GameState::apply_chance_action(int action) {
    /**
     * Flop
     * 
     * suita 000000000
     * suitb 000000000
     * suitc 000000000
     * suitd 000000000
     * 
     * 
     * 
     * 
     * 
     * 
     */
    int count = 0;
    for (uint64_t i=0; i<0xFFFFFFF; i++) {
        if (__builtin_popcount(i)==3) count++;
    }
    std::cout << "count: " << count << "\n";
}