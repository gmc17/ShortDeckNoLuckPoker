#include "game_state.h"
#include "cfr.h"

int rank_table[NUM_CARDS][NUM_CARDS][NUM_CARDS][NUM_CARDS];

GameState::GameState(): player(false),
                        op1(0), 
                        op2(0), 
                        ip1(0), 
                        ip2(0),
                        fp1(0), 
                        fp2(0),
                        fp3(0), 
                        trn(0), 
                        rvr(0), 
                        pfp_history(0),
                        flp_history(0),
                        trn_history(0),
                        rvr_history(0),
                        flp_seen(false), 
                        trn_seen(false),
                        rvr_seen(false),
                        is_terminal(false),
                        pot_size(0.0f) 
{
    bets.push(0.0f);
} 

std::string GameState::to_string(bool verbose) const {
    std::stringstream ss;
    
    // OOP player cards
    ss << "OOP cards:    ";
    if (op1 != 0 && op2 != 0) ss << CARD_NAMES[rank(op1)] << SUIT_NAMES[suit(op1)] << CARD_NAMES[rank(op2)] << SUIT_NAMES[suit(op2)];

    // IP player cards
    ss << "\nIP cards:     ";
    if (ip1 != 0 && ip2 != 0) ss << CARD_NAMES[rank(ip1)] << SUIT_NAMES[suit(ip1)] << CARD_NAMES[rank(ip2)] << SUIT_NAMES[suit(ip2)];

    // Flop cards
    ss << "\nFlop cards:   ";
    if (flp_seen) ss << CARD_NAMES[rank(fp1)] << SUIT_NAMES[suit(fp1)] << CARD_NAMES[rank(fp2)] << SUIT_NAMES[suit(fp2)]
                                                                                         << CARD_NAMES[rank(fp3)] << SUIT_NAMES[suit(fp3)];

    // Turn
    ss << "\nTurn card:    ";
    if (trn_seen) ss << CARD_NAMES[rank(trn)] << SUIT_NAMES[suit(trn)];
    
    // River
    ss << "\nRiver card:   ";
    if (rvr_seen) ss << CARD_NAMES[rank(rvr)] << SUIT_NAMES[suit(rvr)];

    if (verbose) {
        // To be implemented
        return ss.str();
    } else {
        std::bitset<32> binary_pfp_history(pfp_history);
        std::bitset<32> binary_flp_history(flp_history);
        std::bitset<32> binary_trn_history(trn_history);
        std::bitset<32> binary_rvr_history(rvr_history);
        ss << "\nPflp history: " << binary_pfp_history << "\n";
        if (flp_seen) ss << "Flop history: " << binary_flp_history << "\n";
        if (trn_seen) ss << "Turn history: " << binary_trn_history << "\n";
        if (rvr_seen) ss << "Rivr history: " << binary_rvr_history << "\n";
    }

    std::stack<float> temp = bets;
    
    ss << "Player:       " << player << "\n";
    // ss << "Biggest_bet:  " << temp.top() << "\n";
    // temp.pop();
    // if (!temp.empty()) ss << "Mutual_bet:   " << temp.top() << "\n";

    ss << "Bets:         " << stack_to_string(bets) << "\n";

    ss << "Pot_size:     " << pot_size << "\n";
    ss << "Num_actions:  " << num_actions() << "\n";
    ss << "Is_terminal:  " << is_terminal << "\n";
    
    return ss.str();
}

bool GameState::operator==(const GameState& other) const {
    return player == other.player &&
           op1 == other.op1 && 
           op2 == other.op2 && 
           ip1 == other.ip1 && 
           ip2 == other.ip2 &&
           fp1 == other.fp1 &&
           fp2 == other.fp2 &&
           fp3 == other.fp3 &&
           trn == other.trn &&
           rvr == other.rvr &&
           pfp_history == other.pfp_history &&
           flp_history == other.flp_history &&
           trn_history == other.trn_history &&
           rvr_history == other.rvr_history &&
           pot_size == other.pot_size &&
           flp_seen == other.flp_seen &&
           trn_seen == other.trn_seen &&
           rvr_seen == other.rvr_seen;
}

bool GameState::is_chance() const {
    //  Flop
    if ((!flp_seen) && 
       ((pfp_history == 0b111001) || (((pfp_history & 0b111) == 0b111) && ((pfp_history != 0b111)))))
        return true;

    //  Turn
    if ((!trn_seen) &&
       (((flp_history & 0b111) == 0b111) || ((flp_history & 0b111111) == 0b1001))) // no turn, but flop is done
        return true;

    //  River
    if ((!rvr_seen) &&
       (((trn_history & 0b111) == 0b111) || ((trn_history & 0b111111) == 0b1001))) // no river, but turn is done
        return true;

    //  All-in, call
    if (!is_terminal && 
       (((pfp_history & 0b111111) == 0b110111) || 
        ((flp_history & 0b111111) == 0b110111) ||
        ((trn_history & 0b111111) == 0b110111)))
        return true;

    return false;
}

int GameState::best_hand(bool p) const {
    std::array<uint16_t, 4> suits = {0, 0, 0, 0};
    
    uint8_t c1 = (p==0) ? op1 : ip1;
    uint8_t c2 = (p==0) ? op2 : ip2;

    suits[suit(c1)] |= (1U << rank(c1));
    suits[suit(c2)] |= (1U << rank(c2));

    if (fp1 > 0) suits[suit(fp1)] |= (1U << rank(fp1));
    if (fp2 > 0) suits[suit(fp2)] |= (1U << rank(fp2));
    if (fp3 > 0) suits[suit(fp3)] |= (1U << rank(fp3));

    if (trn > 0) suits[suit(trn)] |= (1U << rank(trn));
    if (rvr > 0) suits[suit(rvr)] |= (1U << rank(rvr));

    /************************* Straight flush *************************/

    for (int i=0; i<6; i++) {
        if (((suits[0] & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) ||
            ((suits[1] & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) ||
            ((suits[2] & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i]) || 
            ((suits[3] & STRAIGHT_MASKS[i]) == STRAIGHT_MASKS[i])) {
            return 900000 + (6-i)*10000;
        }
    }

    /************************* Four of a kind *************************/
    
    int best = -1;
    int kicker = -1;

    for (int i=0; i<9; i++) {
        if ((suits[0] & SINGLE_MASKS[i]) && 
            (suits[1] & SINGLE_MASKS[i]) &&
            (suits[2] & SINGLE_MASKS[i]) &&
            (suits[3] & SINGLE_MASKS[i])) {
            best = 9-i;
            for (int k=0; k<9; k++) {
                if ((k != i) && 
                    ((suits[0] & SINGLE_MASKS[k]) || 
                     (suits[1] & SINGLE_MASKS[k]) ||
                     (suits[2] & SINGLE_MASKS[k]) ||
                     (suits[3] & SINGLE_MASKS[k]))) {
                    kicker = 9-k;
                    break;
                }
            }
        }
    }

    if (kicker != -1 && best != -1) return 800000 + best*10000 + kicker*1000;

    /************************* Flush *************************/
    
    best = -1;

    for (int i=0; i<4; i++) {
        if (__builtin_popcount(suits[i]) >= 5) {
            // find highest card in the flush
            for (int k=0; k<9; k++) {
                if (suits[i] & SINGLE_MASKS[k]) {
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
        int count = ((suits[0] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[1] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[2] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[3] & SINGLE_MASKS[i]) ? 1 : 0);
        if (count >= 3) {
            trips = 9-i;
            break;
        }
    }

    // Find highest pair
    if (trips != -1) {
        for (int i=0; i<9; i++) {
            if ((9-i) == trips) continue; // skip trips we already found
            int count = ((suits[0] & SINGLE_MASKS[i]) ? 1 : 0) +
                        ((suits[1] & SINGLE_MASKS[i]) ? 1 : 0) +
                        ((suits[2] & SINGLE_MASKS[i]) ? 1 : 0) +
                        ((suits[3] & SINGLE_MASKS[i]) ? 1 : 0);
            if (count >= 2) {
                pair = 9-i;
                break;
            }
        }
    }

    if (pair != -1 && trips != -1) return 600000 + trips*10000 + pair*1000;

    /************************* Straight *************************/

    uint16_t all_cards = suits[0] | suits[1] | suits[2] | suits[3];

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
        int count = ((suits[0] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[1] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[2] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[3] & SINGLE_MASKS[i]) ? 1 : 0);
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
        int count = ((suits[0] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[1] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[2] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[3] & SINGLE_MASKS[i]) ? 1 : 0);
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
        int count = ((suits[0] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[1] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[2] & SINGLE_MASKS[i]) ? 1 : 0) +
                    ((suits[3] & SINGLE_MASKS[i]) ? 1 : 0);
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
    int kicker4 = -1;
    int kicker5 = -1;

    // Find best kickers
    for (int i=0; i<9; i++) {
        if (all_cards & SINGLE_MASKS[i]) {
            if (kicker1 == -1) {
                kicker1 = 9-i;
            } else if (kicker2 == -1) {
                kicker2 = 9-i;
            } else if (kicker3 == -1) {
                kicker3 = 9-i;
            } else if (kicker4 == -1) {
                kicker4 = 9-i;
            } else if (kicker5 == -1) {
                kicker5 = 9-i;
                break;
            }
        }
    }

    if (kicker1 != -1 && kicker2 != -1 && kicker3 != -1 && kicker4 != -1 && kicker5 != -1) {
        return 100000 + kicker1*10000 + kicker2*1000 + kicker3*100 + kicker4*10 + kicker5;
    }

    std::cout << "ERROR: NO BEST HAND\n" << to_string() << "\n";

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
    if (((pfp_history & 0b111) == 0b1) &&
         (pfp_history != 0b111001)) return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Fold on flop
    if (((flp_history & 0b111) == 0b1) &&
        (flp_history != 0b1001) &&
        (flp_history != 0b1))
        return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Fold on turn
    if (((trn_history & 0b111) == 0b1) &&
        (trn_history != 0b1001) &&
        (trn_history != 0b1))
        return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Fold on river
    if (((rvr_history & 0b111) == 0b1) &&
        (rvr_history != 0b1001) &&
        (rvr_history != 0b1))
        return (p == player) ? -0.5f * pot_size : 0.5f * pot_size;

    // Check check on river
    if ((rvr_history & 0b111111) == 0b1001) return showdown(p);

    // Call on river
    if ((rvr_history & 0b111) == 0b111) return showdown(p);

    // All ins
    if (((pfp_history & 0b111111) == 0b110111) || 
        ((flp_history & 0b111111) == 0b110111) ||
        ((trn_history & 0b111111) == 0b110111))
        return showdown(p);

    return -1;
}

bool GameState::is_fold() const {
    // Preflop fold
    if (((pfp_history & 0b111) == 0b1) &&
         (pfp_history != 0b111001)) return true;

    // Fold on flop
    if (((flp_history & 0b111) == 0b1) &&
        (flp_history != 0b1001) &&
        (flp_history != 0b1))
        return true;

    // Fold on turn
    if (((trn_history & 0b111) == 0b1) &&
        (trn_history != 0b1001) &&
        (trn_history != 0b1))
        return true;

    // Fold on river
    if (((rvr_history & 0b111) == 0b1) &&
        (rvr_history != 0b1001) &&
        (rvr_history != 0b1))
        return true;
    
    return false;
}

int GameState::num_actions() const {

    if (pot_size / 2.0f + bets.top() == STACK_SIZE) return 2;
    
    /************************* Preflop *************************/

    if (pfp_history == 0) return 7; // open; can do anything 1-7
    
    if (pfp_history == 0b111) return 6; // call; can check or bet any size 1-6
    
    if (((pfp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!flp_seen))
         return 2;
    
    if ((pfp_history < 0b1000000000) && // responding to raise; respond 1-7
        (!flp_seen)) {
        // raise
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + bets.top() * RAISE_SIZES[i]) < STACK_SIZE) i++;
        // 4-i is number of forbidden actions
        return 7-(4-i);
    } 
    
    if ((pfp_history > 0b1000000000) && // bet/raise but at limit; 1 or 7
        (pfp_history < 0b1111111111111) &&
        (!flp_seen))
        return 2; 

    /************************* Flop *************************/

    if ((flp_history == 0) || (flp_history == 1)) {
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + pot_size * BET_SIZES[i]) < STACK_SIZE) i++;
        return 6-(4-i); // 6 b/c you can't call
    }
    
    if (((flp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!trn_seen)) 
         return 2; 
    
    if ((flp_history < 0b1000000000) && // responding to raise; respond 1-7
        (!trn_seen)) {
        // raise
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + bets.top() * RAISE_SIZES[i]) < STACK_SIZE) i++;
        return 7-(4-i);
    }
        
    
    if ((flp_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (flp_history < 0b1111111111111) &&
        (!trn_seen))
        return 2;

    /************************* Turn *************************/

    if ((trn_history == 0) || (trn_history == 1)) { // open action; can do anything 1-7; overlap all-ins
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + pot_size * BET_SIZES[i]) < STACK_SIZE) i++;
        return 6-(4-i);
    } 
    
    if (((trn_history & 0b111) == 0b110) && // all-in; must fold or call
         (!rvr_seen)) 
         return 2;
    
    if ((trn_history < 0b1000000000) && // responding to raise; respond 1-7
        (!rvr_seen)) {
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + bets.top() * RAISE_SIZES[i]) < STACK_SIZE) i++;
        return 7-(4-i);
    }
        
    if ((trn_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (trn_history < 0b1111111111111) &&
        (!rvr_seen))
        return 2;

    /************************* River *************************/

    if ((rvr_history == 0) || (rvr_history == 1)) { // open action / check; can do anything 1-7; overlap all-ins
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + pot_size * BET_SIZES[i]) < STACK_SIZE) i++;
        return 6-(4-i);
    }
    
    if ((rvr_history & 0b111) == 0b110) // all-in; must fold or call 
        return 2; 
    
    if (rvr_history < 0b1000000000) { // bet/raise; respond 1-7
        int i=0;
        while ((i < 4) && (pot_size / 2.0f + bets.top() * RAISE_SIZES[i]) < STACK_SIZE) i++;
        return 7-(4-i);
    }

    if ((rvr_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (rvr_history < 0b1111111111111))
        return 2;
    
    return 0;
}

int GameState::num_in_deck() const {
    int res = 0;
    
    if (op1 != 0) res++;
    if (op2 != 0) res++;
    if (ip1 != 0) res++;
    if (ip2 != 0) res++;
    if (fp1 != 0) res++;
    if (fp2 != 0) res++;
    if (fp3 != 0) res++;
    if (trn != 0) res++;
    if (rvr != 0) res++;

    return NUM_CARDS-res;
}

int GameState::index_to_action(int index) const {

    if (pot_size / 2.0f + bets.top() == STACK_SIZE) return (index == 0) ? 1 : 7;
    
    /************************* Preflop *************************/
    
    if (pfp_history == 0) return index+1; // open; can do anything 1-7
    
    if (pfp_history == 0b111) return index+1; // call; can check or bet any size 1-6
    
    if (((pfp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!flp_seen)) 
        return (index == 0) ? 1 : 7;
    
    if ((pfp_history < 0b1000000000) && // bet/raise; respond 1-7
        (!flp_seen)) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if ((pfp_history > 0b1000000000) && // bet/raise but at limit; 1 or 7
        (pfp_history < 0b1111111111111) &&
        (!flp_seen))
        return (index == 0) ? 1 : 7; 

    /************************* Flop *************************/
    
    if ((flp_history == 0) || (flp_history == 1)) {
        int num = num_actions();
        int num_banned = 6-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    } 
    
    if (((flp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!trn_seen)) 
        return (index == 0) ? 1 : 7;
    
    if ((flp_history < 0b1000000000) && // bet/raise; respond 1-7
        (!trn_seen)) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    } 
    
    if ((flp_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (flp_history < 0b1111111111111) &&
        (!trn_seen))
        return (index == 0) ? 1 : 7;

    /************************* Turn *************************/
    
    if ((trn_history == 0) || (trn_history == 1)) {
        int num = num_actions();
        int num_banned = 6-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if (((trn_history & 0b111) == 0b110) && // all-in; must fold or call
         (!rvr_seen)) 
        return (index == 0) ? 1 : 7;
    
    if ((trn_history < 0b1000000000) && // bet/raise; respond 1-7
        (!rvr_seen)) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if ((trn_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (trn_history < 0b1111111111111) &&
        (!rvr_seen))
        return (index == 0) ? 1 : 7;

    /************************* River *************************/

    if ((rvr_history == 0) || (rvr_history == 1)) {
        int num = num_actions();
        int num_banned = 6-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }  
    
    if ((rvr_history & 0b111) == 0b110) // all-in; must fold or call 
        return (index == 0) ? 1 : 7;
    
    if (rvr_history < 0b1000000000) {
        int num = num_actions();
        int num_banned = 7-num;

        if (index <= 4-num_banned) return index+1;
        if (index >  4-num_banned) return index+num_banned+1;
    }
    
    if ((rvr_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (rvr_history < 0b1111111111111))
        return (index == 0) ? 1 : 7;

    return 0;
}

int GameState::action_to_index(int action) const {
    for (uint8_t i=0; i<7; i++) {
        if (index_to_action(i) == action) {
            return i;
        }
    }
    return 0;
}

void GameState::apply_index(int index) {

    uint8_t a = index_to_action(index);

    player = !player; // player alternates

    if (a == 7) { // call
        bets.push(bets.top());
        pot_size += 2.0f * bets.top();
        if (rvr_seen) is_terminal = true;
    }
   
    else if ((a >= 2) && (a <= 5)) {
        if (bets.top() == 0.0f) {
            // bet
            bets.push(pot_size * BET_SIZES[a-2]);
        } else {
            // raise
            bets.push(bets.top() * RAISE_SIZES[a-2]);
        }
    }

    else if (a == 6) bets.push(STACK_SIZE - (pot_size / 2.0f));

    else if (a == 1) {
        if (!flp_seen) {
            if ((pfp_history != 0b111)) { // can't be a check. must be fold
                float unmatched_bet = bets.top();
                bets.pop();
                pot_size += 2.0f * bets.top(); // this is the biggest mutual bet
                bets.push(unmatched_bet); // preserve bets for printing purposes
                player = !player;
                is_terminal = true;
                bets.push(0.0f);
            } else {
                bets.push(0.0f); // check. pushing this will make it easier to undo actions
            }
        } else if (!trn_seen) {
            if ((flp_history != 0) && (flp_history != 0b1)) { // not check or check-check. must be a fold
                float unmatched_bet = bets.top();
                bets.pop();
                pot_size += 2.0f * bets.top();
                bets.push(unmatched_bet);
                player = !player;
                is_terminal = true;
                bets.push(0.0f);
            } else {
                bets.push(0.0f);
            }
        } else if (!rvr_seen) {
            if ((trn_history != 0) && (trn_history != 0b1)) {
                float unmatched_bet = bets.top();
                bets.pop();
                pot_size += 2.0f * bets.top();
                bets.push(unmatched_bet);
                player = !player;
                is_terminal = true;
                bets.push(0.0f);
            } else {
                bets.push(0.0f);
            }
        } else {
            if ((rvr_history != 0) && (rvr_history != 0b1)) {
                float unmatched_bet = bets.top();
                bets.pop();
                pot_size += 2.0f * bets.top();
                bets.push(unmatched_bet);
                player = !player;
                is_terminal = true;
                bets.push(0.0f);
            } else if (rvr_history == 0b1) { 
                bets.push(0.0f);
                is_terminal = true; // check check is terminal on river
            } else {
                bets.push(0.0f);
            }
        }
    }
    
    /************************* Preflop *************************/

    if (!flp_seen) {
        pfp_history = (pfp_history << 3) | a;
        return;
    }

    /************************* Flop *************************/

    if (!trn_seen) {
        flp_history = (flp_history << 3) | a;
        return;
    }

    /************************* Turn *************************/

    if (!rvr_seen) {
        trn_history = (trn_history << 3) | a;
        return;
    }

    /************************* River *************************/

    rvr_history = (rvr_history << 3) | a;
}

void GameState::undo(bool prev_player, float prev_pot) {
    is_terminal = false;
    player = prev_player;
    pot_size = prev_pot;
    bets.pop();

    // Chance actions
    if (flp_seen && (flp_history==0)) {
        fp3=0; 
        flp_seen=false;
        return;
    }

    if ((fp2!=0) && (flp_history==0)) {
        fp2=0; 
        return;
    }

    if ((fp1!=0) && (flp_history==0)) {
        fp1=0;
        return;
    }

    if (trn_seen && (trn_history==0)) {
        trn=0;
        trn_seen=false;
        return;
    }

    if (rvr_seen && (rvr_history==0)) {
        rvr=0;
        rvr_seen=false;
        return;
    }

    // Actual actions
    if (rvr_history != 0) {
        rvr_history >>= 3;
        return;
    } 

    if (trn_history != 0) {
        trn_history >>= 3;
        return;
    }

    if (flp_history != 0) {
        flp_history >>= 3;
        return;
    }

    if (pfp_history != 0) {
        pfp_history >>= 3;
        return;
    }
}

void GameState::deal_card(uint8_t card) {
    if (has_card(card)) throw std::runtime_error("Card already dealt");

    // if (op1 == 0) op1 = card;
    // else if (op2 == 0) op2 = card;
    // else if (ip1 == 0) ip1 = card;
    // else if (ip2 == 0) ip2 = card;

    if (fp1 == 0) fp1 = card;
    else if (fp2 == 0) fp2 = card;
    else if (fp3 == 0) fp3 = card, flp_seen = true;
    else if (trn == 0) trn = card, trn_seen = true;
    else if (rvr == 0) {
        rvr = card;
        rvr_seen = true;
        if (((pfp_history & 0b111111) == 0b110111) || 
            ((flp_history & 0b111111) == 0b110111) ||
            ((trn_history & 0b111111) == 0b110111) ||
            ((rvr_history & 0b111111) == 0b110111))
            is_terminal = true; // all-ins that have run out
    }

    player = 0;
    bets.push(0.0f);
}

InfoSet GameState::to_information_set() const {
    InfoSet is;

    is.player = player;

    is.cr1 = (player == 0) ? op1 : ip1;
    is.cr2 = (player == 0) ? op2 : ip2;
    
    is.fp1 = fp1;
    is.fp2 = fp2;
    is.fp3 = fp3;
    is.trn = trn;
    is.rvr = rvr;

    is.pfp_history = pfp_history;
    is.flp_history = flp_history; 
    is.trn_history = trn_history; 
    is.rvr_history = rvr_history;

    is.num_actions = num_actions();

    return is;
}

float GameState::rivr_hand_strength() {

    float wins = 0.0f;
    float total = 0.0f;

    ip1 = 0;
    ip2 = 0;

    for (int c1=1; c1<=NUM_CARDS; c1++) {
        for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
            if (has_card(c1) || has_card(c2)) continue;
            if (c1==c2) continue;

            ip1 = c1;
            ip2 = c2;

            trn_seen = true;
            rvr_seen = true;

            int op_best = best_hand(0);
            int ip_best = best_hand(1);

            if (op_best > ip_best) {
                wins += 1.0f;   
            } else if (op_best == ip_best) {
                wins += 0.5f;
            }

            total += 1.0f;
        }
    }

    return wins/total;
}

int GameState::p_id(bool p) const {
    return (p==0) ? pocket_id(op1, op2) : pocket_id(ip1, ip2);
}

std::string GameState::action_to_string(int action) const {

    std::stringstream ss;

    if (pot_size / 2.0f + bets.top() == STACK_SIZE) return (action == 1) ? "Fold" : "Call";
    
    /************************* Preflop *************************/
    
    if (pfp_history == 0) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " to " << bets.top() * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if (pfp_history == 0b111) {
        if (action == 1) return "Check";
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " to " << bets.top() * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if (((pfp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!flp_seen)) 
        return (action == 1) ? "Fold" : "Call";
    
    if ((pfp_history < 0b1000000000) && // bet/raise; respond 1-7
        (!flp_seen)) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " to " << bets.top() * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if ((pfp_history > 0b1000000000) && // bet/raise but at limit; 1 or 7
        (pfp_history < 0b1111111111111) &&
        (!flp_seen))
        return (action == 1) ? "Fold" : "Call";

    /************************* Flop *************************/
    
    if ((flp_history == 0) || (flp_history == 1)) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    } 
    
    if (((flp_history & 0b111) == 0b110) && // all-in; must fold or call
         (!trn_seen)) 
        return (action == 1) ? "Fold" : "Call";
    
    if ((flp_history < 0b1000000000) && // bet/raise; respond 1-7
        (!trn_seen)) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " to " << bets.top() * RAISE_SIZES[action-2];
        return ss.str();
    } 
    
    if ((flp_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (flp_history < 0b1111111111111) &&
        (!trn_seen))
        return (action == 1) ? "Fold" : "Call";

    /************************* Turn *************************/
    
    if ((trn_history == 0) || (trn_history == 1)) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    }
    
    if (((trn_history & 0b111) == 0b110) && // all-in; must fold or call
         (!rvr_seen)) 
        return (action == 1) ? "Fold" : "Call";
    
    if ((trn_history < 0b1000000000) && // bet/raise; respond 1-7
        (!rvr_seen)) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " to " << bets.top() * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if ((trn_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (trn_history < 0b1111111111111) &&
        (!rvr_seen))
        return (action == 1) ? "Fold" : "Call";

    /************************* River *************************/

    if ((rvr_history == 0) || (rvr_history == 1)) {
        ss << BET_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " " << pot_size * BET_SIZES[action-2];
        return ss.str();
    }  
    
    if ((rvr_history & 0b111) == 0b110) // all-in; must fold or call 
        return (action == 1) ? "Fold" : "Call";
    
    if (rvr_history < 0b1000000000) {
        ss << RAISE_ACTION_NAMES[action-1];
        if ((action >= 2) && (action<=5)) ss << " to " << bets.top() * RAISE_SIZES[action-2];
        return ss.str();
    }
    
    if ((rvr_history > 0b1000000000) && // bet/raise but at limit; call or fold
        (rvr_history < 0b1111111111111))
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

void GameState::print_range(int index) const {

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    int action = index_to_action(index);

    std::string player_name = (player==0) ? "OOP" : "IP"; 

    std::cout << "\n******************* Board ********************\n" << to_string();
    std::cout << "\n************** " << player_name << " Range: " << action_to_string(action) << " ***************\n";

    std::array<std::array<float, 9>, 9> range;

    for (int p1=1; p1<=NUM_CARDS; p1++) {
        for (int p2=p1+1; p2<=NUM_CARDS; p2++) {
            GameState temp = *this;

            if ((temp.has_card(p1)) ||
                (temp.has_card(p2))) continue;

            if (temp.player==0) {
                temp.op1 = p1;
                temp.op2 = p2;
            } else {
                temp.ip1 = p1;
                temp.ip2 = p2;
            }

            InfoSet is = temp.to_information_set();

            // std::cout << strategy_sum[is.hash()] << "\n";

            std::array<int, 2> row_col = pocket_id_to_row_col(temp.p_id(player));
            
            range[row_col[0]][row_col[1]] = get_average_strategy(is)[action_to_index(action)];
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

void GameState::print_range_turn(int index) const {

    try {
        load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }

    int action = index_to_action(index);

    std::string player_name = (player==0) ? "OOP" : "IP"; 

    std::cout << "\n******************* Board ********************\n" << to_string();
    std::cout << "\n************** " << player_name << " Range: " << action_to_string(action) << " ***************\n";

    std::array<std::array<float, 9>, 9> range;
    std::array<std::array<float, 9>, 9> count;

    for (int r=0; r<9; r++) {
        for (int c=0; c<9; c++) {
            range[r][c] = count[r][c] = 0.0f;
        }
    }

    for (int p1=1; p1<=NUM_CARDS; p1++) {
        for (int p2=p1+1; p2<=NUM_CARDS; p2++) {
            GameState temp = *this;
            // std::cout << temp.to_string() << "\n";
            
            if ((temp.has_card(p1)) ||
                (temp.has_card(p2))) continue;

            if (temp.player==0) {
                temp.op1 = p1;
                temp.op2 = p2;
            } else {
                temp.ip1 = p1;
                temp.ip2 = p2;
            }

            InfoSet is = temp.to_information_set();

            // std::cout << strategy_sum[is.hash()] << "\n";

            std::array<int, 2> row_col = pocket_id_to_row_col(temp.p_id(player));
            
            range[row_col[0]][row_col[1]] += get_average_strategy(is)[action_to_index(action)];
            count[row_col[0]][row_col[1]] += 1.0f;
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
                std::cout << FIXED_FLOAT(range[r-1][c-1]/count[r-1][c-1]) << " ";
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

    // Generate unique card indices
    std::array<int, 4> card_indices;
    for (int i=0; i<4; ++i) {
        do {
            card_indices[i] = get_card_distribution()(get_random_generator());
        } while (std::find(card_indices.begin(), card_indices.begin() + i, card_indices[i]) != card_indices.begin() + i);
    }

    GameState gs;

    gs.op1 = card_indices[0];
    gs.op2 = card_indices[1];
    gs.ip1 = card_indices[2];
    gs.ip2 = card_indices[3];
    gs.bets.push(1.0f);
    gs.bets.push(2.0f);

    return gs;
}

void play_computer() {
    bool p = 0; // start as oop
    float cumulative_winnings = 0.0f;
    bool keep_playing = true;

    load_cfr_data("latest_checkpoint.dat", regret_sum, strategy_sum);

    while (keep_playing == true) {
        GameState gs = generate_random_initial_state();

        while (!gs.is_terminal) {
            while (gs.is_chance()){
                // get random card out of cards remaining
                uint8_t c = get_card_distribution()(get_random_generator());
                while (gs.has_card(c)) 
                    c = get_card_distribution()(get_random_generator());
                gs.deal_card(c);
            }

            if (gs.is_terminal) {
                break;
            }

            InfoSet is = gs.to_information_set();
            std::array<float, 7> average_strategy = get_average_strategy(is);

            // Mask out opponent's cards when displaying to user
            GameState temp = gs;

            if (p==0) temp.ip1 = 0, temp.ip2 = 0;
            if (p==1) temp.op1 = 0, temp.op2 = 0;

            std::cout << "**********************************************\n";
            std::cout << temp.to_string() << "\n";

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

                std::cout << "Computer turn. " << gs.action_to_string(gs.index_to_action(sampled_index)) << "\n**********************************************\n\n";
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

void generate_rank_table(GameState state) {
    if (state.trn==0) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) {
                    for (int c4=c3+1; c4<=NUM_CARDS; c4++) {
                        state.op1 = 0; state.op2 = 0; state.trn = 0; state.rvr = 0;
                        if (state.has_card(c1) || state.has_card(c2) || state.has_card(c3) || state.has_card(c4)) continue;

                        state.op1 = c1;
                        state.op2 = c2;
                        state.trn = c3;
                        state.rvr = c4;
                        int hand_rank = state.best_hand(0);

                        rank_table[c1 - 1][c2 - 1][c3 - 1][c4 - 1] = hand_rank;
                        rank_table[c1 - 1][c2 - 1][c4 - 1][c3 - 1] = hand_rank;
                        rank_table[c1 - 1][c3 - 1][c2 - 1][c4 - 1] = hand_rank;
                        rank_table[c1 - 1][c3 - 1][c4 - 1][c2 - 1] = hand_rank;
                        rank_table[c1 - 1][c4 - 1][c2 - 1][c3 - 1] = hand_rank;
                        rank_table[c1 - 1][c4 - 1][c3 - 1][c2 - 1] = hand_rank;

                        rank_table[c2 - 1][c1 - 1][c3 - 1][c4 - 1] = hand_rank;
                        rank_table[c2 - 1][c1 - 1][c4 - 1][c3 - 1] = hand_rank;
                        rank_table[c2 - 1][c3 - 1][c1 - 1][c4 - 1] = hand_rank;
                        rank_table[c2 - 1][c3 - 1][c4 - 1][c1 - 1] = hand_rank;
                        rank_table[c2 - 1][c4 - 1][c1 - 1][c3 - 1] = hand_rank;
                        rank_table[c2 - 1][c4 - 1][c3 - 1][c1 - 1] = hand_rank;

                        rank_table[c3 - 1][c1 - 1][c2 - 1][c4 - 1] = hand_rank;
                        rank_table[c3 - 1][c1 - 1][c4 - 1][c2 - 1] = hand_rank;
                        rank_table[c3 - 1][c2 - 1][c1 - 1][c4 - 1] = hand_rank;
                        rank_table[c3 - 1][c2 - 1][c4 - 1][c1 - 1] = hand_rank;
                        rank_table[c3 - 1][c4 - 1][c1 - 1][c2 - 1] = hand_rank;
                        rank_table[c3 - 1][c4 - 1][c2 - 1][c1 - 1] = hand_rank;

                        rank_table[c4 - 1][c1 - 1][c2 - 1][c3 - 1] = hand_rank;
                        rank_table[c4 - 1][c1 - 1][c3 - 1][c2 - 1] = hand_rank;
                        rank_table[c4 - 1][c2 - 1][c1 - 1][c3 - 1] = hand_rank;
                        rank_table[c4 - 1][c2 - 1][c3 - 1][c1 - 1] = hand_rank;
                        rank_table[c4 - 1][c3 - 1][c1 - 1][c2 - 1] = hand_rank;
                        rank_table[c4 - 1][c3 - 1][c2 - 1][c1 - 1] = hand_rank;
                    }
                }
            }
        }
    } else if (state.rvr==0) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) {
                    state.op1 = 0; state.op2 = 0; state.rvr = 0;
                    if (state.has_card(c1) || state.has_card(c2) || state.has_card(c3)) continue;

                    state.op1 = c1;
                    state.op2 = c2;
                    state.rvr = c3;
                    int hand_rank = state.best_hand(0);

                    rank_table[c1 - 1][c2 - 1][state.trn - 1][c3 - 1] = hand_rank;
                    rank_table[c1 - 1][c3 - 1][state.trn - 1][c2 - 1] = hand_rank;
                    rank_table[c2 - 1][c1 - 1][state.trn - 1][c3 - 1] = hand_rank;
                    rank_table[c2 - 1][c3 - 1][state.trn - 1][c1 - 1] = hand_rank;
                    rank_table[c3 - 1][c1 - 1][state.trn - 1][c2 - 1] = hand_rank;
                    rank_table[c3 - 1][c2 - 1][state.trn - 1][c1 - 1] = hand_rank;
                }
            }
        }
    } else {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                state.op1 = 0; state.op2 = 0;
                if (state.has_card(c1) || state.has_card(c2)) continue;
                
                state.op1 = c1;
                state.op2 = c2;
                int hand_rank = state.best_hand(0);

                rank_table[c1 - 1][c2 - 1][state.trn - 1][state.rvr - 1] = hand_rank;
                rank_table[c2 - 1][c1 - 1][state.trn - 1][state.rvr - 1] = hand_rank;
            }
        }
    }
}

void test_rank_table(GameState state) {

    int samples = 50000;
    double total_time = 0.0f;
    auto start = std::chrono::high_resolution_clock::now();

    generate_rank_table(state);

    for (int i=0; i<samples; i++) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) { 
                    state.op1 = 0; state.op2 = 0; state.rvr = 0;
                    if (state.has_card(c1) || state.has_card(c2) || state.has_card(c3)) continue;                    
                    state.op1 = c1;
                    state.op2 = c2;
                    state.rvr = c3;
                    state.flp_seen = true;
                    state.trn_seen = true;
                    state.rvr_seen = true;
                    state.pfp_history = 0b111001;
                    state.flp_history = 0b1001;
                    state.best_hand_fast();
                }
            }
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    std::cout << samples << " samples of best_hand_fast: "
          << total_time << " seconds. " << 4960 * samples/total_time << " hands/second." << "\n";

    total_time = 0.0f;
    start = std::chrono::high_resolution_clock::now();

    for (int i=0; i<samples; i++) {
        for (int c1=1; c1<=NUM_CARDS; c1++) {
            for (int c2=c1+1; c2<=NUM_CARDS; c2++) {
                for (int c3=c2+1; c3<=NUM_CARDS; c3++) { 
                    state.op1 = 0; state.op2 = 0; state.rvr = 0;
                    if (state.has_card(c1) || state.has_card(c2) || state.has_card(c3)) continue;                    
                    state.op1 = c1;
                    state.op2 = c2;
                    state.rvr = c3;
                    state.flp_seen = true;
                    state.trn_seen = true;
                    state.rvr_seen = true;
                    state.pfp_history = 0b111001;
                    state.flp_history = 0b1001;
                    state.best_hand(0);
                }
            }
        }
    }

    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;
    total_time += elapsed.count();

    std::cout << samples << " samples of best_hand: "
          << total_time << " seconds. " << 4960 * samples/total_time << " hands/second." << "\n";
}