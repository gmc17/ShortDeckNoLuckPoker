#include "game_state.h"
#include "ars_table.h"
#include "constants.h"
#include "info_set.h"
#include "cfr.h"

GameState::GameState(): player(false),
                        suita(0), 
                        suitb(0), 
                        suitc(0), 
                        suitd(0),
                        turn(0), 
                        rivr(0),
                        pflp_history(0), 
                        flop_history(0), 
                        turn_history(0), 
                        rivr_history(0),
                        pot_size(0.0f),
                        biggest_bet(2.0f),
                        biggest_mutual_bet(1.0f),
                        flop_seen(false), 
                        turn_seen(false),
                        rivr_seen(false) {}

std::string GameState::to_string(bool verbose) const {
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

    if (turn_seen) {
        int turn_rank = turn & 0b1111;
        int turn_suit = (turn>>4) & 0b11;

        ss << CARD_NAMES[turn_rank] << SUIT_NAMES[turn_suit];
    }

    ss << "\n";

    ss << "River card: ";

    if (rivr_seen) {
        int rivr_rank = rivr & 0b1111;
        int rivr_suit = (rivr>>4) & 0b11;

        ss << CARD_NAMES[rivr_rank] << SUIT_NAMES[rivr_suit];
    }

    ss << "\n";

    if (verbose) {
        // ss << "Pflp history: " << history_to_string(pflp_history) << "\n";
        // if (flop_seen) ss << "Flop history: " << history_to_string(flop_history) << "\n";
        // if (turn_seen) ss << "Turn history: " << history_to_string(turn_history) << "\n";
        // if (rivr_seen) ss << "Rivr history: " << history_to_string(rivr_history) << "\n";
    } else {
        std::bitset<32> binary_pflp_history(pflp_history);
        std::bitset<32> binary_flop_history(flop_history);
        std::bitset<32> binary_turn_history(turn_history);
        std::bitset<32> binary_rivr_history(rivr_history);
        ss << "Pflp history: " << binary_pflp_history << "\n";
        if (flop_seen) ss << "Flop history: " << binary_flop_history << "\n";
        if (turn_seen) ss << "Turn history: " << binary_turn_history << "\n";
        if (rivr_seen) ss << "Rivr history: " << binary_rivr_history << "\n";
    }
    
    ss << "Player:       " << player << "\n";
    ss << "Biggest_bet:  " << biggest_bet << "\n";
    ss << "Mutual_bet:   " << biggest_mutual_bet << "\n";
    ss << "Pot_size:     " << pot_size << "\n";
    ss << "Num_actions:  " << num_actions() << "\n";
    ss << "Is_terminal:  " << is_terminal() << "\n";
    // ss << "p_id(0):      " << p_id(0) << "\n";
    
    return ss.str();
}

bool GameState::operator==(const GameState& other) const {
    return player == other.player &&
           suita == other.suita && 
           suitb == other.suitb && 
           suitc == other.suitc && 
           suitd == other.suitd &&
           turn == other.turn &&
           rivr == other.rivr &&
           pflp_history == other.pflp_history &&
           flop_history == other.flop_history &&
           turn_history == other.turn_history &&
           rivr_history == other.rivr_history &&
           pot_size == other.pot_size &&
           biggest_bet == other.biggest_bet &&
           biggest_mutual_bet == other.biggest_mutual_bet &&
           flop_seen == other.flop_seen &&
           turn_seen == other.turn_seen &&
           rivr_seen == other.rivr_seen;
}

bool GameState::is_terminal() const {

    // Preflop terminals
    if (((pflp_history & 0b111) == 1) && (pflp_history != 0b111001))
        return true;

    // Flop terminals
    if (((flop_history & 0b111) == 1) && (flop_history != 0b1001) && (flop_history != 1)) 
        return true;

    // Turn terminals
    if (((turn_history & 0b111) == 1) && (turn_history != 0b1001) && (turn_history != 1)) 
        return true;

    // River terminals
    if ((((rivr_history & 0b111) == 1) && (rivr_history != 1)) || // fold or check-check
         ((rivr_history & 0b111) == 0b111)) // call
        return true;

    // All-ins that have finished running out
    if ((rivr_seen) &&
       (((pflp_history & 0b111111) == 0b110111) || 
        ((flop_history & 0b111111) == 0b110111) ||
        ((turn_history & 0b111111) == 0b110111) ||
        ((rivr_history & 0b111111) == 0b110111)))
        return true;

    return false;
}

bool GameState::is_chance() const {
    //  Flop
    if ((!flop_seen) && 
       ((pflp_history == 0b111001) || (((pflp_history & 0b111) == 0b111) && ((pflp_history != 0b111)))))
        return true;

    //  Turn
    if ((!turn_seen) &&
       (((flop_history & 0b111) == 0b111) || ((flop_history & 0b111111) == 0b1001))) // no turn, but flop is done
        return true;

    //  River
    if ((!rivr_seen) &&
       (((turn_history & 0b111) == 0b111) || ((turn_history & 0b111111) == 0b1001))) // no river, but turn is done
        return true;

    //  All-in, call
    if (!is_terminal() && 
       (((pflp_history & 0b111111) == 0b110111) || 
        ((flop_history & 0b111111) == 0b110111) ||
        ((turn_history & 0b111111) == 0b110111)))
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

    if (turn_seen) {
        int turn_rank = turn & 0b1111;
        int turn_suit = (turn>>4) & 0b11;
        
        if (turn_suit == 0) suita_all |= (0b1<<turn_rank);
        if (turn_suit == 0b1) suitb_all |= (0b1<<turn_rank);
        if (turn_suit == 0b10) suitc_all |= (0b1<<turn_rank);
        if (turn_suit == 0b11) suitd_all |= (0b1<<turn_rank);
    }
    
    if (rivr_seen) {
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

    if (best != -1) return 700000 + best*10000;

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

    if (pair != -1 && trips != -1) return 600000 + trips*10000 + pair*1000;

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

    std::cout << "ERROR: NO BEST HAND\n";

    return -1;
}

float GameState::showdown(bool p) const {
    int p_best = best_hand(p);
    int o_best = best_hand(!p);

    if (p_best == o_best) return 0.0f;

    return (p_best > o_best) ? pot_size * 0.5f : pot_size * -0.5f;
}

float GameState::utility(bool p) const {
    // Preflop fold
    if ((pflp_history & 0b111) == 0b1) return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Fold on flop
    if (((flop_history & 0b111) == 0b1) &&
        (flop_history != 0b11) &&
        (flop_history != 0b1))
        return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Fold on turn
    if (((turn_history & 0b111) == 0b1) &&
        (turn_history != 0b11) &&
        (turn_history != 0b1))
        return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Fold on river
    if (((rivr_history & 0b111) == 0b1) &&
        (rivr_history != 0b11) &&
        (rivr_history != 0b1))
        return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Check check on river
    if ((rivr_history & 0b111111) == 0b1001) return showdown(p);

    // Call on river
    if ((rivr_history & 0b111) == 0b111) return showdown(p);

    // All ins
    if (((pflp_history & 0b111111) == 0b110111) || 
        ((flop_history & 0b111111) == 0b110111) ||
        ((turn_history & 0b111111) == 0b110111))
        return showdown(p);

    return -1;
}

int GameState::num_actions() const {

    if (biggest_bet==100) return 2;
    
    /************************* Preflop *************************/

    if (pflp_history == 0) return 7; // open; can do anything 1-7
    
    if (pflp_history == 0b111) return 6; // call; can check or bet any size 1-6
    
    if (((pflp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!flop_seen))
         return 2;
    
    if ((pflp_history < 0b1000000000) && // responding to raise; respond 1-7
        (!flop_seen)) {
        // raise
        int i=0;
        while ((i < 4) && (pot_size / 2 + biggest_bet * RAISE_SIZES[i]) <= STACK_SIZE) i++;
        // 4-i is number of forbidden actions
        return 7-(4-i);
    } 
    
    if ((pflp_history > 0b1000000000) && // bet/raise but at limit; 1 or 7
        (pflp_history < 0b1111111111111) &&
        (!flop_seen))
        return 2; 

    /************************* Flop *************************/

    if ((flop_history == 0) || (flop_history == 1)) {
        int i=0;
        while ((i < 4) && (pot_size / 2 + pot_size * BET_SIZES[i]) <= STACK_SIZE) i++;
        return 6-(4-i); // 6 b/c you can't call
    }
    
    if (((flop_history & 0b111) == 0b110) && // all-in; must fold or call
         (!turn_seen)) 
         return 2; 
    
    if ((flop_history < 0b1000000000) && // responding to raise; respond 1-7
        (!turn_seen)) {
        // raise
        int i=0;
        while ((i < 4) && (pot_size / 2 + biggest_bet * RAISE_SIZES[i]) <= STACK_SIZE) i++;
        return 7-(4-i);
    }
        
    
    if ((flop_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (flop_history < 0b1111111111111) &&
        (!turn_seen))
        return 2;

    /************************* Turn *************************/

    if ((turn_history == 0) || (turn_history == 1)) { // open action; can do anything 1-7; overlap all-ins
        int i=0;
        while ((i < 4) && (pot_size / 2 + pot_size * BET_SIZES[i]) <= STACK_SIZE) i++;
        return 6-(4-i);
    } 
    
    if (((turn_history & 0b111) == 0b110) && // all-in; must fold or call
         (!rivr_seen)) 
         return 2;
    
    if ((turn_history < 0b1000000000) && // responding to raise; respond 1-7
        (!rivr_seen)) {
        int i=0;
        while ((i < 4) && (pot_size / 2 + biggest_bet * RAISE_SIZES[i]) <= STACK_SIZE) i++;
        return 7-(4-i);
    }
        
    
    if ((turn_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (turn_history < 0b1111111111111) &&
        (!rivr_seen))
        return 2;

    /************************* River *************************/

    if ((rivr_history == 0) || (rivr_history == 1)) { // open action / check; can do anything 1-7; overlap all-ins
        int i=0;
        while ((i < 4) && (pot_size / 2 + pot_size * BET_SIZES[i]) <= STACK_SIZE) i++;
        return 6-(4-i);
    }
    
    if ((rivr_history & 0b111) == 0b110) // all-in; must fold or call 
        return 2; 
    
    if (rivr_history < 0b1000000000) { // bet/raise; respond 1-7
        int i=0;
        while ((i < 4) && (pot_size / 2 + biggest_bet * RAISE_SIZES[i]) <= STACK_SIZE) i++;
        return 7-(4-i);
    }

    if ((rivr_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (rivr_history < 0b1111111111111))
        return 2;
    
    return -1;
}

int GameState::num_chance_actions() const {
    //  Flop
    if ((!flop_seen) && 
        (pflp_history > 0)) {
        return 36 - (__builtin_popcount(suita) + __builtin_popcount(suitb) + __builtin_popcount(suitc) + __builtin_popcount(suitd));
    }

    //  Turn
    if ((!turn_seen) &&
        (flop_history > 0))
        return 29;
    
    //  River
    if ((!rivr_seen) &&
        (turn_history > 0))               
        return 28;

    //  All-ins
    if (!flop_seen) return 32;
    if (!turn_seen) return 29;
    if (!rivr_seen) return 28;

    return 0;
}

int GameState::index_to_action(int index) const {

    if (biggest_bet == STACK_SIZE) return (index == 0) ? 1 : 7;
    
    /************************* Preflop *************************/
    
    if (pflp_history == 0) return index+1; // open; can do anything 1-7
    
    if (pflp_history == 0b111) return index+1; // call; can check or bet any size 1-6
    
    if (((pflp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!flop_seen)) 
        return (index == 0) ? 1 : 7;
    
    if ((pflp_history < 0b1000000000) && // bet/raise; respond 1-7
        (!flop_seen)) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if ((pflp_history > 0b1000000000) && // bet/raise but at limit; 1 or 7
        (pflp_history < 0b1111111111111) &&
        (!flop_seen))
        return (index == 0) ? 1 : 7; 

    /************************* Flop *************************/
    
    if ((flop_history == 0) || (flop_history == 1)) {
        int num = num_actions();
        int num_banned = 6-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    } 
    
    if (((flop_history & 0b111) == 0b110) && // all-in; must fold or call
         (!turn_seen)) 
        return (index == 0) ? 1 : 7;
    
    if ((flop_history < 0b1000000000) && // bet/raise; respond 1-7
        (!turn_seen)) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    } 
    
    if ((flop_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (flop_history < 0b1111111111111) &&
        (!turn_seen))
        return (index == 0) ? 1 : 7;

    /************************* Turn *************************/
    
    if ((turn_history == 0) || (turn_history == 1)) {
        int num = num_actions();
        int num_banned = 6-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if (((turn_history & 0b111) == 0b110) && // all-in; must fold or call
         (!rivr_seen)) 
        return (index == 0) ? 1 : 7;
    
    if ((turn_history < 0b1000000000) && // bet/raise; respond 1-7
        (!rivr_seen)) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if ((turn_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (turn_history < 0b1111111111111) &&
        (!rivr_seen))
        return (index == 0) ? 1 : 7;

    /************************* River *************************/

    if ((rivr_history == 0) || (rivr_history == 1)) {
        int num = num_actions();
        int num_banned = 6-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }  
    
    if ((rivr_history & 0b111) == 0b110) // all-in; must fold or call 
        return (index == 0) ? 1 : 7;
    
    if (rivr_history < 0b1000000000) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if ((rivr_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (rivr_history < 0b1111111111111))
        return (index == 0) ? 1 : 7;

    return -1;
}

int GameState::action_to_index(int action) const {
    for (int i=0; i<7; i++) {
        if (index_to_action(i) == action) {
            return i;
        }
    }
    return -1;
}

void GameState::apply_index(int index) {

    int a = index_to_action(index);

    player = !player; // player alternates

    if (a == 7) { // call
        biggest_mutual_bet = biggest_bet;
        if (flop_seen || turn_seen || rivr_seen) {
            pot_size += 2 * biggest_mutual_bet;
            biggest_bet = 0;
            biggest_mutual_bet = 0;
        }
    }
   
    else if ((a >= 2) && (a <= 5)) {
        if (biggest_bet == 0) {
            // bet
            biggest_bet = pot_size * BET_SIZES[a-2];
        } else {
            // raise
            biggest_mutual_bet = biggest_bet;
            biggest_bet *= RAISE_SIZES[a-2];
        }
    }

    else if (a == 6) biggest_bet = STACK_SIZE - (pot_size / 2);

    else if (a == 1) {
        if (!flop_seen) {
            if ((pflp_history != 0b111)) { // can't be a check. must be fold
                pot_size += biggest_mutual_bet * 2;
                biggest_bet = 0;
                biggest_mutual_bet = 0;
                player = !player;
            }
        } else if (!turn_seen) {
            if ((flop_history != 0) && (flop_history != 0b1)) { // not check or check-check. must be a fold
                pot_size += biggest_mutual_bet * 2;
                biggest_bet = 0;
                biggest_mutual_bet = 0;
                player = !player;
            }
        } else if (!rivr_seen) {
            if ((turn_history != 0) && (turn_history != 0b1)) {
                pot_size += biggest_mutual_bet * 2;
                biggest_bet = 0;
                biggest_mutual_bet = 0;
                player = !player;
            }
        } else {
            if ((rivr_history != 0) && (rivr_history != 0b1)) {
                pot_size += biggest_mutual_bet * 2;
                biggest_bet = 0;
                biggest_mutual_bet = 0;
                player = !player;
            }
        }
    }
    
    /************************* Preflop *************************/

    if (!flop_seen) {
        pflp_history = (pflp_history << 3) | a;
        return;
    }

    /************************* Flop *************************/

    if (!turn_seen) {
        flop_history = (flop_history << 3) | a;
        return;
    }

    /************************* Turn *************************/

    if (!rivr_seen) {
        turn_history = (turn_history << 3) | a;
        return;
    }

    /************************* River *************************/

    rivr_history = (rivr_history << 3) | a;
}

void GameState::apply_chance_action(int actions) {

    uint16_t suita_all = ((suita>>18) | (suita>>9) | suita) & 0b111111111;
    uint16_t suitb_all = ((suitb>>18) | (suitb>>9) | suitb) & 0b111111111;
    uint16_t suitc_all = ((suitc>>18) | (suitc>>9) | suitc) & 0b111111111;
    uint16_t suitd_all = ((suitd>>18) | (suitd>>9) | suitd) & 0b111111111;

    if (turn_seen) {
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

    while (acted == false) {
        k = get_card_distribution()(get_random_generator());
        if (!((suit_cards[k/9] & SINGLE_MASKS[k%9])==SINGLE_MASKS[k%9])) {
            if (k/9==0 && actions>=30) suita = suita | (SINGLE_MASKS[k%9]<<18);
            if (k/9==1 && actions>=30) suitb = suitb | (SINGLE_MASKS[k%9]<<18);
            if (k/9==2 && actions>=30) suitc = suitc | (SINGLE_MASKS[k%9]<<18);
            if (k/9==3 && actions>=30) suitd = suitd | (SINGLE_MASKS[k%9]<<18);

            acted = true;
        }
    }

    if (actions>30) {
        flop_seen = false;
    }

    if (actions==30) {
        flop_seen = true;
    } 

    if (actions==29) {
        turn = ((k/9)<<4) | (8-(k%9));
        turn_seen = true;
    }

    if (actions==28) {
        rivr = ((k/9)<<4) | (8-(k%9));
        rivr_seen = true;
    }

    player = 0;
    pot_size += 2 * biggest_mutual_bet;
    biggest_bet = 0;
    biggest_mutual_bet = 0;
}

InfoSet GameState::to_information_set() {
    int p = p_id(player);

    InfoSet is;
    is.player = player;
    is.pflp_history = pflp_history;
    is.flop_history = flop_history; 
    is.turn_history = turn_history; 
    is.rivr_history = rivr_history;
    is.flop_seen = flop_seen;
    is.turn_seen = turn_seen;
    is.rivr_seen = rivr_seen;
    is.num_actions = num_actions();
    is.pocket_id = p;
    
    bool rivr_seen_temp = rivr_seen;
    bool turn_seen_temp = turn_seen;
    
    if (flop_seen) {
        rivr_seen = false;
        turn_seen = false;
        is.flop_bucket = ars_to_bucket_flop(ars_table(0, best_hand(player)/100, p));
        turn_seen = turn_seen_temp;
        if (turn_seen) {
            is.turn_bucket = ars_to_bucket_turn(ars_table(1, best_hand(player)/100, p));
            rivr_seen = rivr_seen_temp;
            if (rivr_seen) is.rivr_bucket = ars_to_bucket_rivr(ars_table(2, best_hand(player)/100, p));
        }
    }

    turn_seen = turn_seen_temp;
    rivr_seen = rivr_seen_temp;

    return is;
}

float GameState::rivr_hand_strength() {

    float wins = 0.0f;
    float total = 0.0f;

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
        for (int c2=c1+1; c2<36; c2++) {
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
                turn_seen = true;
                rivr_seen = true;
                
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
        if (player_cards[k/9] & (0b1<<(k%9))) {
            if (p1==-1) p1=k;
            else if (p2==-1) p2=k;
        }
    }

    return pocket_id(p1, p2);
}

std::string GameState::action_to_string(int action) const {

    std::stringstream ss;

    if (biggest_bet == 100) return (action == 1) ? "Fold" : "Call";
    
    /************************* Preflop *************************/
    
    if (pflp_history == 0) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << biggest_bet * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if (pflp_history == 0b111) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    }
    
    if (((pflp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!flop_seen)) 
        return (action == 1) ? "Fold" : "Call";
    
    if ((pflp_history < 0b1000000000) && // bet/raise; respond 1-7
        (!flop_seen)) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << biggest_bet * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if ((pflp_history > 0b1000000000) && // bet/raise but at limit; 1 or 7
        (pflp_history < 0b1111111111111) &&
        (!flop_seen))
        return (action == 1) ? "Fold" : "Call";

    /************************* Flop *************************/
    
    if ((flop_history == 0) || (flop_history == 1)) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    } 
    
    if (((flop_history & 0b111) == 0b110) && // all-in; must fold or call
         (!turn_seen)) 
        return (action == 1) ? "Fold" : "Call";
    
    if ((flop_history < 0b1000000000) && // bet/raise; respond 1-7
        (!turn_seen)) {
        ss << RAISE_ACTION_NAMES[action-1] << " ";
        if ((action >= 2) && (action<=5)) ss << biggest_bet * RAISE_SIZES[action-2];
        return ss.str();
    } 
    
    if ((flop_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (flop_history < 0b1111111111111) &&
        (!turn_seen))
        return (action == 1) ? "Fold" : "Call";

    /************************* Turn *************************/
    
    if ((turn_history == 0) || (turn_history == 1)) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    }
    
    if (((turn_history & 0b111) == 0b110) && // all-in; must fold or call
         (!rivr_seen)) 
        return (action == 1) ? "Fold" : "Call";
    
    if ((turn_history < 0b1000000000) && // bet/raise; respond 1-7
        (!rivr_seen)) {
        ss << RAISE_ACTION_NAMES[action-1] << " ";
        if ((action >= 2) && (action<=5)) ss << biggest_bet * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if ((turn_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (turn_history < 0b1111111111111) &&
        (!rivr_seen))
        return (action == 1) ? "Fold" : "Call";

    /************************* River *************************/

    if ((rivr_history == 0) || (rivr_history == 1)) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    }  
    
    if ((rivr_history & 0b111) == 0b110) // all-in; must fold or call 
        return (action == 1) ? "Fold" : "Call";
    
    if (rivr_history < 0b1000000000) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << biggest_bet * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if ((rivr_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (rivr_history < 0b1111111111111))
        return (action == 1) ? "Fold" : "Call";

    return "";
}

std::string GameState::histories_to_string() const {
    std::stringstream ss;

    // int i=0;

    // while (ith_action(history, i) != 0) i++;

    // for (int k=i-1; k>=0; k--) {
    //     ss << action_to_string(ith_action(history, i)) << " ";
    // }

    return ss.str();
}

std::array<int, 2> pocket_id_to_row_col(int id) {
    return {8-(id/9), 8-(id%9)};
}

void GameState::print_range(int action) const {

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    std::string player_name = (player) ? "BB" : "SB"; 

    std::cout << "\n******************* Board ********************\n" << to_string();
    std::cout << "\n*************** " << player_name << " Range: " << action_to_string(action) << " ****************\n";

    std::array<std::array<float, 9>, 9> range;

    for (int p1=0; p1<36; p1++) {
        for (int p2=p1+1; p2<36; p2++) {
            GameState temp = *this;
            
            std::array<uint32_t, 4> suit_cards = {suita, suitb, suitc, suitd};

            if ((suit_cards[p1/9] & (0b1 << ((p1%9)+18))) ||
                (suit_cards[p2/9] & (0b1 << ((p2%9)+18)))) continue;

            if (temp.player==false) {
                suit_cards[p1/9] |= (0b1 << (p1%9));
                suit_cards[p2/9] |= (0b1 << (p2%9));
            } else {
                suit_cards[p1/9] |= (0b1 << ((p1%9)+9));
                suit_cards[p2/9] |= (0b1 << ((p2%9)+9));
            }

            temp.suita = suit_cards[0];
            temp.suitb = suit_cards[1];
            temp.suitc = suit_cards[2];
            temp.suitd = suit_cards[3];

            // std::cout << temp.to_string() << "\n";

            InfoSet is = temp.to_information_set();

            std::array<int, 2> row_col = pocket_id_to_row_col(temp.p_id(player));
            
            range[row_col[0]][row_col[1]] = get_average_strategy(is)[action_to_index(action)];
            //     // suited
            //     range[8-p2][8-p1] = get_average_strategy(is)[action_to_index(action)];
            // } else {
            //     // unsuited
            //     if ((8-p1)>(17-p2)) range[8-p1][17-p2] = get_average_strategy(is)[action_to_index(action)];
            //     else range[17-p2][8-p1] = get_average_strategy(is)[action_to_index(action)];
            // }

            // std::cout << is.to_string() << "Average strategy: " << get_average_strategy(is) << "\n";
        }
    }

    for (int r=0; r<10; r++) {
        for (int c=0; c<10; c++) {
            if (c==0 && r==0) {
                std::cout << "    ";
            } else if (r==0 && c>0) {
                std::cout << CARD_NAMES[9-c] << "s   ";
            } else if (c==0 && r>0) {
                std::cout << CARD_NAMES[9-r] << " ";
            } else {
                std::cout << FIXED_FLOAT(range[r-1][c-1]) << " ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int ith_action(uint32_t history, int i) {
    return ((history & OCTAL_MASKS[i]) >> (3*i));
}

GameState generate_random_initial_state() {
    std::array<uint32_t, 4> suit_cards = {0, 0, 0, 0};

    // Generate unique card indices
    std::array<int, 4> card_indices;
    for (int i=0; i<4; ++i) {
        do {
            card_indices[i] = get_card_distribution()(get_random_generator());
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

    GameState gs;
    gs.suita = suit_cards[0];
    gs.suitb = suit_cards[1];
    gs.suitc = suit_cards[2];
    gs.suitd = suit_cards[3];

    return gs;
}

void play_computer() {

    bool p = 0; // start as small blind
    float cumulative_winnings = 0.0f;
    bool keep_playing = true;

    load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    while (keep_playing == true) {
        GameState gs = generate_random_initial_state();

        while (!gs.is_terminal()) {
            while (gs.is_chance()){
                gs.apply_chance_action(gs.num_chance_actions());
            }

            if (gs.is_terminal()) {
                break;
            }

            InfoSet is = gs.to_information_set();
            std::array<float, 7> average_strategy = get_average_strategy(is);
        
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

                int action;
                std::cin >> action;

                std::cout << "GTO strategy:\n";

                for (int i=0; i<gs.num_actions(); i++) {
                    std::cout << gs.action_to_string(gs.index_to_action(i)) << ": " << FIXED_FLOAT(average_strategy[i])*100 << "%";
                    if (i!=gs.num_actions()-1) std::cout << ", ";
                }

                std::cout << "\n**********************************************\n\n";

                // std::cout << "GTO strategy: " << average_strategy << "\n**********************************************\n\n";

                gs.apply_index(gs.action_to_index(action));
            } else {
                // Computer's turn
                int sampled_index = sample_action(average_strategy);

                std::cout << "Computer turn. Sampled action: " << gs.action_to_string(gs.index_to_action(sampled_index)) << "\n**********************************************\n\n";
                // std::cout << "GTO strategy: " << average_strategy << "\n**********************************************\n\n";
                
                gs.apply_index(sampled_index);
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
        p = !p; // alternate player
    }
}