#include "game_state.h"
#include "ars_table.h"
#include "constants.h"
#include "info_set.h"
#include "cfr.h"

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
                     player(player) {}

ARSTable ars_table;
std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE> regret_sum;
std::array<std::array<float, 3>, STRATEGY_ARRAY_SIZE> strategy_sum;

std::string GameState::to_string() const {
    std::stringstream ss;

    std::array<uint32_t, 4> suits = {suita, suitb, suitc, suitd};
    
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
                ss << CARD_NAMES[hole_cards[j]] << SUIT_NAMES[i];
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
                ss << CARD_NAMES[hole_cards[j]] << SUIT_NAMES[i];
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
                ss << CARD_NAMES[comm_cards[j]] << SUIT_NAMES[i];
            }
        }
    }
    ss << "\n";

    ss << "Turn card:  ";

    if (turn_history % 2) {
        int turn_rank = turn & 0b1111;
        int turn_suit = (turn>>4) & 0b11;

        ss << CARD_NAMES[turn_rank] << SUIT_NAMES[turn_suit];
    }

    ss << "\n";

    ss << "River card: ";

    if (rivr_history % 2) {
        int rivr_rank = rivr & 0b1111;
        int rivr_suit = (rivr>>4) & 0b11;

        ss << CARD_NAMES[rivr_rank] << SUIT_NAMES[rivr_suit];
    }

    ss << "\n";

    std::bitset<8> binary_flop_history(flop_history);
    std::bitset<8> binary_turn_history(turn_history);
    std::bitset<8> binary_rivr_history(rivr_history);

    ss << "Flop history: " << binary_flop_history << "\n";
    ss << "Turn history: " << binary_turn_history << "\n";
    ss << "Rivr history: " << binary_rivr_history << "\n";
    
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
           player == other.player;
}

bool GameState::is_terminal() const {

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t rivr_action = rivr_history >> 1;

    // Flop terminals
    if ((flop_action == 0b11) ||          // fold preflop
        (flop_action == 0b1110) ||        // bet fold
        (flop_action == 0b111010) ||      // bet raise fold
        (flop_action == 0b111001) ||      // check bet fold
        (flop_action == 0b1111111))       // check bet raise fold
        return true;

    // Turn terminals
    if ((turn_action == 0b1110) ||        // bet fold
        (turn_action == 0b111001))        // check bet fold
        return true;

    // River terminals
    if ((rivr_action == 0b101) ||        // check check
        (rivr_action == 0b110) ||        // bet call
        (rivr_action == 0b1110) ||       // bet fold
        (rivr_action == 0b111010) ||     // bet raise fold
        (rivr_action == 0b11010) ||      // bet raise call
        (rivr_action == 0b1111111) ||    // check bet raise fold (all 1's)
        (rivr_action == 0b1101001) ||    // check bet raise call
        (rivr_action == 0b11001) ||      // check bet call
        (rivr_action == 0b111001))       // check bet fold
        return true;

    return false;
}

bool GameState::is_chance() const {
    //  Preflop
    if ((call_preflop == 1) && 
        (flop_history == 0)) 
        return true;

    //  Flop
    if (((turn_history % 2) == 0) &&          // ensure turn has not already happened       
       ((flop_history == 0b1011) ||        // check check
        (flop_history == 0b1101) ||        // bet call
        (flop_history == 0b110101) ||      // bet raise call
        (flop_history == 0b11010011) ||    // check bet raise call
        (flop_history == 0b110011)))        // check bet call
        return true;

    //  Turn
    if (((rivr_history % 2) == 0) &&         // ensure river has not already happened
       ((turn_history == 0b1011) ||        // check check
        (turn_history == 0b1101) ||        // bet call
        (turn_history == 0b110011)))      // check bet call  
        return true;

    return false;
}

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
        if (((suita_all & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) ||
            ((suitb_all & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) ||
            ((suitc_all & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) || 
            ((suitd_all & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i])) {
            return 900000 + (6-i)*10000;
        }
    }

    /************************* Four of a kind *************************/
    
    int best = -1;
    int kicker = -1;

    for (int i=0; i<9; i++) {
        if ((suita_all & SINGLE_MASKS[i]) && 
            (suitb_all & SINGLE_MASKS[i]) &&
            (suitc_all & SINGLE_MASKS[i]) &&
            (suitd_all & SINGLE_MASKS[i])) {
            best = i;
            for (int k=8; k>=0; k--) {
                if ((k != i) && 
                    ((suita_all & SINGLE_MASKS[k]) || 
                     (suitb_all & SINGLE_MASKS[k]) ||
                     (suitc_all & SINGLE_MASKS[k]) ||
                     (suitd_all & SINGLE_MASKS[k]))) {
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
        int count = ((suita_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitb_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitc_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitd_all & SINGLE_MASKS[i]) ? 1 : 0);
        if (count >= 3) {
            trips = 9-i;
            break;
        }
    }

    // Find highest pair
    if (trips != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == trips) continue; // skip trips we already found
            int count = ((suita_all & SINGLE_MASKS[i]) ? 1 : 0) +
                        ((suitb_all & SINGLE_MASKS[i]) ? 1 : 0) +
                        ((suitc_all & SINGLE_MASKS[i]) ? 1 : 0) +
                        ((suitd_all & SINGLE_MASKS[i]) ? 1 : 0);
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
                if (suit_cards[i] & SINGLE_MASKS[k]) {
                    best = 9-k;
                    break;
                }
            }
        }
    }

    if (best != -1) return 600000 + best*10000;

    /************************* Straight *************************/

    uint16_t all_cards = suita_all | suitb_all | suitc_all | suitd_all;

    for (int i=0; i<6; i++) {
        if ((all_cards & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) {
            return 500000 + (6-i)*10000;
        }
    }

    /************************* Three of a kind *************************/

    trips = -1;
    int kicker1 = -1;
    int kicker2 = -1;

    // Find highest trips
    for (int i=0; i<9; i++) {
        int count = ((suita_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitb_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitc_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitd_all & SINGLE_MASKS[i]) ? 1 : 0);
        if (count >= 3) {
            trips = 9-i;
            break;
        }
    }

    // Find highest kickers
    if (trips != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == trips) continue; // skip trips we already found
            if (all_cards & SINGLE_MASKS[i]) {
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
        int count = ((suita_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitb_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitc_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitd_all & SINGLE_MASKS[i]) ? 1 : 0);
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
                if (all_cards & SINGLE_MASKS[i]) {
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
        int count = ((suita_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitb_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitc_all & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suitd_all & SINGLE_MASKS[i]) ? 1 : 0);
        if (count >= 2) {
            pair = 9-i;
            break;
        }
    }    

    // Find best kickers
    if (pair != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == pair) continue;
            if (all_cards & SINGLE_MASKS[i]) {
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
        if (all_cards & SINGLE_MASKS[i]) {
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

    // throw std::invalid_argument("Player has less than 5 cards; cannot calculate best hand.");

    return -1;
}

float GameState::showdown() const {
    if (best_hand(0) == best_hand(1)) return 0.0f;

    return (best_hand(0) > best_hand(1)) ? pot_size() * 0.5f : pot_size() * -0.5f;
}

float GameState::pot_size() const {

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t rivr_action = rivr_history >> 1;

    float pot = 20.0f;

    //   flop bet call             flop check bet call
    if ((flop_action == 0b110) || (flop_action == 0b11001)) pot *= 3;

    //   flop bet raise call         flop check bet raise call
    if ((flop_action == 0b11010) || (flop_action == 0b1101001)) pot *= 6;

    //   flop bet raise fold          flop check bet raise fold
    if ((flop_action == 0b111010) || (flop_action == 0b11101001)) pot *= 3;


    //   turn bet call             turn check bet call
    if ((turn_action == 0b110) || (turn_action == 0b11001)) pot *= 3;


    //   river bet call            river check bet call
    if ((rivr_action == 0b110) || (rivr_action == 0b11001)) pot *= 2;

    //   river bet raise call        river check bet raise call
    if ((rivr_action == 0b11010) || (rivr_action == 0b1101001)) pot = 800;

    //   river bet raise fold         river check bet raise fold
    if ((rivr_action == 0b111010) || (rivr_action == 0b11101001)) pot *= 3;

    return pot;
}

float GameState::utility(bool p) const {
    if (!is_terminal()) exit(1);

    uint8_t flop_action = flop_history >> 1;  // Ignore appearance flag
    uint8_t turn_action = turn_history >> 1;
    uint8_t rivr_action = rivr_history >> 1;

    //  fold preflop
    if (flop_action == 0b11) return (p == 0) ? -5.0f : 5.0f; 

    //  bet fold
    if (flop_action == 0b1110) return (p == 0) ? 10.0f : -10.0f;

    //  check bet fold
    if (flop_action == 0b111001) return (p == 0) ? -10.0f : 10.0f;

    //  bet raise fold
    if (flop_action == 0b111010) return (p == 0) ? -30.0f : 30.0f;

    //  check bet raise fold
    if (flop_action == 0b1111111) return (p == 0) ? 30.0f : -30.0f;
    
    //  bet fold
    if (turn_action == 0b1110) return (p == 0) ? pot_size() * 0.5f : pot_size() * -0.5f;

    //  check bet fold
    if (turn_action == 0b111001) return (p == 0) ? pot_size() * -0.5f : pot_size() * 0.5f;
    
    //  check check
    if (rivr_action == 0b101) return (p == 0) ? showdown() : -1.0f * showdown();

    //  bet call
    if (rivr_action == 0b110) return (p == 0) ? showdown() : -1.0f * showdown();

    //  bet fold
    if (rivr_action == 0b1110) return (p == 0) ? pot_size() * 0.5f : pot_size() * -0.5f;

    //  bet raise fold
    if (rivr_action == 0b111010) return (p == 0) ? -1 * pot_size() * 0.5f : pot_size() * 0.5f;

    //  bet raise call
    if (rivr_action == 0b11010) return (p == 0) ? showdown() : -1.0f * showdown();

    //  check bet fold
    if (rivr_action == 0b111001) return (p == 0) ? pot_size() * -0.5f : pot_size() * 0.5f;

    //  check bet call
    if (rivr_action == 0b11001) return (p == 0) ? showdown() : -1.0f * showdown();

    //  check bet raise fold
    if (rivr_action == 0b1111111) return (p == 0) ? pot_size() * 0.5f : pot_size() * -0.5f;

    //  check bet raise call
    if (rivr_action == 0b1101001) return (p == 0) ? showdown() : -1.0f * showdown();
     
    // throw std::invalid_argument("Terminal history not able to be evaluated for utility.");
    
    return -1;
}

int GameState::num_actions() const {
    
    /************************* Preflop *************************/
    if (call_preflop == 0 && 
        flop_history == 0) return 2;

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
    if (turn_history == 0b10011) return 2; // check bet

    /************************* River *************************/
    if (rivr_history == 0b1) return 2; // first river action
    if (rivr_history == 0b11) return 2; // check
    if (rivr_history == 0b101) return 3; // bet
    if (rivr_history == 0b10101) return 2; // bet raise
    if (rivr_history == 0b10011) return 3; // check bet
    if (rivr_history == 0b1010011) return 2; // check bet raise

    std::ostringstream oss;
        oss << "Tried to find number of actions for non-action node. "
            << "flop_history: " << static_cast<int>(flop_history)
            << ", turn_history: " << static_cast<int>(turn_history)
            << ", rivr_history: " << static_cast<int>(rivr_history);

    throw std::invalid_argument(oss.str());
    
    return -1;
}

int GameState::num_chance_actions() const {
    //  Preflop
    if ((call_preflop == 1) && 
        (flop_history == 0)) {
        return 36 - (__builtin_popcount(suita) + __builtin_popcount(suitb) + __builtin_popcount(suitc) + __builtin_popcount(suitd));
    }
    //  Flop
    if (((turn_history % 2) == 0) &&          // ensure turn has not already happened       
        ((flop_history == 0b1011) ||        // check check
         (flop_history == 0b1101) ||        // bet call
         (flop_history == 0b110101) ||      // bet raise call
         (flop_history == 0b11010011) ||    // check bet raise call
         (flop_history == 0b110011)))  {      // check bet call
        return 29;
    }

    //  Turn
    if (((rivr_history % 2) == 0) &&         // ensure river has not already happened
        ((turn_history == 0b1011) ||        // check check
         (turn_history == 0b1101) ||        // bet call
         (turn_history == 0b110011)))      // check bet call             
        return 28;

    return 0;
}

void GameState::apply_action(int action) {
    
    /************************* Preflop *************************/

    if ((call_preflop == 0) && (flop_history == 0)) {
        if (action == 0) {
            flop_history = 0b111;  // fold
            return;
        } else if (action == 1) {
            call_preflop = 1; // call
            flop_history = 0;
            player = 0;
            return;
        }
    } 

    /************************* Flop *************************/

    if (flop_history == 1) {
        if (action == 0) {
            flop_history = 0b11;  // check
            player = 1;
            return;
        } else if (action == 1) {
            flop_history = 0b101; // bet
            player = 1;
            return;
        }
    }

    if (flop_history == 0b11) {
        if (action == 0) {
            flop_history = 0b1011;  // check check
            player = 0;
            return;
        } else if (action == 1) {
            flop_history = 0b10011; // check bet
            player = 0;
            return;
        }
    }

    if (flop_history == 0b101) {
        if (action == 0) {
            flop_history = 0b11101;  // bet fold
            return;
        } else if (action == 1) {
            flop_history = 0b1101; // bet call
            player = 0;
            return;
        } else if (action == 2) {
            flop_history = 0b10101; // bet raise
            player = 0;
            return;
        }
    }

    if (flop_history == 0b10101) {
        if (action == 0) {
            flop_history = 0b1110101;  // bet raise fold
            return;
        } else if (action == 1) {
            flop_history = 0b110101; // bet raise call
            player = 0;
            return;
        }
    }

    if (flop_history == 0b10011) {
        if (action == 0) {
            flop_history = 0b1110011;  // check bet fold
            return;
        } else if (action == 1) {
            flop_history = 0b110011; // check bet call
            player = 0;
            return;
        } else if (action == 2) {
            flop_history = 0b1010011; // check bet raise
            player = 1;
            return;
        }
    }

    if (flop_history == 0b1010011) {
        if (action == 0) {
            flop_history = 0b11111111;  // check bet raise fold
            return;
        } else if (action == 1) {
            flop_history = 0b11010011; // check bet raise call
            player = 0;
            return;
        }
    }
    
    // /************************* Turn *************************/

    if (turn_history == 1) {
        if (action == 0) {
            turn_history = 0b11;  // check
            player = 1;
            return;
        } else if (action == 1) {
            turn_history = 0b101; // bet
            player = 1;
            return;
        }
    }

    if (turn_history == 0b11) {
        if (action == 0) {
            turn_history = 0b1011;  // check check
            player = 0;
            return;
        } else if (action == 1) {
            turn_history = 0b10011; // check bet
            player = 0;
            return;
        }
    }

    if (turn_history == 0b101) {
        if (action == 0) {
            turn_history = 0b11101;  // bet fold
            return;
        } else if (action == 1) {
            turn_history = 0b1101; // bet call
            player = 0;
            return;
        }
    }

    if (turn_history == 0b10011) {
        if (action == 0) {
            turn_history = 0b1110011;  // check bet fold
            return;
        } else if (action == 1) {
            turn_history = 0b110011; // check bet call
            player = 0;
            return;
        }
    }

    // /************************* River *************************/
    
    if (rivr_history == 1) {
        if (action == 0) {
            rivr_history = 0b11;  // check
            player = 1;
            return;
        } else if (action == 1) {
            rivr_history = 0b101; // bet
            player = 1;
            return;
        }
    }

    if (rivr_history == 0b11) {
        if (action == 0) {
            rivr_history = 0b1011;  // check check
            player = 0;
            return;
        } else if (action == 1) {
            rivr_history = 0b10011; // check bet
            player = 0;
            return;
        }
    }

    if (rivr_history == 0b101) {
        if (action == 0) {
            rivr_history = 0b11101;  // bet fold
            return;
        } else if (action == 1) {
            rivr_history = 0b1101; // bet call
            player = 0;
            return;
        } else if (action == 2) {
            rivr_history = 0b10101; // bet raise
            player = 0;
            return;
        }
    }

    if (rivr_history == 0b10101) {
        if (action == 0) {
            rivr_history = 0b1110101;  // bet raise fold
            return;
        } else if (action == 1) {
            rivr_history = 0b110101; // bet raise call
            player = 0;
            return;
        }
    }

    if (rivr_history == 0b10011) {
        if (action == 0) {
            rivr_history = 0b1110011;  // check bet fold
            return;
        } else if (action == 1) {
            rivr_history = 0b110011; // check bet call
            player = 0;
            return;
        } else if (action == 2) {
            rivr_history = 0b1010011; // check bet raise
            player = 1;
            return;
        }
    }

    if (rivr_history == 0b1010011) {
        if (action == 0) {
            rivr_history = 0b11111111;  // check bet raise fold
            return;
        } else if (action == 1) {
            rivr_history = 0b11010011; // check bet raise call
            player = 0;
            return;
        }
    }
}

void GameState::apply_chance_action(int actions) {

    uint16_t suita_all = ((suita>>18) | (suita>>9) | suita) & 0b111111111;
    uint16_t suitb_all = ((suitb>>18) | (suitb>>9) | suitb) & 0b111111111;
    uint16_t suitc_all = ((suitc>>18) | (suitc>>9) | suitc) & 0b111111111;
    uint16_t suitd_all = ((suitd>>18) | (suitd>>9) | suitd) & 0b111111111;

    if (turn_history % 2) {
        int turn_rank = turn & 0b1111;
        int turn_suit = (turn>>4) & 0b11;
        
        if (turn_suit == 0) suita_all |= (0b1<<turn_rank);
        if (turn_suit == 0b1) suitb_all |= (0b1<<turn_rank);
        if (turn_suit == 0b10) suitc_all |= (0b1<<turn_rank);
        if (turn_suit == 0b11) suitd_all |= (0b1<<turn_rank);
    }

    std::array<uint16_t, 4> suit_cards = {suita_all, suitb_all, suitc_all, suitd_all};

    int k;
    bool acted = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 35);

    while (acted == false) {
        k = dis(gen);
        if (!((suit_cards[k/9] & SINGLE_MASKS[k%9])==SINGLE_MASKS[k%9])) {
            if (k/9==0 && actions>=30) suita = suita | (SINGLE_MASKS[k%9]<<18);
            if (k/9==1 && actions>=30) suitb = suitb | (SINGLE_MASKS[k%9]<<18);
            if (k/9==2 && actions>=30) suitc = suitc | (SINGLE_MASKS[k%9]<<18);
            if (k/9==3 && actions>=30) suitd = suitd | (SINGLE_MASKS[k%9]<<18);

            acted = true;
        }
    }

    if (actions>30) {
        flop_history = 0;
        call_preflop = 1;
        player = 0;
    } 

    if (actions==30) {
        flop_history |= 0b1;
        player = 0;
    } 

    if (actions==29) {
        turn = ((k/9)<<4) | (8-(k%9));
        turn_history |= 0b1;
        player = 0;
    }

    if (actions==28) {
        rivr = ((k/9)<<4) | (8-(k%9));
        rivr_history |= 0b1;
        player = 0;
    }
}

uint8_t ars_to_bucket_flop(float ars) {
    uint8_t i=0;
    while ((i<FLOP_BUCKETS) && FLOP_BUCKETS_arr[i]<ars) {
        i++;
    }
    return i;
}

uint8_t ars_to_bucket_turn(float ars) {
    uint8_t i=0;
    while ((i<TURN_BUCKETS) && TURN_BUCKETS_arr[i]<ars) {
        i++;
    }
    return i;
}

uint8_t ars_to_bucket_rivr(float ars) {
    uint8_t i=0;
    while ((i<RIVR_BUCKETS) && RIVR_BUCKETS_arr[i]<ars) {
        i++;
    }
    return i;
}

InfoSet GameState::to_information_set() {
    int p = p_id(player);

    uint8_t rivr_history_temp = rivr_history;
    uint8_t turn_history_temp = turn_history;
    rivr_history = 0;
    turn_history = 0;
    int flop_rank = best_hand(player)/100;

    turn_history = turn_history_temp;
    int turn_rank = best_hand(player)/100;

    rivr_history = rivr_history_temp;
    int rivr_rank = best_hand(player)/100;

    int flop_bucket = -1;
    int turn_bucket = -1;
    int rivr_bucket = -1;

    if (flop_history%2==1) flop_bucket = ars_to_bucket_flop(ars_table(0, flop_rank, p));
    if (turn_history%2==1) turn_bucket = ars_to_bucket_turn(ars_table(1, turn_rank, p));
    if (rivr_history%2==1) rivr_bucket = ars_to_bucket_rivr(ars_table(2, rivr_rank, p));

    return InfoSet(p,
                   call_preflop,
                   flop_history, 
                   turn_history, 
                   rivr_history,
                   flop_bucket,
                   turn_bucket,
                   rivr_bucket,
                   player);
}

float GameState::rivr_hand_strength() {

    float total = 0.0f;
    float wins = 0.0f;

    suita &= 0b111111111000000000111111111;
    suitb &= 0b111111111000000000111111111;
    suitc &= 0b111111111000000000111111111;
    suitd &= 0b111111111000000000111111111;

    uint16_t suita_all = ((suita>>18) | (suita>>9) | suita) & 0b111111111;
    uint16_t suitb_all = ((suitb>>18) | (suitb>>9) | suitb) & 0b111111111;
    uint16_t suitc_all = ((suitc>>18) | (suitc>>9) | suitc) & 0b111111111;
    uint16_t suitd_all = ((suitd>>18) | (suitd>>9) | suitd) & 0b111111111;

    int turn_rank = turn & 0b1111;
    int turn_suit = (turn>>4) & 0b11;
    if (turn_suit == 0) suita_all |= (0b1<<turn_rank);
    if (turn_suit == 0b1) suitb_all |= (0b1<<turn_rank);
    if (turn_suit == 0b10) suitc_all |= (0b1<<turn_rank);
    if (turn_suit == 0b11) suitd_all |= (0b1<<turn_rank);

    int rivr_rank = rivr & 0b1111;
    int rivr_suit = (rivr>>4) & 0b11;
    if (rivr_suit == 0) suita_all |= (0b1<<rivr_rank);
    if (rivr_suit == 0b1) suitb_all |= (0b1<<rivr_rank);
    if (rivr_suit == 0b10) suitc_all |= (0b1<<rivr_rank);
    if (rivr_suit == 0b11) suitd_all |= (0b1<<rivr_rank);

    std::array<uint16_t, 4> suit_cards_all = {suita_all, suitb_all, suitc_all, suitd_all};

    for (int c1=0; c1<36; c1++) {
        for (int c2=0; c2<36; c2++) {
            if ((c2!=c1) &&
                (!((suit_cards_all[c1/9] & SINGLE_MASKS[8-(c1%9)])==SINGLE_MASKS[8-(c1%9)])) &&
                (!((suit_cards_all[c2/9] & SINGLE_MASKS[8-(c2%9)])==SINGLE_MASKS[8-(c2%9)]))) {

                std::array<uint32_t, 4> suit_cards = {suita, suitb, suitc, suitd};
                uint32_t tempa = suita;
                uint32_t tempb = suitb;
                uint32_t tempc = suitc;
                uint32_t tempd = suitd;

                suit_cards[c1/9] |= (0b1 << (9+(c1%9)));
                suit_cards[c2/9] |= (0b1 << (9+(c2%9)));
                turn_history = 1;
                rivr_history = 1;
                
                suita = suit_cards[0];
                suitb = suit_cards[1];
                suitc = suit_cards[2];
                suitd = suit_cards[3];

                int player_best = best_hand(0);
                int opponent_best = best_hand(1);

                if (player_best > opponent_best) {
                    wins += 1.0f;   
                } else if (player_best == opponent_best) {
                    wins += 0.5f;
                }

                total += 1;

                suita = tempa;
                suitb = tempb;
                suitc = tempc;
                suitd = tempd;
            }
        }
    }

    return wins/total;
}

int GameState::p_id(bool p) const {
    std::array<uint32_t, 4> player_cards;

    if (p==0) player_cards = {(suita & 0b111111111), (suitb & 0b111111111), (suitc & 0b111111111), (suitd & 0b111111111)};
    if (p==1) player_cards = {((suita>>9) & 0b111111111), ((suitb>>9) & 0b111111111), ((suitc>>9) & 0b111111111), ((suitd>>9) & 0b111111111)};

    int p1 = -1;
    int p2 = -1;

    for (int k=0; k<36; k++) {
        if (player_cards[k/9] & (0b1<<k%9)) {
            if (p1==-1) p1=k;
            else if (p2==-1) p2=k;
        }
    }

    return pocket_id(p1, p2);
}

size_t hash_gamestate(const GameState& gs) {
    size_t seed = 0x9e3779b9;
    std::hash<uint32_t> hash_uint32;
    std::hash<uint8_t> hash_uint8;
    std::hash<bool> hash_bool;

    seed ^= hash_uint32(gs.suita) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint32(gs.suitb) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint32(gs.suitc) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint32(gs.suitd) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint8(gs.turn) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint8(gs.rivr) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint8(gs.flop_history) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint8(gs.turn_history) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_uint8(gs.rivr_history) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_bool(gs.call_preflop) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash_bool(gs.player) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

    return seed % 40000007;
}

int pocket_id(int p1, int p2) {
    bool suited = false;
    if ((p1/9)==(p2/9)) {
        suited = true;
    }
    return (suited) ? std::max((p1%9)*9 + p2%9, (p2%9)*9 + p1%9)
                    : std::min((p1%9)*9 + p2%9, (p2%9)*9 + p1%9);
}

GameState generate_random_initial_state() {
    std::array<uint32_t, 4> suit_cards = {0, 0, 0, 0};
    uint8_t turn = 0, rivr = 0;
    uint8_t flop_history = 0, turn_history = 0, rivr_history = 0;
    bool call_preflop = false;
    bool player = false;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 35);

    // Generate unique card indices
    std::array<int, 4> card_indices;
    for (int i=0; i<4; ++i) {
        do {
            card_indices[i] = dis(gen);
        } while (std::find(card_indices.begin(), card_indices.begin() + i, card_indices[i]) != card_indices.begin() + i);
    }

    // Assign cards to players
    for (int i=0; i<2; ++i) {
        int suit = card_indices[i] / 9;
        int rank = card_indices[i] % 9;
        suit_cards[suit] |= (1U << rank);
    }

    for (int i=2; i<4; ++i) {
        int suit = card_indices[i] / 9;
        int rank = card_indices[i] % 9;
        suit_cards[suit] |= (1U << (rank + 9));
    }

    return GameState(suit_cards[0], suit_cards[1], suit_cards[2], suit_cards[3],
                     turn, rivr, flop_history, turn_history, rivr_history,
                     call_preflop, player);
}

void play_computer(bool p) {
    
    float cumulative_winnings = 0.0f;
    bool keep_playing = true;

    load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    while (keep_playing == true) {
        GameState gs = generate_random_initial_state();

        while (!gs.is_terminal()) {
            while (gs.is_chance()){
                int num_chance_actions = gs.num_chance_actions();
                gs.apply_chance_action(num_chance_actions);
                // if (num_chance_actions==30) std::cout << "******************** FLOP ********************\n";
                // if (num_chance_actions==29) std::cout << "******************** TURN ********************\n";
                // if (num_chance_actions==28) std::cout << "******************** RIVER ********************\n";
            }

            InfoSet is = gs.to_information_set();
            std::array<float, 3> average_strategy = get_average_strategy(is);
        
            std::array<uint32_t, 4> suits_backup = {gs.suita, gs.suitb, gs.suitc, gs.suitd};
            uint32_t mask = (p == 0) ? 0b111111111000000000111111111
                                             : 0b111111111111111111000000000;

            gs.suita &= mask;
            gs.suitb &= mask;
            gs.suitc &= mask;
            gs.suitd &= mask;

            std::cout << "**********************************************\n";
            std::cout << gs.to_string() << "\n";

            gs.suita = suits_backup[0];
            gs.suitb = suits_backup[1];
            gs.suitc = suits_backup[2];
            gs.suitd = suits_backup[3];

            if (gs.player==p) {
                // Player's turn
                std::cout << "Player turn. Input action: ";

                std::string action_str;
                std::cin >> action_str;

                std::cout << "GTO strategy: " << average_strategy << "\n**********************************************\n\n";

                gs.apply_action(STRING_TO_ACTION.at(action_str));
            } else {
                // Computer's turn
                int sampled_action = sample_action(average_strategy);

                std::cout << "Computer turn. Sampled action: " << sampled_action << "\n";
                // std::cout << "GTO strategy: " << average_strategy << "\n**********************************************\n\n";

                gs.apply_action(sampled_action);
            }
        }
        std::cout << "Terminal game state:\n" << gs.to_string() << "\n";
        cumulative_winnings += gs.utility(p);
        std::cout << "HAND WINNINGS: " << gs.utility(p) << "\n";
        std::cout << "CUMULATIVE WINNINGS: " << cumulative_winnings << "\n**********************************************\n";
        std::cout << "Continue playing? (Y/n): "; 

        char in;
        std::cin >> in;
        if (in=='n') keep_playing = false;
    }
}